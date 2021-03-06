#ifndef LOCAL_MAXIMA_H
#define LOCAL_MAXIMA_H

#include <set>
#include <itkImage.h>

#include "types.h"

typedef bool        BeatenPixel;
typedef BeatenPixel UncollectedPixel;

typedef itk::Image< BeatenPixel, Dimension >      BeatenImage;

// Seed locations are in pixels

typedef std::vector<ImageIndexType> Seeds;

//typedef std::set<BeatenNit::ImageIndexType,
typedef std::set<ImageIndexType,
                 itk::Functor::IndexLexicographicCompare<Dimension> > PointSet;

// Called 'region' because occasionally there will be a plateau of
// equal seeds.
// By PointSet* because it's too much of a pain to define a '<' operator
// for a pointset.

typedef std::set<PointSet*> SeedRegionSet;

void find_seeds(InputImageType::Pointer image,
                double threshold,
                Seeds& outSeeds);

#endif // LOCAL_MAXIMA_H
