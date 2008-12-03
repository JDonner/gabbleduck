#include "settings.h"
#include "pipeline.h"
#include "geometry.h"

#include <itkGaussianBlurImageFunction.h>
#include <iostream>


using namespace std;


BetaPipeline::BetaPipeline(ImageType::Pointer fullImage,
                           PointType const& physCenter,
                           // In cells. no point in fractional cells (I believe)
                           int region_width)
  : bStructureTensorUpdated_(false)
{
   (void)region_width;
//cout << "input: " << endl;
//fullImage->Print(cout, 2);

   ImageType::IndexType index;
   bool isWithin = fullImage->TransformPhysicalPointToIndex(physCenter, index);
   assert(isWithin);

// ImageRegion region;
// region.SetIndex(index);
// // add 'size'. Don't know what units these are, physical or pixel.
// // The '+ 1' to compensate for the phys -> index truncation
// // And another '+ 1' to deal with our fractional translation
// region.PadByRadius(region_width + 1 + 1);

   // It's a filter, so, we want to set requested region on the input image,
   // and then a 'shift' transform of a fraction of a cell, right?
   // output- to- input, and in physical coordinates (itkResampleImageFilter.h)

   set_up_resampler(fullImage, physCenter);

   for (unsigned i = 0; i < Dimension; ++i) {
      derivative_filter_[i] = DerivativeFilterType::New();
      derivative_filter_[i]->SetDirection(i);
      derivative_filter_[i]->SetInput(resampler_->GetOutput());
   }

   // At this point we break the ITK pipeline
   multiply_gaussian_components();

   for (unsigned i = 0; i < 6; ++i) {
      tensor_component_adaptor_[i] = TensorComponentAdaptorType::New();
      tensor_component_adaptor_[i]->SetImage(tensor_image_);
      tensor_component_adaptor_[i]->Allocate();
      tensor_component_adaptor_[i]->SelectNthElement(i);

      // They're each the same function, but they have to be templated
      // to a different adaptor (1-6)
      gaussians_[i] = GaussianFilterType::New();
      gaussians_[i]->SetInput(tensor_component_adaptor_[i]);
      // &&& should handle different spacings.. wait, doesn't it already?
      // physical ('world') coordinates
      gaussians_[i]->SetSigma(constants::SigmaOfFeatureGaussian);
   }

   // At this point, <point_structure_tensor_>
   // Compute eigenvalues.. order them in descending order
   totalEigenFilter_ = EigenAnalysisFilterType::New();
   totalEigenFilter_->SetDimension( Dimension );
   totalEigenFilter_->SetInput( point_structure_tensor_ );
   totalEigenFilter_->OrderEigenValuesBy(
      EigenAnalysisFilterType::JgdCalculatorType::OrderByValue);
}


// We will have many levels of offset (offsets from offsets from ..)
// as we chase around the image for beta-like points, but we want to
// avoid accumulating floating point error as much as possible. So, we
// always resample against the original image. Any accumulated error
// will be in the position, but at least not in the voxels themselves.
void BetaPipeline::set_up_resampler(ImageType::Pointer fullImage,
                                    PointType const& physCenter)
{
  /**
   * Set the coordinate transform to use for resampling.  Note that
   * this must be in physical coordinates and it is the
   * output-to-input transform, NOT the input-to-output transform that
   * you might naively expect.  By default the filter uses an identity
   * transform. You must provide a different transform here, before
   * attempting to run the filter, if you do not want to use the
   * default identity transform. */
//   VectorType offset = transform_shift(physCenter, fullImage->GetSpacing());

// cout << "physCenter(phys): " << physCenter
//      << "; spacing(phys): " << fullImage->GetSpacing()
//      << "; offset: " << offset << endl;

//   translation_ = TranslationTransform::New();
//   translation_->SetOffset(offset);

   // itkTranslationTransformTest.cxx
   // Examples/Filtering/ResampleImageFilter.cxx is most helpful
   // Resample, to shift the image to (slightly) new coordinates
   resampler_ = ResampleFilterType::New();

//   resampler_->SetTransform(translation_);
   resampler_->SetInput(fullImage);

   // We always resample from the whole, original image, so using 0
   // is just fine. Unless we're restricted to the region we sample from.
   resampler_->SetDefaultPixelValue(0.0);

   ImageType::SizeType sizeSample;
   sizeSample.Fill(constants::GaussianSupportSize);
   // &&& Ah, here's the place we need to change...
   // Size is in pixels; the usual meaning of 'size'
   resampler_->SetSize(sizeSample);

physCenter
   ImageIndex idxResampledCenter_;

   // &&& let it warn us if the support is too small
//   resampler_->SetDebug(true);
   itk::Object::SetGlobalWarningDisplay(true);

   ImageType::PointType physOrigin;
   // fastest-moving (ie, X) first, says the resampling section of the guide
   for (unsigned i = 0; i < Dimension; ++i) {
      physOrigin[i] = physCenter[i] -
         (sizeSample[i] * fullImage->GetSpacing()[i]) / 2.0 +
         0.5 * fullImage->GetSpacing()[i];
   }
   // &&& Is this what I want, or do I want it shifted...?
   // We'd ideally like it to have the same physical coordinates as the
   // original. But is it possible?
   // origin is physical coords of 0,0,0 pixel, no?
   resampler_->SetOutputOrigin(physOrigin);

//   resampler_->SetOutputStartIndex();

//cout << "resampler (before update): " << endl;
//resampler_->Print(cout, 2);

   // We don't need to Update up front like this (later stages will
   // 'pull' from earlier ones), it just makes tracing through easier
   resampler_->Update();

ImageType::Pointer resampled = resampler_->GetOutput();
ImageType::RegionType region = resampled->GetRequestedRegion();
assert(region.GetSize()[0] == sizeSample[0] and
       region.GetSize()[1] == sizeSample[1] and
       region.GetSize()[2] == sizeSample[2]);
// ImageType::PointType physResampledCenter;
// physResampledCenter[0] = resampled->GetOrigin()[0] + region.GetSize()[0] * region.GetSpacing()[0];
// physResampledCenter[1] = resampled->GetOrigin()[1] + region.GetSize()[1] * region.GetSpacing()[1];
// physResampledCenter[2] = resampled->GetOrigin()[2] + region.GetSize()[2] * region.GetSpacing()[2];

ImageType::IndexType idxCenter;
idxCenter[0] = region.GetSize()[0] / 2;
idxCenter[1] = region.GetSize()[1] / 2;
idxCenter[2] = region.GetSize()[2] / 2;

//ImageType::PointType physAlsoCenter;
//resampled->TransformIndexToPhysicalPoint(idxCenter, physAlsoCenter);
// cout
//    << physCenter[0] << " "
//    << physCenter[1] << " "
//    << physCenter[2] << " "
//    << "original phys center:\n"
//    << physAlsoCenter[0] << " "
//    << physAlsoCenter[1] << " "
//    << physAlsoCenter[2] << " "
//    << "sampled phys center: "
//    << endl;

//cout << "resampler (after update): " << endl;
//resampler_->Print(cout, 2);
}

// IN_PROGRESS
void BetaPipeline::reset()
{
   resampler_->ResetPipeline();
   // &&& Maybe the later ones, too
}


void BetaPipeline::update()
{
   if (not bStructureTensorUpdated_) {
      bStructureTensorUpdated_ = true;
      // first half of the pipeline
      update_first_half();
      fuse_into_tensor();
      // This should be just about a no-op if it's already updated.
      totalEigenFilter_->Update();
   }
}


void BetaPipeline::update_first_half()
{
   // &&& This wants to be, not a filter but a function, since we
   // only want one value, the value at the center of the resampled
   // area.
   //
   // <GaussianBlurImageFunction>
   //
   // Pull from resampler through Tensor, into Gaussian
   for (unsigned i = 0; i < 6; ++i) {
      gaussian_filter_[i]->Update();
   }
}


void BetaPipeline::multiply_gaussian_components()
{
   typedef itk::ImageConstIterator<DerivativeFilterType::OutputImageType> DerivativeIteratorType;
   DerivativeIteratorType itDerived[3];

   for (unsigned i = 0; i < Dimension; ++i) {
      itDerived[i] = DerivativeIteratorType(derivative_filter_->GetOutput(),
                                            // &&& LargestPossibleRegion would also be fine
                                            derivative_filter_->GetRequestedRegion());
      itDerived[i].GoToBegin();
   }


   // Allocate a SymmTensorType

   typedef itk::ImageRegionIterator<TensorImageType> TensorIteratorType;
   TensorIteratorType itGradientTensor(tensor_image_,
                                       tensor_image_->GetRequestedRegion());

   // We write the product into the tensor here.
   for (itTensor.GoToBegin(); not itTensor.IsAtEnd(); ++itTensor) {
      for (unsigned i = 0; i < 6; ++i) {
         itTensor.Value()[i] = itDerived[i].Get();
         unsigned row = row_col[i][0];
         unsigned col = row_col[i][1];

         itTensor.Value()[i] = itDerived[row] * itDerived[col];
      }

      for (unsigned i = 0; i < Dimension; ++i) {
         ++itDerived[i];
      }
   }

   // Then we take the gaussian at the central point, with the rest of the sampled
   // image as its support.
}


unsigned row_col[6][2] = {
   // yes should work the arithmetic out instead
   {0,0}, {0,1}, {0,2},
          {1,1}, {1,2},
                 {2,2}
};


// void BetaPipeline::fuse_into_tensor()
// {
//    typedef itk::ImageRegionIterator<GaussianFilterType::OutputImageType> GaussianIteratorType;
//    GaussianIteratorType itGaussed[6];

//    // initialize iterators
//    for (unsigned i = 0; i < 6; ++i) {
//       itGaussed[i] = GaussianIteratorType(gaussian_filter_[i]->GetOutput(),
//                                           gaussian_filter_[i]->GetOutput()->GetRequestedRegion());
//       itGaussed[i].GoToBegin();
//    }

//    typedef itk::ImageRegionIterator<TensorImageType> TensorIteratorType;
//    TensorIteratorType itTensor(tensor_filter_->GetOutput(),
//                                 tensor_filter_->GetOutput()->GetRequestedRegion());

// }


void BetaPipeline::TakeGaussianAtOrignalPoint()
{
   typedef itk::GaussianBlurImageFunction<TensorComponentAdaptorType,
      TensorType> GaussianFunction;

   GaussianFunction gaussians[6];

   for (unsigned i = 0; i < 6; ++i) {
      point_structure_tensor_ = gaussians_[i].GetValue() * tensor_image_[i].GetValue();
   }

}


void BetaPipeline::CalcEigenSystemAtOriginalPoint()
{
   typedef SymmetricEigenAnalysis<InputPixelType, EValuesPixelType, EVectorsPixelType>
      EigenSystemAnalyzer;

   EigenSystemAnalyzerType eigenSystemAnalyzer;
   // We want magnitude, we don't care about direction
   // &&& (wait, what would negative eigenvalues mean?)
   // <OrderByValue> is also possible, not sure what it'd mean though.
   eigenSystemAnalyzer.SetOrderEigenValues(OrderByMagnitude);

   typedef  InternalPrecisionType                         EigenValue;
   typedef  VectorType                                    EigenVector;

   typedef  EigenValue                                    EigenValues[Dimension];
   typedef  EigenVector                                   EigenVectors[Dimension];

   EigenValues eigenValues_;
   EigenVectors eigenVectors_;

   unsigned n = eigenSystemAnalyzer.ComputeEigenValuesAndVectors(
      point_structure_tensor_, TVector &EigenValues, TEigenMatrix &EigenVectors);

   // &&& We want to order by magnitude (of eigenvalue, I hope this is referring to)
}

// Given <row>, <col>, find corresponding index into symmetric tensor's
// straight array storage.
unsigned flat_index_into_symmetric_storage(unsigned row, unsigned col)
{
   unsigned k;
   if ( row < col )
   {
      k = row * Dimension + col - row * ( row + 1 ) / 2;
   }
   else
   {
      k = col * Dimension + row - col * ( col + 1 ) / 2;
   }
   return k;
}
