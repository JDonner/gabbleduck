#ifndef GABBLE_TYPES_H
#define GABBLE_TYPES_H

#include <itkImage.h>
#include <itkHessianRecursiveGaussianImageFilter.h>

#include <itkImageAdaptor.h>
#include <itkCastImageFilter.h>
#include <itkResampleImageFilter.h>

#include "PixelAccessors.h"
#include "jgdTotalEigenImageFilter.h"



typedef   double                                        InputPixelType;
typedef   InputPixelType                                PixelType;
typedef   PixelType                                     BetaPixelType;
typedef   unsigned char                                 OverlayPixelType;
typedef   double                                        EigenComponentType;

const unsigned Dimension = 3;

// pointer to array of size 3
//   typedef   EigenComponentType (*HardEVector)[Dimension];
// array of size 3
//   typedef   EigenComponentType HardEVector[Dimension];

typedef   itk::FixedArray<EigenComponentType, Dimension> FixedVectorType;
typedef   itk::Vector<PixelType, Dimension>              VectorType;

typedef   VectorType                                     EVector;



typedef   itk::CovariantVector< PixelType, Dimension >  CovariantVectorType;

typedef   itk::Image< InputPixelType, Dimension >       InputImageType;
typedef   InputImageType                                ImageType;
typedef   InputImageType                                BetaImageType;
typedef   itk::Image< VectorType, Dimension >           VectorImageType;
typedef   itk::Image< CovariantVectorType, Dimension >  CovariantVectorImageType;

//   typedef   itk::ImageFileReader< InputImageType >        VolumeReaderType;

typedef   itk::HessianRecursiveGaussianImageFilter<InputImageType>
   HessianFilterType;
typedef   HessianFilterType::OutputImageType            HessianImageType;
typedef   HessianImageType::PixelType                   HessianPixelType;

// Set of 3 eigenvalues
typedef   itk::FixedArray<double, HessianPixelType::Dimension>
   EigenValueArrayType;

typedef   itk::Matrix<double,
                      HessianPixelType::Dimension,
                      HessianPixelType::Dimension>     EVectorMatrixType;

// 3, of 3
typedef   itk::Image<EigenValueArrayType,
                     HessianImageType::ImageDimension> EigenValueImageType;
typedef   itk::Image<EVectorMatrixType,
                     HessianImageType::ImageDimension> EVectorImageType;

typedef   itk::TotalEigenImageFilter<HessianImageType, EigenValueImageType, EVectorImageType>
   EigenAnalysisFilterType;
#if 0
typedef   itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType>
#endif


///////////////////////////////////////////////////////////////////
// Eigenvalue stuff
///////////////////////////////////////////////////////////////////

// Reads off the eigenvalues
   typedef   itk::ImageAdaptor<EigenValueImageType,
                               EigenvalueAccessor<EigenValueArrayType> > EValueImageAdaptorType;

// Each eigenvalue is a 3D mesh? We have 3 of these, one each per eigenvalue
// (which you'd never guess by the name).
// -- yeah, it ends up being just a 3D array of float.
typedef   itk::Image< EigenComponentType, Dimension > EachEigenValueImageType;


// extracts eigenvalues
typedef   itk::CastImageFilter<
   EValueImageAdaptorType, EachEigenValueImageType>  EValueCastImageFilterType;

typedef itk::ResampleImageFilter<ImageType, ImageType, double> ResampleFilterType;

///////////////////////////////////////////////////////////////////
// EVector
///////////////////////////////////////////////////////////////////


// This Cast filter we can exchange for an 'extract' filter.

// Reads off the eigenvectors
typedef   itk::ImageAdaptor<EVectorImageType,
                            EigenvectorAccessor<EVectorMatrixType, EVector> > EVectorImageAdaptorType;

// Each eigenvector is a 3D mesh? We have 3 of these, one each per eigenvector
// (which you'd never guess by the name).
// -- yeah, it ends up being just a 3D array of float.
// &&& HEY HERE'S THE SOURCE OF MUCH TROUBLE
// &&& Why are we going to 'mesh' in the first place? Why not just
// an image of a vector row? Let's go fix adaptors.

typedef   itk::Image<EVector, Dimension>           EachEVectorImageType;

// extracts eigenvectors
typedef   itk::CastImageFilter<
   EVectorImageAdaptorType, EachEVectorImageType>  EVectorCastImageFilterType;

// typedef   itk::ImageFileWriter<EValueCastImageFilterType::OutputImageType> WriterType;

// typedef   itk::ImageFileWriter<EValueCastImageFilterType::OutputImageType> EigenValueWriterType;
// typedef   itk::ImageFileWriter<EVectorCastImageFilterType::OutputImageType> EVectorWriterType;

// typedef   itk::Image< OverlayPixelType, Dimension > OverlayImageType;
// typedef   itk::ImageFileWriter< OverlayImageType > OverlayWriterType;

typedef   HessianFilterType::RealType     RealType;

#endif // GABBLE_TYPES_H
