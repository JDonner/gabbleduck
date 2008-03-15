#include "gabble.h"

#include <iostream>

using namespace std;


// Hook the pipelines up
BetaFinder::BetaFinder()
{
   m_ImageLoaded = false;

   m_Reader      = VolumeReaderType::New();

   //  || Gradient( Image ) ||
   //  m_GradientMagnitude = GradientMagnitudeFilterType::New();
   //  m_GradientMagnitude->SetInput( m_Reader->GetOutput() );

   m_Hessian = HessianFilterType::New();
   m_Hessian->SetInput( m_Reader->GetOutput() );

   BetaFinder::HookUpEigenStuff();

   // SphereSpatialFunction
   // function that returns 0 for points inside or on the surface of a sphere,
   // 1 for points outside the sphere
   // What the heck this is for, I couldn't say.
   // Maybe to trim out regions that are too small?
   // SpatialFunction here applies to a mesh,
   // and writes to a mesh

   // m_SpatialFunctionFilter = SpatialFunctionFilterType::New();
   // m_SpatialFunctionFilter->SetInput(m_ParametricEigenvalueSpace->GetOutput());

   //   m_SpatialFunctionControl = SpatialFunctionControlType::New();
   //   m_SpatialFunctionControl->SetSpatialFunction(
   //                   m_SpatialFunctionFilter->GetSpatialFunction() );


   // I think this conversion is needless (mmm, maybe).
   // m_InverseParametricFilter = InverseParametricFilterType::New();
   // m_InverseParametricFilter->SetInput(
   //    m_SpatialFunctionFilter->GetOutput() );

   // Here's where it comes back to image, after getting some spatial thing.
   // Might learn how to translate the point set to image filter, from it.
   //
   // Generate an image from the extracted points for an overlay display

   // m_PointSetToImageFilter = PointSetToImageFilterType::New();
   // m_PointSetToImageFilter->SetInput(
   //    m_InverseParametricFilter->GetOutput() );

   // m_MergedWriter = MergedWriterType::New();
   // m_MergedWriter->SetFileName( "Remerged.vtk");
   // m_MergedWriter->SetInput(m_PointSetToImageFilter->GetOutput());

   // m_OverlayResampleFilter = OverlayImageResampleFilterType::New();
   // m_OverlayResampleFilter->SetInput( m_PointSetToImageFilter->GetOutput() );

   // m_ResampleWriter = ResampleWriterType::New();
   // m_ResampleWriter->SetFileName( "Resampled.vtk");
   // m_ResampleWriter->SetInput(m_OverlayResampleFilter->GetOutput());

   // Output of the ThresholdImageFilter will be used as the
   // overlay image of extracted points
   // m_ThresholdImageFilter     = ThresholdImageFilterType::New();
   // //  m_ThresholdImageFilter->SetLowerThreshold( 0.1 ); // crank it up a bit.
   // m_ThresholdImageFilter->SetLowerThreshold( 0.0001 ); // Get all non-zero pixels
   // m_ThresholdImageFilter->SetUpperThreshold( itk::NumericTraits<
   //                                            ThresholdImageFilterType::OutputPixelType >::max() );
   // // The either/or chosen output values for those pixels in the
   // // input image that fall between lower & upper.
   // m_ThresholdImageFilter->SetOutsideValue( 0 );
   // m_ThresholdImageFilter->SetInsideValue( 255 );
   // m_ThresholdImageFilter->SetInput( m_OverlayResampleFilter->GetOutput() );

   // m_OverlayWriter = OverlayWriterType::New();
   // m_OverlayWriter->SetInput( m_ThresholdImageFilter->GetOutput() );
   // m_OverlayWriter->SetFileName( "PostThreshold.vtk");

#if defined(INTERMEDIATE_OUTPUTS)
   m_EValueWriter = EigenValueWriterType::New();
#endif
}

void BetaFinder::HookUpEigenStuff()
{
   // Compute eigenvalues.. order them in ascending order
   m_TotalEigenFilter = EigenAnalysisFilterType::New();
   m_TotalEigenFilter->SetDimension( HessianPixelType::Dimension );
   m_TotalEigenFilter->SetInput( m_Hessian->GetOutput() );
   m_TotalEigenFilter->OrderEigenValuesBy(
      EigenAnalysisFilterType::JgdCalculatorType::OrderByValue);

   // Eigenvalue
   // Create an adaptor and plug the output to the parametric space
   m_EValueAdaptor1 = EValueImageAdaptorType::New();
   EigenvalueAccessor< EigenValueArrayType > accessor1;
   accessor1.SetEigenIdx( 0 );
   m_EValueAdaptor1->SetImage( m_TotalEigenFilter->GetEigenValuesImage() );
   m_EValueAdaptor1->SetPixelAccessor( accessor1 );

   m_EValueAdaptor2 = EValueImageAdaptorType::New();
   EigenvalueAccessor< EigenValueArrayType > accessor2;
   accessor2.SetEigenIdx( 1 );
   m_EValueAdaptor2->SetImage( m_TotalEigenFilter->GetEigenValuesImage() );
   m_EValueAdaptor2->SetPixelAccessor( accessor2 );

   m_EValueAdaptor3 = EValueImageAdaptorType::New();
   EigenvalueAccessor< EigenValueArrayType > accessor3;
   accessor3.SetEigenIdx( 2 );
   m_EValueAdaptor3->SetImage( m_TotalEigenFilter->GetEigenValuesImage() );
   m_EValueAdaptor3->SetPixelAccessor( accessor3 );


   // Eigenvector
   // Create an adaptor and plug the output to the parametric space
   m_EVectorAdaptor1 = EVectorImageAdaptorType::New();
   EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor1;
   accessor1.SetEigenIdx( 0 );
   m_EVectorAdaptor1->SetImage( m_TotalEigenFilter->GetEigenVectorsImage() );
   m_EVectorAdaptor1->SetPixelAccessor( vecAccessor1 );

   m_EVectorAdaptor2 = EVectorImageAdaptorType::New();
   EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor2;
   accessor2.SetEigenIdx( 1 );
   m_EVectorAdaptor2->SetImage( m_TotalEigenFilter->GetEigenVectorsImage() );
   m_EVectorAdaptor2->SetPixelAccessor( vecAccessor2 );

   m_EVectorAdaptor3 = EVectorImageAdaptorType::New();
   EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor3;
   accessor3.SetEigenIdx( 2 );
   m_EVectorAdaptor3->SetImage( m_TotalEigenFilter->GetEigenVectorsImage() );
   m_EVectorAdaptor3->SetPixelAccessor( vecAccessor3 );


   // m_EValueCastfilter1 will give the eigenvalues with the maximum
   // eigenvalue. m_EValueCastfilter3 will give the eigenvalues with
   // the minimum eigenvalue.
   m_EValueCastfilter1 = EValueCastImageFilterType::New();
   m_EValueCastfilter1->SetInput( m_EValueAdaptor3 );
   m_EValueCastfilter2 = EValueCastImageFilterType::New();
   m_EValueCastfilter2->SetInput( m_EValueAdaptor2 );
   m_EValueCastfilter3 = EValueCastImageFilterType::New();
   m_EValueCastfilter3->SetInput( m_EValueAdaptor1 );

   // Shoot shoot shoot - I want the matching eigenvector with each value;
   // have to figure out how to keep track of that.
   // - heh - I think it's ok as-is!

   m_EVectorCastfilter1 = EVectorCastImageFilterType::New();
   m_EVectorCastfilter1->SetInput( m_EVectorAdaptor3 );
   m_EVectorCastfilter2 = EVectorCastImageFilterType::New();
   m_EVectorCastfilter2->SetInput( m_EVectorAdaptor2 );
   m_EVectorCastfilter3 = EVectorCastImageFilterType::New();
   m_EVectorCastfilter3->SetInput( m_EVectorAdaptor1 );



   // I think this parametric stuff is just, about having to use 3-valued
   // points.

   // m_ParametricEigenvalueSpace = ParametricEigenvalueSpaceFilterType::New();
   // m_ParametricEigenvectorSpace = ParametricEigenvectorSpaceFilterType::New();

   // maximum, to minimum eigenvalue

   // "The mesh contains one point for every pixel on the images. The
   // coordinate of the point being equal to the gray level of the
   // associated input pixels.  This class is intended to produce the
   // population of points that represent samples in a parametric space. In
   // this particular case the parameters are the gray levels of the input
   // images. The dimension of the mesh points should be equal to the number
   // of input images to this filter."
   //
   // So, the grey level at each point in the 3 images, is one component of
   // a vector of parameters for that point. A sort of Muxer / demuxer.

   // 3 images -> a mesh
   // m_ParametricEigenvalueSpace->SetInput( 0, m_EValueCastfilter1->GetOutput() );
   // m_ParametricEigenvalueSpace->SetInput( 1, m_EValueCastfilter2->GetOutput() );
   // m_ParametricEigenvalueSpace->SetInput( 2, m_EValueCastfilter3->GetOutput() );

   // m_ParametricEigenvectorSpace->SetInput( 0, m_EVectorCastfilter1->GetOutput() );
   // m_ParametricEigenvectorSpace->SetInput( 1, m_EVectorCastfilter2->GetOutput() );
   // m_ParametricEigenvectorSpace->SetInput( 2, m_EVectorCastfilter3->GetOutput() );

   // BOOST_MPL_ASSERT((boost::is_same<ImageSpaceMeshType, MeshType>));
}


void BetaFinder::Load(char const* basename)
{
   if( !basename ) {
      cout << "No filename!" << endl;
   }
   else {
      m_fileBasename = basename;
      m_Reader->SetFileName( m_fileBasename + ".vtk" );
      m_Reader->Update();

      InputImageType::Pointer m_inputImage = m_Reader->GetOutput();

      m_inputImage->SetRequestedRegion(m_inputImage->GetLargestPossibleRegion());

      m_ImageLoaded = true;
   }
}

// Used to classify the seeds
bool IsBeta(double sheetMin, double sheetMax,
            double t1, double t2, double t3)
{
   bool isBeta = sheetMin <= t1 and t1 <= sheetMax and
      min(t2 / t3, t3 / t2) > max(t1 / t2, t1 / t3);

   return isBeta;
}

// betaimage == inputimage, ie scalar
BetaFinder::BetaImageType::Pointer
   BetaFinder::extract_beta(InputImageType::ConstPointer density,
                            EachEigenValueImageType::Pointer e0,
                            EachEigenValueImageType::Pointer e1,
                            EachEigenValueImageType::Pointer e2,
                            double flatness_ratio)
{
   m_beta_ratio = flatness_ratio;

   typedef itk::ImageDuplicator<InputImageType> ImageDuplicator;
   ImageDuplicator::Pointer dupper = ImageDuplicator::New();
   dupper->SetInputImage(m_Reader->GetOutput());
   m_Reader->Update();
   dupper->Update();

   BetaImageType::Pointer beta = dupper->GetOutput();
   //    output_image->Print(cout);
   beta->FillBuffer(BetaPixelType(0));

   // Do I need to initialize this?

   typedef itk::ImageRegionConstIterator< InputImageType > InputConstIterType;

   typedef itk::ImageRegionConstIterator< EachEigenValueImageType > EigenConstIteratorType;
   EigenConstIteratorType itE0( e0, e0->GetRequestedRegion() );
   EigenConstIteratorType itE1( e1, e1->GetRequestedRegion() );
   EigenConstIteratorType itE2( e2, e2->GetRequestedRegion() );

   InputConstIterType itDensity ( density, density->GetRequestedRegion() );

   typedef itk::ImageRegionIterator< BetaImageType > BetaOutIteratorType;
   BetaOutIteratorType itBeta( beta, beta->GetRequestedRegion() );


   BetaPixelType max_flatness = -1.0;
   unsigned num_beta_pixels = 0;
   unsigned num_pixels = 0;
   BetaPixelType
      max_v0 = -1e38, max_v1 = -1e38,
      min_v0 = 1e38, min_v1 = 1e38,
      sum_v0 = 0, sum_v1 = 0;

   for ( ; !itE0.IsAtEnd(); ++itE0, ++itE1, ++itE2, ++itBeta, ++itDensity) {

      BetaPixelType v0(itE0.Value()), v1(itE1.Value()); //, v2(itE2.Value());
      BetaPixelType flatness = (v0 - v1) / v0;
      //     InputPixelType density(itDensity.Value());

      ++num_pixels;

      min_v0 = min(min_v0, v0);
      min_v1 = min(min_v1, v1);

      max_v0 = max(max_v0, v0);
      max_v1 = max(max_v1, v1);

      sum_v0 += v0;
      sum_v1 += v1;

      if (5.0 <= v0 / (v1 + 1.0e-20) && 0.8 <= flatness) {
         itBeta.Set(flatness);
         ++num_beta_pixels;
         if (num_beta_pixels < 100) {
            cout << "beta pixel " << flatness << "; v0: " << v0 << "; v1: " << v1 << endl;
         }
      }
      if (max_flatness < flatness ) {
         max_flatness = flatness;
      }
   }

   BetaPixelType avg_v0 = sum_v0 / num_pixels, avg_v1 = sum_v1 / num_pixels;
   cout << "avg v0: " << avg_v0 << "avg v1: " << avg_v1 << endl;
   cout << "min v0: " << min_v0 << "min v1: " << min_v1 << endl;
   cout << "max v0: " << max_v0 << "max v1: " << max_v1 << endl;
   cout << "max flatness: " << max_flatness << "; " << num_beta_pixels << endl;

   return beta;
}


void BetaFinder::Execute()
{
   if (! m_ImageLoaded )
   {
      cout << "Please load an image first" << endl;
      return;
   }

   m_TotalEigenFilter->UpdateLargestPossibleRegion();

#if defined(INTERMEDIATE_OUTPUTS)
   m_EValueWriter->SetInput( m_EValueCastfilter1->GetOutput() );
   m_EValueWriter->SetFileName( "EigenValueImage1.vtk");
   m_EValueWriter->Update();
   m_EValueWriter->SetInput( m_EValueCastfilter2->GetOutput() );
   m_EValueWriter->SetFileName( "EigenValueImage2.vtk");
   m_EValueWriter->Update();
   m_EValueWriter->SetInput( m_EValueCastfilter3->GetOutput() );
   m_EValueWriter->SetFileName( "EigenValueImage3.vtk");
   m_EValueWriter->Update();
#endif

   BetaImageType::Pointer beta = this->extract_beta(
      m_inputImage,
       m_EValueCastfilter1->GetOutput(),
       m_EValueCastfilter2->GetOutput(),
       m_EValueCastfilter3->GetOutput(),
       5.0);
   typedef itk::ImageFileWriter< BetaImageType > BetaWriterType;
   BetaWriterType::Pointer betaWriter = BetaWriterType::New();
   ostringstream oss;
   oss << m_fileBasename << ".beta." << m_beta_ratio << ".vtk";
   betaWriter->SetFileName( oss.str().c_str() );
   betaWriter->SetInput(beta);
   betaWriter->Update();

   // m_MergedWriter->Update();

   // m_ResampleWriter->Update();

   // m_OverlayWriter->Update();
}
