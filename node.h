#ifndef NODE_H
#define NODE_H

#include "types.h"
#include "point.h"
#include "polygon.h"

#include <vector>

class Node
{
public:
   Node(PointType const& pos, Polygon const& polygon)
   : pos_(pos)
   , polygon_(polygon)
   {}

   PointType const& pos() const { return pos_; }

   static bool IsFarEnoughAwayFromOthers(PointType const& pt);

   // In 'cell-length' units, not real-world distance.
   static void setMinAllowedDist(double minAllowedDist);

private:
   // Should this be in pixels, or physical coordinates?
   PointType pos_;

   // (or triangle list)
   Polygon polygon_;

   ImageType::Pointer source_;
   typedef std::vector<Node*> Nodes;
   Nodes children_;

//   static Nodes s_all_beta_points;
};

typedef std::vector<Node*> Nodes;

#endif // NODE_H
