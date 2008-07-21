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

void dump_instrument_vars()
{
   cout << "n_total_visited: " << n_total_visited << endl;

   cout << "lo densities: " << n_lo_density_centers << endl;
   cout << "ok densities: " << n_ok_density_centers << endl;

   cout << "n_non_zero_lengths: " << n_non_zero_lengths << endl;

   cout << "n_far_enough_away: " << n_far_enough_away << endl;
   cout << "n_planelike_nodes: " << n_planelike_nodes << endl;
   cout << "n_non_planelike_nodes: " << n_non_planelike_nodes << endl;
   cout << "n_beta_nodes: " << n_beta_nodes << endl;
   cout << "n_total_hashed_pts: " << n_total_hashed_pts << endl;
   cout << "n_rejected_as_too_close: " << n_rejected_as_too_close << endl;
}
