#include "pipeline.h"


// /big/common/software/insight/InsightToolkit-3.4.0/Testing/Code/Common/itkTranslationTransformTest.cxx

// ImageType::ConstPointer
//    resample_image(Point const& offset_from_original, ImageType::ConstPointer image)
// {
//    typedef itk::TranslationTransform< PixelType, Dimension > TransformType;

//    TransformType::Pointer id3 = TransformType::New();
//    VectorType                   offset;

//    // Create and show a simple 2D transform from given parameters
//    offset[0] = offset_from_original[0];
//    offset[1] = offset_from_original[1];
//    offset[2] = offset_from_original[2];
//    TransformType::Pointer aff2 = TransformType::New();
//    aff2->SetOffset(offset);

//    ResampleFilterType::Pointer resampler = new ResampleFilterType();
//    resampler->SetTransform(transform);

//    resampler->SetInput(image);
// //   resampler.Update();

//    ImageType::ConstPointer outImage = resampler->GetOutput();
//    return outImage;
// }


BetaPipeline::BetaPipeline(ImageType::Pointer image,
                           PointPos const& center,
                           // In cells. no point in fractional cells (I believe)
                           int region_width)
{
   ImageType::IndexType index;
   bool isWithin = image->
      TransformPhysicalPointToIndex(center.absolute_position, index);
   assert(isWithin);
   ImageRegion region;
   region.SetIndex(index);
   // don't know what units these are, physical or 'cell'.
   // The '+ 1' to compensate for the phys -> index truncation
   // And another '+ 1' to deal with our fractional translation
   region.PadByRadius(region_width + 1 + 1);

   typedef itk::Vector<double, Dimension> VectorType;

   // It's a filter, so, we want to set requested region on the input image,
   // and then a 'shift' transform of a fraction of a cell, right?
   // output-to-input, and in physical coordinates (itkResampleImageFilter.h -
   // stupid documentation)
   VectorType offset;
   offset[0] = center.fractional_offset[0];
   offset[1] = center.fractional_offset[1];
   offset[2] = center.fractional_offset[2];

   TranslationTransform::Pointer translation_;
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

BetaPipeline::~BetaPipeline()
{
#warning "Need to delete pipeline filters"
}
