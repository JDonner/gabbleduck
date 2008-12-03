#ifndef GABBLE_TYPES_H
#define GABBLE_TYPES_H

// All types at once, to get them out of the way

#include <itkImage.h>
#include <itkTranslationTransform.h>

#include <itkImageAdaptor.h>
#include <itkCastImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>

#include "PixelAccessors.h"
// Discussed in section 6.5.1 (page 158) of the itk Software Guide pdf.
#include <itkHessianRecursiveGaussianImageFilter.h>
#include "jgdTotalEigenImageFilter.h"

#include "settings.h"


typedef double                                         InputPixelType;
typedef InputPixelType                                PixelType;
typedef PixelType                                     BetaPixelType;
typedef double                                         InternalPrecisionType;
typedef InternalPrecisionType                         EigenComponentType;

const unsigned Dimension = 3;

// pointer to array of size 3
//   typedef   EigenComponentType (*HardEigenVector)[Dimension];
// array of size 3
//   typedef   EigenComponentType HardEigenVector[Dimension];

typedef  itk::FixedArray<EigenComponentType, Dimension> FixedVectorType;
typedef  itk::Vector<InternalPrecisionType, Dimension>  VectorType;

typedef  InternalPrecisionType                         EigenValue;
typedef  VectorType                                    EigenVector;

typedef  EigenValue                                    EigenValues[Dimension];
typedef  EigenVector                                   EigenVectors[Dimension];

typedef  itk::CovariantVector< PixelType, Dimension >  CovariantVectorType;

typedef  itk::Image< InputPixelType, Dimension >       InputImageType;
typedef  itk::Image< InternalPrecisionType, Dimension > InternalImageType;
typedef  InputImageType                                ImageType;
typedef  ImageType                                     Image;
typedef  InputImageType                                BetaImageType;
typedef  itk::ImageRegion<Dimension>                   ImageRegion;
typedef  itk::Image< VectorType, Dimension >           VectorImageType;

typedef  itk::TranslationTransform<InternalPrecisionType, Dimension> TranslationTransform;


typedef  itk::HessianRecursiveGaussianImageFilter<InputImageType> HessianFilterType;
typedef  itk::SymmetricSecondRankTensor< InternalPrecisionType, Dimension > SymmTensorType;
typedef  HessianFilterType::OutputImageType            HessianImageType;
typedef  HessianImageType::PixelType                   HessianPixelType;


///////////////////////////////////////////////////////////////////
// Eigenvalue stuff
///////////////////////////////////////////////////////////////////

// Set of 3 eigenvalues
typedef  itk::FixedArray<EigenComponentType, HessianPixelType::Dimension>
   EigenValueArrayType;

typedef  itk::Matrix<EigenComponentType,
                     HessianPixelType::Dimension,
                     HessianPixelType::Dimension>     EigenVectorMatrixType;

// 3, of 3
typedef  itk::Image<EigenValueArrayType,
                     HessianImageType::ImageDimension> EigenValueImageType;
typedef  itk::Image<EigenVectorMatrixType,
                     HessianImageType::ImageDimension> EigenVectorImageType;

typedef  itk::TotalEigenImageFilter<HessianImageType, EigenValueImageType, EigenVectorImageType>
   EigenAnalysisFilterType;

//typedef  itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType>


// Reads off the eigenvalues
typedef  itk::ImageAdaptor<EigenValueImageType,
                           EigenvalueAccessor<EigenValueArrayType> > EValueImageAdaptorType;

// Each eigenvalue is a 3D mesh? We have 3 of these, one each per eigenvalue
// (which you'd never guess by the name).
// -- yeah, it ends up being just a 3D array of float.
typedef  itk::Image<EigenComponentType, Dimension>               EachEigenValueImageType;


// extracts eigenvalues
typedef  itk::CastImageFilter<
   EValueImageAdaptorType, EachEigenValueImageType>  EValueCastImageFilterType;

//typedef  itk::ResampleImageFilter<ImageType, ImageType, double> ResampleFilterType;

#if defined(GABBLE_INTERPOLATOR_IS_SPLINE) && (GABBLE_INTERPOLATOR_IS_SPLINE == 1)
  typedef  itk::BSplineInterpolateImageFunction<ImageType, PixelType> InterpolatorType;
#elif defined(GABBLE_INTERPOLATOR_IS_LINEAR)
  typedef itk::LinearInterpolateImageFunction<ImageType, PixelType> InterpolatorType;
#else
#  warning "no interpolator defined!"
#endif

///////////////////////////////////////////////////////////////////
// EigenVector
///////////////////////////////////////////////////////////////////


// This Cast filter we can exchange for an 'extract' filter.

// Reads off the eigenvectors
typedef  itk::ImageAdaptor<EigenVectorImageType,
                           EigenvectorAccessor<EigenVectorMatrixType, EigenVector> > EigenVectorImageAdaptorType;

typedef  itk::Image<EigenVector, Dimension>           EachEigenVectorImageType;

// extracts eigenvectors
typedef  itk::CastImageFilter<
   EigenVectorImageAdaptorType, EachEigenVectorImageType>  EigenVectorCastImageFilterType;

// This same types works for all images
typedef itk::Index<Dimension> IndexType;

#endif // GABBLE_TYPES_H
