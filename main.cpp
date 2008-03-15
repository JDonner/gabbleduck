#include "gabble.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  --argc, ++argv;
  BetaFinder finder;
  if (argc != 1) {
     cout << "pass in the name of a file, to get the eigenvalue images" << endl;
  }
  else {
     finder.Load(argv[0]);
     finder.SetSigma(10.0);
     finder.Execute();
  }
  return 0;
}
