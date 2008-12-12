#ifndef GABBLE_TYPES_H
#define GABBLE_TYPES_H

#include <itkImage.h>
#include <itkFixedArray.h>

#include <itkImageAdaptor.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>

#include <itkSymmetricSecondRankTensor.h>

#include "settings.h"


typedef float                                         InputPixelType;
typedef InputPixelType                                PixelType;
typedef PixelType                                     BetaPixelType;
typedef float                                         Flt;
typedef Flt                         EigenComponentType;

const unsigned Dimension = 3;

typedef  itk::FixedArray<EigenComponentType, Dimension> FixedVectorType;
typedef  itk::Vector<Flt, Dimension>  VectorType;

typedef  Flt                         EigenValueType;
typedef  VectorType                                    EigenVectorType;

typedef  itk::FixedArray<EigenValueType, Dimension>    EigenValuesType;
typedef  itk::FixedArray<VectorType, Dimension>        EigenVectorsType;

typedef  itk::Image< InputPixelType, Dimension >       InputImageType;
typedef  itk::Image< Flt, Dimension > InternalImageType;
typedef  InternalImageType                             ImageType;
//typedef  ImageType                                     Image;
//typedef  InputImageType                                BetaImageType;
typedef  itk::ImageRegion<Dimension>                   ImageRegionType;
typedef  itk::Index<Dimension>                         ImageIndexType;
typedef  itk::Size<Dimension>                          ImageSizeType;

typedef  itk::SymmetricSecondRankTensor< Flt, Dimension > TensorType;

#if defined(GABBLE_INTERPOLATOR_IS_SPLINE) && (GABBLE_INTERPOLATOR_IS_SPLINE == 1)
  typedef  itk::BSplineInterpolateImageFunction<ImageType, PixelType> InterpolatorType;
#elif defined(GABBLE_INTERPOLATOR_IS_LINEAR)
  typedef itk::LinearInterpolateImageFunction<ImageType, PixelType> InterpolatorType;
#else
#  warning "no interpolator defined!"
#endif

#endif // GABBLE_TYPES_H
