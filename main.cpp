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

// From 'base' from 'base.ext'
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

   string fname = vm["input-file"].as<string>();
   string output_dir = vm["OutputDir"].as<string>();
   string basename = extract_basename(fname);
   // Only the extensions are lacking
   string basepath = output_dir + "/" + basename;

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(fname.c_str());
   reader->Update();

   ImageType::Pointer image = reader->GetOutput();

   string temp_basepath = beta_output_name(
      basepath,
      constants::BetaThickness,
      constants::SigmaOfGaussian,
      constants::WindowSize,
      constants::SeedDensityFalloff);


   string temp_logname = temp_basepath + ".log";
   g_log.open(temp_logname.c_str());
   set_nice_numeric_format(g_log);

dump_settings(g_vm, g_log);

   Seeds allSeeds;
   find_seeds(image, constants::SeedDensityThreshold, allSeeds);

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
      if (constants::SeedDensityWorthinessThreshold * maxSeedDensity < image->GetPixel(*it)) {
         trueMaxSeeds.push_back(*it);
      }
   }

g_log << "safe seeds: " << trueMaxSeeds.size()
      << "; all seeds: " << allSeeds.size()
      << "; max seed density: " << maxSeedDensity
      << endl;


#if WANT_SNAPSHOTS
   setup_snapshot_image(temp_basepath, image);
#endif

   Nodes betaNodes;
   bool bExhaustedNaturally = true;
   try {
      FindBetaNodes(image, trueMaxSeeds, betaNodes);
   }
   catch (LongEnoughException const& long_enough) {
      g_log << "... quitting at user-defined limit (not exhaustion)" << endl;
      bExhaustedNaturally = false;
   }

#if WANT_SNAPSHOTS
   snapshot_beta_points(betaNodes);
#endif

//   // &&& could make this separate image...
//   add_seeds_to_snapshot(trueMaxSeeds, image, g_vm["SeedsEmphFactor"].as<double>());

   // rename file, now that we know how it turned out
   ostringstream oss;
   oss << temp_basepath << "-pts=" << betaNodes.size();
   string final_basepath = oss.str();

#if WANT_SNAPSHOTS
   write_snapshot_image(final_basepath + ".vtk");
#endif

   write_vertices(betaNodes, final_basepath + ".vertices");

   string final_logname = final_basepath + ".log";
   ::rename(temp_logname.c_str(), final_logname.c_str());

//cout << "found: " << betaNodes.size() << " beta nodes" << endl;

dump_instrument_vars(g_log);

   return 0;
}
