#include "settings.h"
#include "pipeline.h"
#include "geometry.h"

#include <itkImageDuplicator.h>
#include <iostream>


using namespace std;



BetaPipeline::BetaPipeline(ImageType::Pointer fullImage,
                           PointType const& physCenter,
                           // In cells. no point in fractional cells (I believe)
                           int /*region_width*/)
{
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

   // file:///big/common/software/insight/install/html/classitk_1_1SymmetricEigenAnalysisImageFilter.html
   // Compute eigenvalues.. order them in descending order
   totalEigenFilter_ = EigenAnalysisFilterType::New();
   totalEigenFilter_->SetDimension( Dimension );
   totalEigenFilter_->SetInput( hessian_maker_->GetOutput() );
   totalEigenFilter_->OrderEigenValuesBy(
      EigenAnalysisFilterType::JgdCalculatorType::OrderByValue);
   // -- We may not need these cast + adaptor + accessor things
   // (maybe some). They're for the benefit of other filters, I think.
}

// unsigned index_of_symmetric(unsigned row, unsigned col)
// {
//    unsigned k;
//    if ( row < col )
//    {
//       k = row * Dimension + col - row * ( row + 1 ) / 2;
//    }
//    else
//    {
//       k = col * Dimension + row - col * ( col + 1 ) / 2;
//    }

//    return k;
// }

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
   VectorType offset = transform_shift(physCenter, fullImage->GetSpacing());

// cout << "physCenter(phys): " << physCenter
//      << "; spacing(phys): " << fullImage->GetSpacing()
//      << "; offset: " << offset << endl;

   translation_ = TranslationTransform::New();
   translation_->SetOffset(offset);

   // itkTranslationTransformTest.cxx
   // Examples/Filtering/ResampleImageFilter.cxx is most helpful
   // Resample, to shift the image to (slightly) new coordinates
   resampler_ = ResampleFilterType::New();
//   resampler_.SetInterpolator();

   resampler_->SetTransform(translation_);
   resampler_->SetInput(fullImage);
   // &&& Couldn't we extend the edge or something instead?
   resampler_->SetDefaultPixelValue(0.0);

   ImageType::SpacingType spacing = fullImage->GetSpacing();
   resampler_->SetOutputSpacing(spacing);

//   ImageType::RegionType region = fullImage->GetLargestPossibleRegion();

   // As the Gaussian support sigma gets larger, we'll need more 'support'
   // here probably.
   // &&& No, 5 (well, 4) is the max it's needed,at least even with:
   // find-sheets --BetaThickness=10
   //   --SigmaOfDerivativeGaussian=10.0
   //   --SigmaOfFeatureGaussian=10.0
   ImageType::SizeType too_small;
   too_small.Fill(35);
   // &&& Ah, here's the place we need to change...
   // Size is in pixels; the usual meaning of 'size'
   resampler_->SetSize(too_small);

   // &&& let it warn us if the support is too small
//   resampler_->SetDebug(true);
   itk::Object::SetGlobalWarningDisplay(true);

   ImageType::PointType physOrigin = fullImage->GetOrigin();
   // fastest-moving (ie, X) first, says the resampling section of the guide
   for (unsigned i = 0; i < Dimension; ++i) {
      physOrigin[i] = physCenter[i] - (too_small[i] * spacing[i]) / 2.0;
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

//cout << "resampler (after update): " << endl;
//resampler_->Print(cout, 2);
}

void BetaPipeline::update_first_half()
{
//cout << "hessian; before" << endl;
//hessian_maker_->Print(cout, 2);
   hessian_maker_->Update();
//cout << "hessian; after" << endl;
//hessian_maker_->Print(cout, 2);
   HessianImageType::Pointer hessian = hessian_maker_->GetOutput();

//cout << "hessian IMAGE; after" << endl;
//hessian_maker_->GetOutput()->Print(cout, 2);

   typedef itk::ImageDuplicator<HessianImageType> DuplicatorType;
   DuplicatorType::Pointer duplicator = DuplicatorType::New();
   duplicator->SetInputImage(hessian.GetPointer());
   duplicator->Update();
   HessianImageType::Pointer gaussianed_hessian_ = duplicator->GetOutput();
}

void BetaPipeline::fuse_into_hessian()
{

}

void BetaPipeline::gaussianize()
{

}

void BetaPipeline::update()
{
   update_first_half();
   gaussianize();
   fuse_into_hessian();
   totalEigenFilter_->Update();
}
