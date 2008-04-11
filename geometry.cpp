#include "types.h"
#include "pipeline.h"
#include "polygon.h"

#include <algorithm>
#include <set>
#include <cmath>
#include <vector>

using namespace std;

void MakePolygon(Points const& planar_points, Polygon& outPolygon)
{
   // find centroid -- needn't be centroid, just center-of-bounds
   // is ok, no? -- at least, I believe under the circumstances that
   // we'll be ok with it.

   PointType lo, hi;
   for (unsigned i = 0; i < Dimension; ++i) {
      lo[i] =  1e38, hi[i] = -1e38;
   }
   for (unsigned pt = 0; pt < planar_points.size(); ++pt) {
      for (unsigned dim = 0; dim < Dimension; ++dim) {
         if (planar_points[pt][dim] < lo[dim]) {
            lo[dim] = planar_points[pt][dim];
         }
         if (hi[dim] < planar_points[pt][dim]) {
            hi[dim] = planar_points[pt][dim];
         }
      }
   }

   // &&& Not sure this is on the plane! The bounds would define
   // a rectangle no (if they're all truly co-planar) and the ctr
   // would be on it, surely. Yah, should be ok.
   PointType ctr;
   for (unsigned i = 0; i < Dimension; ++i) {
      ctr[i] = (hi[i] + lo[i]) / 2.0;
   }

   typedef std::set<std::pair<InternalPrecisionType, unsigned> > VertexSet;
   // use for insertion sort
   VertexSet vertices;

   // We arbitrarily pick the first point as the axis against which
   // to measure the other pts' angles.
   PointType first = planar_points[0];
   VectorType v0 = planar_points[0] - ctr;
   InternalPrecisionType v0Norm = v0.GetNorm();

   // 0 angle with itself
   vertices.insert(std::make_pair(0.0, 0));

   for (unsigned iPt = 1; iPt < planar_points.size(); ++iPt) {
      VectorType vi = planar_points[iPt] - ctr;
      // dot product
      InternalPrecisionType cos_alpha = v0 * vi;
      // cross product
      VectorType crossed = CrossProduct(v0, vi);
      double sin_alpha = crossed.GetNorm() / (vi.GetNorm() * v0Norm);

      InternalPrecisionType angle_with_first = atan2(cos_alpha, sin_alpha);
      vertices.insert(std::make_pair(angle_with_first, iPt));
   }

//   copy(vertices.begin(), vertices.end(), back_insert_iterator<Polygon>(outPolygon));
   for (VertexSet::const_iterator it = vertices.begin(), end = vertices.end();
        it != end; ++it) {
      outPolygon.push_back(planar_points[it->second]);
   }
}


// N = ai + bj + ck;
// a(x-xp) + b(y-yp) + c(z-zp) = 0
// ax + by + cz = d
// where d = axp + byp + czp

static
// Macros, better perhaps
double x_intersect(double d, double a, double b, double y, double c, double z)
{
   double x = (d - b * y - c * z) / a;
   return x;
}

static
double y_intersect(double d, double b, double a, double x, double c, double z)
{
   double y = (d - a * x - c * z) / b;
   return y;
}

static
double z_intersect(double d, double c, double a, double x, double b, double y)
{
   double z = (d - a * x - b * y) / c;
   return z;
}


// box = x0 x1, y0 y1, z0 z1
void planes_intersection_with_box(VectorType normal, PointType const& pt,
                                  // front, lower left, and rear, upper right
                                  PointType const& lo, PointType const& hi,
                                  Points& intersections)
{
   double
      x0 = lo[0],
      y0 = lo[1],
      z0 = lo[2],
      x1 = hi[0],
      y1 = hi[1],
      z1 = hi[2];


#define X_WITHIN x0 <= x and x <= x1
#define Y_WITHIN y0 <= y and y <= y1
#define Z_WITHIN z0 <= z and z <= z1

#define CHECK_X_EDGE(Y, Z)                    \
   {                                          \
      x = x_intersect(d, a, b, Y, c, Z);      \
      if (X_WITHIN) {                         \
         PointType p;                         \
         p[0] = x, p[1] = Y, p[2] = Z;        \
         intersections.push_back(p);          \
      }                                       \
   }

#define CHECK_Y_EDGE(X, Z)                    \
   {                                          \
      y = y_intersect(d, b, a, X, c, Z);      \
      if (Y_WITHIN) {                         \
         PointType p;                         \
         p[0] = X, p[1] = y, p[2] = Z;        \
         intersections.push_back(p);          \
      }                                       \
   }

#define CHECK_Z_EDGE(X, Y)                    \
   {                                          \
      z = z_intersect(d, c, a, X, b, Y);      \
      if (Z_WITHIN) {                         \
         PointType p;                         \
         p[0] = X, p[1] = Y, p[2] = z;        \
         intersections.push_back(p);          \
      }                                       \
   }

   double a = normal[0], b = normal[1], c = normal[2];
   double d = a * pt[0] + b * pt[1] + c * pt[2];
   double x, y, z;


   // each edge can be an intersection, so we check all possibilities.

   // 12 edges

   // front face
   CHECK_X_EDGE(y0, z0); // e1
   CHECK_Y_EDGE(x1, z0); // e2
   CHECK_X_EDGE(y1, z0); // e3
   CHECK_Y_EDGE(x0, z0); // e4

   // back face
   CHECK_X_EDGE(y0, z1); // e5
   CHECK_Y_EDGE(x1, z1); // e6
   CHECK_X_EDGE(y1, z1); // e7
   CHECK_Y_EDGE(x0, z1); // e8

   // the middle
   CHECK_Z_EDGE(x0, y0); // e9
   CHECK_Z_EDGE(x1, y0); // e10
   CHECK_Z_EDGE(x0, y1); // e11
   CHECK_Z_EDGE(x1, y1); // e12

   assert(intersections.size() <= 6);
#undef CHECK_X_EDGE
#undef CHECK_Y_EDGE
#undef CHECK_Z_EDGE
#undef X_WITHIN
#undef Y_WITHIN
#undef Z_WITHIN
}


#if 0
void MakePolygon(vector v, Box box, Triangles& outTriangles)
{
   intersect with edges;

   outTriangles.push_back(triangle);
}

// This makes no sense - how can thickness deal with flatness?
// I think the eigenVALUE stuff was, 'how it's been done before,
// but is not relevant to now'.
// This eigenVECTOR stuff is all there is!
// Do we take the main
void TraceFlatness(seeds,
                   EValueImage::ConstPointer l1Image,
                   EValueImage::ConstPointer l2Image,
                   EValueImage::ConstPointer l3Image)
                   EVectorImage::ConstPointer v1Image,
                   EVectorImage::ConstPointer v2Image,
                   EVectorImage::ConstPointer v3Image)
{
   TriangleList triangles;
   // bottom right of page 9:
   // for each maximal (critical) seed point:
   // - the vector corresponding to the maximum
   // eigenVALUE - ie, eigenvector_0

   for (seeds::const_iterator itSeed = seeds.begin(), endSeed = seeds.end();
        itSeed != endSeed; ++itSeed) {
      Index idx = summat;
      double t1 = magnitude(v1Image[idx]);
      double t2 = magnitude(v2Image[idx]);
      double t3 = magnitude(v2Image[idx]);
      if (IsBeta(sheetMin, sheetMax, t1, t2, t3)) {
         Box box;
         MakePolygon(v1, box, triangles);
      }
   }

   // so now I have my starter triangles....

   // we find the edges that need continuing,
   // and continue.
   //
   // The stopping point I think, is when the 'checked point' doesn't
   // meet the eigenVALUE criterion for flatness (but you'd think
   // it would be, meets IsBeta.... - it is (whew) )

}
#endif
