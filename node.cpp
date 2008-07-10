#include "node.h"

// for 'is-far-enough-away'
Node::Nodes Node::s_all_beta_points;

// Yah it's n^2.
bool Node::IsFarEnoughAwayFromOthers(PointType const& pt)
{
   for (Nodes::const_iterator it = s_all_beta_points.begin(),
           end = s_all_beta_points.end();
        it != end; ++it) {
      // &&& Wow, calculating this could get //sloooow//
      if ((*it)->pos().EuclideanDistanceTo<double>(pt) < RequiredNewPointSeparation) {
         return false;
      }
   }
   return true;
}
