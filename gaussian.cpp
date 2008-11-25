#include <vnl/vnl_matrix.h>


unsigned combinations(unsigned n, unsigned k)
{
   unsigned prod = k;
   for (unsigned i = k+1; i <= n; ++i) {
      prod *= i;
   }
   prod /= k;

   return prod;
}

void fill_binomial_row(unsigned n, vnl_matrix<double>& row)
{
   for (unsigned k = 0; k < (n+1) / 2; ++k) {
      double val = combinations(n, k);
      row(0,k) = val;

      // right hand side by symmetry
      row(0,n-1-k) = val;
   }
}

void fill_gaussian_matrix(unsigned n, vnl_matrix<double>& g)
{
   g.set_size(n, n);
   vnl_matrix<double> r(1,n);
   binomial_row(n, r);
   vnl_matrix<double> rt = r.transpose();

   g = rt * r;
}

double apply_gaussian(image::Pointer image, Index idxCtr, unsigned n)
{
   vector<double> gauss_row = make_row_gauss(n);

   // to normalize the gaussian row
   double sum = 0.0;
   sum *= Dimension;

   double convolved = 0.0;
   // gaussian is separable (ie, can do each dim separately
   // and sum them) so, we do 1D gaussian over each axis.
   NeighborhoodIteratorType::RadiusType radius;
   for (unsigned dim = 0; dim < Dimension; ++dim) {
      radius.Fill(1);
      radius[dim] = n;
      typedef ConstNeighborhoodIterator<ImageType> Nit;
      Nit nit(radius, image, image->GetRequestedRegion());
      // &&& slow but safe; for debugging
      nit.NeedToUseBoundaryConditionOn();

      for (unsigned i = 0; i < n; ++i) {
         convolved += nit.GetPixel(i) * gauss_row[i];
      }
   }
   convolved /= sum;

   return convolved;
}
