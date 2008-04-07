#include "node.h"

// for 'is-far-enough-away'
Node::Nodes Node::s_all_beta_points;
// In 'cell-length' units, not real-world distance.
double Node::s_min_allowed_interpoint_dist = 1.0;

// Yah it's n^2.
bool Node::IsFarEnoughAwayFromOthers(PointType const& pt)
{
   for (Nodes::const_iterator it = s_all_beta_points.begin(),
           end = s_all_beta_points.end();
        it != end; ++it) {
      if ((*it)->pos.absolute_position.EuclideanDistanceTo<double>(pt) < s_min_allowed_interpoint_dist) {
         return false;
      }
   }
   return true;
}
