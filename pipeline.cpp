#include "pipeline.h"
#include "geometry.h"

#include <iostream>


using namespace std;


// /big/common/software/insight/InsightToolkit-3.4.0/Testing/Code/Common/itkTranslationTransformTest.cxx

BetaPipeline::BetaPipeline(ImageType::Pointer image,
                           PointType const& center,
                           // In cells. no point in fractional cells (I believe)
                           int region_width)
{
cout << "input: " << endl;
image->Print(cout, 2);

   ImageType::IndexType index;
   bool isWithin = image->TransformPhysicalPointToIndex(center, index);
   assert(isWithin);

   ImageRegion region;
   region.SetIndex(index);
   // don't know what units these are, physical or 'cell'.
   // The '+ 1' to compensate for the phys -> index truncation
   // And another '+ 1' to deal with our fractional translation
   region.PadByRadius(region_width + 1 + 1);

   // It's a filter, so, we want to set requested region on the input image,
   // and then a 'shift' transform of a fraction of a cell, right?
   // output- to- input, and in physical coordinates (itkResampleImageFilter.h)

  /** Set the coordinate transformation.
   * Set the coordinate transform to use for resampling.  Note that this must
   * be in physical coordinates and it is the output-to-input transform, NOT
   * the input-to-output transform that you might naively expect.  By default
   * the filter uses an Identity transform. You must provide a different
   * transform here, before attempting to run the filter, if you do not want to
   * use the default Identity transform. */
   VectorType offset;

   transform_shift(center, image->GetSpacing(), offset);

cout << "center(phys): " << center
     << "; spacing(phys): " << image->GetSpacing()
     << "; offset: " << offset << endl;

   TranslationTransform::Pointer translation_ = TranslationTransform::New();
   translation_->SetOffset(offset);

   // /big/common/software/insight/InsightToolkit-3.4.0/Examples/Filtering/ResampleImageFilter.cxx is most helpful
   // Resample, to shift the image to (slightly) new coordinates
   resampler_ = ResampleFilterType::New();
//   resampler_.SetInterpolator();
   resampler_->SetTransform(translation_);
   resampler_->SetInput(image);
   resampler_->SetDefaultPixelValue(0.0);

cout << "resampler (before update): " << endl;
resampler_->GetOutput()->Print(cout, 2);

resampler_->Update();
cout << "resampler (after update): " << endl;
resampler_->GetOutput()->Print(cout, 2);


   // file:///big/common/software/insight/install/html/classitk_1_1HessianRecursiveGaussianImageFilter.html

   hessian_ = HessianFilterType::New();
   // 1.0 is default sigma value. Units are image's physical units
   // (see itkGaussianDerivativeImageFunction, which all the rest use).
   hessian_->SetSigma(1.0);
   hessian_->SetInput(resampler_->GetOutput());

   // file:///big/common/software/insight/install/html/classitk_1_1SymmetricEigenAnalysisImageFilter.html
   // Compute eigenvalues.. order them in descending order
   totalEigenFilter_ = EigenAnalysisFilterType::New();
   totalEigenFilter_->SetDimension( Dimension );
   totalEigenFilter_->SetInput( hessian_->GetOutput() );
   totalEigenFilter_->OrderEigenValuesBy(
      EigenAnalysisFilterType::JgdCalculatorType::OrderByValue);

#ifdef DE_NADA
   // Eigenvalue
   // Create an adaptor and plug the output to the parametric space
   eValueAdaptor1_ = EValueImageAdaptorType::New();
   valAccessor1_.SetEigenIdx( 0 );
   eValueAdaptor1_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
   eValueAdaptor1_->SetPixelAccessor( valAccessor1_ );

   eValueAdaptor2_ = EValueImageAdaptorType::New();
   valAccessor2_.SetEigenIdx( 1 );
   eValueAdaptor2_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
   eValueAdaptor2_->SetPixelAccessor( valAccessor2_ );

   eValueAdaptor3_ = EValueImageAdaptorType::New();
   valAccessor3_.SetEigenIdx( 2 );
   eValueAdaptor3_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
   eValueAdaptor3_->SetPixelAccessor( valAccessor3_ );


   // Eigenvector
   // Create an adaptor and plug the output to the parametric space
   eVectorAdaptor1_ = EVectorImageAdaptorType::New();
   vecAccessor1_.SetEigenIdx( 0 );
   eVectorAdaptor1_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
   eVectorAdaptor1_->SetPixelAccessor( vecAccessor1_ );

   eVectorAdaptor2_ = EVectorImageAdaptorType::New();
   vecAccessor2_.SetEigenIdx( 1 );
   eVectorAdaptor2_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
   eVectorAdaptor2_->SetPixelAccessor( vecAccessor2_ );

   eVectorAdaptor3_ = EVectorImageAdaptorType::New();
   vecAccessor3_.SetEigenIdx( 2 );
   eVectorAdaptor3_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
   eVectorAdaptor3_->SetPixelAccessor( vecAccessor3_ );

   // eValueCastfilter1 will give the eigenvalues with the maximum
   // eigenvalue. eValueCastfilter3 will give the eigenvalues with
   // the minimum eigenvalue.
   eValueCastFilter1_ = EValueCastImageFilterType::New();
   eValueCastFilter1_->SetInput( eValueAdaptor1_ );
   eValueCastFilter2_ = EValueCastImageFilterType::New();
   eValueCastFilter2_->SetInput( eValueAdaptor2_ );
   eValueCastFilter3_ = EValueCastImageFilterType::New();
   eValueCastFilter3_->SetInput( eValueAdaptor3_ );

   // Shoot shoot shoot - I want the matching eigenvector with each value;
   // have to figure out how to keep track of that.
   // - heh - I think it's ok as-is!

   eVectorCastFilter1_ = EVectorCastImageFilterType::New();
   eVectorCastFilter1_->SetInput( eVectorAdaptor1_ );
   eVectorCastFilter2_ = EVectorCastImageFilterType::New();
   eVectorCastFilter2_->SetInput( eVectorAdaptor2_ );
   eVectorCastFilter3_ = EVectorCastImageFilterType::New();
   eVectorCastFilter3_->SetInput( eVectorAdaptor3_ );
#endif // DE_NADA

   // -- We may not need these cast + adaptor + accessor things
   // (maybe some). They're for the benefit of other filters, I think.

// /big/common/insight/InsightToolkit-3.4.0/Testing/Code/Common/itkSymmetricEigenAnalysisTest.cxx

// We get our vector as operator[]
}

#ifdef RAW_MATERIAL
  {
  // Test using itk Matrix
  std::cout << "Testing ComputeEigenValuesAndVectors() "
    << "with SymmetricEigenAnalysis< itk::Matrix, itk::FixedArray, itk::Matrix >"
    << std::endl;
  typedef itk::Matrix< double, 6, 6 > InputMatrixType;
  typedef itk::FixedArray< double, 6 > EigenValuesArrayType;
  typedef itk::Matrix< double, 6, 6 > EigenVectorMatrixType;
  typedef itk::SymmetricEigenAnalysis< InputMatrixType,
      EigenValuesArrayType, EigenVectorMatrixType > SymmetricEigenAnalysisType;

  double Sdata[36] = {
   30.0000,   -3.4273,   13.9254,   13.7049,   -2.4446,   20.2380,
   -3.4273,   13.7049,   -2.4446,    1.3659,    3.6702,   -0.2282,
   13.9254,   -2.4446,   20.2380,    3.6702,   -0.2282,   28.6779,
   13.7049,    1.3659,    3.6702,   12.5273,   -1.6045,    3.9419,
   -2.4446,    3.6702,   -0.2282,   -1.6045,    3.9419,    2.5821,
   20.2380,   -0.2282,   28.6779,    3.9419,    2.5821,   44.0636,
  };

  InputMatrixType S;

  for(unsigned int row=0; row<6; row++)
    {
    for(unsigned int col=0; col<6; col++)
      {
      S[row][col] = Sdata[ row * 6 + col ];
      }
    }

  EigenValuesArrayType eigenvalues;
  EigenVectorMatrixType eigenvectors;
  SymmetricEigenAnalysisType symmetricEigenSystem(6);

  symmetricEigenSystem.ComputeEigenValuesAndVectors(S, eigenvalues, eigenvectors );

  std::cout << "EigenValues: " << eigenvalues << std::endl;
  std::cout << "EigenVectors (each row is an an eigen vector): " << std::endl;
  std::cout << eigenvectors << std::endl;

  double eigvec3[6] = { 0.5236407,  -0.0013422,  -0.4199706,  -0.5942299,   0.4381326,   0.0659837 };
  double eigvals[6]= {0.170864, 2.16934, 3.79272, 15.435, 24.6083, 78.2994};

  double tolerance = 0.01;
  for( unsigned int i=0; i<6; i++ )
    {
    if (vnl_math_abs( eigvals[i] - eigenvalues[i] ) > tolerance)
      {
      std::cout << "Eigen value computation failed" << std::endl;
      return EXIT_FAILURE;
      }

     if (vnl_math_abs( eigvec3[i] - eigenvectors[2][i] ) > tolerance)
      {
      std::cout << "Eigen vector computation failed" << std::endl;
      return EXIT_FAILURE;
      }
    }
  }

#endif // RAW_MATERIAL
