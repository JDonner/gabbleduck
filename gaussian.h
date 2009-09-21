#ifndef HOMEMADE_GAUSSIAN_H
#define HOMEMADE_GAUSSIAN_H

#include "mask.h"

extern void init_gaussian_mask(int n, double sigma);

typedef Mask_T<double> GaussianMaskType;

extern GaussianMaskType* g_GaussianMask;
extern void init_gaussian_mask(int n, double sigma);
// we use 2 sigma (but support on both sides, totals 4 sigma
// We assume 1 pixel = 1 physical unit. Correct equation given
// (but not used) in code.
extern unsigned support_of_sigma(double sigma);

#endif // HOMEMADE_GAUSSIAN_H
