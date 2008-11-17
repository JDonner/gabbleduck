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

// &&& don't know what this is. Should be BetaThickness, though, apparently.
// Should be the size of the feature to be detected, ie, BetaThickness.
// AKA the 'window size'
extern double SigmaOfGaussian;

// &&& This 20% is arbitrary
extern double BetaThicknessFlex;

extern double BetaMin;
extern double BetaMax;
//extern double SkeletonMergeThreshold;

// Density, below which we don't even bother to check whether it's
// a local maxima. As good as 0, in other words.
extern double SeedDensityThreshold;

// Once we've got the seed, this is the
extern double SeedDensityWorthinessThreshold;

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
extern double BetaPointFakeDensity;
extern bool ShowSeeds;
extern double SeedsDisplayEmphFactor;
extern unsigned SnapshotImageZoom;

// Physical coordinates
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

   * Find way to tune Gaussian support. SigmaOfGaussian, supposed to be
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
