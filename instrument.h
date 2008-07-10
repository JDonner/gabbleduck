#ifndef INSTRUMENT_H
#define INSTRUMENT_H

extern unsigned n_lo_density_centers;
extern unsigned n_ok_density_centers;
extern unsigned n_non_zero_lengths;

extern unsigned n_total_visited;
extern unsigned n_far_enough_away;
extern unsigned n_planelike_nodes;
extern unsigned n_non_planelike_nodes;
extern unsigned n_beta_nodes;

extern void dump_instrument_vars();

#endif // INSTRUMENT_H
