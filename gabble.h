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


#include <itkImage.h>
#include <itkHessianRecursiveGaussianImageFilter.h>
#include <itkImageToParametricSpaceFilter.h>
#include <itkMesh.h>
#include <itkVertexCell.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkInteriorExteriorMeshFilter.h>
#include <itkParametricSpaceToImageSpaceMeshFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkSymmetricSecondRankTensor.h>
#include <itkSymmetricEigenAnalysisImageFilter.h>
#include <itkImageAdaptor.h>
#include <itkCastImageFilter.h>
#include <itkPointSetToImageFilter.h>
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


// Define which type of spatial function to use
// Only one of the following lines should be uncommented.
//#define SPHERE_FUNCTION
#define FRUSTUM_FUNCTION

#if defined(SPHERE_FUNCTION)
#   include <itkSphereSpatialFunction.h>
#endif

#if defined(FRUSTUM_FUNCTION)
#   include <itkFrustumSpatialFunction.h>
#endif


// Write intermediate images for debug purposes
//    image of eigenvalues.
//    ImageSpace-ParametricSpace map,
//    ExtractedCurve points image.
#define INTERMEDIATE_OUTPUTS


class BetaFinder
{
public:
   typedef   double                                        InputPixelType;
   typedef   InputPixelType                                BetaPixelType;
   typedef   double                                        PixelType;
   typedef   unsigned char                                 OverlayPixelType;
   typedef   float                                         MeshPixelType;
   typedef   MeshPixelType                                 EigenPixelType;

   itkStaticConstMacro(Dimension, unsigned int, 3);

   typedef   itk::Vector< PixelType, Dimension >           VectorType;

   typedef   itk::CovariantVector< PixelType, Dimension >  CovariantVectorType;

   typedef   itk::Image< InputPixelType, Dimension >       InputImageType;
   typedef   InputImageType                                BetaImageType;
   typedef   itk::Image< PixelType, Dimension >            ImageType;
   typedef   itk::Image< VectorType, Dimension >           VectorImageType;
   typedef   itk::Image< CovariantVectorType, Dimension >  CovariantVectorImageType;

   // So the mesh is indeed, of points. 3 floats? MeshPixelType == float
   typedef   itk::Point< MeshPixelType, Dimension>         MeshPointDataType;

   // 3D 'sparse' array of float[3]
   typedef   itk::Mesh< MeshPointDataType, 3 >             MeshType;

   typedef   itk::ImageFileReader< InputImageType >        VolumeReaderType;

   // hmmm. Looks the same as <MeshType>; ie, 3D 'sparse' array of 3D points
   typedef   itk::Mesh< MeshType::PointType, Dimension >   ImageSpaceMeshType;

   // typedef   itk::GradientMagnitudeRecursiveGaussianImageFilter<
   //                           InputImageType,
   //                           ImageType        > GradientMagnitudeFilterType;

   typedef   itk::HessianRecursiveGaussianImageFilter<InputImageType>
                                                           HessianFilterType;
   typedef   HessianFilterType::OutputImageType            HessianImageType;
   typedef   HessianImageType::PixelType                   HessianPixelType;

   // Set of 3 eigenvalues
   typedef   itk::FixedArray<double, HessianPixelType::Dimension>
                                                           EigenValueArrayType;

   typedef   itk::Matrix<double,
                         HessianPixelType::Dimension,
                         HessianPixelType::Dimension>    EVectorMatrixType;

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

   typedef   itk::UnaryFunctorImageFilter< HessianImageType, ImageType,
                                           Functor::HessianToLaplacianFunction< HessianPixelType, PixelType > >
      HessianToLaplacianImageFilter;


   ///////////////////////////////////////////////////////////////////
   // Eigenvalue stuff
   ///////////////////////////////////////////////////////////////////

   // Reads off the eigenvalues
   typedef   itk::ImageAdaptor<EigenValueImageType,
                               EigenvalueAccessor<EigenValueArrayType> > EValueImageAdaptorType;

   // Each eigenvalue is a 3D mesh? We have 3 of these, one each per eigenvalue
   // (which you'd never guess by the name).
   // -- yeah, it ends up being just a 3D array of float.
   typedef   itk::Image< MeshPointDataType::ValueType,
                         MeshPointDataType::PointDimension >
      EachEigenValueImageType;


   // extracts eigenvalues - need to make it extract eigenvectors, too
   typedef   itk::CastImageFilter<
      EValueImageAdaptorType, EachEigenValueImageType>  EValueCastImageFilterType;



   ///////////////////////////////////////////////////////////////////
   // EVector
   ///////////////////////////////////////////////////////////////////

   // Reads off the eigenvalues
   typedef   itk::ImageAdaptor<EVectorImageType,
                               EigenvectorAccessor<EVectorMatrixType> > EVectorImageAdaptorType;

   // Each eigenvalue is a 3D mesh? We have 3 of these, one each per eigenvalue
   // (which you'd never guess by the name).
   // -- yeah, it ends up being just a 3D array of float.
   typedef   itk::Image< MeshPointDataType::ValueType,
                         MeshPointDataType::PointDimension >
      EachEVectorImageType;

   // extracts eigenvalues - need to make it extract eigenvectors, too
   typedef   itk::CastImageFilter<
      EVectorImageAdaptorType, EachEVectorImageType>  EVectorCastImageFilterType;



   typedef   itk::ImageFileWriter<EValueCastImageFilterType::OutputImageType> WriterType;

   typedef   itk::ImageFileWriter<EValueCastImageFilterType::OutputImageType> EigenValueWriterType;
   typedef   itk::ImageFileWriter<EVectorCastImageFilterType::OutputImageType> EVectorWriterType;

   // MeshType is 3 points, each 3D
   typedef   itk::ImageToParametricSpaceFilter<
      EachEigenValueImageType, MeshType> ParametricEigenvalueSpaceFilterType;

   // MeshType is 3 points, each 3D
   typedef   itk::ImageToParametricSpaceFilter<
      EachEVectorImageType, MeshType> ParametricEVectorSpaceFilterType;

   typedef   itk::RescaleIntensityImageFilter<
      ImageType, ImageType> RescaleIntensityFilterType;

#if defined(SPHERE_FUNCTION)
   typedef   itk::SphereSpatialFunction<
      MeshType::PointDimension,
      MeshType::PointType >  SphereSpatialFunctionType;
#endif

#if defined(FRUSTUM_FUNCTION)
   typedef   itk::FrustumSpatialFunction<
      MeshType::PointDimension,
      MeshType::PointType >  FrustumSpatialFunctionType;
#endif



   //   typedef fltk::SphereFunctionControl<
   //                                   SphereSpatialFunctionType >
   //                                             SphereSpatialFunctionControlType;

   //   typedef fltk::FrustumFunctionControl<
   //                                   FrustumSpatialFunctionType >
   //                                             FrustumSpatialFunctionControlType;

   // These typedefs select the particular SpatialFunction
#ifdef SPHERE_FUNCTION
   typedef  SphereSpatialFunctionType          SpatialFunctionType;
   //    typedef  SphereSpatialFunctionControlType   SpatialFunctionControlType;
#endif

#ifdef FRUSTUM_FUNCTION
   typedef  FrustumSpatialFunctionType         SpatialFunctionType;
   //    typedef  FrustumSpatialFunctionControlType  SpatialFunctionControlType;
#endif


   typedef   itk::InteriorExteriorMeshFilter<
      MeshType,
      MeshType,
      SpatialFunctionType  >
      SpatialFunctionFilterType;

   // I was thinking <MeshType> == <ImageSpaceMeshType> but, not sure.
   typedef   itk::ParametricSpaceToImageSpaceMeshFilter<
      // /These are provably the same type!/
      // BOOST_MPL_ASSERT((boost::is_same<ImageSpaceMeshType, MeshType>));
      MeshType,
      ImageSpaceMeshType
      >         InverseParametricFilterType;

   typedef   itk::PointSetToImageFilter< MeshType, ImageType >
      PointSetToImageFilterType;

   typedef   PointSetToImageFilterType::OutputImageType
      PointSetImageType;

   typedef   itk::ImageFileWriter< PointSetImageType >
      MergedWriterType;

   typedef   itk::Image< OverlayPixelType, Dimension > OverlayImageType;
   typedef   itk::ImageFileWriter< OverlayImageType > OverlayWriterType;

   typedef   itk::ResampleImageFilter< PointSetImageType, PointSetImageType >
      OverlayImageResampleFilterType;

   typedef   itk::ImageFileWriter< OverlayImageResampleFilterType::OutputImageType >
      ResampleWriterType;

   typedef   itk::BinaryThresholdImageFilter<
      PointSetImageType, OverlayImageType >  ThresholdImageFilterType;

   //  typedef   GradientMagnitudeFilterType::RealType     RealType;
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

   void extract_alpha(MeshType::Pointer mesh);
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

//  GradientMagnitudeFilterType::Pointer    m_GradientMagnitude;

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

   EVectorCastImageFilterType::Pointer     m_EVectorCastfilter1;
   EVectorCastImageFilterType::Pointer     m_EVectorCastfilter2;
   EVectorCastImageFilterType::Pointer     m_EVectorCastfilter3;


   HessianToLaplacianImageFilter::Pointer  m_Laplacian;

   ParametricEigenvalueSpaceFilterType::Pointer m_ParametricEigenvalueSpace;

   SpatialFunctionFilterType::Pointer      m_SpatialFunctionFilter;

   //   SpatialFunctionControlType::Pointer     m_SpatialFunctionControl;

   InverseParametricFilterType::Pointer    m_InverseParametricFilter;

   PointSetToImageFilterType::Pointer      m_PointSetToImageFilter;

   OverlayImageResampleFilterType::Pointer m_OverlayResampleFilter;

   ThresholdImageFilterType::Pointer       m_ThresholdImageFilter;

   WriterType::Pointer                     m_Writer;

   EigenValueWriterType::Pointer           m_EValueWriter;
   EVectorWriterType::Pointer          m_EVectorWriter;

   MergedWriterType::Pointer               m_MergedWriter;

   ResampleWriterType::Pointer             m_ResampleWriter;
   OverlayWriterType::Pointer              m_OverlayWriter;

   bool                                    m_ImageLoaded;
};

#endif // GABBLE_H
