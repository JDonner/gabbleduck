#ifndef BETA_PIPELINE_H
#define BETA_PIPELINE_H

#include "types.h"
#include "point.h"
#include "polygon.h"
#include <itkDerivativeImageFilter.h>
#include <itkNthElementImageAdaptor.h>
#include <itkResampleImageFilter.h>
#include <itkGaussianBlurImageFunction.h>
#include <itkSymmetricEigenAnalysis.h>

// After:
//    http://www.itk.org/pipermail/insight-users/2006-May/017729.html

// The ITK filters pipelined together

// Thrown when eigenanalysis fails
struct FailedEigenAnalysis {
   // Its existence is enough
};

// &&& Frick, could we re-use this filter, just re-locate it as needed?
// It'd save so much memory thrashing.
struct BetaPipeline
{
   BetaPipeline(ImageType::Pointer image,
                PointType const& center);

   EigenValuesType const& eigenValues();
   EigenVectorsType const& eigenVectors();

   void set_up_resampler(ImageType::Pointer image,
                         PointType const& center);

private:
   void update_first_half();
   void update();
   void TakeGaussiansAtOriginalPoint();
   void multiply_derivative_components();
   void CalcEigenSystemAtOriginalPoint() throw (FailedEigenAnalysis);

   // &&& Could try to reuse the pipeline, for speed's sake.
   void reset();

private:
   typedef itk::ResampleImageFilter< ImageType, ImageType, InternalPrecisionType > ResampleFilterType;
   ResampleFilterType::Pointer         resampler_;

   typedef itk::DerivativeImageFilter< ImageType, ImageType > DerivativeFilterType;
   DerivativeFilterType::Pointer       derivative_filter_[Dimension];

   typedef  itk::Image<TensorType, Dimension>  TensorImageType;

   TensorImageType::Pointer                    tensor_image_;

   TensorType                                  point_structure_tensor_;

   typedef itk::NthElementImageAdaptor< TensorImageType, InternalPrecisionType >
      TensorComponentAdaptorType;

   TensorComponentAdaptorType::Pointer tensor_component_adaptor_[6];

   // We don't need a filter, because we want the result for one point
   // only.
   typedef itk::GaussianBlurImageFunction<TensorComponentAdaptorType,
      InternalPrecisionType> GaussianImageFunction;

   // One for each component of the gradient image, even though the
   // functions themselves are all the same. The 3D-ness of these
   // functions should come from the adaptor, which in turn comes from
   // the tensor image type.
   GaussianImageFunction::Pointer gaussian_functions_[6];

   typedef itk::SymmetricEigenAnalysis<TensorType, EigenValuesType, EigenVectorsType>
      EigenSystemAnalyzerType;

   EigenSystemAnalyzerType eigenSystemAnalyzer_;

   bool bStructureTensorUpdated_;

   EigenValuesType eigenValues_;
   EigenVectorsType eigenVectors_;
};

#endif // BETA_PIPELINE_H
