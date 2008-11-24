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

void binomial_row(unsigned n, vnl_matrix<double>& row)
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
