#include "pipeline.h"


// /big/common/software/insight/InsightToolkit-3.4.0/Testing/Code/Common/itkTranslationTransformTest.cxx
ImageType::ConstPointer
   resample_image(XForm xform, ImageType::ConstPointer image)
{
   typedef itk::TranslationTransform< PixelType, Dimensions > TransformType;

   TransformType::Pointer id3 = TransformType::New();
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
//   resampler.Update();

   ImageType::ConstPointer outImage = resampler.GetOutput();
   return outImage;
}


BetaPipeline::BetaPipeline(ImageType::Pointer image,
                           PointPos const& shift)
{
   typedef itk::Vector<double, Dimension> VectorType;

   VectorType offset;
   offset[0] = shift.offset[0];
   offset[1] = shift.offset[1];
   offset[2] = shift.offset[2];

   translation_ = TransformType::New();
   translation_->SetOffset(offset);

   // Resample, to shift the image to new coordinates
   resampler_ = ResampleFilterType::New();
//   resampler_.SetInterpolator();
   resampler_->SetTransform(translation_);
   resampler_->SetInput(image);


   hessian_ = HessianFilterType::New();
   hessian_->SetInput(resampler_->GetOutput());

   // Compute eigenvalues.. order them in ascending order
   totalEigenFilter_ = EigenAnalysisFilterType::New();
   totalEigenFilter_->SetDimension( Dimension );
   totalEigenFilter_->SetInput( hessian_->GetOutput() );
   totalEigenFilter_->OrderEigenValuesBy(
      EigenAnalysisFilterType::JgdCalculatorType::OrderByValue);

   // Eigenvalue
   // Create an adaptor and plug the output to the parametric space
   eValueAdaptor1_ = EValueImageAdaptorType::New();
   accessor1_.SetEigenIdx( 0 );
   eValueAdaptor1_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
   eValueAdaptor1_->SetPixelAccessor( accessor1_ );

   eValueAdaptor2_ = EValueImageAdaptorType::New();
   accessor2_.SetEigenIdx( 1 );
   eValueAdaptor2_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
   eValueAdaptor2_->SetPixelAccessor( accessor2_ );

   eValueAdaptor3_ = EValueImageAdaptorType::New();
   accessor3_.SetEigenIdx( 2 );
   eValueAdaptor3_->SetImage( totalEigenFilter_->GetEigenValuesImage() );
   eValueAdaptor3_->SetPixelAccessor( accessor3_ );


   // Eigenvector
   // Create an adaptor and plug the output to the parametric space
   eVectorAdaptor1_ = EVectorImageAdaptorType::New();
   accessor1_.SetEigenIdx( 0 );
   eVectorAdaptor1_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
   eVectorAdaptor1_->SetPixelAccessor( vecAccessor1_ );

   eVectorAdaptor2_ = EVectorImageAdaptorType::New();
   accessor2_.SetEigenIdx( 1 );
   eVectorAdaptor2_->SetImage( totalEigenFilter_->GetEigenVectorsImage() );
   eVectorAdaptor2_->SetPixelAccessor( vecAccessor2_ );

   eVectorAdaptor3_ = EVectorImageAdaptorType::New();
   accessor3_.SetEigenIdx( 2 );
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
}
