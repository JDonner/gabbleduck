/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: jgdTotalEigenImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2005/11/12 06:01:26 $
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef jgdTotalEigenImageFilter_txx
#define jgdTotalEigenImageFilter_txx

#include <iostream>

#include "jgdTotalEigenImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkNeighborhoodIterator.h>
#include <vector>

namespace itk
{

template <class TInputImage,class TEValuesImage, class TEVectorsImage>
   TotalEigenImageFilter<TInputImage,TEValuesImage, TEVectorsImage>
   ::TotalEigenImageFilter()
{

   this->SetNumberOfRequiredOutputs( 2 );

   EValuesImagePointer evalImage = EValuesImageType::New();
   this->SetNthOutput( 0, evalImage.GetPointer() );

   EVectorsImagePointer evectorImage = EVectorsImageType::New();
   this->SetNthOutput( 1, evectorImage.GetPointer() );

}

/**
 *  Return the thinning Image pointer
 */
template <class TInputImage,class TEValuesImage, class TEVectorsImage>
   typename TotalEigenImageFilter<
   TInputImage,TEValuesImage, TEVectorsImage>::EValuesImageType *
   TotalEigenImageFilter<TInputImage,TEValuesImage, TEVectorsImage>
   ::GetEigenValuesImage()
{
   return  dynamic_cast< EValuesImageType * >(
                                              this->ProcessObject::GetOutput(0) );
}


/**
 *  Return the thinning Image pointer
 */
template <class TInputImage, class TEValuesImage, class TEVectorsImage>
   typename TotalEigenImageFilter<
   TInputImage, TEValuesImage, TEVectorsImage>::EVectorsImageType *
   TotalEigenImageFilter<TInputImage, TEValuesImage, TEVectorsImage>
   ::GetEigenVectorsImage()
{
   return dynamic_cast< EVectorsImageType * >(
                                              this->ProcessObject::GetOutput(1) );
}

/**
 *  Prepare data for computation
 *  Copy the input image to the output image, changing from the input
 *  type to the output type.
 */
template <class TInputImage, class TEValuesImage, class TEVectorsImage>
   void
   TotalEigenImageFilter<TInputImage, TEValuesImage, TEVectorsImage>
   ::PrepareData()
{

   itkDebugMacro(<< "PrepareData Start");
   typename TEValuesImage::Pointer evalImage = GetEigenValuesImage();
   typename TEVectorsImage::Pointer evecImage = GetEigenVectorsImage();

   InputImagePointer  inputImage  =
      dynamic_cast<const TInputImage  *>( ProcessObject::GetInput(0) );

   evalImage->SetBufferedRegion( evalImage->GetRequestedRegion() );
   evalImage->Allocate();

   evecImage->SetBufferedRegion( evecImage->GetRequestedRegion() );
   evecImage->Allocate();


   // typename EValuesImageType::RegionType region  = evalImage->GetRequestedRegion();


   // ImageRegionConstIterator< TInputImage >  it( inputImage,  region );
   // ImageRegionIterator< TEValuesImage >  otVal( evalImage,  region );
   // ImageRegionIterator< TEVectorsImage > otVec( evecImage,  region );

   // it.GoToBegin();
   // otVal.GoToBegin();
   // otVec.GoToBegin();

   // itkDebugMacro(<< "PrepareData: Copy input to output");

   // // Copy the input to the output, changing all foreground pixels to
   // // have value 1 in the process.
   // typedef typename EValuesImageType::PixelType OutputImagePixelType;
   // while( !otVal.IsAtEnd() )
   //     {
   //     if ( it.Get() )
   //       {
   //       ot.Set( NumericTraits<OutputImagePixelType>::One );
   //       }
   //     else
   //       {
   //       ot.Set( NumericTraits<OutputImagePixelType>::Zero );
   //       }
   //     ++it;
   //     ++ot;
   //     }
   itkDebugMacro(<< "PrepareData End");
}

/**
 *  Post processing for computing thinning
 */
template <class TInputImage,class TEValuesImage, class TEVectorsImage>
   void
   TotalEigenImageFilter<TInputImage, TEValuesImage, TEVectorsImage>
   ::ComputeEigenImages()
{
   itkDebugMacro( << "ComputeEigenImages Start");
   typename TInputImage::ConstPointer inputImage = this->GetInput();
   typename TEValuesImage::Pointer evalImage = this->GetEigenValuesImage();
   typename TEVectorsImage::Pointer evecImage = this->GetEigenVectorsImage();

   typename EValuesImageType::RegionType region  = evalImage->GetRequestedRegion();

   ImageRegionConstIterator< TInputImage >  it( inputImage,  region );
   ImageRegionIterator< TEValuesImage >   otVal( evalImage,  region );
   ImageRegionIterator< TEVectorsImage >  otVec( evecImage,  region );


   it.GoToBegin();
   otVal.GoToBegin();
   otVec.GoToBegin();

   while( !it.IsAtEnd() )
   {
      // &&& Look to speed up, by passing a direct reference if possible.
      EValuesPixelType evals = otVal.Value();
      EVectorsPixelType evecs = otVec.Value();
      // return value of 0 means all eigenvalues / vectors found;
      // 1 or more, is the index of the first that could not be computed.
      unsigned bad_eigen_index =
         m_Calculator.ComputeEigenValuesAndVectors(it.Value(), evals, evecs);

      (void)bad_eigen_index;
      //    m_Functor( inputIt.Get(), one, two );
      otVal.Set(evals);
      otVec.Set(evecs);

      ++it;
      ++otVal;
      ++otVec;
   }

   itkDebugMacro( << "ComputeEigenImages End");
}

/**
 *  Generate ThinImage
 */
template <class TInputImage,class TEValuesImage, class TEVectorsImage>
   void
   TotalEigenImageFilter<TInputImage,TEValuesImage, TEVectorsImage>
   ::GenerateData()
{

   this->PrepareData();

   itkDebugMacro(<< "GenerateData: Computing Thinning Image");
   this->ComputeEigenImages();

}

/**
 *  Print Self
 */
template <class TInputImage, class TEValuesImage, class TEVectorsImage>
   void
   TotalEigenImageFilter<TInputImage, TEValuesImage, TEVectorsImage>
   ::PrintSelf(std::ostream& os, Indent indent) const
{
   Superclass::PrintSelf(os,indent);

   os << indent << "Thinning image: " << std::endl;
}

} // end namespace itk

#endif
