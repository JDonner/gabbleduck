#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include "tracing.h"
#include "local-maxima.h"
#include "node.h"
#include "settings.h"
#include "instrument.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <assert.h>
#include <time.h>


typedef itk::ImageFileReader<InputImageType> VolumeReaderType;
typedef itk::ImageFileWriter<InputImageType> WriterType;

using namespace std;

struct LongEnoughException {};


string s_snapshot_basename;
ImageType::Pointer g_snapshot_image;


string extract_basename(string fname)
{
   string::size_type dot_pos = fname.rfind(".");
   string base = fname.substr(0, dot_pos);
cout << "base: " << base << endl;
   return base;
}

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

void take_snapshot(Nodes const& nodes, string fname)
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
      take_snapshot(nodes, oss.str());

      s_snap_at = SnapshotIntervalBase * unsigned(::pow(SnapshotIntervalPower, s_iSeries));

      // If <FinalSnapshot> == 0 indicates infinite
      if (FinalSnapshot and FinalSnapshot <= s_iSeries) {
         LongEnoughException long_enough;
         throw long_enough;
      }
   }

   if (MaxPoints and MaxPoints <= n_betas) {
      ostringstream oss;
      oss << s_snapshot_basename << "." << n_betas << ".vtk";

      take_snapshot(nodes, oss.str());

      LongEnoughException long_enough;
      throw long_enough;
   }

   if ((n_betas % 1000) == 0) {
      time_t now = ::time(0);
      time_t elapsed = now - s_then;
      cout << "Progress: " << n_betas << " / " << MaxPoints << "; "
           << elapsed << " secs" << endl;
      s_then = now;
   }
}

int main(int argc, char** argv)
{
   cout << fixed << setprecision(3);

   po::variables_map& vm = set_up_options(argc, argv);

   // if (argc < 1) {
   //    cout << "pass in the name of a file, to get the eigenvalue images" << endl;
   // }

   // double threshold = SeedDensityThreshold;
   // if (2 <= argc) {
   //    threshhold = atof(argv[1]);
   // }

   string fname = vm["input-file"].as<string>();

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(fname.c_str());
   reader->Update();

   ImageType::Pointer image = reader->GetOutput();
//image->Print(cout);


dump_settings(g_vm, cout);

   Seeds allSeeds;
   find_seeds(image, SeedDensityThreshold, allSeeds);

   // find maximum seed density
   PixelType maxSeedDensity = -1.0;
   for (Seeds::const_iterator it = allSeeds.begin(), end = allSeeds.end();
        it != end; ++it) {
      PixelType seedDensity = image->GetPixel(*it);
      if (maxSeedDensity < seedDensity) {
         maxSeedDensity = seedDensity;
      }
   }

   Seeds trueMaxSeeds;
   for (Seeds::const_iterator it = allSeeds.begin(), end = allSeeds.end();
        it != end; ++it) {
      if (SeedDensityWorthinessThreshold * maxSeedDensity < image->GetPixel(*it)) {
         trueMaxSeeds.push_back(*it);
      }
   }

//IndexType oneTestSeed = seeds[0];
//seeds.clear();
//seeds.push_back(oneTestSeed);

cout << "safe seeds: " << trueMaxSeeds.size()
     << "; all seeds: " << allSeeds.size()
     << "; max seed density: " << maxSeedDensity
     << endl;

   setup_snapshot_image(extract_basename(fname), image);

   Nodes betaNodes;
   try {
      FindBetaNodes(image, trueMaxSeeds, betaNodes);
   }
   catch (LongEnoughException const& long_enough) {
      cout << "... quitting at user-defined limit (not exhaustion)" << endl;
   }

cout << "found: " << betaNodes.size() << " beta nodes" << endl;

dump_instrument_vars();

   return 0;
}
