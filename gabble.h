/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: ceExtractorConsoleBase.h,v $
  Language:  C++
  Date:      $Date: 2005/06/13 13:51:59 $
  Version:   $Revision: 1.5 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef GABBLE_H
#define GABBLE_H

// Explanation of what this does.
// http://www.itk.org/HTML/Curve3DExtraction.htm

// original in:
// /big/common/insight/InsightApplications-3.4.0/Curves3DExtractor/ceExtractorConsoleBase.cxx

#include <itkImage.h>
#include <itkHessianRecursiveGaussianImageFilter.h>
//#include <itkImageToParametricSpaceFilter.h>
//#include <itkVertexCell.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
//#include <itkRescaleIntensityImageFilter.h>
#include <itkSymmetricSecondRankTensor.h>
#include <itkSymmetricEigenAnalysisImageFilter.h>
#include <itkImageAdaptor.h>
#include <itkCastImageFilter.h>
//#include <itkPointSetToImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkJoinImageFilter.h>
#include <itkUnaryFunctorImageFilter.h>
#include <itkImageDuplicator.h>
#include "PixelAccessors.h"
#include "jgdTotalEigenImageFilter.h"

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <string>


// surface-tracing thoughts:

// For interpolation, this looks like the thing - you specify
// only the arbitrary points you're interested in. We'll probably
// call it over and over again.
// <InterpolateImagePointsFilter>
// /big/common/software/insight/InsightToolkit-3.4.0/
//    Testing/Code/BasicFilters/itkInterpolateImagePointsFilterTest.cxx

// &&& no! We need to feed it to other filters, so we can't just specify
// a few points, we have to have it be able to get them on demand.
// Fuck it fuck it fuck it.
//
// This doesn't kill us though. We can re-base an interpolated image,
// and wastefully resample the whole thing, though we'll use just a small region.
// ResampleImageFilter
//
// It seems to be ok-ish for 3-dimensional transforms, spline at any rate
// advertises itself as being N-dimensional.

// The points are at the center of their cube
// we need to be able to take a normal of two
//
// There won't be a /tree/ of transforms, but a single platform...
// No, not so, when one peters out - yes, so - we'll backtrack, but
// the transformation for each step will be the same.
// so, triangles,

//
// -- mesh TriangleCell
// Should look up triangulating polygons

// Screw triangles, for now.
// How to make a convex polygon, though?
// Take the plane normal, pick a point. Go counter-clockwise, by taking
// centroid, & dot products...?

// No - use marching cubes tables to get the edges, and make triangles
// from those (we know which edges our plane intersects).


// Write intermediate images for debug purposes
//    image of eigenvalues.
//    ImageSpace-ParametricSpace map,
//    ExtractedCurve points image.
#define INTERMEDIATE_OUTPUTS


class BetaFinder
{
public:
   typedef   double                                        InputPixelType;
   typedef   InputPixelType                                PixelType;
   typedef   PixelType                                     BetaPixelType;
   typedef   unsigned char                                 OverlayPixelType;
   typedef   double                                        EigenComponentType;

   static const unsigned Dimension = 3;

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

   typedef   itk::ImageFileReader< InputImageType >        VolumeReaderType;

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

#define WANT_EIGENVECTOR
#if defined(WANT_EIGENVECTOR)
   typedef   itk::TotalEigenImageFilter<HessianImageType, EigenValueImageType, EVectorImageType>
#else
   typedef   itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType>
#endif // WANT_EIGENVECTOR
      EigenAnalysisFilterType;


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

   typedef itk::ResampleImageFilter<InputImageType, InputImageType, double>
                                                                    ResampleFilterType;

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

   typedef   itk::ImageFileWriter<EValueCastImageFilterType::OutputImageType> WriterType;

   typedef   itk::ImageFileWriter<EValueCastImageFilterType::OutputImageType> EigenValueWriterType;
   typedef   itk::ImageFileWriter<EVectorCastImageFilterType::OutputImageType> EVectorWriterType;

   typedef   itk::Image< OverlayPixelType, Dimension > OverlayImageType;
   typedef   itk::ImageFileWriter< OverlayImageType > OverlayWriterType;

   typedef   HessianFilterType::RealType     RealType;

public:
   BetaFinder();
   virtual ~BetaFinder() {/* shut compiler up */}
   void HookUpEigenStuff();
   virtual void Load(const char* filename);
   //   virtual void ShowProgress(float);
   //   virtual void ShowStatus(const char * text);
   //   virtual void ShowSpatialFunctionControl( void );
   //   virtual void HideSpatialFunctionControl( void );
   virtual void Execute();
   virtual void SetSigma(RealType value) {
      //GradientMagnitudeRecursiveGaussianImageFilter
      //  m_GradientMagnitude->SetSigma( value );
      //HessianRecursiveGaussianImageFilter
      m_Hessian->SetSigma(value);
   }

//   void extract_alpha(MeshType::Pointer mesh);

   BetaImageType::Pointer
      extract_beta(InputImageType::ConstPointer density,
                   EachEigenValueImageType::Pointer e0,
                   EachEigenValueImageType::Pointer e1,
                   EachEigenValueImageType::Pointer e2,
                   double flatness_ratio);

protected:
   std::string                             m_fileBasename;
   double                                  m_beta_ratio;
   double                                  m_beta_threshold;
   VolumeReaderType::Pointer               m_Reader;
   InputImageType::ConstPointer            m_inputImage;

   HessianFilterType::Pointer              m_Hessian;

   EigenAnalysisFilterType::Pointer        m_TotalEigenFilter;

   EValueImageAdaptorType::Pointer         m_EValueAdaptor1;
   EValueImageAdaptorType::Pointer         m_EValueAdaptor2;
   EValueImageAdaptorType::Pointer         m_EValueAdaptor3;

   EVectorImageAdaptorType::Pointer        m_EVectorAdaptor1;
   EVectorImageAdaptorType::Pointer        m_EVectorAdaptor2;
   EVectorImageAdaptorType::Pointer        m_EVectorAdaptor3;


   EValueCastImageFilterType::Pointer      m_EValueCastfilter1;
   EValueCastImageFilterType::Pointer      m_EValueCastfilter2;
   EValueCastImageFilterType::Pointer      m_EValueCastfilter3;

   // problematic
   EVectorCastImageFilterType::Pointer     m_EVectorCastfilter1;
   EVectorCastImageFilterType::Pointer     m_EVectorCastfilter2;
   EVectorCastImageFilterType::Pointer     m_EVectorCastfilter3;


   WriterType::Pointer                     m_Writer;

   EigenValueWriterType::Pointer           m_EValueWriter;
//   EVectorWriterType::Pointer              m_EVectorWriter;

   bool                                    m_ImageLoaded;
};

#endif // GABBLE_H
