#include "gabble.h"

#include <iostream>

using namespace std;

/************************************
 *
 *  Constructor
 *
 ***********************************/
ceExtractorConsoleBase::ceExtractorConsoleBase()
{
  m_ImageLoaded = false;

  m_Reader     = VolumeReaderType::New();

  //  || Gradient( Image ) ||
//  m_GradientMagnitude = GradientMagnitudeFilterType::New();
//  m_GradientMagnitude->SetInput( m_Reader->GetOutput() );

  m_Hessian = HessianFilterType::New();
  m_Hessian->SetInput( m_Reader->GetOutput() );

  ////////////////////////////////////////////////////////////////////
  // Eigen stuff
  ////////////////////////////////////////////////////////////////////

  // Compute eigenvalues.. order them in ascending order
  m_EigenFilter = EigenAnalysisFilterType::New();
  m_EigenFilter->SetDimension( HessianPixelType::Dimension );
  m_EigenFilter->SetInput( m_Hessian->GetOutput() );
  m_EigenFilter->OrderEigenValuesBy(
      EigenAnalysisFilterType::JgdCalculatorType::OrderByValue );

  // Create an adaptor and plug the output to the parametric space
  m_EigenAdaptor1 = ImageAdaptorType::New();
  EigenValueAccessor< EigenValueArrayType > accessor1;
  accessor1.SetEigenIdx( 0 );
  m_EigenAdaptor1->SetImage( m_EigenFilter->GetOutput() );
  m_EigenAdaptor1->SetPixelAccessor( accessor1 );

  m_EigenAdaptor2 = ImageAdaptorType::New();
  EigenValueAccessor< EigenValueArrayType > accessor2;
  accessor2.SetEigenIdx( 1 );
  m_EigenAdaptor2->SetImage( m_EigenFilter->GetOutput() );
  m_EigenAdaptor2->SetPixelAccessor( accessor2 );

  m_EigenAdaptor3 = ImageAdaptorType::New();
  EigenValueAccessor< EigenValueArrayType > accessor3;
  accessor3.SetEigenIdx( 2 );
  m_EigenAdaptor3->SetImage( m_EigenFilter->GetOutput() );
  m_EigenAdaptor3->SetPixelAccessor( accessor3 );

  // m_EigenCastfilter1 will give the eigenvalues with the maximum
  // eigenvalue. m_EigenCastfilter3 will give the eigenvalues with
  // the minimum eigenvalue.
  m_EigenCastfilter1 = CastImageFilterType::New();
  m_EigenCastfilter1->SetInput( m_EigenAdaptor3 );
  m_EigenCastfilter2 = CastImageFilterType::New();
  m_EigenCastfilter2->SetInput( m_EigenAdaptor2 );
  m_EigenCastfilter3 = CastImageFilterType::New();
  m_EigenCastfilter3->SetInput( m_EigenAdaptor1 );

  // I think this parametric stuff is just, about having to use 3-valued
  // points.

  m_ParametricSpace = ParametricSpaceFilterType::New();
  // maximum, to minimum eigenvalue

  // "The mesh contains one point for every pixel on the images. The
  // coordinate of the point being equal to the gray level of the
  // associated input pixels.  This class is intended to produce the
  // population of points that represent samples in a parametric space. In
  // this particular case the parameters are the gray levels of the input
  // images. The dimension of the mesh points should be equal to the number
  // of input images to this filter."
  // So, the grey level at each point in the 3 images, is one component of
  // a vector of parameters for that point.

  // 3 images -> a mesh
  m_ParametricSpace->SetInput( 0, m_EigenCastfilter1->GetOutput() );
  m_ParametricSpace->SetInput( 1, m_EigenCastfilter2->GetOutput() );
  m_ParametricSpace->SetInput( 2, m_EigenCastfilter3->GetOutput() );

BOOST_MPL_ASSERT((boost::is_same<ImageSpaceMeshType, MeshType>));


#if defined(NEW_STOFF)
  // Wait. We don't care /which direction/ the greatest eigenvalues are in,
  // somehow - we only want those pixels that are tube-like (in /some/
  // direction) and flat-like (in some direction). For these we don't
  // mind losing the orientations of the eigenvalues.

  // Make image

#endif



  // SphereSpatialFunction
  // function that returns 0 for points inside or on the surface of a sphere,
  // 1 for points outside the sphere
  // What the heck this is for, I couldn't say.
  // Maybe to trim out regions that are too small?
  // SpatialFunction here applies to a mesh,
  // and writes to a mesh
  m_SpatialFunctionFilter = SpatialFunctionFilterType::New();
  m_SpatialFunctionFilter->SetInput(
          m_ParametricSpace->GetOutput() );

//   m_SpatialFunctionControl = SpatialFunctionControlType::New();
//   m_SpatialFunctionControl->SetSpatialFunction(
//                   m_SpatialFunctionFilter->GetSpatialFunction() );

#ifdef FRUSTUM_FUNCTION
//   // We must choose samples in the parametric space
//   //  such that lambda_2 =~ lambda_3   and  lambda_1 =~ 0
//   m_SpatialFunctionControl->SetAngleZ( -45.0f );
//   m_SpatialFunctionControl->SetApertureAngleX( 14.0f );
//   m_SpatialFunctionControl->SetApertureAngleY(  19.0f );
//   m_SpatialFunctionControl->SetTopPlane( 53.0f );
//   m_SpatialFunctionControl->SetBottomPlane( 6.0f );
//   m_SpatialFunctionControl->SetApex( 0.0f, 0.0f, 0.0f );
//   m_SpatialFunctionControl->SetRotationPlane(
//           SpatialFunctionControlType::RotateInYZPlane );
#endif

#if defined(SPHERE_FUNCTION)
//   m_SpatialFunctionControl->SetRadius( 1.0f );
#endif

  // I think this conversion is needless (mmm, maybe).
  m_InverseParametricFilter = InverseParametricFilterType::New();
  m_InverseParametricFilter->SetInput(
          m_SpatialFunctionFilter->GetOutput() );

  // Here's where it comes back to image, after getting some spatial thing.
  // might learn how to translate the point set to image filter, from it.
  // Generate an image from the extracted points for an overlay display
  m_PointSetToImageFilter = PointSetToImageFilterType::New();
  m_PointSetToImageFilter->SetInput(
          m_InverseParametricFilter->GetOutput() );


m_MergedWriter = MergedWriterType::New();
m_MergedWriter->SetFileName( "Remerged.vtk");
m_MergedWriter->SetInput(m_PointSetToImageFilter->GetOutput());


  m_OverlayResampleFilter = OverlayImageResampleFilterType::New();
  m_OverlayResampleFilter->SetInput( m_PointSetToImageFilter->GetOutput() );

m_ResampleWriter = ResampleWriterType::New();
m_ResampleWriter->SetFileName( "Resampled.vtk");
m_ResampleWriter->SetInput(m_OverlayResampleFilter->GetOutput());

  // Output of the ThresholdImageFilter will be used as the
  // overlay image of extracted points
  m_ThresholdImageFilter     = ThresholdImageFilterType::New();
//  m_ThresholdImageFilter->SetLowerThreshold( 0.1 ); // crank it up a bit.
  m_ThresholdImageFilter->SetLowerThreshold( 0.0001 ); // Get all non-zero pixels
  m_ThresholdImageFilter->SetUpperThreshold( itk::NumericTraits<
          ThresholdImageFilterType::OutputPixelType >::max() );
  // The either/or chosen output values for those pixels in the input image
  // that fall between lower & upper.
  m_ThresholdImageFilter->SetOutsideValue( 0 );
  m_ThresholdImageFilter->SetInsideValue( 255 );
  m_ThresholdImageFilter->SetInput( m_OverlayResampleFilter->GetOutput() );

  m_OverlayWriter = OverlayWriterType::New();
  m_OverlayWriter->SetInput( m_ThresholdImageFilter->GetOutput() );
  m_OverlayWriter->SetFileName( "PostThreshold.vtk");

#if defined(INTERMEDIATE_OUTPUTS)
  m_EigenValueWriter = EigenValueWriterType::New();
#endif
}


/************************************
 *
 *  Destructor
 *
 ***********************************/
ceExtractorConsoleBase
::~ceExtractorConsoleBase()
{
}


/************************************
 *
 *  Load
 *
 ***********************************/
void ceExtractorConsoleBase::Load( const char * basename )
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


/************************************
 *
 *  Set Sigma
 *
 ***********************************/
void
ceExtractorConsoleBase
::SetSigma( RealType value )
{
//GradientMagnitudeRecursiveGaussianImageFilter
//  m_GradientMagnitude->SetSigma( value );
//HessianRecursiveGaussianImageFilter
  m_Hessian->SetSigma( value );
}


void
ceExtractorConsoleBase
::extract_alpha(MeshType::Pointer mesh)
{
    // Software Guide : BeginCodeSnippet
    typedef MeshType::CellType                      CellType;
    typedef MeshType::CellsContainer::ConstIterator CellIterator;
    typedef itk::VertexCell< CellType >             VertexType;

    CellIterator cellIterator = mesh->GetCells()->Begin();
    CellIterator cellEnd      = mesh->GetCells()->End();

    while( cellIterator != cellEnd )
      {
      CellType * cell = cellIterator.Value();
      switch( cell->GetType() )
        {
        case CellType::VERTEX_CELL:
          {
          std::cout << "VertexCell : " << std::endl;
          VertexType * line = dynamic_cast<VertexType *>( cell );
          std::cout << "dimension = " << line->GetDimension()      << std::endl;
          std::cout << "# points  = " << line->GetNumberOfPoints() << std::endl;
          break;
          }
        default:
          {
          assert(false);
          std::cout << "Cell with more than three points" << std::endl;
          std::cout << "dimension = " << cell->GetDimension()      << std::endl;
          std::cout << "# points  = " << cell->GetNumberOfPoints() << std::endl;
          break;
          }
        }
      ++cellIterator;
      }
}


ceExtractorConsoleBase::BetaImageType::Pointer
ceExtractorConsoleBase
::extract_beta(InputImageType::Pointer density,
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


/************************************
 *
 *  Execute
 *
 ***********************************/
void
ceExtractorConsoleBase
::Execute()
{
  if (! m_ImageLoaded )
  {
    cout << "Please load an image first" << endl;
    return;
  }

  m_EigenFilter->UpdateLargestPossibleRegion();

#if defined(INTERMEDIATE_OUTPUTS)
  m_EigenValueWriter->SetInput( m_EigenCastfilter1->GetOutput() );
  m_EigenValueWriter->SetFileName( "EigenValueImage1.vtk");
  m_EigenValueWriter->Update();
  m_EigenValueWriter->SetInput( m_EigenCastfilter2->GetOutput() );
  m_EigenValueWriter->SetFileName( "EigenValueImage2.vtk");
  m_EigenValueWriter->Update();
  m_EigenValueWriter->SetInput( m_EigenCastfilter3->GetOutput() );
  m_EigenValueWriter->SetFileName( "EigenValueImage3.vtk");
  m_EigenValueWriter->Update();
#endif

  BetaImageType::Pointer beta = extract_beta(m_inputImage,
                                             m_EigenCastfilter1->GetOutput(),
                                             m_EigenCastfilter2->GetOutput(),
                                             m_EigenCastfilter3->GetOutput(),
                                             5.0);
  typedef itk::ImageFileWriter< BetaImageType > BetaWriterType;
  BetaWriterType::Pointer betaWriter = BetaWriterType::New();
  ostringstream oss;
  oss << m_fileBasename << ".beta." << m_beta_ratio << ".vtk";
  betaWriter->SetFileName( oss.str().c_str() );
  betaWriter->SetInput(beta);
  betaWriter->Update();

  m_MergedWriter->Update();

  m_ResampleWriter->Update();

  m_OverlayWriter->Update();
}

#if 0
// Peaks, of /what/ remains to be determined.
void find_local_peaks(ImageType::Pointer image, Ties& out_seeds)
{
    NitType::RadiusType radius;
    radius.Fill(1);
    NitType nit(radius, image, image->GetLargestPossibleRegion());

    unsigned long nnbrs = nit.Size();

    nit.GoToBegin();
    unsigned nsingletonwinners = 0;
    for (; !nit.IsAtEnd(); ++nit) {
        PixelType ctr = nit.GetCenterPixel();
        unsigned idxCtr = nit.Size() / 2;
        if (ctr == 0.0)
            // not a seed; ignore it
            continue;

        for (unsigned i = 0; i < nnbrs; ++i) {
            if (i != idxCtr) {
                PixelType val = nit.GetPixel(i);
                if (ctr <= val) {
                    // Not a local maximum
                    goto loop;
                }
            }
        }

        {
        ++nsingletonwinners;
        PixelSet* winner = new PixelSet;
        winner->insert(nit.GetIndex(nit.Size() / 2));
        out_seeds.insert(winner);
        }

    loop:
    }
    cout << nsingletonwinners << " naive winners" << endl;
}
#endif // 0
