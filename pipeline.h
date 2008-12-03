#ifndef BETA_PIPELINE_H
#define BETA_PIPELINE_H

#include "types.h"
#include "point.h"
#include "polygon.h"
#include <itkDerivativeImageFilter.h>
#include <itkNthElementImageAdaptor.h>
#include <itkGaussianBlurImageFunction.h>
#include <itkSymmetricEigenAnalysis.h>

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

   EigenValuesType const& eigenValues() {
      update();
      return eigenValues_;
   }

   EigenVectorsType const& eigenVectors() {
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
   void TakeGaussiansAtOrignalPoint();
   void multiply_gaussian_components();
   void CalcEigenSystemAtOriginalPoint();

   // &&& Should try to reuse the pipeline, for speed's sake.
   void reset();

private:
   typedef itk::ResampleImageFilter< ImageType, ImageType, InternalPrecisionType > ResampleFilterType;
   ResampleFilterType::Pointer         resampler_;

   typedef itk::DerivativeImageFilter< ImageType, ImageType > DerivativeFilterType;
   DerivativeFilterType::Pointer       derivative_filter_[Dimension];

   typedef  itk::Image<TensorType, Dimension>           TensorImageType;

   TensorImageType::Pointer                    tensor_image_;

   TensorType                                  point_structure_tensor_;

   typedef itk::NthElementImageAdaptor< TensorImageType, InternalPrecisionType >
      TensorComponentAdaptorType;

   TensorComponentAdaptorType::Pointer tensor_component_adaptor_[6];

   typedef itk::GaussianBlurImageFunction<TensorComponentAdaptorType,
      TensorType> GaussianImageFunction;

   GaussianImageFunction::Pointer gaussians_[6];

   typedef itk::SymmetricEigenAnalysis<TensorType, EigenValuesType, EigenVectorsType>
      EigenSystemAnalyzerType;

   EigenSystemAnalyzerType eigenSystemAnalyzer_;

   bool bStructureTensorUpdated_;

   EigenValuesType eigenValues_;
   EigenVectorsType eigenVectors_;
};

#endif // BETA_PIPELINE_H
