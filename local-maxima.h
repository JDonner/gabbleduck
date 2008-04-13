#ifndef LOCAL_MAXIMA_H
#define LOCAL_MAXIMA_H

#include <set>
#include <itkImage.h>

#include "types.h"

typedef double      InputPixel;
typedef bool        BeatenPixel;
typedef BeatenPixel UncollectedPixel;

typedef itk::Image< InputPixel, Dimension >       InputImage;
typedef itk::Image< BeatenPixel, Dimension >      BeatenImage;

typedef std::vector<IndexType> Seeds;

//typedef std::set<BeatenNit::IndexType,
typedef std::set<IndexType,
                 itk::Functor::IndexLexicographicCompare<Dimension> > PointSet;

// Called 'region' because occasionally there will be a plateau of
// equal seeds.
// By PointSet* because it's too much of a pain to define a '<' operator
// for a pointset.

typedef std::set<PointSet*> SeedRegionSet;

void find_seeds(InputImage::Pointer image,
                double threshhold,
                Seeds& outSeeds);

#endif // LOCAL_MAXIMA_H
