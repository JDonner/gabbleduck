#ifndef NODE_H
#define NODE_H

#include "polygon.h"

#include <vector>

class Node
{
public:
   static bool IsFarEnoughAwayFromOthers(Point const& pt);
   // In 'cell-length' units, not real-world distance.
   static void setMinAllowedDist(double minAllowedDist);

private:
   PointPos pos;

   // (or triangle list)
   Polygon polygon_;

   ImageType::ConstPointer source_;
   std::vector<Node*> Nodes;
   Nodes children_;

   static Points s_all_beta_points;
   static double s_min_allowed_interpoint_dist;
};

#endif // NODE_H
