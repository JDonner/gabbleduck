#ifndef BETA_PIPELINE_H
#define BETA_PIPELINE_H

#include "types.h"
#include "point.h"
#include "polygon.h"

// After:
//    http://www.itk.org/pipermail/insight-users/2006-May/017729.html

// The ITK filters pipelined together

// &&& Frick, could we re-use this filter, just re-locate it as needed?
// It'd save so much memory thrashing.
struct BetaPipeline
{
   BetaPipeline(ImageType::Pointer image,
                PointType const& center,
                // In cells. no point in fractional cells (I believe)
                int region_width);

   EigenValuesType const& eigenValues() const {
      update();
      return eigenValues_;
   }

   EigenVectorsType const& eigenVectors() const {
      update();
      return eigenVectors_;
   }

   void set_up_resampler(ImageType::Pointer image,
                         PointType const& center);

private:
   void update_first_half();
   void gaussianize();
   void fuse_into_tensor();
   void update();

   // &&& Should try to reuse the pipeline, for speed's sake.
   void reset();

private:
   typedef itk::ResampleImageFilter< ImageType, ImageType, InternalPrecisionType > ResampleFilterType;
   ResampleFilterType::Pointer         resampler_;

   typedef DerivativeImageFilter<ImageType, ImageType> DerivativeFilterType;
   DerivativeFilterType::Pointer       derivative_filter_[3];

   typedef itk::NthElementImageAdaptor<
      TensorImageType, InternalPrecisionType >
      TensorComponentAdaptorType;

   TensorComponentAdaptorType::Pointer tensor_component_adaptor_[6];

   typedef itk::RecursiveGaussianImageFilter< InternalImageType, InternalImageType > GaussianFilterType;

   bool bStructureTensorUpdated_;

   // make a separate one just to avoid complications
   TensorFilterType::OutputImageType::Pointer  gaussianed_tensor_;

   TensorType                          point_structure_tensor_;

   typedef  EigenValue                 EigenValuesType[Dimension];
   typedef  EigenVector                EigenVectorsType[Dimension];

   EigenValuesType eigenValues_;
   EigenVectorsType eigenVectors_;
};

#endif // BETA_PIPELINE_H
