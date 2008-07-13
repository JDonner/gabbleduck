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
#include <assert.h>
#include <stdlib.h>


typedef itk::ImageFileReader<InputImage> VolumeReaderType;
typedef itk::ImageFileWriter<InputImage> WriterType;

using namespace std;


string s_snapshot_basename;
ImageType::Pointer g_snapshot_image;


string extract_basename(string fname)
{
   string::size_type dot_pos = fname.rfind(".");
   string base = fname.substr(0, dot_pos);
cout << "base: " << base << endl;
   return base;
}

void create_snapshot_image(string basename, ImageType::Pointer model)
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


void maybe_snap_image(unsigned n_betas, Nodes const& nodes)
{
   // clear it
   g_snapshot_image->FillBuffer(0);

   static unsigned s_iSeries = 0;
   static unsigned s_snap_at = SnapshotIntervalBase;

   if (n_betas == s_snap_at) {
      ++s_iSeries;
      cout << "==============================================================\n"
           << "DUMPED IMAGE: " << s_snap_at << endl;
      s_snap_at = SnapshotIntervalBase * unsigned(::pow(SnapshotIntervalPower, s_iSeries));

      ostringstream oss;
      oss << s_snapshot_basename << "." << s_iSeries << ".vtk";
      string snapshot_name = oss.str();

      for (Nodes::const_iterator it = nodes.begin(), end = nodes.end();
           it != end; ++it) {

         ImageType::IndexType index;
         g_snapshot_image->TransformPhysicalPointToIndex((*it)->pos(), index);

         double density = g_snapshot_image->GetPixel(index);
         // There is no natural beta intensity (except maybe beta-like-ness,
         // which we don't keep)
         density += FauxBetaPointDensity;
         g_snapshot_image->SetPixel(index, density);
      }

      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(snapshot_name.c_str());
      writer->SetInput(g_snapshot_image);
      try {
         writer->Update();
      }
      catch (itk::ExceptionObject &err) {
         std::cout << "ExceptionObject caught !" << std::endl;
         std::cout << err << std::endl;
         assert(false);
      }

      // If <FinalSnapshot> == 0 indicates infinite
      if (FinalSnapshot and FinalSnapshot <= s_iSeries) {
         ::exit(EXIT_SUCCESS);
      }
   }
}


int main(int argc, char** argv)
{
   --argc, ++argv;

   cout << fixed << setprecision(3);

   if (argc < 1) {
      cout << "pass in the name of a file, to get the eigenvalue images" << endl;
   }

   double threshhold = ScrubDensity;
   if (2 <= argc) {
      threshhold = atof(argv[1]);
   }

   string fname = argv[0];

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(fname.c_str());
   reader->Update();

   ImageType::Pointer image = reader->GetOutput();

   Seeds allSeeds;
   find_seeds(image, threshhold, allSeeds);

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
     << "; all seeds: " << allSeeds.size() << endl;

   create_snapshot_image(extract_basename(fname), image);

   Nodes betaNodes;
   FindBetaNodes(image, trueMaxSeeds, betaNodes);

cout << "found: " << betaNodes.size() << " beta nodes" << endl;

dump_instrument_vars();

   return 0;
}
