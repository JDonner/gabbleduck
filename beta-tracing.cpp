#include "BetaNode.h"
#include "BetaPipeline.h"
#include "geometry.h"


// non-recursive version
void FindBetaNodes(seeds, ImageType::ConstPointer image)
{
   possible_beta_points.push(seeds);

   while (not possible_beta_points.empty()) {
      node = possible_beta_points.top();
      possible_beta_points.pop();

      bool isBeta = PointIsBeta(PointPos stoff);

      if (point is far enough away and IsBetaLike()) {
         // we don't need the machinery anymore, the point is enough
         node = new BetaNode(all the relevant info);
         get geometrical on its ass;
         possible_beta_points.push(node.children());

         vector<point>& intersections;
         planes_intersection_with_box(vector normal, point pt,
                                      point pt0, point pt1,
                                      intersections);


         Polygon polygon = MakePolygon(v normal, points& intersections);


         push children;
         push self into frontier;

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
