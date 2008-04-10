#ifndef BETA_POINT_H
#define BETA_POINT_H

#include <itkPoint.h>
#include <vector>

typedef itk::Point<double, Dimension> PointType;

struct PointPos
{
   // Physical, or fractional index? Fractional index, I think.

   // offset of cell from original. Components should always be < 1.0.
   // We do this to allow ourselves to resample from the original image,
   // instead of already-resampled images, to minimize accumulated error.
   PointType fractional_offset;

   // absolute
   PointType absolute_position;
};

typedef std::vector<PointPos> PointPoses;
typedef std::vector<PointType> Points;

#endif // BETA_POINT_H
