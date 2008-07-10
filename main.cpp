#include <itkImageFileReader.h>

#include "tracing.h"
#include "local-maxima.h"
#include "node.h"
#include "settings.h"
#include "instrument.h"

#include <iostream>
#include <iomanip>

typedef itk::ImageFileReader< InputImage > VolumeReaderType;


using namespace std;

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

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(argv[0]);
   reader->Update();

   ImageType::Pointer image = reader->GetOutput();
   Seeds seeds;
   find_seeds(image, threshhold, seeds);

//IndexType oneTestSeed = seeds[0];
//seeds.clear();
//seeds.push_back(oneTestSeed);

cout << seeds.size() << " seed regions" << endl;

   Nodes betaNodes;
   FindBetaNodes(image, seeds, betaNodes);

cout << "found: " << betaNodes.size() << " beta nodes" << endl;

dump_instrument_vars();

   return 0;
}
