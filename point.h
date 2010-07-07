#ifndef BETA_POINT_H
#define BETA_POINT_H

#include <itkPoint.h>
#include <vector>

// Physical?
// &&& oer - PointType is an ITK type, too.
typedef itk::Point<double, Dimension> PointType;

typedef std::vector<PointType> Points;

#endif // BETA_POINT_H
