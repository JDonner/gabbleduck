//#include <vnl/vnl_matrix.h>
#include <vector>
#include <iostream>
#include <assert.h>
#include <math.h>
#include "gaussian.h"

using namespace std;

typedef vector<int> Row;


void fill_3D_gaussian(int n, double sigma,
                      GaussianMaskType* gaussian_mask);

// Gives 98% of the area under the gaussian.
// &&& True formula should be:
//   support = 2 Sigma / image spacing
// But we assume unit spacing (our case) for now.
unsigned support_of_sigma(double sigma)
{
   // <sigma> is on /one/ side, support on both. 2 x 2sigma
   unsigned support = 4 * sigma;
   if (support % 2 == 0) {
      // Make sure it's odd
      ++support;
   }
   return support;
}

GaussianMaskType* g_GaussianMask;

// There's just one per run of the program
void init_gaussian_mask(int n, double sigma)
{
   g_GaussianMask = new GaussianMaskType(n);

   fill_3D_gaussian(n, sigma, g_GaussianMask);
}


// Texts say just to sample the gaussian at our discrete points of interest.

// n and sigma should match each other, roughly;
// Note sure how to do that yet. Isn't it
// Note that this assumes that 1 pixel == 1 physical unit! (ok for us but
// bad in general).
void fill_3D_gaussian(int n, double sigma,
                      GaussianMaskType* gaussian_mask)
{
   // odd only, for no good reason
   assert(n % 2 == 1);
   // 2 PI ^ 3/2
   double factor = 1.0 / pow(2 * M_PI, 1.5);

   double two_sigma_squared = 2.0 * sigma * sigma;
   int ctr = n / 2;
   for (int z = 0; z < n; ++z) {
      for (int y = 0; y < n; ++y) {
         for (int x = 0; x < n; ++x) {
            gaussian_mask->at(x, y, z) = factor *
               ::exp(- (x-ctr)*(x-ctr) + (y-ctr)*(y-ctr) + (z-ctr)*(z-ctr) ) /
                     two_sigma_squared;
         }
      }
   }
}

// void fill_gaussian_matrix(unsigned n, vnl_matrix<double>& g)
// {
//    g.set_size(n, n);
//    vnl_matrix<double> r(1,n);
//    binomial_row(n, r);
//    vnl_matrix<double> rt = r.transpose();
//
//    g = rt * r;
// }

// double apply_gaussian(image::Pointer image, Index idxCtr, unsigned n)
// {
//    vector<double> gauss_row = make_row_gauss(n);

//    // to normalize the gaussian row
//    double sum = 0.0;
//    sum *= Dimension;

//    double convolved = 0.0;
//    // gaussian is separable (ie, can do each dim separately
//    // and sum them) so, we do 1D gaussian over each axis.
//    NeighborhoodIteratorType::RadiusType radius;
//    for (unsigned dim = 0; dim < Dimension; ++dim) {
//       radius.Fill(1);
//       radius[dim] = n;
//       typedef ConstNeighborhoodIterator<ImageType> Nit;
//       Nit nit(radius, image, image->GetRequestedRegion());
//       // &&& slow but safe; for debugging
//       nit.NeedToUseBoundaryConditionOn();

//       for (unsigned i = 0; i < n; ++i) {
//          convolved += nit.GetPixel(i) * gauss_row[i];
//       }
//    }
//    convolved /= sum;

//    return convolved;
// }


unsigned factorial(unsigned n)
{
   unsigned fact = 1;
   for (unsigned i = 1; i <= n; ++i) {
      fact *= i;
   }
   return fact;
}


unsigned num_combinations(unsigned n, unsigned k)
{
   unsigned prod = 1;
   for (unsigned i = n-k+1; i <= n; ++i) {
      prod *= i;
   }
   prod /= factorial(k);

   return prod;
}

void fill_binomial_row(unsigned n, Row& row)
{
   row.clear();
   for (unsigned k = 0; k <= n; ++k) {
      double val = num_combinations(n, k);
      row.push_back(val);

      // right hand side by symmetry
//      row(0, n-1-k) = val;
   }
   assert(row.size() % 2 == 1);
}

void print_row(Row& row)
{
   cout << "row: ";
   for (unsigned i = 0; i < row.size(); ++i) {
      cout << row[i] << ' ';
   }
   cout << endl;
}


#ifdef TESTING
int main()
{
   cout << "running tests.." << endl;
   assert(factorial(3) == 6);
   assert(factorial(4) == 24);
   assert(num_combinations(6, 0) == 1);
   assert(num_combinations(6, 1) == 6);
   assert(num_combinations(52, 5) == 2598960);

   Row row;
   fill_binomial_row(6, row);
   print_row(row);

   row.clear();
   fill_binomial_row(7, row);
   print_row(row);

   cout << "seem ok!" << endl;
}
#endif
