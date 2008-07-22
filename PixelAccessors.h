/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: PixelAccessors.h,v $
  Language:  C++
  Date:      $Date: 2005/06/13 13:48:10 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __PixelAccessors_h
#define __PixelAccessors_h


// Eigenvalue pixel accessor to access vector of eigenvalue pixels
// as individual images
template< class TPixel >
   class EigenvalueAccessor
{
public:
   typedef TPixel                     InternalType;
   typedef typename TPixel::ValueType ExternalType;

   inline ExternalType Get( const InternalType & input ) const
   {
      return static_cast<ExternalType>( input[m_EigenIdx] );
   }

   void SetEigenIdx( unsigned i )
   {
      this->m_EigenIdx = i;
   }

private:
   unsigned m_EigenIdx;
};


// Eigenvalue pixel accessor to access vector of eigenvalue pixels
// as individual images
template< class TPixel, typename TExternal >
   class EigenvectorAccessor
{
public:
   typedef TPixel                     InternalType;
   // ValueType* is the row type, for which there is no nice typedef
   // (Do I know that it's valid?)

   // So writing
   // int (*a)[10];
   // declares that a is a pointer to an array of 10 ints. By constrast,
   // int *a[10];
   // declares that a is an array of 10 pointers to ints.

   // To get back to the original question, instead of writing
   // object (*ptr)[10] = new object[5][10];
   // or
   // object (*ptr)[10][15] = new object[5][10][15];

   // you must write
   // typedef object (*ObjectArray)[5][10][15];
   // ObjectArray ptr = (ObjectArray) new object[5][10][15];
   // You would also have to dereference the array whenever you refer to an element. For example,
   // (*ptr)[4][3][2] = 0;

//   typedef typename TPixel::ValueType* ExternalType;

//   typedef typename TPixel::ValueType ExternalType[3];

   // <ExternalType> is required by the adaptor filter
   typedef TExternal ExternalType;

   TExternal Get(const InternalType& input) const
   {
      // I believe this is a row, as would be correct. At least,
      // operator[] of <Matrix> returns a row in the form of T*, so
      // it looks completely correct (yay!).
      // ultimately, the type is a vnl_matrix_fixed, a T m_matrix[numrows][numcols],
      // so a pointer is all there is.
      //
      // But if we're going to copy, how do we know when to stop?
      // arg. Can we cast it to an array[3]?
      return static_cast<TExternal>( input[m_EigenIdx] );
   }

   void SetEigenIdx(unsigned i) { this->m_EigenIdx = i; }

private:
   unsigned m_EigenIdx;
};

#if 0
// // Functor to get trace of the hessian matrix (laplacian of the image )
// namespace Functor {

// template< typename TInput, typename TOutput >
//    class HessianToLaplacianFunction
// {
// public:
//    typedef typename TInput::RealValueType  RealValueType;
//    HessianToLaplacianFunction() {}
//    ~HessianToLaplacianFunction() {}

//    inline TOutput operator()( const TInput & x ) const
//    {
//       return static_cast< TOutput >( x(0,0) + x(1,1) + x(2,2) );
//    }
// };
#endif // 0

#endif // __PixelAccessors_h
