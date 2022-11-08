/*==============================================================================

  Program: 3D Cjyx

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

// DynamicModeler DMML includes
#include "vtkDMMLDynamicModelerNode.h"

// DMML includes
#include <vtkDMMLScene.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
vtkDMMLNodeNewMacro(vtkDMMLDynamicModelerNode);

//----------------------------------------------------------------------------
vtkDMMLDynamicModelerNode::vtkDMMLDynamicModelerNode()
= default;

//----------------------------------------------------------------------------
vtkDMMLDynamicModelerNode::~vtkDMMLDynamicModelerNode()
= default;

//----------------------------------------------------------------------------
void vtkDMMLDynamicModelerNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkDMMLWriteXMLBeginMacro(of);
  vtkDMMLWriteXMLStringMacro(toolName, ToolName);
  vtkDMMLWriteXMLBooleanMacro(continuousUpdate, ContinuousUpdate);
  vtkDMMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLDynamicModelerNode::ReadXMLAttributes(const char** atts)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkDMMLReadXMLBeginMacro(atts);
  vtkDMMLReadXMLStringMacro(toolName, ToolName);
  vtkDMMLReadXMLBooleanMacro(continuousUpdate, ContinuousUpdate);
  vtkDMMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkDMMLDynamicModelerNode::Copy(vtkDMMLNode *anode)
{
  DMMLNodeModifyBlocker blocker(this);
  Superclass::Copy(anode);
  vtkDMMLCopyBeginMacro(anode);
  vtkDMMLCopyStringMacro(ToolName);
  vtkDMMLCopyBooleanMacro(ContinuousUpdate);
  vtkDMMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLDynamicModelerNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkDMMLPrintBeginMacro(os, indent);
  vtkDMMLPrintStringMacro(ToolName);
  vtkDMMLPrintBooleanMacro(ContinuousUpdate);
  vtkDMMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkDMMLDynamicModelerNode::ProcessDMMLEvents(vtkObject* caller, unsigned long eventID, void* callData)
{
  Superclass::ProcessDMMLEvents(caller, eventID, callData);
  if (!this->Scene)
    {
    vtkErrorMacro("ProcessDMMLEvents: Invalid DMML scene");
    return;
    }

  vtkDMMLNode* node = vtkDMMLNode::SafeDownCast(caller);
  if (!node)
    {
    return;
    }
  this->InvokeEvent(InputNodeModifiedEvent, caller);
}
