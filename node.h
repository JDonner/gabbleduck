#ifndef NODE_H
#define NODE_H

#include "types.h"
#include "point.h"
#include "triangle.h"

#include <vector>

class Node
{
public:
   Node(PointType const& pos, TriangleBunch const& triangles)
   : pos_(pos)
   , triangles_(triangles)
   {}

   PointType const& pos() const { return pos_; }

private:
   // Should this be in pixels, or physical coordinates?
   PointType pos_;

   // (or triangle list)
   TriangleBunch triangles_;

   ImageType::Pointer source_;
//   typedef std::vector<Node*> Nodes;
//   Nodes children_;
};

typedef std::vector<Node*> Nodes;

#endif // NODE_H
