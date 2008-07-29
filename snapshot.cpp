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
   spacing[0] /= ImageZoom;
   spacing[1] /= ImageZoom;
   spacing[2] /= ImageZoom;

   g_snapshot_image = ImageType::New();
   g_snapshot_image->SetSpacing( spacing );
   g_snapshot_image->SetOrigin( model->GetOrigin() );

   ImageType::RegionType const& region = model->GetLargestPossibleRegion();
   ImageType::RegionType::SizeType size = region.GetSize();

   // size is in pixels
   ImageType::SizeType doubled_size(size);
   doubled_size[0] *= ImageZoom;
   doubled_size[1] *= ImageZoom;
   doubled_size[2] *= ImageZoom;
   g_snapshot_image->SetRegions( doubled_size );

   g_snapshot_image->Allocate();

   s_snapshot_basename = basename;
}


void add_seeds_to_snapshot(Seeds const& seeds,
                           ImageType::Pointer original_image,
                           double seeds_emph_factor)
{
   for (Seeds::const_iterator it = seeds.begin(), end = seeds.end();
        it != end; ++it) {
      double seed_density = original_image->GetPixel(*it);
      PointType physPoint;
      original_image->TransformIndexToPhysicalPoint(*it, physPoint);

      ImageType::IndexType index;

      g_snapshot_image->TransformPhysicalPointToIndex(physPoint, index);
      g_snapshot_image->SetPixel(index, seed_density * seeds_emph_factor);
   }
}


// <basename> == eg, 1AGW
string beta_point_image_name(string basename,
                             double beta_thickness,
                             double sigma,
                             double window_width,
                             double beta_falloff_factor
                             )
{
   ostringstream oss;
   set_nice_numeric_format(oss);
   oss << basename
       << ".bt=" << beta_thickness
       << ".sig=" << sigma
       << ".wnd=" << window_width
       << ".bfal=" << beta_falloff_factor
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
      density += FauxBetaPointDensity;
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
   static unsigned s_snap_at = SnapshotIntervalBase;
   static time_t s_then = ::time(0);

   if (n_betas == s_snap_at) {
      ++s_iSeries;

      ostringstream oss;
      oss << s_snapshot_basename << "." << n_betas << ".vtk";

      cout << "==============================================================\n"
           << "DUMPING IMAGE: " << s_snap_at << endl;
      snapshot_beta_points(nodes);
      write_snapshot_image(s_snapshot_basename + ".vtk");

      s_snap_at = SnapshotIntervalBase * unsigned(::pow(SnapshotIntervalPower, s_iSeries));

      // If <FinalSnapshot> == 0 indicates infinite
      if (FinalSnapshot and FinalSnapshot <= s_iSeries) {
         LongEnoughException long_enough;
         throw long_enough;
      }
   }

   if (MaxPoints and MaxPoints <= n_betas) {
      //      ostringstream oss;
      //      oss << s_snapshot_basename << "." << n_betas << ".vtk";

      //      write_beta_point_image(nodes, oss.str());

      LongEnoughException long_enough;
      throw long_enough;
   }

   if ((n_betas % 1000) == 0) {
      time_t now = ::time(0);
      time_t elapsed = now - s_then;
      g_log << "Progress: " << n_betas << " / " << MaxPoints << "; "
            << elapsed << " secs" << endl;
      s_then = now;
   }
}