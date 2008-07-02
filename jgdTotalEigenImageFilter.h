/*=========================================================================

// I believe this was drawn from:
// big/common/software/insight-3.6/InsightToolkit-3.6.0
// <Code/BasicFilters/itkSymmetricEigenAnalysisImageFilter.h>

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef jgdTotalEigenImageFilter_h
#define jgdTotalEigenImageFilter_h

#include <itkImageToImageFilter.h>
#include <itkSymmetricEigenAnalysis.h>

// - This says that the QL method (I believe!) for finding eigenvectors
// of a symmetric matrix, has them come out orthogonal and unit, ie orthonormal.
// http://mathalacarte.com/cb/mom.fcg/ch05-01.pdf?_;ydm=0;II=30


// The class is templated over the input matrix (which is expected to
// provide access to its elements with the [][] operator), matrix to
// store eigenvalues, (must provide write operations on its elements
// with the [] operator), EigenMatrix to store eigen vectors (must
// provide write access to its elements with the [][] operator).

// From the other filter:
// typedef   itk::FixedArray< double, HessianPixelType::Dimension >
//                           EigenValueArrayType;

// From the testing code (size is just to make it non-trivial):
//  typedef itk::FixedArray< double, 6 > EigenValuesArrayType;
//  typedef itk::Matrix< double, 6, 6 > EigenVectorMatrixType;

// Our input is already taken care of, namely
//   typedef   itk::HessianRecursiveGaussianImageFilter<
//                            InputImageType >              HessianFilterType;
// And we know it outputs the right Matrix type.

namespace itk
{

/** \class TotalEigenImageFilter
 *
 * \brief This filter computes one-pixel-wide edges of the input image.
 *
 * This class is parameterized over the type of the input image and the
 * type of the output image.
 *
 */
// jgd - Indeed, seems to be a matrix type. TEVectorsImage == Image<Matrix..>
template <class TInputImage, class TEValuesImage, class TEVectorsImage>
   class TotalEigenImageFilter :
      public ImageToImageFilter<TInputImage, TEValuesImage>
{
public:
   /** Standard class typedefs. */
   typedef TotalEigenImageFilter    Self;
   typedef ImageToImageFilter<TInputImage, TEValuesImage> Superclass;
   typedef SmartPointer<Self> Pointer;
   typedef SmartPointer<const Self> ConstPointer;

   typedef typename TInputImage::   PixelType InputPixelType;
   typedef typename TEValuesImage:: PixelType EValuesPixelType;
   typedef typename TEVectorsImage::PixelType EVectorsPixelType;
   typedef typename InputPixelType::ValueType InputValueType;

   typedef SymmetricEigenAnalysis<InputPixelType, EValuesPixelType, EVectorsPixelType>
      JgdCalculatorType;

   typedef typename JgdCalculatorType::EigenValueOrderType EigenValueOrderType;

   /** Method for creation through the object factory */
   itkNewMacro(Self);

   /** Run-time type information (and related methods). */
   itkTypeMacro( TotalEigenImageFilter, ImageToImageFilter );

   /** Type for input image. */
   typedef TInputImage       InputImageType;

//   /** Type for output image: Skelenton of the object.  */
//   typedef TEValuesImage     EValuesImageType;
//   typedef TEVectorsImage    EVectorsImageType;

   /** Type for the region of the input image. */
   typedef typename InputImageType::RegionType   RegionType;

   /** Type for the index of the input image. */
   typedef typename RegionType::IndexType  IndexType;

   /** Type for the index of the input image. */
   typedef typename InputImageType::PixelType PixelType ;

   /** Type for the size of the input image. */
   typedef typename RegionType::SizeType   SizeType;

   /** Pointer Type for input image. */
   typedef typename InputImageType::ConstPointer InputImagePointer;

   /** Pointer Type for the output image. */
   typedef typename TEValuesImage::Pointer EValuesImagePointer;
   typedef typename TEVectorsImage::Pointer EVectorsImagePointer;

   // // &&& Why not const?
   // EValuesImageType* GetEigenValuesImage();
   // EVectorsImageType* GetEigenVectorsImage();

   typename TEValuesImage::Pointer GetEigenValuesImage();
   typename TEVectorsImage::Pointer GetEigenVectorsImage();

   /** ImageDimension enumeration   */
   itkStaticConstMacro(InputImageDimension, unsigned,
                       TInputImage::ImageDimension );
   itkStaticConstMacro(OutputImageDimension, unsigned,
                       TEValuesImage::ImageDimension );

#ifdef ITK_USE_CONCEPT_CHECKING
   /** Begin concept checking */
   itkConceptMacro(InputHasNumericTraitsCheck,
                   (Concept::HasNumericTraits<InputValueType>));
   itkConceptMacro(SameDimensionCheck,
                   (Concept::SameDimension<InputImageDimension, OutputImageDimension>));
   /** End concept checking */
#endif

   void SetDimension( unsigned int n )
   {
      m_Calculator.SetDimension(n);
   }

   void OrderEigenValuesBy( typename JgdCalculatorType::EigenValueOrderType order )
   {
      if( order == JgdCalculatorType::OrderByMagnitude )
      {
         m_Calculator.SetOrderEigenMagnitudes( true );
      }
      else if( order == JgdCalculatorType::DoNotOrder )
      {
         m_Calculator.SetOrderEigenValues( false );
      }
   }

protected:
   TotalEigenImageFilter();
   virtual ~TotalEigenImageFilter() {}
   void PrintSelf(std::ostream& os, Indent indent) const;

   void GenerateData();

   /** Prepare data. */
   void PrepareData();

   void ComputeEigenImages();

private:
   TotalEigenImageFilter(const Self&); //purposely not implemented
   void operator=(const Self&); //purposely not implemented

private:
   JgdCalculatorType m_Calculator;

}; // end of TotalEigenImageFilter class

} //end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "jgdTotalEigenImageFilter.txx"
#endif

#endif
