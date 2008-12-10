#ifndef CONSTANTS_H
#define CONSTANTS_H

/*********************************************************************
  Between 2.5 and 15 Angstroms! Page 13 of Yu and Bajaj.

  For an 8 Angstrom resolution map, 5 Angstroms is about right (pg 13)
*********************************************************************/

namespace constants
{
// &&& Make it a function of the map resolution?
extern double BetaThickness;

// g_alpha of the paper, the 'window size', to spread the above
// 2nd derivative around.
extern double SigmaOfFeatureGaussian;

// Should be odd. This should truthfully be calculated from the size
// of the feature gaussian.
// JGD is not sure ITK uses it though, which is disturbing; what does
// it do when it goes off the edge of the sampled region, just use a
// default value without complaint? Something's wrong, it should
// complain. This is a parameter just to experiment with, but this
// shouldn't be here.

// http://www.mvtec.com/halcon/download/documentation/reference/hdevelop/binomial_filter.html

// sigma = sqrt(n-1)/2

//  n  |  sigma
// -------------
//  3  |  0.7523
//  5  |  1.0317
//  7  |  1.2505
//  9  |  1.4365
// 11  |  1.6010
// 13  |  1.7502
// 15  |  1.8876
// 17  |  2.0157
// 19  |  2.1361
// 21  |  2.2501
// 23  |  2.3586
// 25  |  2.4623
// 27  |  2.5618
// 29  |  2.6576
// 31  |  2.7500
// 33  |  2.8395
// 35  |  2.9262
// 37  |  3.0104
// (n above == GaussianSupportSize)
extern int GaussianSupportSize;

// Should be the size of the feature to be detected, ie, BetaThickness.
// AKA the 'window size'

// &&& This 20% is arbitrary
extern double BetaThickRangeRatio;

extern double BetaMin;
extern double BetaMax;

// Density, below which a point isn't even looked at as far as being
// a beta point.
extern double SeedDensityThreshold;

// What we're willing to think of as a reasonable beta density,
// as a proportion of the highest density found in the map.
// &&& Unfortunately, this method is totally arbitrary. Perhaps it'd
// be an ok way to measure it in an interactive program but,
// it just seems to take a lot of tuning.
extern double RelativeSeedDensityThreshold;

// The actual density value guessed for beta sheets from the above
// <RelativeSeedDensityThreshold> * the maximum sheet density.
extern double CandidateDensityThreshold;

// At what falloff, from a maximum, is it the end of the beta region?
// We're saying, here, when it reaches half the highest density.
// This is absolutely arbitrary and wants to be experimental.
extern double SeedDensityFalloff;

// &&& Or, a binary search?
extern double LineIncrement;

// Not a physical constant; maybe belongs in instrument...
extern unsigned SnapshotIntervalBase;
extern unsigned SnapshotIntervalPower;

// Beta points don't have an intensity but we want to use 3D density
// methods to show them, so we give them this fake value.
extern double BetaPointDisplayFakeDensity;
extern bool ShowSeeds;
extern double SeedsDisplayEmphFactor;
extern unsigned SnapshotImageZoom;

// In physical coordinates
extern double RequiredNewPointSeparation;

extern unsigned MaxPoints;
extern unsigned FinalSnapshot;

/* Things to check / vary:

   Resolution
   Line length (and whether to limit it at all, on t2,t3 -
     but that's just a speed thing)
   density falloff

 - for speed, lower the degree of the interpolation perhaps.

   get to grips with the formula!

   How do I know I have the true local maximae? Is there another way,
   how does the paper do it?

   * Find way to tune Gaussian support. SigmaOfFeatureGaussian, supposed to be
     the same as the feature size.
   * Work on triangularization, skeletonization
     - because, this way I can automate the search for good parameters.
   * Look at their data
   * translate the support for gaussian, from physical.
      ('too_small') - ach, just give it a surviving size

   * The big time-saver would be to convert from double to float.

   * subtract out the alphas' contributions!
 */

// Dr He's recommendation on generating images
// pdb2mrc 2.pdb a.mrc res=10 apix=1.0 center

} // namespace constants

#endif // CONSTANTS_H
