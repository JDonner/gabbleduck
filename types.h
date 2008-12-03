#ifndef GABBLE_TYPES_H
#define GABBLE_TYPES_H

// All types at once, to get them out of the way

#include <itkImage.h>

#include <itkImageAdaptor.h>
#include <itkResampleImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>

#include "PixelAccessors.h"
#include <itkSymmetricSecondRankTensor.h>

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

typedef  InternalPrecisionType                         EigenValueType;
typedef  VectorType                                    EigenVectorType;

typedef  EigenValueType                                EigenValuesType[Dimension];
typedef  EigenVectorType                               EigenVectorsType[Dimension];

typedef  itk::Image< InputPixelType, Dimension >       InputImageType;
typedef  itk::Image< InternalPrecisionType, Dimension > InternalImageType;
typedef  InputImageType                                ImageType;
typedef  ImageType                                     Image;
typedef  InputImageType                                BetaImageType;
typedef  itk::ImageRegion<Dimension>                   ImageRegion;
typedef  itk::Index<Dimension>                         IndexType;

typedef  itk::SymmetricSecondRankTensor< InternalPrecisionType, Dimension > TensorType;

#if defined(GABBLE_INTERPOLATOR_IS_SPLINE) && (GABBLE_INTERPOLATOR_IS_SPLINE == 1)
  typedef  itk::BSplineInterpolateImageFunction<ImageType, PixelType> InterpolatorType;
#elif defined(GABBLE_INTERPOLATOR_IS_LINEAR)
  typedef itk::LinearInterpolateImageFunction<ImageType, PixelType> InterpolatorType;
#else
#  warning "no interpolator defined!"
#endif

#endif // GABBLE_TYPES_H
