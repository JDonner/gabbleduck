#include "node.h"
#include "pipeline.h"
#include "geometry.h"


// non-recursive version
void FindBetaNodes(seeds, ImageType::ConstPointer image)
{
   possible_beta_points.push(seeds);

   while (not possible_beta_points.empty()) {
      pt = possible_beta_points.top();
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
bool PointIsBeta(PointPos )
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