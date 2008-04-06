#ifndef BETA_PIPELINE_H
#define BETA_PIPELINE_H

#include "types.h"
#include <itkTranslationTransform.h>

struct BetaPipeline
{
   BetaPipeline(ImageType::Pointer image,
                PointPos const& xform);

   typedef itk::ResampleImageFilter< ImageType, ImageType, double > ResampleFilterType;
   ResampleFilterType::Pointer resampler_;

   typedef itk::TranslationTransform< PixelType, Dimension > TransformType;
   TransformType::Pointer translation_;

   VectorType                   offset_;

   HessianFilterType::Pointer hessian_;

   // Eigenvalue
   // Create an adaptor and plug the output to the parametric space
   EValueImageAdaptorType::Pointer
      eValueAdaptor1_,
      eValueAdaptor2_,
      eValueAdaptor3_;

   // Eigenvector
   EVectorImageAdaptorType::Pointer
      eVectorAdaptor1_,
      eVectorAdaptor2_,
      eVectorAdaptor3_;

   EigenvalueAccessor< EigenValueArrayType >
      accessor1_,
      accessor2_,
      accessor3_;

   EigenvectorAccessor< EVectorMatrixType, EVector >
      vecAccessor1_,
      vecAccessor2_,
      vecAccessor3_;

   EigenAnalysisFilterType::Pointer
      totalEigenFilter_;

   // eValueCastfilter1 will give the eigenvalues with the maximum
   // eigenvalue. eValueCastfilter3 will give the eigenvalues with
   // the minimum eigenvalue.
   EValueCastImageFilterType::Pointer
      eValueCastFilter1_,
      eValueCastFilter2_,
      eValueCastFilter3_;

   EVectorCastImageFilterType::Pointer
      eVectorCastFilter1_,
      eVectorCastFilter2_,
      eVectorCastFilter3_;

   std::vector<itk::Object*> objects_;
};

#endif // BETA_PIPELINE_H
