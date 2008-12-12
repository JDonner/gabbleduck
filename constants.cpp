#include "constants.h"

// These constants would be used if we didn't have the boost::options
// stuff.  I believe those obviate all of this, the boost framework
// gives them their own default values.  Believe those instead.

namespace constants
{
// &&& This 20% is arbitrary
float BetaThickRangeRatio = 0.2;

float BetaMin = BetaThickness * (1.0 - BetaThickRangeRatio);
float BetaMax = BetaThickness * (1.0 + BetaThickRangeRatio);

float BetaThickness = 5.0;
// Gaussian sigma of paper's formula (7)
/// paper's recommended value (as compromise between speed and accuracy)
float SigmaOfFeatureGaussian = 3.0;
int GaussianSupportSize = 5;

// Physical coordinates
float RequiredNewPointSeparation = 0.5;

// Density in the original image, below which we don't even bother to
// check whether it's a local maxima. As good as 0, in other words.
float SeedDensityThreshold = 0.05;

// A seed (local maxima)'s density must be at least this % (x 100 of
// course) of the highest seed density.
float RelativeSeedDensityThreshold = 0.7;

float CandidateDensityThreshold;

// At what falloff, from a maximum, is it the end of the beta region?
// We're saying, here, when it reaches half the highest density.
// This is absolutely arbitrary and wants to be experimental.
float SeedDensityFalloff = 0.8;

// Or, a binary search?
float LineIncrement = 0.25;

} // namespace constants
