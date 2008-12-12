#ifndef HOMEMADE_GAUSSIAN_H
#define HOMEMADE_GAUSSIAN_H

#include "types.h"

#include "mask.h"

extern void init_gaussian_mask(int n, Flt sigma);

typedef Mask_T<Flt> GaussianMaskType;

extern GaussianMaskType* g_GaussianMask;
extern void init_gaussian_mask(int n, Flt sigma);
// we use 2 sigma (but support on both sides, totals 4 sigma
// We assume 1 pixel = 1 physical unit. Correct equation given
// (but not used) in code.
extern unsigned support_of_sigma(Flt sigma);

#endif // HOMEMADE_GAUSSIAN_H
