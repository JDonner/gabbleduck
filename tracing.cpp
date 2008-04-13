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


// non-recursive version
void FindBetaNodes(ImageType::Pointer image,
                   Seeds const& seeds,
                   Nodes& outNodes)
{
   PointQueue possible_beta_points;

   for (Seeds::const_iterator it = seeds.begin(), end = seeds.end();
        it != end; ++it) {
      PointType pt;
      image->TransformIndexToPhysicalPoint(*it, pt);
      possible_beta_points.push(pt);
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


// Used to classify the seeds
bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3)
{
   bool isBeta =
      sheetMin <= t1 and t1 <= sheetMax and
      std::max(t1 / t2, t1 / t3) < std::min(t2 / t3, t3 / t2);

   return isBeta;
}


// The /whole/ condition
bool PointIsBeta(Image::Pointer image, PointType const& pt)
{
   // &&& arbitrary. I believe this is cell units for the time being
   int region_width = 6;

   BetaPipeline pipeline(image, pt, region_width);

// pipeline.eigValImage();
// pipeline.eigVecImage();

   EVectorImageType::Pointer evecImage = pipeline.eigVecImage();

   EVectorImageType::IndexType index;
   index[0] = 0;
   index[1] = 0;
   index[2] = 0;
   double* evec = evecImage->GetPixel(index)[0];

// /big/common/insight/InsightToolkit-3.4.0/Testing/Code/Common/itkSymmetricEigenAnalysisTest.cxx

   double sheetMin, sheetMax;
   double t1, t2, t3;
   bool isBeta = MeetsBetaCondition(sheetMin, sheetMax, t1, t2, t3);

   return isBeta;
}


// cross-product(v a, v b)
// {
//    v cross;
//    cross.i = + a.j * b.k - b.j * a.k;
//    cross.j = - a.i * b.k - b.i * a.k;
//    cross.k = + a.i * b.j - b.i * a.j;
// }
