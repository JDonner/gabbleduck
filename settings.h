#ifndef SETTINGS_H
#define SETTINGS_H

#include "constants.h"

// Also, make the interpolation type changeable - degree for

enum GabbleSampleInterpolator {
   GABBLE_INTERPOLATOR_LINEAR,
   GABBLE_INTERPOLATOR_SPLINE_2,
   GABBLE_INTERPOLATOR_SPLINE_3,
   GABBLE_INTERPOLATOR_SPLINE_4,
   GABBLE_INTERPOLATOR_SPLINE_5
};

const unsigned GabbleSplineOrder = 1;

#define GABBLE_INTERPOLATOR GABBLE_INTERPOLATOR_SPLINE_3
#  define GABBLE_INTERPOLATOR_IS_SPLINE 1

// #if GABBLE_INTERPOLATOR == GABBLE_INTERPOLATOR_SPLINE_2
// #  warning GABBLE_INTERPOLATOR_SPLINE_2
// #  warning "SPLINE-2"
// #  define GABBLE_INTERPOLATOR_IS_SPLINE 1
// const unsigned GabbleSplineOrder = 2;
// #elif GABBLE_INTERPOLATOR == GABBLE_INTERPOLATOR_SPLINE_3
// #  warning "SPLINE-3"
// #  define GABBLE_INTERPOLATOR_IS_SPLINE 1
// const unsigned GabbleSplineOrder = 3;
// #elif GABBLE_INTERPOLATOR == GABBLE_INTERPOLATOR_SPLINE_4
// #  warning "SPLINE-4"
// #  define GABBLE_INTERPOLATOR_IS_SPLINE 1
// const unsigned GabbleSplineOrder = 4;
// #elif GABBLE_INTERPOLATOR == GABBLE_INTERPOLATOR_SPLINE_5
// #  warning "SPLINE-5"
// #  define GABBLE_INTERPOLATOR_IS_SPLINE 1
// const unsigned GabbleSplineOrder = 5;
// #else
// #  define GABBLE_INTERPOLATOR_IS_LINEAR 1
// #endif

//#define WANT_GRID_BOUNDS_CHECKING 0
#define WANT_GRID_BOUNDS_CHECKING 1

#endif // SETTINGS_H
