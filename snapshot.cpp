#include "snapshot.h"
#include "util.h"
#include "types.h"
#include <itkImageFileWriter.h>

using namespace std;

typedef itk::ImageFileWriter<InputImageType> WriterType;

string s_snapshot_basename;
ImageType::Pointer g_snapshot_image;

void setup_snapshot_image(string basename, ImageType::Pointer model)
{
   ImageType::SpacingType spacing = model->GetSpacing();
   spacing[0] /= settings::SnapshotImageZoom;
   spacing[1] /= settings::SnapshotImageZoom;
   spacing[2] /= settings::SnapshotImageZoom;

   g_snapshot_image = ImageType::New();
   g_snapshot_image->SetSpacing( spacing );
   g_snapshot_image->SetOrigin( model->GetOrigin() );

   ImageType::RegionType const& region = model->GetLargestPossibleRegion();
   ImageType::RegionType::SizeType size = region.GetSize();

   // size is in pixels
   ImageType::SizeType doubled_size(size);
   doubled_size[0] *= settings::SnapshotImageZoom;
   doubled_size[1] *= settings::SnapshotImageZoom;
   doubled_size[2] *= settings::SnapshotImageZoom;
   g_snapshot_image->SetRegions( doubled_size );

   g_snapshot_image->Allocate();

   s_snapshot_basename = basename;
}


void add_seeds_to_snapshot(Seeds const& seeds,
                           ImageType::Pointer original_image,
                           Flt seeds_emph_factor)
{
   for (Seeds::const_iterator it = seeds.begin(), end = seeds.end();
        it != end; ++it) {
      Flt seed_density = original_image->GetPixel(*it);
      PointType physPoint;
      original_image->TransformIndexToPhysicalPoint(*it, physPoint);

      ImageType::IndexType index;

      g_snapshot_image->TransformPhysicalPointToIndex(physPoint, index);
      g_snapshot_image->SetPixel(index, seed_density * seeds_emph_factor);
   }
}


// <basename> == eg, 1AGW
string beta_output_name(string basename,
                        Flt beta_thickness,
                        Flt thickness_flex,
                        Flt sigma,
                        int gaussian_support,
                        Flt beta_falloff_factor,
                        Flt beta_density_rel_max,
                        Flt point_sep
                        )
{
   ostringstream oss;
   set_nice_numeric_format(oss);
   // &&& Maybe resolution as well, from the file itself.
   oss << basename
       << "-bt=" << beta_thickness
       << "-brange=" << thickness_flex
       << "-sig=" << sigma
       << "-supp=" << gaussian_support
       << "-bfal=" << beta_falloff_factor
       << "-bdrel=" << beta_density_rel_max
       << "-sep=" << point_sep
      ;

   return oss.str();
}


void snapshot_beta_points(Nodes const& nodes)
{
   // clear it
   g_snapshot_image->FillBuffer(0);

   for (Nodes::const_iterator it = nodes.begin(), end = nodes.end();
        it != end; ++it) {

      // &&& Hmm, maybe we can do better than this; make it truly sparse.
      ImageType::IndexType index;
      g_snapshot_image->TransformPhysicalPointToIndex((*it)->pos(), index);

      PixelType density = g_snapshot_image->GetPixel(index);
      // There is no natural beta intensity (except maybe beta-like-ness,
      // which we don't keep)
      density += settings::BetaPointDisplayFakeDensity;
      g_snapshot_image->SetPixel(index, density);
   }
}


void write_snapshot_image(string fname)
{
   WriterType::Pointer writer = WriterType::New();

   writer->SetFileName(fname.c_str());

   writer->SetInput(g_snapshot_image);
   try {
      writer->Update();
      give_wide_permissions(fname.c_str());
   }
   catch (itk::ExceptionObject &err) {
      std::cout << "ExceptionObject caught !" << std::endl;
      std::cout << err << std::endl;
      assert(false);
   }
}


void maybe_snap_image(unsigned n_betas, Nodes const& nodes)
{
   static unsigned s_iSeries = 0;
   static unsigned s_snap_at = settings::SnapshotIntervalBase;
   static time_t s_then = ::time(0);

   if (n_betas == s_snap_at) {
      ++s_iSeries;

      ostringstream oss;
      oss << s_snapshot_basename << "." << n_betas << ".vtk";

      cout << "==============================================================\n"
           << "DUMPING IMAGE: " << s_snap_at << endl;
      snapshot_beta_points(nodes);
      write_snapshot_image(s_snapshot_basename + ".vtk");

      s_snap_at = settings::SnapshotIntervalBase * unsigned(::pow(settings::SnapshotIntervalPower, s_iSeries));

      // If <FinalSnapshot> == 0 indicates infinite
      if (settings::FinalSnapshot and settings::FinalSnapshot <= s_iSeries) {
         LongEnoughException long_enough;
         throw long_enough;
      }
   }

   if (settings::MaxPoints and settings::MaxPoints <= n_betas) {
      //      ostringstream oss;
      //      oss << s_snapshot_basename << "." << n_betas << ".vtk";

      //      write_beta_point_image(nodes, oss.str());

      LongEnoughException long_enough;
      throw long_enough;
   }

   if ((n_betas % 1000) == 0) {
      time_t now = ::time(0);
      time_t elapsed = now - s_then;
      g_log << "beta pts: " << n_betas << " / " << settings::MaxPoints << "; "
            << elapsed << " secs" << endl;
      s_then = now;
   }
}

void write_vertices(Nodes const& nodes, string vertex_filename)
{
   ofstream of(vertex_filename.c_str());
   give_wide_permissions(vertex_filename.c_str());

   for (Nodes::const_iterator it = nodes.begin(), end = nodes.end();
        it != end; ++it) {
      of << (*it)->pos()[0] << " "
         << (*it)->pos()[1] << " "
         << (*it)->pos()[2] << endl;
   }
}

// for debugging
// Should go in 'instrument' I guess
void write_ray(ostream& os, PointType const& base,
               EigenValuesType const& vals,
               EigenVectorsType const& vec)
{
   os << "at:"
      << base[0] << ','
      << base[1] << ','
      << base[2] << ';'
      << "dirs:";

   for (unsigned k = 0; k < Dimension; ++k) {
      for (unsigned i = 0; i < Dimension; ++i) {
         os << vec[k][i] << ",";
      }
      os << ';';
   }

   os << "vals:";

   for (unsigned k = 0; k < Dimension; ++k) {
      os << vals[k] << ",";
   }

   os << endl;
}
