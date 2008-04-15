#include <itkImageFileReader.h>

#include "tracing.h"
#include "local-maxima.h"
#include "node.h"

#include <iostream>

typedef itk::ImageFileReader< InputImage > VolumeReaderType;


using namespace std;

int main(int argc, char** argv)
{
   --argc, ++argv;

   if (argc < 1) {
      cout << "pass in the name of a file, to get the eigenvalue images" << endl;
   }

   double threshhold = 0.3;
   if (2 <= argc) {
      threshhold = atof(argv[1]);
   }

   VolumeReaderType::Pointer reader = VolumeReaderType::New();
   reader->SetFileName(argv[0]);
   reader->Update();

   ImageType::Pointer image = reader->GetOutput();
   Seeds seeds;
   find_seeds(image, threshhold, seeds);

   cout << seeds.size() << " seed regions" << endl;

   Nodes betaNodes;
   FindBetaNodes(image, seeds, betaNodes);

   cout << "found: " << betaNodes.size() << " beta nodes" << endl;

   return 0;
}
