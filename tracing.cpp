#include "node.h"
#include "pipeline.h"
#include "geometry.h"
#include "point.h"
#include "tracing.h"

#include <vector>
#include <queue>

using namespace std;

typedef queue<PointType> PointQueue;

extern bool PointIsBeta(Image::Pointer image, PointType const& pt);
bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3);
bool PointIsPlanelike(EigenValue u1, EigenValue u2, EigenValue u3);
void new_pt(PointType const& pt, Image::SpacingType const& spacing,
            PointType& newPt);
void new_pt_index(PointType const& pt, Image::SpacingType const& spacing,
                  ImageType::IndexType& newPt);


// non-recursive version
void FindBetaNodes(ImageType::Pointer image,
                   Seeds const& seeds,
                   Nodes& outNodes)
{
   PointQueue possible_beta_points;

cout << "image origin: " << image->GetOrigin() << endl;

   // Load possible betas
   for (Seeds::const_iterator it = seeds.begin(), end = seeds.end();
        it != end; ++it) {
      PointType physPt;
      image->TransformIndexToPhysicalPoint(*it, physPt);
      possible_beta_points.push(physPt);
//cout << __FILE__ << " seed: " << *it
//     << "; physPt: " << physPt << endl;
   }

   unsigned nthVisited;
   unsigned n_far_enough_away = 0;
   unsigned n_beta = 0;
   for (nthVisited = 0; not possible_beta_points.empty();
        ++nthVisited) {
      cout << nthVisited << "th visited" << endl;
      PointType physPt = possible_beta_points.front();
      possible_beta_points.pop();

      if (Node::IsFarEnoughAwayFromOthers(physPt)) {
         ++n_far_enough_away;

         if (PointIsBeta(image, physPt)) {
            ++n_beta;
            // we don't need the machinery anymore, the point is enough
            PointType loCell, hiCell;
            VectorType normal;

            Points intersections;
            planes_intersection_with_box(normal, physPt,
                                         // &&& ack, cell stuff, here
                                         loCell, hiCell,
                                         intersections);
            Polygon polygon;
            MakePolygon(normal, intersections, polygon);

            for (Points::const_iterator it = intersections.begin(),
                    end = intersections.end();
                 it != end; ++it) {
cout << "adding a pt " << endl;
               possible_beta_points.push(*it);
            }
            outNodes.push_back(new Node(physPt, polygon));
         }
      }
   }
cout << nthVisited << " total visited" << "\n"
     << n_far_enough_away << " far enough away" << "\n"
     << n_beta << " beta" << "\n"
     << endl;
}

// Yay!
//BSplineInterpolateImageFunction::EvaluateAtContinuousIndex

// The /whole/ condition
// <physPt> is in pixels.(???) Hrm; the problem with that is that when we shift
// the image around slightly, we'll have to convert those. Ie,
// we'll require the nodes to remember their offsets... Hm.
bool PointIsBeta(Image::Pointer fullImage, PointType const& physPt)
{
   // &&& arbitrary. I believe this is cell units for the time being
   int region_width = 7;

   // pipeline does resampling
   BetaPipeline pipeline(fullImage, physPt, region_width);

   VectorType physShift;
   pt_shift(physPt, fullImage->GetSpacing(), physShift);

ImageType::IndexType indexUseless;
pipeline.resampler_->GetOutput()->TransformPhysicalPointToIndex(physPt, indexUseless);
cout << __FILE__ << " (useless) index: " << indexUseless << endl;

   PointType newPt = physPt + physShift;
   ImageType::IndexType index;
   bool isWithinImage = pipeline.resampler_->GetOutput()->TransformPhysicalPointToIndex(newPt, index);

cout << __FILE__ << " (awkward) index: " << index << "; within?: " << isWithinImage << endl;

   EigenValueImageType::Pointer evalImage = pipeline.eigValImage();
   EigenValue u1 = evalImage->GetPixel(index)[0];
   EigenValue u2 = evalImage->GetPixel(index)[1];
   EigenValue u3 = evalImage->GetPixel(index)[2];

   if (PointIsPlanelike(u1, u2, u3)) {
      cout << "woo hoo, planelike!" << endl;
      // &&& just for the encouraging effect
      return true;
   }

   EigenVectorImageType::Pointer evecImage = pipeline.eigVecImage();
EigenVectorImageType::RegionType eigDefinedRegion = evecImage->GetBufferedRegion();

ImageType::RegionType snipDefinedRegion = fullImage->GetBufferedRegion();
cout << __FILE__ << "\nsnip defined region: \n" << endl;
//snipDefinedRegion.Print(cout);

// &&& grrr - defined region is at 0,0,0, [5,5,5]
// and index is way in the middle somewhere.
cout << __FILE__ << "\neig defined region: \n" << endl;
//eigDefinedRegion.Print(cout);
   EigenVector v1 = evecImage->GetPixel(index)[0];
   EigenVector v2 = evecImage->GetPixel(index)[1];
   EigenVector v3 = evecImage->GetPixel(index)[2];



   double sheetMin, sheetMax;
   double t1 = v1.GetNorm(), t2 = v2.GetNorm(), t3 = v3.GetNorm();
cout << "vx: " << v1 << "; " << v2 << "; " << v3 << endl;
   bool isBeta = MeetsBetaCondition(sheetMin, sheetMax, t1, t2, t3);

   return isBeta;
}


// u_x are eigenvalues
bool PointIsPlanelike(EigenValue u1, EigenValue u2, EigenValue u3)
{
   double p1 = (u1 - u2) / u1;
   double p2 = (u2 - u3) / u1;
   double p3 = u3 / u1;

   bool planelike = p1 > p2 and p1 > p3;

   return planelike;
}


// Used to classify the seeds
bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3)
{
cout << sheetMin << " <= " << t1 << " <= " << sheetMax << endl;
cout << "t1: " << t1 << "; t2: " << t2 << "; t3: " << t3 << endl;
cout << "std::max(t1 / t2, t1 / t3) < std::min(t2 / t3, t3 / t2)"
     << std::max(t1 / t2, t1 / t3) << " "
     << std::min(t2 / t3, t3 / t2) << endl;
   bool isBeta =
      sheetMin <= t1 and t1 <= sheetMax and
      std::max(t1 / t2, t1 / t3) < std::min(t2 / t3, t3 / t2);

cout << "Is beta? " << isBeta << endl;
   return isBeta;
}


void new_pt(PointType const& pt,
            Image::SpacingType const& spacing,
            PointType& newPt)
{
   VectorType shift;
   pt_shift(pt, spacing, shift);
   for (unsigned i = 0; i < Dimension; ++i) {
      newPt[i] += shift[i];
   }
}


// void new_pt_index(PointType const& pt,
//                   Image::SpacingType const& spacing,
//                   ImageType::IndexType& newIndex)
// {
//    for (unsigned i = 0; i < Dimension; ++i) {
//       newIndex[i] = int(pt[i] + spacing[i] / 2.0);
//       assert(0 <= newPt[i]);
//    }
// }

// Seeds are in pixels, no sense in anything else.
// We want to resample (for a new image) against the original image,
// for fidelity's sake. The shift is in physical coords.
// This offset is: pt loc (in whichever coords) fmod(pt.X, imagespace.X)
// Intersections will be in whatever coords we start with.
// (Can we keep from accumulating error?)
// We can't, as long as we calculate via a long chain of moves, which
// we must. We can resample wrt to the original image which is good, but
// location, we can't avoid multiple offsets.
