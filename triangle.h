#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>

#include "point.h"

// We're too lazy to figure out what to do for triangles so, we'll work it out later.

struct Triangle
{
   Triangle(unsigned a, unsigned b, unsigned c)
   {
      vtxes[0] = a, vtxes[1] = b, vtxes[2] = c;
   }

   Triangle(unsigned (&vs)[3])
   {
      vtxes[0] = vs[0], vtxes[1] = vs[1], vtxes[2] = vs[2];
   }

   unsigned vtxes[3];
};

class TriangleBunch
{
public:
   TriangleBunch()
   {}

   TriangleBunch(Points const& pts)
   : points_(pts)
   {}

   void set_points(Points const& pts)
   {
      points_ = pts;
   }

   void add_point(PointType const& pt)
   {
      points_.push_back(pt);
   }

   void add_triangle(unsigned a, unsigned b, unsigned c)
   {
      assert(a < points_.size());
      assert(b < points_.size());
      assert(c < points_.size());
      triangles_.push_back(Triangle(a, b, c));
   }

private:
   typedef std::vector<Triangle> Triangles;
   Triangles triangles_;
   Points points_;
};

#endif // TRIANGLE_H
