#ifndef BETA_PIPELINE_H
#define BETA_PIPELINE_H

#include "types.h"
#include "point.h"
#include "polygon.h"

//#include <itkDiscreteGaussianImageFilter.h>
#include <itkRecursiveGaussianImageFilter.h>

// After:
//    http://www.itk.org/pipermail/insight-users/2006-May/017729.html

// The ITK filters pipelined together

struct BetaPipeline
{
   BetaPipeline(ImageType::Pointer image,
                PointType const& center,
                // In cells. no point in fractional cells (I believe)
                int region_width);

   EigenValueImageType::Pointer eigValImage() {
      update();
      return totalEigenFilter_->GetEigenValuesImage();
   }

   EigenVectorImageType::Pointer eigVecImage() {
      update();
      return totalEigenFilter_->GetEigenVectorsImage();
   }

   void set_up_resampler(ImageType::Pointer image,
                         PointType const& center);

private:
   void update();
   void update_first_half();
   void fuse_into_hessian();

   bool everythings_updated() const;

#define DEBUG_PRIVATE public
DEBUG_PRIVATE:
   typedef itk::ResampleImageFilter< ImageType, ImageType, InternalPrecisionType > ResampleFilterType;
   ResampleFilterType::Pointer         resampler_;

   typedef itk::TranslationTransform< InternalPrecisionType, Dimension > TransformType;
   TransformType::Pointer              translation_;

   VectorType                          offset_;

   HessianFilterType::Pointer          hessian_maker_;

//   typedef HessianFilterType::OutputImageType HessianImageType;

   typedef itk::NthElementImageAdaptor<
      HessianImageType, InternalPrecisionType >
      HessianComponentAdaptorType;

   HessianComponentAdaptorType::Pointer hess_component_adaptor_[6];

   typedef itk::RecursiveGaussianImageFilter<
      HessianComponentAdaptorType, InternalImageType > GaussianFilterType;
   GaussianFilterType::Pointer         gaussian_maker_[6];

   bool bStructureTensorUpdated_;

   // make a separate one just to avoid complications
   HessianFilterType::OutputImageType::Pointer  gaussianed_hessian_;

   EigenAnalysisFilterType::Pointer    totalEigenFilter_;

   // // &&& What's the difference between an adaptor and an accessor?
   // // Eigenvalue
   // // Create an adaptor and plug the output to the parametric space
   // EValueImageAdaptorType::Pointer
   //    eValueAdaptor1_,
   //    eValueAdaptor2_,
   //    eValueAdaptor3_;

   // // Eigenvector
   // EigenVectorImageAdaptorType::Pointer
   //    eVectorAdaptor1_,
   //    eVectorAdaptor2_,
   //    eVectorAdaptor3_;

   EigenvalueAccessor< EigenValueArrayType >
      valAccessor1_,
      valAccessor2_,
      valAccessor3_;

   EigenvectorAccessor< EigenVectorMatrixType, EigenVector >
      vecAccessor1_,
      vecAccessor2_,
      vecAccessor3_;

   // eValueCastfilter1 is the eigenvalues with the maximum eigenvalue.
   // eValueCastfilter3 is the eigenvalues with the minimum eigenvalue.
   EValueCastImageFilterType::Pointer
      eValueCastFilter1_,
      eValueCastFilter2_,
      eValueCastFilter3_;

   EigenVectorCastImageFilterType::Pointer
      eVectorCastFilter1_,
      eVectorCastFilter2_,
      eVectorCastFilter3_;
};

#endif // BETA_PIPELINE_H
