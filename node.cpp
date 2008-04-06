#include "node.h"

// for 'is-far-enough-away'
static Node::Points Node::s_all_beta_points;
// In 'cell-length' units, not real-world distance.
static double Node::s_min_allowed_interpoint_dist = 1.0;

// Yah it's n^2.
bool Node::IsFarEnoughAwayFromOthers(Point const& pt)
{
   Node::Points Node::s_all_beta_points;

   for (Points::const_iterator it = s_all_beta_points.begin(),
           end = s_all_beta_points.end();
        it != end; ++it) {
      if (it->EuclidianDistanceTo(pt) < s_min_allowed_interpoint_dist) {
         return false;
      }
   }
   return true;
}
