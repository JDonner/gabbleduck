#include "constants.h"

// These constants would be used if we didn't have the boost::options
// stuff.  I believe those obviate all of this, the boost framework
// gives them their own default values.  Believe those instead.

namespace constants
{
// &&& This 20% is arbitrary
double BetaThicknessFlex = 0.2;

double BetaMin = BetaThickness * (1.0 - BetaThicknessFlex);
double BetaMax = BetaThickness * (1.0 + BetaThicknessFlex);

// from equation (7) from paper.
double SigmaOfDerivativeGaussian = BetaThickness;


double BetaThickness = 5.0;
// paper's recommended value (as compromise between speed and accuracy)
double SigmaOfFeatureGaussian = 3.0;
int GaussianSupportSize = 37;

// Physical coordinates
double RequiredNewPointSeparation = 0.5;

// Density in the original image, below which we don't even bother to
// check whether it's a local maxima. As good as 0, in other words.
double SeedDensityThreshold = 0.05;

// A seed (local maxima)'s density must be at least this % (x 100 of
// course) of the highest seed density.
double SeedDensityWorthinessThreshold = 0.7;

// At what falloff, from a maximum, is it the end of the beta region?
// We're saying, here, when it reaches half the highest density.
// This is absolutely arbitrary and wants to be experimental.
double SeedDensityFalloff = 0.8;

// Or, a binary search?
double LineIncrement = 0.25;

// double SkeletonMergeThreshold = BetaThickness;

// Not a physical constant; maybe belongs in instrument...
unsigned SnapshotIntervalBase = 500;
unsigned SnapshotIntervalPower = 2;
unsigned FinalSnapshot = 1;

unsigned MaxPoints = 10000;

// Beta points don't have an intensity but we want to use 3D density
// methods to show them, so this is the density we give them.
double BetaPointFakeDensity = 0.1;
bool ShowSeeds = false;
double SeedsDisplayEmphFactor = 2.0;

unsigned SnapshotImageZoom = 2;

} // namespace constants
