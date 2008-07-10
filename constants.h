#ifndef CONSTANTS_H
#define CONSTANTS_H

/*********************************************************************
  Page 13 of Yu and Bajaj.

  Between 2.5 and 15 Angstroms! Page 13 of Yu and Bajaj.

  For an 8 Angstrom resolution map, 5 Angstroms is about right (pg 13)
*********************************************************************/


// &&& Make it a function of the map resolution?
const double BetaThickness = 5.0;

// &&& unused, but needs to be
// Should be the size of the feature to be detected, ie, BetaThickness.
const double WindowSize = BetaThickness;

// &&& This 20% is arbitrary
const double BetaThicknessRange = 0.2;

const double BetaMin = BetaThickness * (1.0 - BetaThicknessRange);
const double BetaMax = BetaThickness * (1.0 + BetaThicknessRange);

const double SkeletonMergeThreshold = BetaThickness;

// Density, below which we don't even bother to check whether it's
// a local maxima. As good as 0, in other words.
const double ScrubDensity = 0.3;

// At what falloff, from a maximum, is it the end of the beta region?
// We're saying, here, when it reaches half the highest density.
// This is absolutely arbitrary and wants to be experimental.
const double SeedFalloff = 0.7;

// Or, a binary search?
const double LineIncrement = 0.25;

// &&& don't know what this is. Should be BetaThickness, though, apparently.
const double GaussianSupport = BetaThickness;


// Not a physical constant; maybe belongs in instrument...
const unsigned SnapshotIntervalBase = 500;
const unsigned SnapshotIntervalPower = 2;
const double FauxBetaPointDensity = 0.1;
const unsigned ImageZoom = 4;

const double RequiredNewPointSeparation = 0.5;

#endif // CONSTANTS_H
