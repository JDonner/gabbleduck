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

   EValuesImagePointer evalImage = TEValuesImage::New();
   this->SetNthOutput( 0, evalImage.GetPointer() );

   EVectorsImagePointer evectorImage = TEVectorsImage::New();
   this->SetNthOutput( 1, evectorImage.GetPointer() );
}

/**
 *  Return the thinning Image pointer
 */
template <class TInputImage,class TEValuesImage, class TEVectorsImage>
   typename TEValuesImage::Pointer
   TotalEigenImageFilter<TInputImage, TEValuesImage, TEVectorsImage>
   ::GetEigenValuesImage()
{
   typename TEValuesImage::Pointer p(
      dynamic_cast<TEValuesImage*>(
         this->ProcessObject::GetOutput(0)));

   return p;
}


/**
 *  Return the thinning Image pointer
 */
template <class TInputImage, class TEValuesImage, class TEVectorsImage>
   typename TEVectorsImage::Pointer
   TotalEigenImageFilter<TInputImage, TEValuesImage, TEVectorsImage>
   ::GetEigenVectorsImage()
{
   typename TEVectorsImage::Pointer p(
      dynamic_cast<TEVectorsImage*>(
         (this->ProcessObject::GetOutput(1))));

   return p;
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

   typename TEValuesImage::RegionType region  = evalImage->GetRequestedRegion();

   ImageRegionConstIterator< TInputImage >  it( inputImage,  region );
   ImageRegionIterator< TEValuesImage >   otVal( evalImage,  region );
   ImageRegionIterator< TEVectorsImage >  otVec( evecImage,  region );


   it.GoToBegin();
   otVal.GoToBegin();
   otVec.GoToBegin();

   while (!it.IsAtEnd())
   {
      // &&& Look to speed up, by passing a direct reference if possible.
      EValuesPixelType evals = otVal.Value();
      EVectorsPixelType evecs = otVec.Value();
      // return value of 0 means all eigenvalues / vectors found;
      // 1 or more, is the index of the first that could not be computed.
      unsigned bad_eigen_index =
         m_Calculator.ComputeEigenValuesAndVectors(it.Value(), evals, evecs);

      (void)bad_eigen_index;
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
