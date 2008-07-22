#ifndef MAIN_H
#define MAIN_H

#include "node.h"
#include <iosfwd>

void maybe_snap_image(unsigned n_betas, Nodes const& nodes);

extern std::ofstream g_log;

#endif // MAIN_H
