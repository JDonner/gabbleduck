#include "gabble.h"

#include <iostream>

using namespace std;


// Hook the pipelines up
BetaFinder::BetaFinder()
{
   m_ImageLoaded = false;

   m_Reader      = VolumeReaderType::New();

   m_Hessian = HessianFilterType::New();
   m_Hessian->SetInput( m_Reader->GetOutput() );

   BetaFinder::HookUpEigenStuff();

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
   EigenvalueAccessor< EigenValueArrayType > accessor1;
   accessor1.SetEigenIdx( 0 );
   m_EValueAdaptor1 = EValueImageAdaptorType::New();
   m_EValueAdaptor1->SetImage( m_TotalEigenFilter->GetEigenValuesImage() );
   m_EValueAdaptor1->SetPixelAccessor( accessor1 );

   EigenvalueAccessor< EigenValueArrayType > accessor2;
   accessor2.SetEigenIdx( 1 );
   m_EValueAdaptor2 = EValueImageAdaptorType::New();
   m_EValueAdaptor2->SetImage( m_TotalEigenFilter->GetEigenValuesImage() );
   m_EValueAdaptor2->SetPixelAccessor( accessor2 );

   EigenvalueAccessor< EigenValueArrayType > accessor3;
   accessor3.SetEigenIdx( 2 );
   m_EValueAdaptor3 = EValueImageAdaptorType::New();
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


// betaimage == inputimage, ie scalar
// Finds spots of supposed flatness, by eigenvalue
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

   BetaImageType::Pointer betaImage = dupper->GetOutput();
   //    output_image->Print(cout);
   betaImage->FillBuffer(BetaPixelType(0));

   // Do I need to initialize this?

   typedef itk::ImageRegionConstIterator< InputImageType > InputConstIterType;

   typedef itk::ImageRegionConstIterator< EachEigenValueImageType > EigenConstIteratorType;
   EigenConstIteratorType itE0( e0, e0->GetRequestedRegion() );
   EigenConstIteratorType itE1( e1, e1->GetRequestedRegion() );
   EigenConstIteratorType itE2( e2, e2->GetRequestedRegion() );

   InputConstIterType itDensity ( density, density->GetRequestedRegion() );

   typedef itk::ImageRegionIterator< BetaImageType > BetaOutIteratorType;
   BetaOutIteratorType itBeta( betaImage, betaImage->GetRequestedRegion() );


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

      // Where's this from?
      if (5.0 <= v0 / (v1 + 1.0e-20) && 0.8 <= flatness) {
         itBeta.Set(flatness);
         ++num_beta_pixels;
         if (num_beta_pixels < 100) {
            cout << "beta pixel " << flatness << "; v0: " << v0 << "; v1: " << v1 << endl;
         }
      }
      if (max_flatness < flatness) {
         max_flatness = flatness;
      }
   }

   BetaPixelType avg_v0 = sum_v0 / num_pixels, avg_v1 = sum_v1 / num_pixels;
   cout << "avg v0: " << avg_v0 << "avg v1: " << avg_v1 << endl
        << "min v0: " << min_v0 << "min v1: " << min_v1 << endl
        << "max v0: " << max_v0 << "max v1: " << max_v1 << endl
        << "max flatness: " << max_flatness << "; " << num_beta_pixels
        << endl;

   return betaImage;
}


void BetaFinder::Execute()
{
   if (not m_ImageLoaded)
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
}
