#include "settings.h"
#include "pipeline.h"
#include "geometry.h"

#include <itkImageDuplicator.h>
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

   hessian_maker_ = HessianFilterType::New();
   // 1.0 is default sigma value. Units are image's physical units
   // (see itkGaussianDerivativeImageFunction, which all the rest use).
   hessian_maker_->SetSigma(constants::SigmaOfDerivativeGaussian);
   hessian_maker_->SetInput(resampler_->GetOutput());

   for (unsigned i = 0; i < 6; ++i) {
      // &&& This is vastly wasteful
      hess_component_adaptor_[i] = HessianComponentAdaptorType::New();
      hess_component_adaptor_[i]->SetImage(hessian_maker_->GetOutput());
      hess_component_adaptor_[i]->Allocate();
      hess_component_adaptor_[i]->SelectNthElement(i);

      gaussian_maker_[i] = GaussianFilterType::New();
      gaussian_maker_[i]->SetInput(hess_component_adaptor_[i]);
//      gaussian_maker_[i]->SetUseImageSpacing(false);
      // physical ('world') coordinates
      gaussian_maker_[i]->SetSigma(constants::SigmaOfFeatureGaussian);
   }

   // Compute eigenvalues.. order them in descending order
   totalEigenFilter_ = EigenAnalysisFilterType::New();
   totalEigenFilter_->SetDimension( Dimension );
   totalEigenFilter_->SetInput( hessian_maker_->GetOutput() );
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

   // Don't need this
//   ImageType::SpacingType spacing = fullImage->GetSpacing();
//   resampler_->SetOutputSpacing(spacing);

//   ImageType::RegionType region = fullImage->GetLargestPossibleRegion();

   // As the Gaussian support sigma gets larger, we'll need more 'support'
   // here probably.
   // &&& No, 5 (well, 4) is the max it's needed,at least even with:
   // find-sheets --BetaThickness=10
   //   --SigmaOfDerivativeGaussian=10.0
   //   --SigmaOfFeatureGaussian=10.0
   ImageType::SizeType sizeSample;
   sizeSample.Fill(constants::GaussianSupportSize);
   // &&& Ah, here's the place we need to change...
   // Size is in pixels; the usual meaning of 'size'
   resampler_->SetSize(sizeSample);

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


void BetaPipeline::update()
{
   if (not bStructureTensorUpdated_) {
      bStructureTensorUpdated_ = true;
      // first half of the pipeline
      update_first_half();
      fuse_into_hessian();
      // This should be just about a no-op if it's already updated.
      totalEigenFilter_->Update();
   }

#ifdef LETTHEAIR
   hessian_maker_->Update();
   HessianImageType::Pointer hessian = hessian_maker_->GetOutput();
   typedef itk::ImageDuplicator<HessianImageType> DuplicatorType;
   DuplicatorType::Pointer duplicator = DuplicatorType::New();
   duplicator->SetInputImage(hessian.GetPointer());
   duplicator->Update();
   HessianImageType::Pointer gaussianed_hessian_ = duplicator->GetOutput();
#endif // LETTHEAIR
}


void BetaPipeline::update_first_half()
{
   // &&& This wants to be, not a filter but a function, since we
   // only want one value, the value at the center of the resampled
   // area.
   //
   // <GaussianBlurImageFunction>
   //
   // Pull from resampler through Hessian, into Gaussian
   for (unsigned i = 0; i < 6; ++i) {
      gaussian_maker_[i]->Update();
   }
}


void BetaPipeline::fuse_into_hessian()
{
   typedef itk::ImageRegionIterator<GaussianFilterType::OutputImageType> GaussianIteratorType;
   GaussianIteratorType itGaussed[6];

   // initialize iterators
   for (unsigned i = 0; i < 6; ++i) {
      itGaussed[i] = GaussianIteratorType(gaussian_maker_[i]->GetOutput(),
                                          gaussian_maker_[i]->GetOutput()->GetRequestedRegion());
      itGaussed[i].GoToBegin();
   }

   typedef itk::ImageRegionIterator<HessianImageType> HessianIteratorType;
   HessianIteratorType itHess(hessian_maker_->GetOutput(),
                              hessian_maker_->GetOutput()->GetRequestedRegion());

   // We write in-place here (bad maybe)
   for (itHess.GoToBegin(); not itHess.IsAtEnd(); ++itHess) {
      for (unsigned i = 0; i < 6; ++i) {
         itHess.Value()[i] = itGaussed[i].Get();

         ++itGaussed[i];
      }
   }
}
