#include "settings.h"
#include "pipeline.h"
#include "geometry.h"

#include <itkImageDuplicator.h>
#include <iostream>


using namespace std;

// /big/common/software/insight/InsightToolkit-3.4.0/Testing/Code/Common/itkTranslationTransformTest.cxx

BetaPipeline::BetaPipeline(ImageType::Pointer fullImage,
                           PointType const& physCenter,
                           // In cells. no point in fractional cells (I believe)
                           int region_width)
  : bStructureTensorUpdated_(false)
{
//cout << "input: " << endl;
//fullImage->Print(cout, 2);

   ImageType::IndexType index;
   bool isWithin = fullImage->TransformPhysicalPointToIndex(physCenter, index);
   assert(isWithin);

   ImageRegion region;
   region.SetIndex(index);
   // add 'size'. Don't know what units these are, physical or pixel.
   // The '+ 1' to compensate for the phys -> index truncation
   // And another '+ 1' to deal with our fractional translation
   region.PadByRadius(region_width + 1 + 1);

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
      gaussian_maker_[i]->SetSigma(2.0);
      gaussian_maker_[i]->Update();
   }

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

void BetaPipeline::set_up_resampler(ImageType::Pointer fullImage,
                                    PointType const& physCenter)
{
   // Examples/Filtering/ResampleImageFilter.cxx is most helpful
   // Resample, to shift the image to (slightly) new coordinates
   resampler_ = ResampleFilterType::New();
//   resampler_.SetInterpolator();

  /** Set the coordinate transformation.
   * Set the coordinate transform to use for resampling.  Note that
   * this must be in physical coordinates and it is the
   * output-to-input transform, NOT the input-to-output transform that
   * you might naively expect.  By default the filter uses an Identity
   * transform. You must provide a different transform here, before
   * attempting to run the filter, if you do not want to use the
   * default Identity transform. */
   VectorType offset = transform_shift(physCenter, fullImage->GetSpacing());

// cout << "physCenter(phys): " << physCenter
//      << "; spacing(phys): " << fullImage->GetSpacing()
//      << "; offset: " << offset << endl;

   translation_ = TranslationTransform::New();
   translation_->SetOffset(offset);

   resampler_->SetTransform(translation_);
   resampler_->SetInput(fullImage);
   resampler_->SetDefaultPixelValue(0.0);

   ImageType::SpacingType spacing = fullImage->GetSpacing();
   resampler_->SetOutputSpacing(spacing);

//   ImageType::RegionType region = fullImage->GetLargestPossibleRegion();

   // As the Gaussian support type gets larger, we'll need more 'support'
   // here probably
   ImageType::SizeType too_small;
   too_small[0] = too_small[1] = too_small[2] = 13;
   // &&& Ah, here's the place we need to change...
   // &&& this size is in pixels, right?
   resampler_->SetSize(too_small);


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

   // nice!
//   resampler_->SetOutputStartIndex();

// cout << "resampler (before update): " << endl;
// resampler_->GetOutput()->Print(cout, 2);

   resampler_->Update();

//cout << "resampler (after update): " << endl;
//resampler_->GetOutput()->Print(cout, 2);
}

void BetaPipeline::update()
{
   if (not bStructureTensorUpdated_) {
      hessian_maker_->Update();
      HessianImageType::Pointer hessian = hessian_maker_->GetOutput();
      typedef itk::ImageDuplicator<HessianImageType> DuplicatorType;
      DuplicatorType::Pointer duplicator = DuplicatorType::New();
      duplicator->SetInputImage(hessian.GetPointer());
      duplicator->Update();
      HessianImageType::Pointer gaussianed_hessian_ = duplicator->GetOutput();
   }
   // This should be just about a no-op if it's already updated.
   totalEigenFilter_->Update();
}

void BetaPipeline::FuseIntoHessian()
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

   for (itHess.GoToBegin(); not itHess.IsAtEnd(); ++itHess) {
      for (unsigned i = 0; i < 6; ++i) {
         itHess.Value()[i] = itGaussed[i].Get();

         ++itGaussed[i];
      }
   }
}
