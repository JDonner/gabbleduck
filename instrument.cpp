#include "instrument.h"
#include <iostream>

using namespace std;

unsigned n_lo_density_centers = 0;
unsigned n_ok_density_centers = 0;
unsigned n_non_zero_lengths = 0;

unsigned n_total_visited = 0;
unsigned n_far_enough_away = 0;
unsigned n_planelike_nodes = 0;
unsigned n_non_planelike_nodes = 0;
unsigned n_beta_nodes = 0;
unsigned n_total_hashed_pts = 0;
unsigned n_rejected_as_too_close = 0;

void dump_instrument_vars(ostream& os)
{
   os << "n_total_visited: " << n_total_visited << '\n'

      << "lo densities: " << n_lo_density_centers << '\n'
      << "ok densities: " << n_ok_density_centers << '\n'

      << "n_non_zero_lengths: " << n_non_zero_lengths << '\n'

      << "n_far_enough_away: " << n_far_enough_away << '\n'
      << "n_planelike_nodes: " << n_planelike_nodes << '\n'
      << "n_non_planelike_nodes: " << n_non_planelike_nodes << '\n'
      << "n_beta_nodes: " << n_beta_nodes << '\n'
      << "n_total_hashed_pts: " << n_total_hashed_pts << '\n'
      << "n_rejected_as_too_close: " << n_rejected_as_too_close
      << endl;
}
