#include "pipeline.h"
#include "settings.h"
#include "geometry.h"
#include "instrument.h"
#include "gaussian.h"

#include <itkImageRegionConstIterator.h>
#include <iostream>
#include <assert.h>


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
                           PointType const& physCenter)
  : eigenSystemAnalyzer_(Dimension)
  , bStructureTensorUpdated_(false)
{
   ImageType::IndexType index;
   bool isWithin = fullImage->TransformPhysicalPointToIndex(physCenter, index);
   assert(isWithin);

   // It's a filter, so, we want to set requested region on the input image,
   // and then a 'shift' transform of a fraction of a cell, right?
   // output- to- input, and in physical coordinates (itkResampleImageFilter.h)

   set_up_resampler(fullImage, physCenter);

   for (unsigned i = 0; i < Dimension; ++i) {
      derivative_filter_[i] = DerivativeFilterType::New();
      // order 1 is default anyway but...
      derivative_filter_[i]->SetOrder(1);
      derivative_filter_[i]->SetDirection(i);
      derivative_filter_[i]->SetInput(resampler_->GetOutput());
   }

   tensor_image_ = TensorImageType::New();
   ImageType::IndexType tstart = resampler_->GetOutputStartIndex();
   ImageType::SizeType  tsize = resampler_->GetSize();
   ImageType::RegionType tregion;
   tregion.SetIndex( tstart );
   tregion.SetSize( tsize );
   tensor_image_->SetRegions( tregion );
   tensor_image_->Allocate();

   // here we break off into doing things by hand

   for (unsigned i = 0; i < 6; ++i) {
      tensor_component_adaptor_[i] = TensorComponentAdaptorType::New();
      tensor_component_adaptor_[i]->SetImage(tensor_image_);
      tensor_component_adaptor_[i]->Allocate();
      tensor_component_adaptor_[i]->SelectNthElement(i);

      tensor_component_adaptor_[i] = TensorComponentAdaptorType::New();


      // They're each the same function, but they have to be templated
      // to a different adaptor (1-6)
      gaussian_functions_[i] = GaussianImageFunction::New();
      gaussian_functions_[i]->SetInputImage(tensor_component_adaptor_[i]);

      // &&& should handle different spacings.. wait, doesn't it already?
      // physical ('world') coordinates
      gaussian_functions_[i]->SetSigma(constants::SigmaOfFeatureGaussian);
   }

   // was once, OrderByValue, OrderByMagnitude
   eigenSystemAnalyzer_.SetOrderEigenValues(EigenSystemAnalyzerType::OrderByMagnitude);
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
   sizeSample.Fill(support_of_sigma(constants::SigmaOfFeatureGaussian));
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

   // We don't need to Update up front like this (later stages will
   // 'pull' from earlier ones), it just makes tracing through easier
   resampler_->Update();
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
      // we intervene in the pipeline by hand, here
      multiply_derivative_components();
      TakeGaussiansAtOriginalPoint();
      CalcEigenSystemAtOriginalPoint();
   }
}


void BetaPipeline::update_first_half()
{
   resampler_->Update();
   for (unsigned i = 0; i < Dimension; ++i) {
      derivative_filter_[i]->Update();
   }
}


void BetaPipeline::multiply_derivative_components()
{
   typedef itk::ImageRegionConstIterator<DerivativeFilterType::OutputImageType> DerivativeIteratorType;
   DerivativeIteratorType itDerived[Dimension];

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
   for (itGradientTensor.GoToBegin(); not itGradientTensor.IsAtEnd();
        ++itGradientTensor) {
      for (unsigned i = 0; i < 6; ++i) {
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


void BetaPipeline::TakeGaussiansAtOriginalPoint()
{
ImageSizeType tensorSize = tensor_image_->GetBufferedRegion().GetSize();

unsigned size = g_GaussianMask->size;
assert(tensorSize[0] == size);
assert(tensorSize[1] == size);
assert(tensorSize[2] == size);

   for (unsigned i = 0; i < 6; ++i) {

      double gaussian_of_center = 0.0;
      ImageIndexType idxTensor;
      for (unsigned z = 0; z < size; ++z) {
         idxTensor[2] = z;
         for (unsigned y = 0; y < size; ++y) {
            idxTensor[1] = y;
            for (unsigned x = 0; x < size; ++x) {
               idxTensor[0] = x;

               gaussian_of_center +=
                  tensor_image_->GetPixel(idxTensor)[i] * g_GaussianMask->at(x, y, z);
            }
         }
      }

      point_structure_tensor_[i] = gaussian_of_center;
   }
}


void BetaPipeline::CalcEigenSystemAtOriginalPoint() throw (FailedEigenAnalysis)
{
   // We want magnitude, we don't care about direction
   // &&& (wait, what would negative eigenvalues mean?)
   // <OrderByValue> is also possible, not sure what it'd mean though.

   int n = eigenSystemAnalyzer_.ComputeEigenValuesAndVectors(
      point_structure_tensor_,
      eigenValues_,
      eigenVectors_);

   // index of first failed eigenvalue
   if (0 < n) {
      throw FailedEigenAnalysis();
      ++n_eigen_failures;
   }
   else {
      ++n_eigen_successes;
   }
}


EigenValuesType const& BetaPipeline::eigenValues()
{
   update();
   return eigenValues_;
}

EigenVectorsType const& BetaPipeline::eigenVectors()
{
   update();
   return eigenVectors_;
}

// static
// // Given <row>, <col>, find corresponding index into symmetric tensor's
// // straight array storage.
// unsigned flat_index_into_symmetric_storage(unsigned row, unsigned col)
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
