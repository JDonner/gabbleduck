#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include "settings.h"
#include "types.h"
#include "point.h"



struct SpatialHash {
public:
   // bunch of grouped points; grid cell. Cell /contents/, that is
   // Yes, vector, because pts could be in diagonal corners. Of course,
   // we could use /that/ distance... But not, now... (hrm).
   // Eh, too much thinking, for now.
   typedef std::vector<PointType> Pts;

   // bunch of cells
   typedef std::vector<Pts*> Cells;

   struct Index {
      int operator[](unsigned i) const { return index_[i]; }
      int& operator[](unsigned i) { return index_[i]; }

      int index_[Dimension];
   };

public:
   SpatialHash();

   void init(ImageType::PointType origin,
             double grid_phys_separation,
             unsigned (&n_pixels)[Dimension],
             double (&phys_separation)[Dimension]);

   void addPt(PointType const& pt);

   bool isWithinDistanceOfAnything(PointType const& pt, double distance);

   unsigned offset_of(Index const& idx);
   Index index_of(PointType const& pt);

   Pts* pts_at(PointType const& pt);
   Pts* pts_at(Index idx);
   Pts* pts_at(int offset);

   void get_neighbors(Index ctr, Cells& neighbors);

private:
   PointType zero_offset_based(PointType const& physPt) const {
      PointType zeroed;
      zeroed[0] = physPt[0] - this->image_origin_[0];
      zeroed[1] = physPt[1] - this->image_origin_[1];
      zeroed[2] = physPt[2] - this->image_origin_[2];
      return zeroed;
   }

private:
   ImageType::PointType image_origin_;
   double total_grid_phys_extent_[Dimension];
   double cell_phys_extent_[Dimension];

   unsigned stride_0_;
   unsigned stride_1_;
   unsigned n_cells_[Dimension];
   Cells cells_;
};

/*
   pt[0] (PointType) is x
   image::SpacingType [0] is x (Guide, p41)
   origin, [0] == x (probably - Guide, p41)

   spatial grid, x (fastest-moving) == 2
 */

#endif // SPATIAL_HASH_H
