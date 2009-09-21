#include "node.h"
#include "pipeline.h"
#include "geometry.h"
#include "point.h"
#include "tracing.h"
#include "settings.h"
#include "instrument.h"
#include "spatial-hash.h"
// for image snapshotting
#include "gaussian.h"
#include "main.h"

#include <vector>
#include <queue>

using namespace std;


void CalcEigenStuff(ImageType::Pointer fullImage, PointType const& physPt,
                    EigenValuesType& outEVals, EigenVectorsType& outEVecs);
bool MeetsBetaCondition(PointType const& physPt,
                        EigenValuesType const& evals,
                        EigenVectorsType const& evec,
                        ImageType::Pointer image,
                        double sheetMin, double sheetMax);

static SpatialHash s_hash;

// Arg, I think this wants to be an object
// non-recursive version
void FindBetaNodes(ImageType::Pointer image,
                   Seeds const& seeds,
                   Nodes& outNodes)
{
   init_gaussian_mask(constants::GaussianSupportSize,
                      constants::SigmaOfFeatureGaussian);

   assert(g_GaussianMask != 0);

   ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
   ImageType::SpacingType spacing = image->GetSpacing();

   unsigned nPixels[Dimension];
   double physSpacing[Dimension];
   ImageType::PointType image_origin = image->GetOrigin();
   // ImageType::SizeType's indexes are fastest-moving first, in ours,
   // they're last. So, we reverse them.
   for (unsigned i = 0; i < Dimension; ++i) {
      nPixels[i] = size[Dimension - 1 - i];
      // I assume 'fastest-moving' == 0 here, too...
      physSpacing[i] = spacing[Dimension - 1 - i];
   }

   s_hash.init(image_origin, constants::RequiredNewPointSeparation, nPixels, physSpacing);

   typedef queue<PointType> PointQueue;
   PointQueue possible_beta_points;

//cout << "image origin: " << image->GetOrigin() << endl;

   // Load possible betas
   for (Seeds::const_iterator it = seeds.begin(), end = seeds.end();
        it != end; ++it) {
      PointType physPt;

      ImageIndexType seed = *it;
      // &&& Could be stricter; none of the seeds should be 0,
      // ie around the edge
      assert(not (seed[0] == 0 or seed[1] == 0 or seed[2] == 0));
      // &&& - we transform idx seed to physPt, and later we transform
      // physPt back to an idx
      image->TransformIndexToPhysicalPoint(seed, physPt);
      possible_beta_points.push(physPt);
   }

   while (not possible_beta_points.empty()) {
      ++n_total_visited;
      PointType physPt = possible_beta_points.front();
      possible_beta_points.pop();

      bool bTooNearAnotherPoint = s_hash.isWithinDistanceOfAnything(
         physPt, constants::RequiredNewPointSeparation);

      if (bTooNearAnotherPoint) {
         ++n_rejected_as_too_close;
      }
      else {
         ++n_far_enough_away;

         EigenValuesType evals;
         EigenVectorsType evecs;

         bool bEigenAnalysisSucceeded = true;
         try {
            CalcEigenStuff(image, physPt, evals, evecs);
         }
         catch (FailedEigenAnalysis& e) {
            // Nothing; just skip the calculations
            bEigenAnalysisSucceeded = false;
         }

         if (bEigenAnalysisSucceeded and
             MeetsBetaCondition(physPt, evals, evecs, image,
                                constants::BetaMin, constants::BetaMax)) {
            ++n_beta_nodes;
            s_hash.addPt(physPt);

            // we don't need the machinery anymore, the point is enough
            PointType loCell, hiCell;

            // point is at center of cell
            for (unsigned i = 0; i < Dimension; ++i) {
               // Inefficient but looks cleaner
               ImageType::SpacingType spacing = image->GetSpacing();
               loCell[i] = physPt[i] - spacing[i] / 2.0;
               hiCell[i] = physPt[i] + spacing[i] / 2.0;
            }

            // dominant eigenvector
            VectorType normal = evecs[2];

            Points intersections;
            planes_intersection_with_box(normal, physPt,
                                         loCell, hiCell,
                                         intersections);

            TriangleBunch triangles;
            MakeTriangles(normal, intersections, triangles);

            for (Points::const_iterator it = intersections.begin(),
                    end = intersections.end();
                 it != end; ++it) {
               possible_beta_points.push(*it);
            }
            outNodes.push_back(new Node(physPt, triangles));

maybe_snap_image(n_beta_nodes, outNodes);
         }
      }
   }
}

void CalcEigenStuff(ImageType::Pointer fullImage, PointType const& physPt,
                    EigenValuesType& outEVals, EigenVectorsType& outEVecs)
{
   // &&& <width> needs to be enough to 'support' the sigma of the gaussian
   unsigned width = 0;

   // &&& Ultimately this could be a single, fixed, static item, with
   // a bit of resetting for speed.
   // Pipeline does resampling
   BetaPipeline pipeline(fullImage, physPt, width);

   outEVals = pipeline.eigenValues();
   outEVecs = pipeline.eigenVectors();
}

// Seeds are in pixels, no sense in anything else.
// We want to resample (for a new image) against the original image,
// for fidelity's sake. The shift is in physical coords.
// This offset is: pt loc (in whichever coords) fmod(pt.X, imagespace.X)
// Intersections will be in whatever coords we start with.
// (Can we keep from accumulating error?)
// We can't, as long as we calculate via a long chain of moves, which
// we must. We can resample wrt to the original image which is good, but
// location, we can't avoid multiple offsets.


// initial, and drop-off - doesn't ITK have such a thing already?
double length_of_density(InterpolatorType::Pointer interpolator,
                         PointType const& initial_pt,
                         double initial_density,
                         VectorType const& direction,
                         double max_dist,
                         double increment)
{
   double density;
   VectorType vec = direction;

if (initial_density < constants::SeedDensityThreshold) {
   // Shouldn't happen at this point in the processing
   cout << "low density? " << initial_density << " < " << constants::SeedDensityThreshold
        << " shouldn't happen, how did it become a seed?" << '\n';
++n_lo_density_centers;
return 0.0;
}
else {
//cout << "ok density " << initial_density << endl;
++n_ok_density_centers;
}

   // forward
   double fwd_length = 0.0;
   density = initial_density;
   for (int times = 1; initial_density * constants::SeedDensityFalloff <= density; ++times) {
      PointType test_pt = initial_pt + direction * times * increment;
      density = interpolator->Evaluate(test_pt);
      fwd_length = times * increment;
   }

   // backward
   double bkwd_length = 0.0;
   density = initial_density;
   for (int times = -1; initial_density * constants::SeedDensityFalloff <= density; --times) {
      PointType test_pt = initial_pt + direction * times * increment;
      density = interpolator->Evaluate(test_pt);
      bkwd_length = times * increment;
   }

   double length_of_comparable_density = fwd_length + -bkwd_length;

if (0 < length_of_comparable_density) {
//cout << "non-zero length: " << length_of_comparable_density << endl;
++n_non_zero_lengths;
}

   return length_of_comparable_density;
}


// The /whole/ condition
// <physPt> is in pixels.(???) Hrm; the problem with that is that when we shift
// the image around slightly, we'll have to convert those. Ie,
// we'll require the nodes to remember their offsets... Hm.
// Used to classify the seeds
bool MeetsBetaCondition(PointType const& physPt,
                        EigenValuesType const& /* evals */,
                        EigenVectorsType const& evec,
                        ImageType::Pointer image,
                        double sheetMin, double sheetMax)
{
   double increment = constants::LineIncrement * image->GetSpacing()[0];

   InterpolatorType::Pointer interpolator = InterpolatorType::New();

#if GABBLE_INTERPOLATOR_IS_SPLINE
cout << "yep it's spline, order: " << GabbleSplineOrder << endl;
   // NOTE: order must come before image
   interpolator->SetSplineOrder(GabbleSplineOrder);
#endif
   interpolator->SetInputImage(image);
   double initial_density = interpolator->Evaluate(physPt);

   // Note, t1 uses evec[2], t2 [1], t3 [0]; that's because ITK orders
   // them in /ascending/ order, ie smallest first. We want largest,
   // first.
   double t1 = length_of_density(interpolator, physPt, initial_density,
                                 evec[2], sheetMax, increment);
   double t2 = length_of_density(interpolator, physPt, initial_density,
                                 evec[1], sheetMax, increment);
   double t3 = length_of_density(interpolator, physPt, initial_density,
                                 evec[0], sheetMax, increment);

// cout << sheetMin << " <=? " << " (t1) " << t1 << " <=? " << sheetMax << " ?"
//      << endl;
// cout << "t1: " << t1 << "; t2: " << t2 << "; t3: " << t3
//      << endl;
// cout << "std::max(t1 / t2, t1 / t3) < std::min(t2 / t3, t3 / t2)\n"
//      << "\t" << std::max(t1 / t2, t1 / t3) << "\t\t"
//      << std::min(t2 / t3, t3 / t2)
//      << endl;

   bool isBeta =
      sheetMin <= t1 and t1 <= sheetMax and
      std::max(t1 / t2, t1 / t3) < std::min(t2 / t3, t3 / t2);

   return isBeta;
}
