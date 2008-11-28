#ifndef CONVOLVE_H
#define CONVOLVE_H

#include "mask_t.h"
#include <itkImageBase.h>

// Does no resampling; just straight, pixel-by-pixel
template <typename MaskComponentT, itkImageComponentT>
double convolve(itkImageBase<itkImageComponentT, 3>,
                Mask_T<MaskComponentT> const& mask,
                // one or the other
                itkImageBase<itkImageComponentT, 3>::IndexType ctrIndex,
                itkImageBase<itkImageComponentT, 3>::IndexType startIndex
                )
{
   ResultT result = 0.0;
   int size = mask->size();

   // &&& not sure of the order, though it might not matter
   int xstartimage = ctrIndex[0] - size / 2;
   int ystartimage = ctrIndex[1] - size / 2;
   int zstartimage = ctrIndex[2] - size / 2;
   // &&& check that it's right sized
   // assert that mask is odd-sized
   for (int iz = 0; iz < size; ++iz) {
      for (int iy = 0; iy < size; ++iy) {
         for (int ix = 0; ix < size; ++ix) {
            Index idx;
            idx[0] = ximagestart + ix;
            idx[1] = yimagestart + iy;
            idx[2] = zimagestart + iz;
            result += mask.at(ix, iy, iz) * image->GetPixel(idx);
         }
      }
   }

   return result;
}

#endif // CONVOLVE_H
