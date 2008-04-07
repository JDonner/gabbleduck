#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "types.h"

bool MeetsBetaCondition(double sheetMin, double sheetMax,
                        double t1, double t2, double t3);
Polygon MakePolygon(Points const& planar_points);

#endif // GEOMETRY_H
