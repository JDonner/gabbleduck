#include "gabble.h"

#include <iostream>

using namespace std;

/**
 *
 *   Main program
 *
 */
int main(int argc, char** argv)
{
  --argc, ++argv;
  ceExtractorConsoleBase* console = new ceExtractorConsoleBase();
  if (argc != 1)
    {
    cout << "pass in the name of a file, to get the eigenvalue images" << endl;
    }
  else
    {
    console->Load(argv[0]);

    console->SetSigma( 10.0 );
    console->Execute();

    delete console;
  }
  return 0;
}
