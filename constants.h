#ifndef CONSTANTS_H
#define CONSTANTS_H

/*********************************************************************
  Page 13 of Yu and Bajaj.

  Between 2.5 and 15 Angstroms! Page 13 of Yu and Bajaj.

  For an 8 Angstrom resolution map, 5 Angstroms is about right (pg 13)
*********************************************************************/


// &&& Make it a function of the map resolution?
extern double BetaThickness;

// &&& unused, but needs to be
// Should be the size of the feature to be detected, ie, BetaThickness.
extern double WindowSize;

// &&& This 20% is arbitrary
extern double BetaThicknessFlex;

extern double BetaMin;
extern double BetaMax;
extern double SkeletonMergeThreshold;

// Density, below which we don't even bother to check whether it's
// a local maxima. As good as 0, in other words.
extern double ScrubDensity;

// At what falloff, from a maximum, is it the end of the beta region?
// We're saying, here, when it reaches half the highest density.
// This is absolutely arbitrary and wants to be experimental.
extern double SeedDensityFalloff;

// A seed (local maxima)'s density must be at least 80% of the highest
// seed density
extern double SeedDensityWorthinessThreshold;

// Or, a binary search?
extern double LineIncrement;

// &&& don't know what this is. Should be BetaThickness, though, apparently.
extern double GaussianSupport;


// Not a physical constant; maybe belongs in instrument...
extern unsigned SnapshotIntervalBase;
extern unsigned SnapshotIntervalPower;
// Beta points don't have an intensity but we want to use 3D density
// methods to show them, so this is the density we give them.
extern double FauxBetaPointDensity;
extern unsigned ImageZoom;

// Physical coordinates
extern double RequiredNewPointSeparation;

extern unsigned MaxPoints;
extern unsigned FinalSnapshot;

/* Things to check / vary:

   Gaussian support
   Resolution
   Line length (and whether to limit it at all, on t2,t3)
   density falloff
 - for speed, the degree of the interpolation.

   get to grips with the formula!
   work on triangularization!

 */

// Jing's recommendation on generating images
// pdb2mrc a.pdb a.mrc res=10 apix=1.0 center

#endif // CONSTANTS_H
