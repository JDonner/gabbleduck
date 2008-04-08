#include "node.h"
#include "pipeline.h"
#include "geometry.h"
#include <vector>

using namespace std;

typedef ImageType::IndexType Seed;
typedef vector<Seed> Seeds;

bool PointIsBeta(PointPos pt);


// non-recursive version
void FindBetaNodes(ImageType::ConstPointer image,
                   Seeds const& seeds)
{
   Points possible_beta_points;

   for (Seeds::const_iterator it = seeds.begin(), end = seeds.end();
        it != end; ++it) {
      PointType pt;
      image->TransformIndexToPhysicalPoint(*it, pt);
      possible_beta_points.push_back(pt);
   }

   while (not possible_beta_points.empty()) {
      PointType pt = possible_beta_points.top();
      possible_beta_points.pop();

      bool isBeta = PointIsBeta(pt);

      if (Node::IsFarEnoughAwayFromOthers(pt) and IsBetaLike(pt)) {
         // we don't need the machinery anymore, the point is enough
         Node* node = new Node(all the relevant info);

         vector<point>& intersections;
         planes_intersection_with_box(vector normal, point pt,
                                      point pt0, point pt1,
                                      intersections);
         Polygon polygon = MakePolygon(v normal, points& intersections);
         node->setPolygon(polygon);
         possible_beta_points.push(intersections);
      }
   }
}

// badly named if nothing else
bool PointIsBeta(PointPos pt)
{
   bool isBeta = false;

   eigenoutput;
   {
      // Now test whether we're Beta enough.
      BetaPipeline pipeline = Pipeline(m_source);

      eigenoutput = pipeline.getresult();
   }

   isBeta = QualifiesAsBeta(all our stuff, us);

   return isBeta;
}

// cross-product(v a, v b)
// {
//    v cross;
//    cross.i = + a.j * b.k - b.j * a.k;
//    cross.j = - a.i * b.k - b.i * a.k;
//    cross.k = + a.i * b.j - b.i * a.j;
// }
