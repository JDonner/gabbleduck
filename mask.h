#ifndef MASK_H
#define MASK_H

template <typename T>
struct Mask_T
{
   Mask_T(int size_in)
   : size(size_in)
     // round up to nearest machine word
   , y_stride(size_in)
   , z_stride(size_in * y_stride)
   , data(0)
   {
      allocate();
   }

  ~Mask_T() { delete[] this->data; }

   void allocate() { this->data = new T[this->size * this->size * this->size]; }

   bool is_valid_index(int x, int y, int z) const
   {
      return
         0 <= x and x < this->size and
         0 <= y and y < this->size and
         0 <= z and z < this->size;
   }

   T const& at(int x, int y, int z) const
   { return this->data[this->z_stride * z + this->y_stride * y + x]; }

   T& at(int x, int y, int z)
   { return this->data[this->z_stride * z + this->y_stride * y + x]; }

public:
   // We assume that it's always a cube
   const int size;

private:
   int y_stride;
   int z_stride;

   T* data;
};

#endif // MASK_H
