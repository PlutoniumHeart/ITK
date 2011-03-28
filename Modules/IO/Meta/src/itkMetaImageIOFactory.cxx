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
#include "itkMetaImageIOFactory.h"
#include "itkCreateObjectFunction.h"
#include "itkMetaImageIO.h"
#include "itkVersion.h"

namespace itk
{
MetaImageIOFactory::MetaImageIOFactory()
{
  this->RegisterOverride( "itkImageIOBase",
                          "itkMetaImageIO",
                          "Meta Image IO",
                          1,
                          CreateObjectFunction< MetaImageIO >::New() );
}

MetaImageIOFactory::~MetaImageIOFactory()
{}

const char *
MetaImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char *
MetaImageIOFactory::GetDescription() const
{
  return "Meta ImageIO Factory, allows the loading of Meta images into insight";
}

// Undocumented API used to register during static initialization.
// DO NOT CALL DIRECTLY.

static bool MetaImageIOFactoryHasBeenRegistered;

void MetaImageIOFactoryRegister__Private(void)
{
  if( ! MetaImageIOFactoryHasBeenRegistered )
    {
    MetaImageIOFactoryHasBeenRegistered = true;
    MetaImageIOFactory::RegisterOneFactory();
    }
}

} // end namespace itk