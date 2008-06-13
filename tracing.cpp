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

   for (Seeds::const_iterator it = seeds.begin(), end = seeds.end();
        it != end; ++it) {
      PointType pt;
      image->TransformIndexToPhysicalPoint(*it, pt);
      possible_beta_points.push(pt);
//cout << __FILE__ << " seed: " << *it
//     << "; pt: " << pt << endl;
   }

   while (not possible_beta_points.empty()) {
      PointType pt = possible_beta_points.front();
      possible_beta_points.pop();

      if (Node::IsFarEnoughAwayFromOthers(pt) and PointIsBeta(image, pt)) {
         // we don't need the machinery anymore, the point is enough
         PointType loCell, hiCell;
         VectorType normal;

         Points intersections;
         planes_intersection_with_box(normal, pt,
                                      // &&& ack, cell stuff, here
                                      loCell, hiCell,
                                      intersections);
         Polygon polygon;
         MakePolygon(normal, intersections, polygon);

         for (Points::const_iterator it = intersections.begin(),
                 end = intersections.end();
              it != end; ++it) {
            possible_beta_points.push(*it);
         }
         outNodes.push_back(new Node(pt, polygon));
      }
   }
}

// The /whole/ condition
// <pt> is in pixels. Hrm; the problem with that is that when we shift
// the image around slightly, we'll have to convert those. Ie,
// we'll require the nodes to remember their offsets... Hm.
bool PointIsBeta(Image::Pointer image, PointType const& origPt)
{
   // &&& arbitrary. I believe this is cell units for the time being
   int region_width = 6;

   BetaPipeline pipeline(image, origPt, region_width);

   VectorType ptShift;
   pt_shift(origPt, image->GetSpacing(), ptShift);

   PointType newPt = origPt + ptShift;
   EigenVectorImageType::IndexType index;
   image->TransformPhysicalPointToIndex(newPt, index);

cout << __FILE__ << " index: " << index << endl;

   EigenVectorImageType::Pointer evecImage = pipeline.eigVecImage();
   EigenVectorImageType::RegionType definedRegion = evecImage->GetBufferedRegion();

// &&& grrr - defined region is at 0,0,0, [5,5,5]
// and index is way in the middle somewhere.
cout << __FILE__ << "\ndefined region: \n" << endl;
definedRegion.Print(cout);
   EigenVector v1 = evecImage->GetPixel(index)[0];
   EigenVector v2 = evecImage->GetPixel(index)[1];
   EigenVector v3 = evecImage->GetPixel(index)[2];

   double sheetMin, sheetMax;
   double t1 = v1.GetNorm(), t2 = v2.GetNorm(), t3 = v3.GetNorm();
   bool isBeta = MeetsBetaCondition(sheetMin, sheetMax, t1, t2, t3);

   return isBeta;
}


// Used to classify the seeds
bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3)
{
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
