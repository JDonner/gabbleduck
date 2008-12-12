#include "settings.h"
#include "gaussian.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;


namespace settings {

bool WithSnapshots = true;
// Not a physical constant; maybe belongs in instrument...
unsigned SnapshotIntervalBase = 500;
unsigned SnapshotIntervalPower = 2;
bool FinalSnapshot = true;

unsigned MaxPoints = 10000;

// Beta points don't have an intensity but we want to use 3D density
// methods to show them, so we give them this fake value.
float BetaPointDisplayFakeDensity = 0.1;
bool ShowSeeds = false;
float SeedsDisplayEmphFactor = 2.0;
unsigned SnapshotImageZoom = 2;

} // settings

// Help for boost::program_options:
//   http://www.boost.org/doc/libs/1_35_0/doc/html/program_options.html

po::variables_map g_vm;

po::variables_map& set_up_options(int argc, char** argv,
                                  po::options_description& config)
{
   // A group of options that will be allowed both on command line and
   // in config file
   config.add_options()
      // Descriptions, in particular:
      // http://www.boost.org/doc/libs/1_35_0/doc/html/program_options/overview.html#id1251632
      ("help", "Show these options")
      ("BetaThickness", po::value<float>(&constants::BetaThickness)->default_value(5.0)->composing(),
       "The number, in physical units (for us, Angstromgs) of the expected beta thickness")
      ("SigmaOfFeatureGaussian", po::value<float>(&constants::SigmaOfFeatureGaussian)->default_value(3.0)->composing(),
       "Sigma of 'neighbors influence' Gaussian, applied to gradient tensor.")

      ("RequiredNewPointSeparation", po::value<float>(&constants::RequiredNewPointSeparation)->default_value(0.25)->composing(),
       "The occupancy grid distance by which each new candidate point "
       "must be further from all others, in order to be "
       "processed. Lower means finer, but more work.")
      // &&& Don't we want this to be based on statistics...? A couple std deviations
      // below the mean or something?
      ("SeedDensityThreshold", po::value<float>(&constants::SeedDensityThreshold)->default_value(0.05)->composing(),
       "When finding initial seeds, don't even look at voxels below this value.")
      ("RelativeSeedDensityThreshold", po::value<float>(&constants::RelativeSeedDensityThreshold)->default_value(0.5)->composing(),
       "Proportion of highest found local-maximum seed's density, above which seeds are to be kept. ie, 0.8 means that seeds (and candidates) of density of 80% of max will be kept")

      // 'Thick' instead of '..Thickness..', to avoid a bug in Boost
      // 1.33, where one parm being a strict substring of another
      // causes it to see them as ambiguous.
      ("BetaThickRangeRatio", po::value<float>(&constants::BetaThickRangeRatio)->default_value(0.4)->composing(),
       "Given Sheet_min = BetaThickness * (1 - BetaThickRangeRatio), Sheet_max = BetaThickness * (1 + BetaThickRangeRatio)")

      ("SeedDensityFalloff", po::value<float>(&constants::SeedDensityFalloff)->default_value(0.8)->composing(),
       "The proportion of a candidate's initial density that we're willing to consider still to be 'beta' density")

      ("LineIncrement", po::value<float>(&constants::LineIncrement)->default_value(0.25)->composing(),
       "We do a crude, linear search from the candidate point outward to where we decide beta density ends. This is the increment in physical coordinates of each step. Wants to be a binary search")

      ("SnapshotIntervalBase", po::value<unsigned>(&settings::SnapshotIntervalBase)->default_value(0)->composing(),
       "When you're debugging and want to see the progress of the algorithm incrementally, this is the base of base ^ power, in the number of points, that you take snapshots at")
      ("SnapshotIntervalPower", po::value<unsigned>(&settings::SnapshotIntervalPower)->default_value(2)->composing(),
       "The power part of base ^ power, in points found, that we take snapshots at (if you specify that)")
      ("FinalSnapshot", po::value<bool>(&settings::FinalSnapshot)->default_value(false)->composing(),
       "After how many snapshots, to quit. 0=go to natural exhaustion (now defunct - used to be needed for old bug)")

      ("BetaPointDisplayFakeDensity", po::value<float>(&settings::BetaPointDisplayFakeDensity)->default_value(0.4)->composing(),
       "BetaPointDisplayFakeDensity - the fake density value to use for beta points, to let us see them in a density image")

      ("ShowSeeds", po::value<bool>(&settings::ShowSeeds)->default_value(true)->composing(),
       "Whether to include the seed points in the final image. They will be brighter than the beta points, see the ..EmphFactor below")
      ("SeedsDisplayEmphFactor", po::value<float>(&settings::SeedsDisplayEmphFactor)->default_value(2.0)->composing(),
       "When we're highlighting the original seeds, what multiple of their original intensity we want.")

      ("SnapshotImageZoom", po::value<unsigned>(&settings::SnapshotImageZoom)->default_value(2)->composing(),
       "Magnifies the snapshots. Just in case you want it")
      ("MaxPoints", po::value<unsigned>(&settings::MaxPoints)->default_value(0)->composing(),
       "The maximum number of points to run to, 0 = inf. For debugging or tuning; shouldn't be necessary")
      ("OutputDir", po::value<string>()->default_value("output"),
       "Output directory for any .vtk snapshots, .log, and .vertices files")
      ;

   // Hidden options, will be allowed both on command line and
   // in config file, but will not be shown to the user.
   // A filename given without an option (eg ./find-seeds 1AGW.mrc)
   // well, this <input-file> gets that implicitly.
   po::options_description hidden("Present, but to the user, implicit");
   hidden.add_options()
      ("inputfile", po::value<string>(), "input file")
      ;

   po::options_description cmdline_options;
   cmdline_options.add(config).add(hidden);

   po::options_description config_file_options;
   config_file_options.add(config).add(hidden);

   po::positional_options_description p;
   p.add("inputfile", -1);

   store(po::command_line_parser(argc, argv).
         options(cmdline_options).positional(p).run(), g_vm);

   ifstream ifs("find-betas.cfg");
   store(parse_config_file(ifs, config_file_options), g_vm);
   notify(g_vm);


   ///////////////////////////////////////////////
   // dependent values, not user-settable

   constants::BetaMin = constants::BetaThickness * (1.0 - constants::BetaThickRangeRatio);
   constants::BetaMax = constants::BetaThickness * (1.0 + constants::BetaThickRangeRatio);

   // We use 2x sigma (per side, total width 4x sigma
   constants::GaussianSupportSize = support_of_sigma(constants::SigmaOfFeatureGaussian);

   return g_vm;
}


void dump_settings(po::variables_map const& vm, ostream& os)
{
   os
      << "vm[SeedDensityThreshold]: " << vm["SeedDensityThreshold"].as<float>() << '\n'
      << "SeedDensityThreshold: " << constants::SeedDensityThreshold << '\n'
      << '\n'
      << "vm[SigmaOfFeatureGaussian]: " << vm["SigmaOfFeatureGaussian"].as<float>() << '\n'
      << "SigmaOfFeatureGaussian: 'Window size' of 'neighbors influence' / blur gaussian, applied to gradient tensor" << constants::SigmaOfFeatureGaussian << '\n'
      << '\n'
      << "GaussianSupportSize: size of 'support' for feature gaussian. Totally depended on sigma" << constants::GaussianSupportSize << '\n'
      << '\n'
      << "vm[BetaThickness]: " << vm["BetaThickness"].as<float>() << '\n'
      << "BetaThickness: " << constants::BetaThickness << '\n'
      << "vm[BetaThickRangeRatio]: " << vm["BetaThickRangeRatio"].as<float>() << '\n'
      << "BetaThickRangeRatio: " << constants::BetaThickRangeRatio << '\n'
      << "BetaMin: " << constants::BetaMin << '\n'
      << "BetaMax: " << constants::BetaMax << '\n'
      << '\n'
      << "vm[RequiredNewPointSeparation]: " << vm["RequiredNewPointSeparation"].as<float>() << '\n'
      << "RequiredNewPointSeparation: " << constants::RequiredNewPointSeparation << '\n'
      << '\n'
      << "vm[LineIncrement]: " << vm["LineIncrement"].as<float>() << '\n'
      << "LineIncrement: " << constants::LineIncrement << '\n'
      << '\n'
      << "vm[SeedDensityFalloff]: " << vm["SeedDensityFalloff"].as<float>() << '\n'
      << "SeedDensityFalloff: " << constants::SeedDensityFalloff << '\n'
      << "vm[RelativeSeedDensityThreshold]: " << vm["RelativeSeedDensityThreshold"].as<float>() << '\n'
      << "RelativeSeedDensityThreshold: " << constants::RelativeSeedDensityThreshold << '\n'
      << '\n'
      << "CandidateDensityThreshold: " << constants::CandidateDensityThreshold << '\n'
      << "(lowest threshold density to be considered a beta point. Calculated from <RelativeSeedDensityThreshold> above"
      << '\n'
      << "vm[SnapshotIntervalBase]: " << vm["SnapshotIntervalBase"].as<unsigned>() << '\n'
      << "SnapshotIntervalBase: " << settings::SnapshotIntervalBase << '\n'
      << "vm[SnapshotIntervalPower]: " << vm["SnapshotIntervalPower"].as<unsigned>() << '\n'
      << "SnapshotIntervalPower: " << settings::SnapshotIntervalPower << '\n'
      << "vm[FinalSnapshot]: " << vm["FinalSnapshot"].as<bool>() << '\n'
      << "FinalSnapshot: " << settings::FinalSnapshot << '\n'
      << '\n'
      << "vm[BetaPointDisplayFakeDensity]: " << vm["BetaPointDisplayFakeDensity"].as<float>() << '\n'
      << "BetaPointDisplayFakeDensity: " << settings::BetaPointDisplayFakeDensity << '\n'
      << "vm[SeedsDisplayEmphFactor]: " << vm["SeedsDisplayEmphFactor"].as<float>() << '\n'
      << "SeedsDisplayEmphFactor: " << settings::SeedsDisplayEmphFactor << '\n'
      << '\n'
      << "vm[SnapshotImageZoom]: " << vm["SnapshotImageZoom"].as<unsigned>() << '\n'
      << "SnapshotImageZoom: " << settings::SnapshotImageZoom << '\n'
      << '\n'
      << "vm[MaxPoints]: " << vm["MaxPoints"].as<unsigned>() << '\n'
      << "MaxPoints: " << settings::MaxPoints << '\n'
      << '\n'
      << flush;
}
