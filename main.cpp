#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include "tracing.h"
#include "local-maxima.h"
#include "node.h"
#include "settings.h"
#include "instrument.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include <stdlib.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;




typedef itk::ImageFileReader<InputImage> VolumeReaderType;
typedef itk::ImageFileWriter<InputImage> WriterType;

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

      double density = g_snapshot_image->GetPixel(index);
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

   if (n_betas == s_snap_at) {
      ++s_iSeries;
      s_snap_at = SnapshotIntervalBase * unsigned(::pow(SnapshotIntervalPower, s_iSeries));

      ostringstream oss;
      oss << s_snapshot_basename << "." << n_betas << ".vtk";

      cout << "==============================================================\n"
           << "DUMPING IMAGE: " << s_snap_at << endl;
      take_snapshot(nodes, oss.str());
      cout << "==============================================================\n";

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

   if ((n_betas % 10) == 0) {
      cout << "Progress: " << n_betas << " / " << MaxPoints << endl;
   }
}

po::variables_map set_up_options(int argc, char** argv)
{
   // Declare a group of options that will be
   // allowed both on command line and in
   // config file
   po::options_description config("Configuration");
   config.add_options()
   ("BetaThickness", po::value<double>(&BetaThickness)->default_value(5.0)->composing(),
    "BetaThickness")
   ("WindowSize", po::value<double>(&WindowSize)->default_value(BetaThickness)->composing(),
    "WindowSize")
   ("BetaThicknessFlex", po::value<double>(&BetaThicknessFlex)->default_value(0.2)->composing(),
    "BetaThicknessFlex")
   // ("BetaMin", po::value<double>(&BetaMin)->default_value(),
   //  "BetaMin")
   // ("BetaMax", po::value<double>(&BetaMax)->default_value(),
   //  "BetaMax")
   ("SkeletonMergeThreshold", po::value<double>(&SkeletonMergeThreshold)->default_value(BetaThickness)->composing(),
    "SkeletonMergeThreshold")
   ("ScrubDensity", po::value<double>(&ScrubDensity)->default_value(0.3)->composing(),
    "ScrubDensity")
   ("SeedDensityFalloff", po::value<double>(&SeedDensityFalloff)->default_value(0.8)->composing(),
    "SeedDensityFalloff")
   ("SeedDensityWorthinessThreshold", po::value<double>(&SeedDensityWorthinessThreshold)->default_value(0.8)->composing(),
    "SeedDensityWorthinessThreshold")
   ("LineIncrement", po::value<double>(&LineIncrement)->default_value(0.25)->composing(),
    "LineIncrement")
   ("GaussianSupport", po::value<double>(&GaussianSupport)->default_value(WindowSize)->composing(),
    "GaussianSupport")
   ("SnapshotIntervalBase", po::value<unsigned>(&SnapshotIntervalBase)->default_value(500)->composing(),
    "SnapshotIntervalBase")
   ("SnapshotIntervalPower", po::value<unsigned>(&SnapshotIntervalPower)->default_value(2)->composing(),
    "SnapshotIntervalPower")
   ("FauxBetaPointDensity", po::value<double>(&FauxBetaPointDensity)->default_value(0.1)->composing(),
    "FauxBetaPointDensity")
   ("ImageZoom", po::value<unsigned>(&ImageZoom)->default_value(4)->composing(),
    "ImageZoom")
   ("RequiredNewPointSeparation", po::value<double>(&RequiredNewPointSeparation)->default_value(0.5)->composing(),
    "RequiredNewPointSeparation")
   ("MaxPoints", po::value<unsigned>(&MaxPoints)->default_value(10000)->composing(),
    "MaxPoints")
   ("FinalSnapshot", po::value<unsigned>(&FinalSnapshot)->default_value(5)->composing(),
    "FinalSnapshot")
      ;

   // Hidden options, will be allowed both on command line and
   // in config file, but will not be shown to the user.
   po::options_description hidden("There but to the user, implicit");
   hidden.add_options()
      ("input-file", po::value<string>(), "input file")
      ;

   po::options_description cmdline_options;
   cmdline_options.add(config).add(hidden);

   po::options_description config_file_options;
   config_file_options.add(config).add(hidden);

   po::positional_options_description p;
   p.add("input-file", -1);

   po::variables_map vm;
   store(po::command_line_parser(argc, argv).
         options(cmdline_options).positional(p).run(), vm);

   ifstream ifs("gabble.cfg");
   store(parse_config_file(ifs, config_file_options), vm);
   notify(vm);


   // dependent values, not user-settable

   if (not vm.count("WindowSize")) {
      WindowSize = vm["BetaThickness"].as<double>();
   }

   if (not vm.count("SkeletonMergeThreshold")) {
      SkeletonMergeThreshold = vm["BetaThickness"].as<double>();
   }

   if (not vm.count("GaussianSupport")) {
      GaussianSupport = vm["WindowSize"].as<double>();
   }


   BetaMin = BetaThickness * (1.0 - BetaThicknessFlex);
   BetaMax = BetaThickness * (1.0 + BetaThicknessFlex);

   return vm;
}


int main(int argc, char** argv)
{
   cout << fixed << setprecision(3);

   po::variables_map vm = set_up_options(argc, argv);

   // if (argc < 1) {
   //    cout << "pass in the name of a file, to get the eigenvalue images" << endl;
   // }

   // double threshhold = ScrubDensity;
   // if (2 <= argc) {
   //    threshhold = atof(argv[1]);
   // }

   string fname = vm["input-file"].as<string>();

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(fname.c_str());
   reader->Update();

   ImageType::Pointer image = reader->GetOutput();

   Seeds allSeeds;
   find_seeds(image, ScrubDensity, allSeeds);

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
