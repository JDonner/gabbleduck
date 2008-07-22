#include "settings.h"
#include <string>
#include <fstream>

using namespace std;

po::variables_map g_vm;

po::variables_map& set_up_options(int argc, char** argv)
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
   ("RequiredNewPointSeparation", po::value<double>(&RequiredNewPointSeparation)->default_value(0.5)->composing(),
    "RequiredNewPointSeparation")
   ("SkeletonMergeThreshold", po::value<double>(&SkeletonMergeThreshold)->default_value(BetaThickness)->composing(),
    "SkeletonMergeThreshold")
      // &&& Don't we want this to be based on statistics...? A couple std deviations
      // below the mean or something?
   ("SeedDensityThreshold", po::value<double>(&SeedDensityThreshold)->default_value(0.1)->composing(),
    "SeedDensityThreshold")
   ("SeedDensityFalloff", po::value<double>(&SeedDensityFalloff)->default_value(0.8)->composing(),
    "SeedDensityFalloff")
   ("SeedDensityWorthinessThreshold", po::value<double>(&SeedDensityWorthinessThreshold)->default_value(0.01)->composing(),
    "SeedDensityWorthinessThreshold")
   ("LineIncrement", po::value<double>(&LineIncrement)->default_value(0.25)->composing(),
    "LineIncrement")
   ("SigmaOfGaussian", po::value<double>(&SigmaOfGaussian)->default_value(WindowSize)->composing(),
    "SigmaOfGaussian")

   ("SnapshotIntervalBase", po::value<unsigned>(&SnapshotIntervalBase)->default_value(0)->composing(),
    "SnapshotIntervalBase")
   ("SnapshotIntervalPower", po::value<unsigned>(&SnapshotIntervalPower)->default_value(2)->composing(),
    "SnapshotIntervalPower")
   ("FinalSnapshot", po::value<unsigned>(&FinalSnapshot)->default_value(0)->composing(),
    "FinalSnapshot")

   ("FauxBetaPointDensity", po::value<double>(&FauxBetaPointDensity)->default_value(0.1)->composing(),
    "FauxBetaPointDensity")
   ("SeedsEmphFactor", po::value<double>(&SeedsEmphFactor)->default_value(2.0)->composing(),
    "SeedsEmphFactor")
   ("ImageZoom", po::value<unsigned>(&ImageZoom)->default_value(2)->composing(),
    "ImageZoom")
   ("MaxPoints", po::value<unsigned>(&MaxPoints)->default_value(10000)->composing(),
    "MaxPoints")
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

   store(po::command_line_parser(argc, argv).
         options(cmdline_options).positional(p).run(), g_vm);

   ifstream ifs("gabble.cfg");
   store(parse_config_file(ifs, config_file_options), g_vm);
   notify(g_vm);


   // dependent values, not user-settable

   if (not g_vm.count("WindowSize")) {
      WindowSize = g_vm["BetaThickness"].as<double>();
   }

   if (not g_vm.count("SkeletonMergeThreshold")) {
      SkeletonMergeThreshold = g_vm["BetaThickness"].as<double>();
   }

   if (not g_vm.count("SigmaOfGaussian")) {
      SigmaOfGaussian = g_vm["WindowSize"].as<double>();
   }

   BetaMin = BetaThickness * (1.0 - BetaThicknessFlex);
   BetaMax = BetaThickness * (1.0 + BetaThicknessFlex);

   return g_vm;
}

void dump_settings(po::variables_map const& vm, ostream& os)
{
   os
      << "vm[SeedDensityThreshold]: " << vm["SeedDensityThreshold"].as<double>() << '\n'
      << "SeedDensityThreshold: " << SeedDensityThreshold << '\n'
      << '\n'
      << "vm[WindowSize]: " << vm["WindowSize"].as<double>() << '\n'
      << "WindowSize: " << WindowSize << '\n'
      << '\n'
      << "vm[SigmaOfGaussian]: " << vm["SigmaOfGaussian"].as<double>() << '\n'
      << "SigmaOfGaussian: " << SigmaOfGaussian << '\n'
      << '\n'
      << "vm[BetaThickness]: " << vm["BetaThickness"].as<double>() << '\n'
      << "BetaThickness: " << BetaThickness << '\n'
      << "vm[BetaThicknessFlex]: " << vm["BetaThicknessFlex"].as<double>() << '\n'
      << "BetaThicknessFlex: " << BetaThicknessFlex << '\n'
      << "BetaMin: " << BetaMin << '\n'
      << "BetaMax: " << BetaMax << '\n'
      << '\n'
      << "vm[RequiredNewPointSeparation]: " << vm["RequiredNewPointSeparation"].as<double>() << '\n'
      << "RequiredNewPointSeparation: " << RequiredNewPointSeparation << '\n'
      << '\n'
      << "vm[LineIncrement]: " << vm["LineIncrement"].as<double>() << '\n'
      << "LineIncrement: " << LineIncrement << '\n'
      << '\n'
      << "vm[SeedDensityFalloff]: " << vm["SeedDensityFalloff"].as<double>() << '\n'
      << "SeedDensityFalloff: " << SeedDensityFalloff << '\n'
      << "vm[SeedDensityWorthinessThreshold]: " << vm["SeedDensityWorthinessThreshold"].as<double>() << '\n'
      << "SeedDensityWorthinessThreshold: " << SeedDensityWorthinessThreshold << '\n'
      << '\n'
      << "vm[SkeletonMergeThreshold]: " << vm["SkeletonMergeThreshold"].as<double>() << '\n'
      << "SkeletonMergeThreshold: " << SkeletonMergeThreshold << '\n'
      << '\n'
      << "vm[SnapshotIntervalBase]: " << vm["SnapshotIntervalBase"].as<unsigned>() << '\n'
      << "SnapshotIntervalBase: " << SnapshotIntervalBase << '\n'
      << "vm[SnapshotIntervalPower]: " << vm["SnapshotIntervalPower"].as<unsigned>() << '\n'
      << "SnapshotIntervalPower: " << SnapshotIntervalPower << '\n'
      << "vm[FinalSnapshot]: " << vm["FinalSnapshot"].as<unsigned>() << '\n'
      << "FinalSnapshot: " << FinalSnapshot << '\n'
      << '\n'
      << "vm[FauxBetaPointDensity]: " << vm["FauxBetaPointDensity"].as<double>() << '\n'
      << "FauxBetaPointDensity: " << FauxBetaPointDensity << '\n'
      << "vm[SeedsEmphFactor]: " << vm["SeedsEmphFactor"].as<double>() << '\n'
      << "SeedsEmphFactor: " << SeedsEmphFactor << '\n'
      << '\n'
      << "vm[ImageZoom]: " << vm["ImageZoom"].as<unsigned>() << '\n'
      << "ImageZoom: " << ImageZoom << '\n'
      << '\n'
      << "vm[MaxPoints]: " << vm["MaxPoints"].as<unsigned>() << '\n'
      << "MaxPoints: " << MaxPoints << '\n'
      << '\n'
      << flush;
}
