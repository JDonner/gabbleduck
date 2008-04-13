#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "types.h"
#include "point.h"

bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3);

void MakePolygon(VectorType const& normal, Points const& planar_points,
                 Polygon& outPolygon);
void MakePolygon(Points const& planar_points, Polygon& outPolygon);

void planes_intersection_with_box(VectorType normal, PointType const& pt,
                                  // front, lower left, and rear, upper right
                                  PointType const& lo, PointType const& hi,
                                  Points& intersections);

#endif // GEOMETRY_H
