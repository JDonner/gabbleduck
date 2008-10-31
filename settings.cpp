#include "settings.h"
#include <string>
#include <fstream>

using namespace std;

// Help for boost::program_options:
//   http://www.boost.org/doc/libs/1_35_0/doc/html/program_options.html

po::variables_map g_vm;

po::variables_map& set_up_options(int argc, char** argv)
{
   // A group of options that will be allowed both on command line and
   // in config file
   po::options_description config("Configuration");
   config.add_options()
      ("help", "show options")
      ("BetaThickness", po::value<double>(&constants::BetaThickness)->default_value(5.0)->composing(),
       "BetaThickness")
      ("WindowSize", po::value<double>(&constants::WindowSize)->default_value(constants::BetaThickness)->composing(),
       "WindowSize")
      ("BetaThicknessFlex", po::value<double>(&constants::BetaThicknessFlex)->default_value(0.2)->composing(),
       "BetaThicknessFlex")
      ("RequiredNewPointSeparation", po::value<double>(&constants::RequiredNewPointSeparation)->default_value(0.5)->composing(),
       "RequiredNewPointSeparation")
      // ("SkeletonMergeThreshold", po::value<double>(&constants::SkeletonMergeThreshold)->default_value(constants::BetaThickness)->composing(),
      //  "SkeletonMergeThreshold")
      // &&& Don't we want this to be based on statistics...? A couple std deviations
      // below the mean or something?
      ("SeedDensityThreshold", po::value<double>(&constants::SeedDensityThreshold)->default_value(0.1)->composing(),
       "SeedDensityThreshold")
      ("SeedDensityFalloff", po::value<double>(&constants::SeedDensityFalloff)->default_value(0.8)->composing(),
       "SeedDensityFalloff")
      ("SeedDensityWorthinessThreshold", po::value<double>(&constants::SeedDensityWorthinessThreshold)->default_value(0.01)->composing(),
       "SeedDensityWorthinessThreshold")
      ("LineIncrement", po::value<double>(&constants::LineIncrement)->default_value(0.25)->composing(),
       "LineIncrement")
      ("SigmaOfGaussian", po::value<double>(&constants::SigmaOfGaussian)->default_value(constants::WindowSize)->composing(),
       "SigmaOfGaussian")

      ("SnapshotIntervalBase", po::value<unsigned>(&constants::SnapshotIntervalBase)->default_value(0)->composing(),
       "SnapshotIntervalBase")
      ("SnapshotIntervalPower", po::value<unsigned>(&constants::SnapshotIntervalPower)->default_value(2)->composing(),
       "SnapshotIntervalPower")
      ("FinalSnapshot", po::value<unsigned>(&constants::FinalSnapshot)->default_value(0)->composing(),
       "FinalSnapshot")

      ("FauxBetaPointDensity", po::value<double>(&constants::FauxBetaPointDensity)->default_value(0.1)->composing(),
       "FauxBetaPointDensity")
      ("SeedsEmphFactor", po::value<double>(&constants::SeedsEmphFactor)->default_value(2.0)->composing(),
       "SeedsEmphFactor")
      ("ImageZoom", po::value<unsigned>(&constants::ImageZoom)->default_value(2)->composing(),
       "ImageZoom")
      ("MaxPoints", po::value<unsigned>(&constants::MaxPoints)->default_value(10000)->composing(),
       "MaxPoints")
      ("OutputDir", po::value<string>()->default_value("output"), "output directory")
      ;

   // Hidden options, will be allowed both on command line and
   // in config file, but will not be shown to the user.
   po::options_description hidden("Present, but to the user, implicit");
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

   ifstream ifs("find-betas.cfg");
   store(parse_config_file(ifs, config_file_options), g_vm);
   notify(g_vm);


   ///////////////////////////////////////////////
   // dependent values, not user-settable

   if (not g_vm.count("WindowSize")) {
      constants::WindowSize = g_vm["BetaThickness"].as<double>();
   }

   // if (not g_vm.count("SkeletonMergeThreshold")) {
   //    constants::SkeletonMergeThreshold = g_vm["BetaThickness"].as<double>();
   // }

   if (not g_vm.count("SigmaOfGaussian")) {
      constants::SigmaOfGaussian = g_vm["WindowSize"].as<double>();
   }

   constants::BetaMin = constants::BetaThickness * (1.0 - constants::BetaThicknessFlex);
   constants::BetaMax = constants::BetaThickness * (1.0 + constants::BetaThicknessFlex);

   return g_vm;
}


void dump_settings(po::variables_map const& vm, ostream& os)
{
   os
      << "vm[SeedDensityThreshold]: " << vm["SeedDensityThreshold"].as<double>() << '\n'
      << "SeedDensityThreshold: " << constants::SeedDensityThreshold << '\n'
      << '\n'
      << "vm[WindowSize]: " << vm["WindowSize"].as<double>() << '\n'
      << "WindowSize: " << constants::WindowSize << '\n'
      << '\n'
      << "vm[SigmaOfGaussian]: " << vm["SigmaOfGaussian"].as<double>() << '\n'
      << "SigmaOfGaussian: " << constants::SigmaOfGaussian << '\n'
      << '\n'
      << "vm[BetaThickness]: " << vm["BetaThickness"].as<double>() << '\n'
      << "BetaThickness: " << constants::BetaThickness << '\n'
      << "vm[BetaThicknessFlex]: " << vm["BetaThicknessFlex"].as<double>() << '\n'
      << "BetaThicknessFlex: " << constants::BetaThicknessFlex << '\n'
      << "BetaMin: " << constants::BetaMin << '\n'
      << "BetaMax: " << constants::BetaMax << '\n'
      << '\n'
      << "vm[RequiredNewPointSeparation]: " << vm["RequiredNewPointSeparation"].as<double>() << '\n'
      << "RequiredNewPointSeparation: " << constants::RequiredNewPointSeparation << '\n'
      << '\n'
      << "vm[LineIncrement]: " << vm["LineIncrement"].as<double>() << '\n'
      << "LineIncrement: " << constants::LineIncrement << '\n'
      << '\n'
      << "vm[SeedDensityFalloff]: " << vm["SeedDensityFalloff"].as<double>() << '\n'
      << "SeedDensityFalloff: " << constants::SeedDensityFalloff << '\n'
      << "vm[SeedDensityWorthinessThreshold]: " << vm["SeedDensityWorthinessThreshold"].as<double>() << '\n'
      << "SeedDensityWorthinessThreshold: " << constants::SeedDensityWorthinessThreshold << '\n'
      << '\n'
      // << "vm[SkeletonMergeThreshold]: " << vm["SkeletonMergeThreshold"].as<double>() << '\n'
      // << "SkeletonMergeThreshold: " << constants::SkeletonMergeThreshold << '\n'
      // << '\n'
      << "vm[SnapshotIntervalBase]: " << vm["SnapshotIntervalBase"].as<unsigned>() << '\n'
      << "SnapshotIntervalBase: " << constants::SnapshotIntervalBase << '\n'
      << "vm[SnapshotIntervalPower]: " << vm["SnapshotIntervalPower"].as<unsigned>() << '\n'
      << "SnapshotIntervalPower: " << constants::SnapshotIntervalPower << '\n'
      << "vm[FinalSnapshot]: " << vm["FinalSnapshot"].as<unsigned>() << '\n'
      << "FinalSnapshot: " << constants::FinalSnapshot << '\n'
      << '\n'
      << "vm[FauxBetaPointDensity]: " << vm["FauxBetaPointDensity"].as<double>() << '\n'
      << "FauxBetaPointDensity: " << constants::FauxBetaPointDensity << '\n'
      << "vm[SeedsEmphFactor]: " << vm["SeedsEmphFactor"].as<double>() << '\n'
      << "SeedsEmphFactor: " << constants::SeedsEmphFactor << '\n'
      << '\n'
      << "vm[ImageZoom]: " << vm["ImageZoom"].as<unsigned>() << '\n'
      << "ImageZoom: " << constants::ImageZoom << '\n'
      << '\n'
      << "vm[MaxPoints]: " << vm["MaxPoints"].as<unsigned>() << '\n'
      << "MaxPoints: " << constants::MaxPoints << '\n'
      << '\n'
      << flush;
}
