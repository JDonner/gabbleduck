#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "types.h"
#include "node.h"
#include "local-maxima.h"
#include <string>

extern void setup_snapshot_image(std::string basename, ImageType::Pointer model);
extern void add_seeds_to_snapshot(Seeds const& seeds,
                           ImageType::Pointer original_image,
                           double seeds_emph_factor);
extern std::string beta_point_image_name(
   std::string basename,
   unsigned n_points,
   bool bExhausted,
   double beta_thickness,
   double sigma,
   double window_width,
   double beta_falloff_factor);
extern void snapshot_beta_points(Nodes const& nodes);
extern void write_snapshot_image(std::string fname);
extern void maybe_snap_image(unsigned n_betas, Nodes const& nodes);

extern std::string s_snapshot_basename;
extern ImageType::Pointer g_snapshot_image;

#endif // SNAPSHOT_H
