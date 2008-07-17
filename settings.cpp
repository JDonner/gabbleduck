#include "settings.h"
#include <string>
#include <fstream>

using namespace std;

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
   ("BetaThicknessFlex", po::value<double>(&BetaThicknessFlex)->default_value(0.3)->composing(),
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
   ("SeedDensityWorthinessThreshold", po::value<double>(&SeedDensityWorthinessThreshold)->default_value(0.01)->composing(),
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

