// Crap - we do for /whole images/ what we want to do only for a single point
// (and a few neighboring points).
#if PSEUDO_CODE

struct XForm {
   // cell's lower, left, front corner.
   // we could recalculate these, but why risk the inaccuracy?
   double x0, y0, z0;
   // absolute? why not?
   double xoff, yoff, zoff;
};

// probably continuous index
struct PointPos
{
   offset from prev;
   position in original coords;
};

struct BetaPoint
{
   PointPos pos;
   // (or triangle list)
   Polygon polygon;
   Explore();
};

// don't come nearer than some distance from any other known point...
struct BetaTree
{
   // polygon, position, etc.
   BetaPoint beta-point;

   vector<BetaTree*> children;
};


void per-cube()
{
   typedef vector<point> Intersections;
   point x0_cube, y0_cube, z0_cube;

   // find intersections;
   planes_intersection_with_box(VectorType const& normal, point pt,
                                Intersections& intersections);

   // oh wait.. (no. was going to say, all xforms will be at 0.5 cell
   // intervals, but not so, I don't believe.. hm...)
   push make-polygon(bunch of intersections);

   for (Intersections::const_iterator it = intersections.begin(),
           end = intersections.end();
        it != end; ++it) {
      tranformation_frame xform(x0_cube, y0_cube, z0_cube,
                                it->xoff, it->yoff, it->zoff);
      // &&& vy expensive!
      ImageType::ConstPointer checked_point_image = resample-at-xform(xform, image);

      if (IsBeta(checked_point, checked_point_image)) {
      }
   }
}


Polygon MakePolygon(v normal, points planar-points)
{
   // find centroid -- needn't be centroid, just center-of-bounds
   // is ok, no? -- at least, I believe under the circumstances that
   // we'll be ok with it.

   find bounds of points;
   ctr = center of bounds;

   vector<pair<angle-type, point-no> > unordered;

   for (each point after the first) {
      cos_alpha = calc dot product with first;
      sin_alpha = calc cross product with first;

      angle-with-first = atan2(cos_alpha, sin_alpha);
      unordered.push_back(angle-with-first, point-no);
   }

   -- now we have a polygon;
}


// N = ai + bj + ck;
// a(x-xp) + b(y-yp) + c(z-zp) = 0
// ax + by + cz = d
// where d = axp + byp + czp


// Macros, better perhaps
double x_intersect(double d, double b, double y, double c, double z)
{
   x = (d - b * y - c * z) / c;
   return x;
}

double y_intersect(double d, double a, double x, double c, double z)
{
   y = (d - a * x - c * z) / b;
   return y;
}

double z_intersect(double d, double a, double x, double b, double y)
{
   z = (d - a * x - b * y) / c;
   return z;
}

// box = x0 x1, y0 y1, z0 z1
void planes_intersection_with_box(vector normal, point pt,
                                  vector<point>& intersections)
{
#define X_WITHIN(x, x0, x1) x0 <= x and x <= x1
#define Y_WITHIN(y, y0, y1) y0 <= y and y <= y1
#define Z_WITHIN(z, z0, z1) z0 <= z and z <= z1

#define CHECK_X_EDGE(Y, Z)                    \
   {                                          \
   x = x_intersect(d, b, Y, c, Z);            \
   if (X_WITHIN)                              \
      intersections.push_back(point(x, Y, Z));\
   }

#define CHECK_Y_EDGE(X, Z)                    \
   {                                          \
   y = y_intersect(d, a, X, c, Z);            \
   if (Y_WITHIN)                              \
      intersections.push_back(point(X, y, Z));\
   }

#define CHECK_Z_EDGE(X, Y)                    \
   {                                          \
   x = x_intersect(d, b, Y, c, Z);            \
   if (Z_WITHIN)                              \
      intersections.push_back(point(X, Y, z));\
   }

   d = a * pt.x + b * pt.y + c * pt.z;

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

// /big/common/software/insight/InsightToolkit-3.4.0/Testing/Code/Common/itkTranslationTransformTest.cxx
ImageType::ConstPointer
   resample_image(XForm xform, ImageType::ConstPointer image)
{
   typedef itk::TranslationTransform< PixelType, Dimensions > TransformType;

   TransformType::Pointer  id3 = TransformType::New();
   VectorType                   offset;

   // Create and show a simple 2D transform from given parameters
   offset[0] = xform.xoff;
   offset[1] = xform.yoff;
   offset[2] = xform.zoff;
   TransformType::Pointer aff2 = TransformType::New();
   aff2->SetOffset(offset);

   ResampleFilterType resampler;
   resampler.SetTransform(transform);

   resampler.SetInput(image);
   resampler.Update();

   ImageType::ConstPointer outImage = resampler.GetOutput();
   return outImage;
}

// cross-product(v a, v b)
// {
//    v cross;
//    cross.i = + a.j * b.k - b.j * a.k;
//    cross.j = - a.i * b.k - b.i * a.k;
//    cross.k = + a.i * b.j - b.i * a.j;
// }

#endif

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
