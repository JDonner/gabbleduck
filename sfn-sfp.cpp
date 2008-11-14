#include <fstream>
#include <iostream>
#include <vector>
#include <limits>

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
      in >> pts.x >> pts.y >> pts.z;
   }
}


# Big, fat n^2
float SFX(Pts const& ys, Pts const& xs)
{
    # 1st elt is distance, 2nd is index of winning vertex
    x_closest_y = [i for i in itertools.repeat(None, len(ys))]
    print >> sys.stderr, "%d ys, %d xs" % (len(ys), len(xs))
    for (Pts::const_iterator ity = ys.begin, endy = ys.end();
         ity != endy; ++ity) {
    for (iy, y) in enumerate(ys):
        nearest_d2 = numeric_limits<float>::max;
        for (Pts::const_iterator itx = xs.begin, endx = xs.end();
            itx != endx; ++itx) {

        for (ix, x) in enumerate(xs):
            d2 = euclidean_dist_2(y, x)
            if (d2 < nearest_d2):
                x_closest_y[iy] = d2
                nearest_d2 = d2
    # ... something...
    sfx = sum(map(math.sqrt, x_closest_y))
    sfx /= len(x_closest_y)
    return sfx
                   }

def SFN(carbons, vertices):
    sfn = SFX(carbons, vertices)
    return sfn


def SFP(vertices, carbons):
    sfp = SFX(vertices, carbons)
    return sfp


int main(int argc, char* argv[])
{
   --argc, ++argv;

   ifstream carbons_in(argv[0]);
   load_pts(carbons_in, carbons);

   ifstream vertices_in(argv[1]);
   load_pts(vertices_in, vertices);
}
