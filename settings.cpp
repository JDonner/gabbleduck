#include "settings.h"
#include <string>
#include <fstream>
// For debugging...
#include <iostream>

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
      // Descriptions, in particular:
      // http://www.boost.org/doc/libs/1_35_0/doc/html/program_options/overview.html#id1251632
      ("help", "show options")
      ("BetaThickness", po::value<double>(&constants::BetaThickness)->default_value(5.0)->composing(),
       "BetaThickness; the number (in what units?) of the expected beta thickness")
      ("BetaThicknessFlex", po::value<double>(&constants::BetaThicknessFlex)->default_value(0.2)->composing(),
       "BetaThicknessFlex - How flexible to be, to count a piece as being beta (&&& when though?)")
      ("RequiredNewPointSeparation", po::value<double>(&constants::RequiredNewPointSeparation)->default_value(0.5)->composing(),
       "RequiredNewPointSeparation - I think this is the grid distance. Lower means finer, but more work.")
      // &&& Don't we want this to be based on statistics...? A couple std deviations
      // below the mean or something?
      ("SeedDensityThreshold", po::value<double>(&constants::SeedDensityThreshold)->default_value(0.1)->composing(),
       "SeedDensityThreshold. Value below which, though a seed may be a local maximum, we're still not interested. &&& Should be relative, bottom third or something.")
      ("SeedDensityFalloff", po::value<double>(&constants::SeedDensityFalloff)->default_value(0.8)->composing(),
       "SeedDensityFalloff - &&& Eh?")
      ("SeedDensityWorthinessThreshold", po::value<double>(&constants::SeedDensityWorthinessThreshold)->default_value(0.01)->composing(),
       "SeedDensityWorthinessThreshold - &&& oops, what DensityThreshold then?")
      ("LineIncrement", po::value<double>(&constants::LineIncrement)->default_value(0.25)->composing(),
       "LineIncrement - we (&&& crudely) check for thickness by making constant-length advances along a line, to probe the end of a beta sheet. This is that constant. Wants binary search instead most likely.")
      ("SigmaOfGaussian", po::value<double>(&constants::SigmaOfGaussian)->default_value(constants::BetaThickness)->composing(),
       "SigmaOfGaussian - &&& Hrm, need to remember")

      ("SnapshotIntervalBase", po::value<unsigned>(&constants::SnapshotIntervalBase)->default_value(0)->composing(),
       "SnapshotIntervalBase - when you're debugging and want to see the progress of the algorithm incrementally, this is the base of base ^ power, in the number of points, that you make snapshots at")
      ("SnapshotIntervalPower", po::value<unsigned>(&constants::SnapshotIntervalPower)->default_value(2)->composing(),
       "SnapshotIntervalPower")
      ("FinalSnapshot", po::value<unsigned>(&constants::FinalSnapshot)->default_value(0)->composing(),
       "FinalSnapshot - after how many snapshots, to quit. 0=go to natural exhaustion (defunct - needed for old bug)")

      ("BetaPointFakeDensity", po::value<double>(&constants::BetaPointFakeDensity)->default_value(0.1)->composing(),
       "BetaPointFakeDensity - the fake density value to use for beta points, to let us see them in a density image")

      ("ShowSeeds", po::value<bool>(&constants::ShowSeeds)->default_value(false)->composing(),
       "ShowSeeds")
      ("SeedsDisplayEmphFactor", po::value<double>(&constants::SeedsDisplayEmphFactor)->default_value(2.0)->composing(),
       "SeedsDisplayEmphFactor - when we're highlighting the original seeds, what emphasis of their original intensity should they have?")

      ("SnapshotImageZoom", po::value<unsigned>(&constants::SnapshotImageZoom)->default_value(2)->composing(),
       "SnapshotImageZoom - Magnifies the snapshots. Why I wanted to do this I don't recall.")
      ("MaxPoints", po::value<unsigned>(&constants::MaxPoints)->default_value(10000)->composing(),
       "MaxPoints - if the thing runs forever, cut it off at this number of points (defunct, needed for old bug)")
      ("OutputDir", po::value<string>()->default_value("output"), "output directory")
      ;

   // Hidden options, will be allowed both on command line and
   // in config file, but will not be shown to the user.
   // A filename given without an option (eg ./find-seeds 1AGW.mrc)
   // well, this <input-file> gets that implicitly.
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

   if (not g_vm.count("SigmaOfGaussian")) {
      cout << "Sigma not specified!" << endl;
      constants::SigmaOfGaussian = g_vm["BetaThickness"].as<double>();
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
      << "vm[SnapshotIntervalBase]: " << vm["SnapshotIntervalBase"].as<unsigned>() << '\n'
      << "SnapshotIntervalBase: " << constants::SnapshotIntervalBase << '\n'
      << "vm[SnapshotIntervalPower]: " << vm["SnapshotIntervalPower"].as<unsigned>() << '\n'
      << "SnapshotIntervalPower: " << constants::SnapshotIntervalPower << '\n'
      << "vm[FinalSnapshot]: " << vm["FinalSnapshot"].as<unsigned>() << '\n'
      << "FinalSnapshot: " << constants::FinalSnapshot << '\n'
      << '\n'
      << "vm[BetaPointFakeDensity]: " << vm["BetaPointFakeDensity"].as<double>() << '\n'
      << "BetaPointFakeDensity: " << constants::BetaPointFakeDensity << '\n'
      << "vm[SeedsDisplayEmphFactor]: " << vm["SeedsDisplayEmphFactor"].as<double>() << '\n'
      << "SeedsDisplayEmphFactor: " << constants::SeedsDisplayEmphFactor << '\n'
      << '\n'
      << "vm[SnapshotImageZoom]: " << vm["SnapshotImageZoom"].as<unsigned>() << '\n'
      << "SnapshotImageZoom: " << constants::SnapshotImageZoom << '\n'
      << '\n'
      << "vm[MaxPoints]: " << vm["MaxPoints"].as<unsigned>() << '\n'
      << "MaxPoints: " << constants::MaxPoints << '\n'
      << '\n'
      << flush;
}
