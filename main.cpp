#include <itkImageFileReader.h>

#include "tracing.h"
#include "local-maxima.h"
#include "node.h"
#include "settings.h"
#include "instrument.h"
#include "snapshot.h"
#include "util.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <time.h>


typedef itk::ImageFileReader<InputImageType> VolumeReaderType;

using namespace std;


ofstream g_log;


string extract_basename(string fname)
{
   string::size_type dot_pos = fname.rfind(".");
   string base = fname.substr(0, dot_pos);
cout << "base: " << base << endl;
   return base;
}

int main(int argc, char** argv)
{
   set_nice_numeric_format(cout);

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

   string basename = extract_basename(fname);
   setup_snapshot_image(basename, image);

   Nodes betaNodes;
   bool bExhaustedNaturally = true;
   try {
      FindBetaNodes(image, trueMaxSeeds, betaNodes);
   }
   catch (LongEnoughException const& long_enough) {
      cout << "... quitting at user-defined limit (not exhaustion)" << endl;
      bExhaustedNaturally = false;
   }

   string out_basename = beta_point_image_name(
      basename,
      betaNodes.size(),
      bExhaustedNaturally,
      BetaThickness,
      SigmaOfGaussian,
      WindowSize,
      SeedDensityFalloff);

   snapshot_beta_points(betaNodes);

   add_seeds_to_snapshot(trueMaxSeeds, image, g_vm["SeedsEmphFactor"].as<double>());

   write_snapshot_image(out_basename + ".vtk");

   string log_fname = out_basename + ".log";
   g_log.open(log_fname.c_str());

//cout << "found: " << betaNodes.size() << " beta nodes" << endl;

dump_instrument_vars(g_log);

   return 0;
}
