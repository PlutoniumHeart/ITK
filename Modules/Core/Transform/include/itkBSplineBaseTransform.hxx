/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkBSplineBaseTransform_hxx
#define __itkBSplineBaseTransform_hxx

#include "itkBSplineBaseTransform.h"

#include "itkContinuousIndex.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"

namespace itk
{

// Constructor with default arguments
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::BSplineBaseTransform() : Superclass( 0 ),
  m_CoefficientImages( this->ArrayOfImagePointerGeneratorHelper() )
{

  this->m_InternalParametersBuffer = ParametersType( 0 );
  // Make sure the parameters pointer is not NULL after construction.
  this->m_InputParametersPointer = &( this->m_InternalParametersBuffer );

  // Instantiate a weights function
  this->m_WeightsFunction = WeightsFunctionType::New();

}

// Destructor
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::~BSplineBaseTransform()
{
}


// Set the parameters
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
void
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::SetIdentity()
{
  if( this->m_InputParametersPointer == &( this->m_InternalParametersBuffer ) )
    {
    // If this->m_InternalParametersBuffer is the this->m_InputParametersPointer
    this->m_InternalParametersBuffer.Fill( 0.0 );
    }
  else
    {
    // Should not be allowed to modify a const parameter set, so
    // make an internal representation that is an identity mapping
    this->m_InternalParametersBuffer.SetSize( this->GetNumberOfParameters() );
    this->m_InternalParametersBuffer.Fill( 0.0 );
    }
  this->SetParameters( this->m_InternalParametersBuffer );
  this->Modified();
}

// Set the parameters
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
void
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::SetParameters( const ParametersType & parameters )
{
  // check if the number of parameters match the
  // expected number of parameters
  if( parameters.Size() != this->GetNumberOfParameters() )
    {
    itkExceptionMacro( << "Mismatch between parameters size "
                       << parameters.Size() << " and expected number of parameters "
                       << this->GetNumberOfParameters()
                       << ( this->m_CoefficientImages[0]->GetLargestPossibleRegion().GetNumberOfPixels() == 0 ?
                            ". \nSince the size of the grid region is 0, perhaps you forgot to "
                            "SetGridRegion or SetFixedParameters before setting the Parameters."
                            : "" ) );
    }

  if( &parameters != &( this->m_InternalParametersBuffer ) )
    {
    // Clean up this->m_InternalParametersBuffer because we will
    // use an externally supplied set of parameters as the buffer
    this->m_InternalParametersBuffer = ParametersType( 0 );
    }

  // Keep a reference to the input parameters
  // directly from the calling environment.
  // this requires that the parameters persist
  // in the calling evironment while being used
  // here.
  this->m_InputParametersPointer = &parameters;

  // Wrap flat array as images of coefficients
  this->WrapAsImages();

  // Modified is always called since we just have a pointer to the
  // parameters and cannot know if the parameters have changed.
  this->Modified();
}

// Set the parameters by value
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
void
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::SetParametersByValue( const ParametersType & parameters )
{
  // check if the number of parameters match the
  // expected number of parameters
  if( parameters.Size() != this->GetNumberOfParameters() )
    {
    itkExceptionMacro( << "Mismatched between parameters size "
                       << parameters.size() << " and region size "
                       << this->GetNumberOfParameters() );
    }

  // copy parameters to this->m_InternalParametersBuffer
  this->m_InternalParametersBuffer = parameters;
  this->SetParameters( this->m_InternalParametersBuffer );
}


template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
void
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::SetFixedParametersFromTransformDomainInformation() const
{
  //  Fixed Parameters store the following information:
  //  Grid Size
  //  Grid Origin
  //  Grid Spacing
  //  Grid Direction
  //  The size of each of these is equal to NDimensions

  this->m_FixedParameters.SetSize( NDimensions * ( NDimensions + 3 ) );

  this->SetFixedParametersGridSizeFromTransformDomainInformation();
  this->SetFixedParametersGridOriginFromTransformDomainInformation();
  this->SetFixedParametersGridSpacingFromTransformDomainInformation();
  this->SetFixedParametersGridDirectionFromTransformDomainInformation();

  this->Modified();
}


// Wrap flat parameters as images
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
void
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::WrapAsImages()
{
  /**
   * Wrap flat parameters array into SpaceDimension number of ITK images
   * NOTE: For efficiency, parameters are not copied locally. The parameters
   * are assumed to be maintained by the caller.
   */
  PixelType *dataPointer = const_cast<PixelType *>(
    this->m_InputParametersPointer->data_block() );
  const NumberOfParametersType numberOfPixels = this->GetNumberOfParametersPerDimension();

  for( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    this->m_CoefficientImages[j]->GetPixelContainer()->
      SetImportPointer( dataPointer + j * numberOfPixels, numberOfPixels );
    }
}

// Get the parameters
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
const typename BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>::ParametersType &
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::GetParameters() const
{
  /** NOTE: For efficiency, this class does not keep a copy of the parameters -
   * it just keeps pointer to input parameters.
   */
  if( this->m_InputParametersPointer == NULL )
    {
    itkExceptionMacro(
      << "Cannot GetParameters() because this->m_InputParametersPointer is NULL." );
    }
  return *( this->m_InputParametersPointer );
}

// Get the parameters
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
const typename BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>::ParametersType &
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::GetFixedParameters() const
{
  // HACK:  This should not be necessary if the
  //       class is kept in a consistent state
  //  this->SetFixedParametersFromCoefficientImageInformation();
  return this->m_FixedParameters;
}

// Print self
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
void
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::PrintSelf( std::ostream & os, Indent indent ) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "CoefficientImage: [ ";
  for( unsigned int j = 0; j < SpaceDimension - 1; j++ )
    {
    os << this->m_CoefficientImages[j].GetPointer() << ", ";
    }
  os << this->m_CoefficientImages[SpaceDimension - 1].GetPointer()
     << " ]" << std::endl;

  os << indent << "InputParametersPointer: "
     << this->m_InputParametersPointer << std::endl;
}


/** Get Jacobian at a point. A very specialized function just for BSplines */
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
void
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::ComputeJacobianFromBSplineWeightsWithRespectToPosition(
  const InputPointType & point, WeightsType & weights,
  ParameterIndexArrayType & indexes ) const
{
  ContinuousIndexType index;

  this->m_CoefficientImages[0]->TransformPhysicalPointToContinuousIndex( point, index );

  // NOTE: if the support region does not lie totally within the grid
  // we assume zero displacement and return the input point
  if( !this->InsideValidRegion( index ) )
    {
    weights.Fill( 0.0 );
    indexes.Fill( 0 );
    return;
    }

  // Compute interpolation weights
  IndexType supportIndex;
  this->m_WeightsFunction->Evaluate(index, weights, supportIndex);

  // For each dimension, copy the weight to the support region
  RegionType supportRegion;
  SizeType   supportSize;
  supportSize.Fill( SplineOrder + 1 );
  supportRegion.SetSize( supportSize );
  supportRegion.SetIndex( supportIndex );
  unsigned long counter = 0;

  typedef ImageRegionIterator<ImageType> IteratorType;

  IteratorType coeffIterator = IteratorType(
      this->m_CoefficientImages[0], supportRegion );
  const ParametersValueType *basePointer =
    this->m_CoefficientImages[0]->GetBufferPointer();
  while( !coeffIterator.IsAtEnd() )
    {
    indexes[counter] = &( coeffIterator.Value() ) - basePointer;

    // go to next coefficient in the support region
    ++counter;
    ++coeffIterator;
    }
}

template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
unsigned int
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::GetNumberOfAffectedWeights() const
{
  return this->m_WeightsFunction->GetNumberOfWeights();
}

// This helper class is used to work around a race condition where the dynamically
// generated images must exist before the references to the sub-sections are created.
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
typename BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>::CoefficientImageArray
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::ArrayOfImagePointerGeneratorHelper(void) const
{
  CoefficientImageArray tempArrayOfPointers;

  for( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    tempArrayOfPointers[j] = ImageType::New();
    }
  return tempArrayOfPointers;
}

// Transform a point
template <class TScalarType, unsigned int NDimensions, unsigned int VSplineOrder>
typename BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::OutputPointType
BSplineBaseTransform<TScalarType, NDimensions, VSplineOrder>
::TransformPoint(const InputPointType & point) const
{
  WeightsType             weights( this->m_WeightsFunction->GetNumberOfWeights() );
  ParameterIndexArrayType indices( this->m_WeightsFunction->GetNumberOfWeights() );
  OutputPointType         outputPoint;
  bool                    inside;

  this->TransformPoint( point, outputPoint, weights, indices, inside );

  return outputPoint;
}

} // namespace
#endif