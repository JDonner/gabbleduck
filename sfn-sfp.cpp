#include <fstream>
#include <iostream>
#include <vector>
#include <limits>
#include <cmath>

// g++ -W -Wall -O2 -o sfn-sfp sfn-sfp.cpp

using namespace std;

struct Pt {
   float x, y, z;
};

typedef vector<Pt> Pts;
Pts carbons;
Pts vertices;

void load_pts(ifstream& in, Pts& pts)
{
   Pt pt;

   while (in) {
      // It doesn't actually matter what they are, as long as we're
      // consistent.
      in >> pt.x >> pt.y >> pt.z;
      pts.push_back(pt);
   }
   // We always run one past the end of good data, so pop the last off.
   pts.pop_back();
}



// O(n^2)
float SFX(Pts const& ys, Pts const& xs)
{
   if (ys.empty() or xs.empty()) {
      return 999.9;
   }
   else {
      float sum_ds = 0.0;
      for (Pts::const_iterator ity = ys.begin(), endy = ys.end();
           ity != endy; ++ity) {
         float nearest_dist2 = numeric_limits<float>::max();
         for (Pts::const_iterator itx = xs.begin(), endx = xs.end();
              itx != endx; ++itx) {
            float dx = ity->x - itx->x,
               dy = ity->y - itx->y,
               dz = ity->z - itx->z;
            float dist2 = dx * dx + dy * dy + dz * dz;
            if (dist2 < nearest_dist2) {
               nearest_dist2 = dist2;
            }
         }
         sum_ds += sqrtf(nearest_dist2);
      }

      return sum_ds / ys.size();
   }
}


// false negatives, ie betas that are in PDB but are undetected
float SFN(Pts const& carbons, Pts const& vertices)
{
   float sfn = SFX(carbons, vertices);
   return sfn;
}

float SFP(Pts const& vertices, Pts const& carbons)
{
   float sfp = SFX(vertices, carbons);
   return sfp;
}

// Use like:
//    ./sfn-sfp   1AGW.carbons   1AGW.<parmvals>.vertices
//
int main(int argc, char* argv[])
{
   --argc, ++argv;

   ifstream carbons_in(argv[0]);
   load_pts(carbons_in, carbons);

   ifstream vertices_in(argv[1]);
   load_pts(vertices_in, vertices);

   float sfn = SFN(carbons, vertices);
   float sfp = SFP(vertices, carbons);

   cout << "SFN " << sfn << " SFP " << sfp << endl;
}
