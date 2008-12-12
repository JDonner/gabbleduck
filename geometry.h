#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "types.h"
#include "point.h"
#include "triangle.h"

//void MakePolygon(Points const& planar_points, Polygon& outPolygon);
void MakeTriangles(VectorType const& /*normal*/,
                   Points const& planar_points,
                   TriangleBunch& outTriangles);

void planes_intersection_with_box(VectorType normal, PointType const& pt,
                                  // front, lower left, and rear, upper right
                                  PointType const& lo, PointType const& hi,
                                  Points& intersections);

#endif // GEOMETRY_H
