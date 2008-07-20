#include "constants.h"

// &&& Make it a function of the map resolution?
double BetaThickness = 5.0;

// &&& This 20% is arbitrary
double BetaThicknessFlex = 0.2;

double BetaMin = BetaThickness * (1.0 - BetaThicknessFlex);
double BetaMax = BetaThickness * (1.0 + BetaThicknessFlex);

// &&& unused, but needs to be
// Should be the size of the feature to be detected, ie, BetaThickness.
double WindowSize = BetaThickness;

// &&& don't know what this is. Should be BetaThickness, though, apparently.
double GaussianSupport = WindowSize;

// Density, below which we don't even bother to check whether it's
// a local maxima. As good as 0, in other words.
double SeedDensityThreshold = 0.1;

// Physical coordinates
double RequiredNewPointSeparation = 0.5;

// A seed (local maxima)'s density must be at least 80% of the highest
// seed density
double SeedDensityWorthinessThreshold = 0.8;

// At what falloff, from a maximum, is it the end of the beta region?
// We're saying, here, when it reaches half the highest density.
// This is absolutely arbitrary and wants to be experimental.
double SeedDensityFalloff = 0.8;

// Or, a binary search?
double LineIncrement = 0.25;

double SkeletonMergeThreshold = BetaThickness;

// Not a physical constant; maybe belongs in instrument...
unsigned SnapshotIntervalBase = 500;
unsigned SnapshotIntervalPower = 2;
unsigned FinalSnapshot = 1;

unsigned MaxPoints = 10000;

// Beta points don't have an intensity but we want to use 3D density
// methods to show them, so this is the density we give them.
double FauxBetaPointDensity = 0.1;

unsigned ImageZoom = 2;
