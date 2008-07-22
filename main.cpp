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
#include <stdio.h>
#include <assert.h>
#include <time.h>


typedef itk::ImageFileReader<InputImageType> VolumeReaderType;

using namespace std;



string extract_basename(string fname)
{
   string::size_type dot_pos = fname.rfind(".");
   string base = fname.substr(0, dot_pos);
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
   string basename = extract_basename(fname);

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(fname.c_str());
   reader->Update();

   ImageType::Pointer image = reader->GetOutput();
//image->Print(cout);

   string temp_basename = beta_point_image_name(
      basename,
      BetaThickness,
      SigmaOfGaussian,
      WindowSize,
      SeedDensityFalloff);

   string temp_logname = temp_basename + ".log";
   g_log.open(temp_logname.c_str());
   set_nice_numeric_format(g_log);

dump_settings(g_vm, g_log);

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

g_log << "safe seeds: " << trueMaxSeeds.size()
      << "; all seeds: " << allSeeds.size()
      << "; max seed density: " << maxSeedDensity
      << endl;

   setup_snapshot_image(temp_basename, image);

   Nodes betaNodes;
   bool bExhaustedNaturally = true;
   try {
      FindBetaNodes(image, trueMaxSeeds, betaNodes);
   }
   catch (LongEnoughException const& long_enough) {
      g_log << "... quitting at user-defined limit (not exhaustion)" << endl;
      bExhaustedNaturally = false;
   }

   snapshot_beta_points(betaNodes);

   // &&& could make this separate image...
   add_seeds_to_snapshot(trueMaxSeeds, image, g_vm["SeedsEmphFactor"].as<double>());

   // rename file, now that we know how it turned out
   ostringstream oss;
   oss << temp_basename << ".ex=" << bExhaustedNaturally << ".pts=" << betaNodes.size();
   string final_basename = oss.str();
   write_snapshot_image(final_basename + ".vtk");

   string final_logname = final_basename + ".log";
   ::rename(temp_logname.c_str(), final_logname.c_str());

//cout << "found: " << betaNodes.size() << " beta nodes" << endl;

dump_instrument_vars(g_log);

   return 0;
}
