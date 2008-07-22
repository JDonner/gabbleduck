#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <iosfwd>

extern unsigned n_lo_density_centers;
extern unsigned n_ok_density_centers;
extern unsigned n_non_zero_lengths;

extern unsigned n_total_visited;
extern unsigned n_far_enough_away;
extern unsigned n_planelike_nodes;
extern unsigned n_non_planelike_nodes;
extern unsigned n_beta_nodes;

extern unsigned n_total_hashed_pts;
extern unsigned n_rejected_as_too_close;

extern void dump_instrument_vars(std::ostream& os);

#endif // INSTRUMENT_H
