#if RAW_MATERIAL
// Obsolete - use <Pipeline> object
void Pipeline::SetUpPipeline(xform, Point point, ImageType::ConstPointer image)
{
   image->SetRequestedRegion(something smallish around the point);

   typedef itk::TranslationTransform< PixelType, Dimensions > TransformType;

   TransformType::Pointer  id3 = TransformType::New();
   VectorType                   offset;

   // Create and show a simple 2D transform from given parameters
   offset[0] = xform.xoff;
   offset[1] = xform.yoff;
   offset[2] = xform.zoff;
   TransformType::Pointer aff2 = TransformType::New();
   aff2->SetOffset(offset);

   ResampleFilterType resampler;
   resampler.SetTransform(transform);

   resampler.SetInput(image);

   HessianFilterType::ConstPointer hessian = HessianFilterType::New();
   hessian->SetInput(resampler.GetOutput());

   // Compute eigenvalues.. order them in ascending order
   EigenAnalysisFilterType::ConstPointer totalEigenFilter = EigenAnalysisFilterType::New();
   totalEigenFilter->SetDimension( HessianPixelType::Dimension );
   totalEigenFilter->SetInput( hessian->GetOutput() );
   totalEigenFilter->OrderEigenValuesBy(
      EigenAnalysisFilterType::JgdCalculatorType::OrderByValue);

   // Eigenvalue
   // Create an adaptor and plug the output to the parametric space
   EValueImageAdaptorType::ConstPointer eValueAdaptor1 = EValueImageAdaptorType::New();
   EigenvalueAccessor< EigenValueArrayType > accessor1;
   accessor1.SetEigenIdx( 0 );
   eValueAdaptor1->SetImage( totalEigenFilter->GetEigenValuesImage() );
   eValueAdaptor1->SetPixelAccessor( accessor1 );

   EValueImageAdaptorType::ConstPointer eValueAdaptor2 = EValueImageAdaptorType::New();
   EigenvalueAccessor< EigenValueArrayType > accessor2;
   accessor2.SetEigenIdx( 1 );
   eValueAdaptor2->SetImage( totalEigenFilter->GetEigenValuesImage() );
   eValueAdaptor2->SetPixelAccessor( accessor2 );

   EValueImageAdaptorType::ConstPointer eValueAdaptor3 = EValueImageAdaptorType::New();
   EigenvalueAccessor< EigenValueArrayType > accessor3;
   accessor3.SetEigenIdx( 2 );
   eValueAdaptor3->SetImage( totalEigenFilter->GetEigenValuesImage() );
   eValueAdaptor3->SetPixelAccessor( accessor3 );


   // Eigenvector
   // Create an adaptor and plug the output to the parametric space
   EVectorImageAdaptorType::ConstPointer eVectorAdaptor1 = EVectorImageAdaptorType::New();
   EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor1;
   accessor1.SetEigenIdx( 0 );
   eVectorAdaptor1->SetImage( totalEigenFilter->GetEigenVectorsImage() );
   eVectorAdaptor1->SetPixelAccessor( vecAccessor1 );

   EVectorImageAdaptorType::ConstPointer eVectorAdaptor2 = EVectorImageAdaptorType::New();
   EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor2;
   accessor2.SetEigenIdx( 1 );
   eVectorAdaptor2->SetImage( totalEigenFilter->GetEigenVectorsImage() );
   eVectorAdaptor2->SetPixelAccessor( vecAccessor2 );

   EVectorImageAdaptorType::ConstPointer eVectorAdaptor3 = EVectorImageAdaptorType::New();
   EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor3;
   accessor3.SetEigenIdx( 2 );
   eVectorAdaptor3->SetImage( totalEigenFilter->GetEigenVectorsImage() );
   eVectorAdaptor3->SetPixelAccessor( vecAccessor3 );

   // eValueCastfilter1 will give the eigenvalues with the maximum
   // eigenvalue. eValueCastfilter3 will give the eigenvalues with
   // the minimum eigenvalue.
   EValueCastImageFilterType::ConstPointer eValueCastFilter1 = EValueCastImageFilterType::New();
   eValueCastFilter1->SetInput( EValueAdaptor3 );
   EValueCastImageFilterType::ConstPointer eValueCastFilter2 = EValueCastImageFilterType::New();
   eValueCastFilter2->SetInput( EValueAdaptor2 );
   EValueCastImageFilterType::ConstPointer eValueCastFilter3 = EValueCastImageFilterType::New();
   eValueCastFilter3->SetInput( EValueAdaptor1 );

   // Shoot shoot shoot - I want the matching eigenvector with each value;
   // have to figure out how to keep track of that.
   // - heh - I think it's ok as-is!

   EVectorCastImageFilterType::ConstPointer eVectorCastFilter1 = EVectorCastImageFilterType::New();
   eVectorCastFilter1->SetInput( eVectorAdaptor3 );
   EVectorCastImageFilterType::ConstPointer eVectorCastFilter2 = EVectorCastImageFilterType::New();
   eVectorCastFilter2->SetInput( eVectorAdaptor2 );
   EVectorCastImageFilterType::ConstPointer eVectorCastFilter3 = EVectorCastImageFilterType::New();
   eVectorCastFilter3->SetInput( eVectorAdaptor1 );
}
#endif // RAW_MATERIAL


#if 0
// BetaPipeline::Process(ImageType::ConstPointer image)
// {
//    BetaPipeline(ImageType::ConstPointer image);

//    itk::ResampleImageFilter< ImageType, ImageType, double >

//    typedef itk::TranslationTransform< PixelType, Dimensions > TransformType;

//    TransformType::Pointer  id3_ = TransformType::New();
//    VectorType                   offset_;

//    ResampleFilterType resampler_;

//       EigenAnalysisFilterType::JgdCalculatorType::OrderByValue);

//    // Eigenvalue
//    // Create an adaptor and plug the output to the parametric space
//    EValueImageAdaptorType::ConstPointer eValueAdaptor1_ = EValueImageAdaptorType::New();
//    EigenvalueAccessor< EigenValueArrayType > accessor1_;
//    accessor1_.SetEigenIdx( 0 );
//    eValueAdaptor1_->SetImage( totalEigenFilter->GetEigenValuesImage() );
//    eValueAdaptor1_->SetPixelAccessor( accessor1_ );

//    EValueImageAdaptorType::ConstPointer eValueAdaptor2_ = EValueImageAdaptorType::New();
//    EigenvalueAccessor< EigenValueArrayType > accessor2_;
//    accessor2_.SetEigenIdx( 1 );
//    eValueAdaptor2_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
//    eValueAdaptor2_->SetPixelAccessor( accessor2_ );

//    EValueImageAdaptorType::ConstPointer eValueAdaptor3_ = EValueImageAdaptorType::New();
//    EigenvalueAccessor< EigenValueArrayType > accessor3_;
//    accessor3_.SetEigenIdx( 2 );
//    eValueAdaptor3_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
//    eValueAdaptor3_->SetPixelAccessor( accessor3_ );


//    // Eigenvector
//    // Create an adaptor and plug the output to the parametric space
//    EVectorImageAdaptorType::ConstPointer eVectorAdaptor1_ = EVectorImageAdaptorType::New();
//    EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor1_;
//    accessor1_.SetEigenIdx( 0 );
//    eVectorAdaptor1_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
//    eVectorAdaptor1_->SetPixelAccessor( vecAccessor1_ );

//    EVectorImageAdaptorType::ConstPointer eVectorAdaptor2_ = EVectorImageAdaptorType::New();
//    EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor2;
//    accessor2_.SetEigenIdx( 1 );
//    eVectorAdaptor2_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
//    eVectorAdaptor2_->SetPixelAccessor( vecAccessor2_ );

//    EVectorImageAdaptorType::ConstPointer eVectorAdaptor3_ = EVectorImageAdaptorType::New();
//    EigenvectorAccessor< EVectorMatrixType, EVector > vecAccessor3_;
//    accessor3_.SetEigenIdx( 2 );
//    eVectorAdaptor3_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
//    eVectorAdaptor3_->SetPixelAccessor( vecAccessor3_ );

//    // eValueCastfilter1 will give the eigenvalues with the maximum
//    // eigenvalue. eValueCastfilter3 will give the eigenvalues with
//    // the minimum eigenvalue.
//    EValueCastImageFilterType::ConstPointer eValueCastFilter1_ = EValueCastImageFilterType::New();
//    eValueCastFilter1_->SetInput( EValueAdaptor3_ );
//    EValueCastImageFilterType::ConstPointer eValueCastFilter2_ = EValueCastImageFilterType::New();
//    eValueCastFilter2_->SetInput( EValueAdaptor2_ );
//    EValueCastImageFilterType::ConstPointer eValueCastFilter3_ = EValueCastImageFilterType::New();
//    eValueCastFilter3_->SetInput( EValueAdaptor1_ );

//    // Shoot shoot shoot - I want the matching eigenvector with each value;
//    // have to figure out how to keep track of that.
//    // - heh - I think it's ok as-is!

//    EVectorCastImageFilterType::ConstPointer eVectorCastFilter1_ = EVectorCastImageFilterType::New();
//    eVectorCastFilter1_->SetInput( eVectorAdaptor1_ );
//    EVectorCastImageFilterType::ConstPointer eVectorCastFilter2_ = EVectorCastImageFilterType::New();
//    eVectorCastFilter2_->SetInput( eVectorAdaptor2_ );
//    EVectorCastImageFilterType::ConstPointer eVectorCastFilter3_ = EVectorCastImageFilterType::New();
//    eVectorCastFilter3_->SetInput( eVectorAdaptor3_ );
// }
#endif
