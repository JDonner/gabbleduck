#ifndef TRACING_H
#define TRACING_H

#include "types.h"
#include "node.h"
#include "local-maxima.h"

void FindBetaNodes(ImageType::Pointer image,
                   Seeds const& seeds,
                   Nodes& outNodes);

#endif // TRACING_H
