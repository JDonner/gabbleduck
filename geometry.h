#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "types.h"
#include "point.h"
#include "triangle.h"

bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3);

//void MakePolygon(Points const& planar_points, Polygon& outPolygon);
void MakeTriangles(VectorType const& /*normal*/,
                   Points const& planar_points,
                   TriangleBunch& outTriangles);

void planes_intersection_with_box(VectorType normal, PointType const& pt,
                                  // front, lower left, and rear, upper right
                                  PointType const& lo, PointType const& hi,
                                  Points& intersections);
// pt
// Must stay in physical coords
VectorType transform_shift(PointType const& pt,
                           ImageType::SpacingType const& spacing);


// void pt_shift(PointType const& pt,
//               ImageType::SpacingType const& spacing,
//               VectorType& outShift);

#endif // GEOMETRY_H
