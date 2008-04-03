#include "BetaNode.h"

static PointSet BetaNode::s_all_beta_points;

bool PointIsBeta(PointPos )
{
   bool isBeta = false;

   if (bIsFarEnoughAway) {
      eigenoutput;
      {
      // Now test whether we're Beta enough.
      Pipeline pipeline = Pipeline(m_source);

      eigenoutput = pipeline.getresult();
      }

      if (not QualifiesAsBeta(all our stuff, us)) {
         return false;
      }

      else {
         find intersection points;

         make polygon;

         push children;
         push self into frontier;

         // for (each point) {
         //    if (point.Explore()) {
         //       children.push_back(
         //    }
         // }
      }

   }
}

void FindBetaNodes(seeds, ImageType::ConstPointer image)
{

   for_exploration.push(seeds);

   while (not for_exploration.empty()) {
      node = for_exploration.top();
      for_exploration.pop();

      if (IsBetaLike() and point is far enough away) {
         // we don't need the machinery anymore, the point is enough
         node = new BetaNode();
         for_exploration.push(node.children());
      }
   }
}


// We need a G(?) or R(?) tree
bool BetaNode::IsFarEnoughAwayFromOthers()
{
   for (each point) {
      if (this is within small dist from other point) {
         return false;
      }
   }
   return true;
}


bool IsBetaLike(point, ImageType::ConstPointer image)
{
   Pipeline pipeline = SetUpPipeline(image);
   eigenstuff = pipeline.getOutput();

   bool isBeta = MeetsBetaCondition();
}


// Used to classify the seeds
bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3)
{
   bool isBeta = sheetMin <= t1 and t1 <= sheetMax and
      max(t1 / t2, t1 / t3) < min(t2 / t3, t3 / t2);

   return isBeta;
}


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
