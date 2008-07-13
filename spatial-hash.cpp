#include "spatial-hash.h"
#include <math.h>
#include <assert.h>


SpatialHash::SpatialHash()
{
   for (unsigned i = 0; i < Dimension; ++i) {
      total_grid_phys_extent_[i] = 0;
      cell_phys_extent_[i] = 0;

      n_cells_[i] = 0;
   }
   stride_0_ = 0;
   stride_1_ = 0;
}

void SpatialHash::init(ImageType::PointType origin,
                       double grid_phys_separation,
                       unsigned (&n_pixels)[Dimension],
                       double (&phys_separation)[Dimension])
{
   image_origin_ = origin;

   for (unsigned i = 0; i < Dimension; ++i) {
      this->cell_phys_extent_[i] = grid_phys_separation;
      this->n_cells_[i] = ::ceil((n_pixels[i] * phys_separation[i]) / this->cell_phys_extent_[i]);

      this->total_grid_phys_extent_[i] = this->n_cells_[i] * this->cell_phys_extent_[i];
   }

   this->stride_1_ = this->n_cells_[2];
   this->stride_0_ = this->stride_1_ * this->n_cells_[1];
   this->cells_.resize(this->n_cells_[0] * this->n_cells_[1] * this->n_cells_[2], 0);
}

void SpatialHash::addPt(PointType const& physPt)
{
   Pts* pts = this->pts_at(physPt);
   pts->push_back(physPt);
}

bool SpatialHash::isWithinDistanceOfAnything(PointType const& physPt,
                                             double distance)
{
   Index idx = index_of(physPt);
   Cells nbrs;
   get_neighbors(idx, nbrs);
   for (Cells::const_iterator itCells = nbrs.begin(), endCells = nbrs.end();
        itCells != endCells; ++itCells) {
      Pts const* pts = *itCells;
      if (not pts) {
         return false;
      }
      else {
         for (Pts::const_iterator itPts = pts->begin(), endPts = (*itCells)->end();
              itPts != endPts; ++itPts) {
            if ((*itPts).EuclideanDistanceTo<double>(physPt) < distance) {
               return true;
            }
         }
      }
   }
   return false;
}

SpatialHash::Index SpatialHash::index_of(PointType const& physPt)
{
   Index idx;
   PointType zeroed = zero_offset_based(physPt);
   for (unsigned i = 0; i < Dimension; ++i) {
      idx[i] = unsigned(zeroed[i] / this->cell_phys_extent_[i]);
   }
#if WANT_GRID_BOUNDS_CHECKING
      assert(0 <= idx[0] and idx[0] < (int)this->n_cells_[0]);
      assert(0 <= idx[1] and idx[1] < (int)this->n_cells_[1]);
      assert(0 <= idx[2] and idx[2] < (int)this->n_cells_[2]);
#endif // WANT_GRID_BOUNDS_CHECKING
   return idx;
}

unsigned SpatialHash::offset_of(Index const& idx)
{
#if WANT_GRID_BOUNDS_CHECKING
   assert(0 <= idx[0] and idx[0] < (int)this->n_cells_[0]);
   assert(0 <= idx[1] and idx[1] < (int)this->n_cells_[1]);
   assert(0 <= idx[2] and idx[2] < (int)this->n_cells_[2]);
#endif // WANT_GRID_BOUNDS_CHECKING

   unsigned offset =
      idx[0] * this->stride_0_ +
      idx[1] * this->stride_1_ +
      idx[2];

   return offset;
}

SpatialHash::Pts* SpatialHash::pts_at(PointType const& physPt)
{
   return pts_at(index_of(physPt));
}

SpatialHash::Pts* SpatialHash::pts_at(int offset)
{
#if WANT_GRID_BOUNDS_CHECKING
   assert(0 <= offset and offset < (int)this->cells_.size());
#endif // WANT_GRID_BOUNDS_CHECKING

   Pts* cell = this->cells_[offset];
   if (not cell) {
      cell = new Pts;
      this->cells_[offset] = cell;
   }

   return cell;
}

SpatialHash::Pts* SpatialHash::pts_at(Index idx)
{
   return pts_at(offset_of(idx));
}

void SpatialHash::get_neighbors(Index ctr, Cells& neighbors)
{
   Index idx;
   for (int k = -1; k <= 1; ++k) {
      idx[0] = ctr[0] + k;
      for (int j = -1; j <= 1; ++j) {
         idx[1] = ctr[1] + j;
         for (int i = -1; i <= 1; ++i) {
            idx[2] = ctr[2] + i;
            Pts* cell = this->cells_[offset_of(idx)];
            neighbors.push_back(cell);
         }
      }
   }
}
