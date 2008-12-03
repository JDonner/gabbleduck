#include "settings.h"
#include "pipeline.h"
#include "geometry.h"

#include <itkImageConstIterator.h>
#include <iostream>


using namespace std;

// Symmetric matrices have only their upper triangles stored.
// This maps 'flat' storage index to corresponding row+col indeces
unsigned row_col[6][2] = {
   // yes should work the arithmetic out instead
   {0,0}, {0,1}, {0,2},
          {1,1}, {1,2},
                 {2,2}
};



BetaPipeline::BetaPipeline(ImageType::Pointer fullImage,
                           PointType const& physCenter,
                           // In cells. no point in fractional cells (I believe)
                           int region_width)
  : eigenSystemAnalyzer_(Dimension)
  , bStructureTensorUpdated_(false)
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

   tensor_image_ = TensorImageType::New();

   // At this point we break the ITK pipeline
   multiply_gaussian_components();

   for (unsigned i = 0; i < 6; ++i) {
      tensor_component_adaptor_[i] = TensorComponentAdaptorType::New();
      tensor_component_adaptor_[i]->SetImage(tensor_image_);
      tensor_component_adaptor_[i]->Allocate();
      tensor_component_adaptor_[i]->SelectNthElement(i);

      tensor_component_adaptor_[i] = TensorComponentAdaptorType::New();


      // They're each the same function, but they have to be templated
      // to a different adaptor (1-6)
      gaussians_[i] = GaussianImageFunction::New();
      gaussians_[i]->SetInputImage(tensor_component_adaptor_[i]);

      // &&& should handle different spacings.. wait, doesn't it already?
      // physical ('world') coordinates
      gaussians_[i]->SetSigma(constants::SigmaOfFeatureGaussian);
   }

   eigenSystemAnalyzer_.SetOrderEigenValues(EigenSystemAnalyzerType::OrderByMagnitude);

   // At this point, <point_structure_tensor_>
}


// We will have many levels of offset (offsets from offsets from ..)
// as we chase around the image for beta-like points, but we want to
// avoid accumulating floating point error as much as possible. So, we
// always resample against the original image. Any accumulated error
// will be in the position, but at least not in the voxels themselves.
void BetaPipeline::set_up_resampler(ImageType::Pointer fullImage,
                                    PointType const& physCenter)
{
   resampler_ = ResampleFilterType::New();

   resampler_->SetInput(fullImage);

   // We always resample from the whole, original image, so using 0 is
   // just fine (there is plenty of empty space at the edges of the
   // image anyway).  Unless we're restricted to the region we sample
   // from.
   resampler_->SetDefaultPixelValue(0.0);

   ImageType::SizeType sizeSample;
   sizeSample.Fill(constants::GaussianSupportSize);
   // &&& Ah, here's the place we need to change...
   // Size is in pixels; the usual meaning of 'size'
   resampler_->SetSize(sizeSample);

   IndexType idxResampledCenter_;

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
      TakeGaussiansAtOrignalPoint();
      // This should be just about a no-op if it's already updated.
      CalcEigenSystemAtOriginalPoint();
   }
}


void BetaPipeline::update_first_half()
{

}

void BetaPipeline::multiply_gaussian_components()
{
   typedef itk::ImageConstIterator<DerivativeFilterType::OutputImageType> DerivativeIteratorType;
   DerivativeIteratorType itDerived[3];

DerivativeIteratorType itD;
++itD;

   for (unsigned i = 0; i < Dimension; ++i) {
      itDerived[i] = DerivativeIteratorType(derivative_filter_[i]->GetOutput(),
                                            // &&& LargestPossibleRegion would also be fine
                                            derivative_filter_[i]->GetOutput()->GetRequestedRegion());
      itDerived[i].GoToBegin();
   }


   // Allocate a TensorType

   typedef itk::ImageRegionIterator<TensorImageType> TensorIteratorType;
   TensorIteratorType itGradientTensor(tensor_image_,
                                       tensor_image_->GetRequestedRegion());

   // Write the product into the tensor here.
   for (itGradientTensor.GoToBegin(); not itGradientTensor.IsAtEnd(); ++itGradientTensor) {
      for (unsigned i = 0; i < 6; ++i) {
         itGradientTensor.Value()[i] = itDerived[i].Get();
         unsigned row = row_col[i][0];
         unsigned col = row_col[i][1];

         // row and col are indexes to dimension
         itGradientTensor.Value()[i] = itDerived[row].Value() * itDerived[col].Value();
      }

      for (unsigned i = 0; i < Dimension; ++i) {
         ++itDerived[i];
      }
   }

   // Then we take the gaussian at the central point, with the rest of the sampled
   // image as its support.
}



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
//    TensorIteratorType itGradientTensor(tensor_filter_->GetOutput(),
//                                 tensor_filter_->GetOutput()->GetRequestedRegion());

// }


void BetaPipeline::TakeGaussianssAtOrignalPoint()
{
   for (unsigned i = 0; i < 6; ++i) {
      point_structure_tensor_ = gaussians_[i].GetValue() * tensor_image_[i].GetValue();
   }

}


void BetaPipeline::CalcEigenSystemAtOriginalPoint()
{
   // We want magnitude, we don't care about direction
   // &&& (wait, what would negative eigenvalues mean?)
   // <OrderByValue> is also possible, not sure what it'd mean though.

   unsigned n = eigenSystemAnalyzer_.ComputeEigenValuesAndVectors(
      point_structure_tensor_,
      eigenValues_,
      eigenVectors_);
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
