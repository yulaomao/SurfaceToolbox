/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.cjyx.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// DynamicModeler includes
#include "vtkCjyxDynamicModelerAppendTool.h"
#include "vtkCjyxDynamicModelerBoundaryCutTool.h"
#include "vtkCjyxDynamicModelerCurveCutTool.h"
#include "vtkCjyxDynamicModelerHollowTool.h"
#include "vtkCjyxDynamicModelerMarginTool.h"
#include "vtkCjyxDynamicModelerMirrorTool.h"
#include "vtkCjyxDynamicModelerPlaneCutTool.h"
#include "vtkCjyxDynamicModelerROICutTool.h"
#include "vtkCjyxDynamicModelerSelectByPointsTool.h"
#include "vtkCjyxDynamicModelerToolFactory.h"
#include "vtkCjyxDynamicModelerTool.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkDataObject.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
// The compression tool manager singleton.
// This MUST be default initialized to zero by the compiler and is
// therefore not initialized here.  The ClassInitialize and ClassFinalize methods handle this instance.
static vtkCjyxDynamicModelerToolFactory* vtkCjyxDynamicModelerToolFactoryInstance;


//----------------------------------------------------------------------------
// Must NOT be initialized.  Default initialization to zero is necessary.
unsigned int vtkCjyxDynamicModelerToolFactoryInitialize::Count;

//----------------------------------------------------------------------------
// Implementation of vtkCjyxDynamicModelerToolFactoryInitialize class.
//----------------------------------------------------------------------------
vtkCjyxDynamicModelerToolFactoryInitialize::vtkCjyxDynamicModelerToolFactoryInitialize()
{
  if (++Self::Count == 1)
    {
    vtkCjyxDynamicModelerToolFactory::classInitialize();
    }
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerToolFactoryInitialize::~vtkCjyxDynamicModelerToolFactoryInitialize()
{
  if (--Self::Count == 0)
    {
    vtkCjyxDynamicModelerToolFactory::classFinalize();
    }
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkCjyxDynamicModelerToolFactory* vtkCjyxDynamicModelerToolFactory::New()
{
  vtkCjyxDynamicModelerToolFactory* ret = vtkCjyxDynamicModelerToolFactory::GetInstance();
  ret->Register(nullptr);
  return ret;
}

//----------------------------------------------------------------------------
// Return the single instance of the vtkCjyxDynamicModelerToolFactory
vtkCjyxDynamicModelerToolFactory* vtkCjyxDynamicModelerToolFactory::GetInstance()
{
  if (!vtkCjyxDynamicModelerToolFactoryInstance)
    {
    // Try the factory first
    vtkCjyxDynamicModelerToolFactoryInstance = (vtkCjyxDynamicModelerToolFactory*)vtkObjectFactory::CreateInstance("vtkCjyxDynamicModelerToolFactory");
    // if the factory did not provide one, then create it here
    if (!vtkCjyxDynamicModelerToolFactoryInstance)
      {
      vtkCjyxDynamicModelerToolFactoryInstance = new vtkCjyxDynamicModelerToolFactory;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
      vtkCjyxDynamicModelerToolFactoryInstance->InitializeObjectBase();
#endif
      }
    }
  // return the instance
  return vtkCjyxDynamicModelerToolFactoryInstance;
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerToolFactory::vtkCjyxDynamicModelerToolFactory()
= default;

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerToolFactory::~vtkCjyxDynamicModelerToolFactory()
= default;

//----------------------------------------------------------------------------
void vtkCjyxDynamicModelerToolFactory::PrintSelf(ostream & os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkCjyxDynamicModelerToolFactory::classInitialize()
{
  // Allocate the singleton
  vtkCjyxDynamicModelerToolFactoryInstance = vtkCjyxDynamicModelerToolFactory::GetInstance();

  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerPlaneCutTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerHollowTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerMarginTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerMirrorTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerCurveCutTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerBoundaryCutTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerAppendTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerROICutTool>::New());
  vtkCjyxDynamicModelerToolFactoryInstance->RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerSelectByPointsTool>::New());
}

//----------------------------------------------------------------------------
void vtkCjyxDynamicModelerToolFactory::classFinalize()
{
  vtkCjyxDynamicModelerToolFactoryInstance->Delete();
  vtkCjyxDynamicModelerToolFactoryInstance = nullptr;
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerToolFactory::RegisterDynamicModelerTool(vtkSmartPointer<vtkCjyxDynamicModelerTool> tool)
{
  for (unsigned int i = 0; i < this->RegisteredTools.size(); ++i)
    {
    if (strcmp(this->RegisteredTools[i]->GetClassName(), tool->GetClassName()) == 0)
      {
      vtkWarningMacro("RegisterStreamingCodec failed: tool is already registered");
      return false;
      }
    }
  this->RegisteredTools.push_back(tool);
  return true;
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerToolFactory::UnregisterDynamicModelerToolByClassName(const std::string & className)
{
  std::vector<vtkSmartPointer<vtkCjyxDynamicModelerTool> >::iterator toolIt;
  for (toolIt = this->RegisteredTools.begin(); toolIt != this->RegisteredTools.end(); ++toolIt)
    {
    vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = *toolIt;
    if (strcmp(tool->GetClassName(), className.c_str()) == 0)
      {
      this->RegisteredTools.erase(toolIt);
      return true;
      }
    }
  vtkWarningMacro("UnRegisterStreamingCodecByClassName failed: tool not found");
  return false;
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerTool* vtkCjyxDynamicModelerToolFactory::CreateToolByClassName(const std::string & className)
{
  std::vector<vtkSmartPointer<vtkCjyxDynamicModelerTool> >::iterator toolIt;
  for (toolIt = this->RegisteredTools.begin(); toolIt != this->RegisteredTools.end(); ++toolIt)
    {
    vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = *toolIt;
    if (strcmp(tool->GetClassName(), className.c_str()) == 0)
      {
      return tool->CreateToolInstance();
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerTool* vtkCjyxDynamicModelerToolFactory::CreateToolByName(const std::string name)
{
  std::vector<vtkSmartPointer<vtkCjyxDynamicModelerTool> >::iterator toolIt;
  for (toolIt = this->RegisteredTools.begin(); toolIt != this->RegisteredTools.end(); ++toolIt)
    {
    vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = *toolIt;
    if (tool->GetName() == name)
      {
      return tool->CreateToolInstance();
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
const std::vector<std::string> vtkCjyxDynamicModelerToolFactory::GetDynamicModelerToolClassNames()
{
  std::vector<std::string> toolClassNames;
  std::vector<vtkSmartPointer<vtkCjyxDynamicModelerTool> >::iterator toolIt;
  for (toolIt = this->RegisteredTools.begin(); toolIt != this->RegisteredTools.end(); ++toolIt)
    {
    vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = *toolIt;
    toolClassNames.emplace_back(tool->GetClassName());
    }
  return toolClassNames;
}

//----------------------------------------------------------------------------
const std::vector<std::string> vtkCjyxDynamicModelerToolFactory::GetDynamicModelerToolNames()
{
  std::vector<std::string> names;
  std::vector<vtkSmartPointer<vtkCjyxDynamicModelerTool> >::iterator toolIt;
  for (toolIt = this->RegisteredTools.begin(); toolIt != this->RegisteredTools.end(); ++toolIt)
    {
    vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = *toolIt;
    names.push_back(tool->GetName());
    }
  return names;
}
