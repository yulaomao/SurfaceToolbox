/*==============================================================================

  Program: 3D Cjyx

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

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

// DynamicModeler Logic includes
#include "vtkCjyxDynamicModelerLogic.h"
#include "vtkCjyxDynamicModelerToolFactory.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLDynamicModelerNode.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCjyxDynamicModelerLogic);

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerLogic::vtkCjyxDynamicModelerLogic()
{
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerLogic::~vtkCjyxDynamicModelerLogic()
{
}

//----------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::SetDMMLSceneInternal(vtkDMMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkDMMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkDMMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkDMMLScene::EndImportEvent);
  events->InsertNextValue(vtkDMMLScene::EndBatchProcessEvent);
  this->SetAndObserveDMMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::RegisterNodes()
{
  if (this->GetDMMLScene() == NULL)
    {
    vtkErrorMacro("Scene is invalid");
    return;
    }
  this->GetDMMLScene()->RegisterNodeClass(vtkSmartPointer<vtkDMMLDynamicModelerNode>::New());
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::OnDMMLSceneNodeAdded(vtkDMMLNode* node)
{
  vtkDMMLDynamicModelerNode* surfaceEditorNode = vtkDMMLDynamicModelerNode::SafeDownCast(node);
  if (!surfaceEditorNode)
    {
    return;
    }
  if (!this->GetDMMLScene() || this->GetDMMLScene()->IsImporting())
    {
    return;
    }

  this->Tools[surfaceEditorNode->GetID()] = nullptr;
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkDMMLDynamicModelerNode::InputNodeModifiedEvent);
  vtkObserveDMMLNodeEventsMacro(surfaceEditorNode, events);
  this->UpdateDynamicModelerTool(surfaceEditorNode);
  this->RunDynamicModelerTool(surfaceEditorNode);
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::OnDMMLSceneNodeRemoved(vtkDMMLNode* node)
{
  vtkDMMLDynamicModelerNode* surfaceEditorNode = vtkDMMLDynamicModelerNode::SafeDownCast(node);
  if (!surfaceEditorNode)
    {
    return;
    }

  DynamicModelerToolList::iterator tool = this->Tools.find(surfaceEditorNode->GetID());
  if (tool == this->Tools.end())
    {
    return;
    }
  this->Tools.erase(tool);
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::OnDMMLSceneEndImport()
{
  if (!this->GetDMMLScene())
    {
    return;
    }

  std::vector<vtkDMMLNode*> parametericSurfaceNodes;
  this->GetDMMLScene()->GetNodesByClass("vtkDMMLDynamicModelerNode", parametericSurfaceNodes);
  for (vtkDMMLNode* node : parametericSurfaceNodes)
    {
    vtkDMMLDynamicModelerNode* dynamicModelerNode =
      vtkDMMLDynamicModelerNode::SafeDownCast(node);
    if (!dynamicModelerNode)
      {
      continue;
      }

    this->Tools[dynamicModelerNode->GetID()] = nullptr;
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkDMMLDynamicModelerNode::InputNodeModifiedEvent);
    vtkObserveDMMLNodeEventsMacro(dynamicModelerNode, events);
    this->UpdateDynamicModelerTool(dynamicModelerNode);
    }
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::ProcessDMMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessDMMLNodesEvents(caller, event, callData);
  if (!this->GetDMMLScene() || this->GetDMMLScene()->IsImporting())
    {
    return;
    }

  vtkDMMLDynamicModelerNode* surfaceEditorNode = vtkDMMLDynamicModelerNode::SafeDownCast(caller);
  if (!surfaceEditorNode)
    {
    return;
    }

  if (surfaceEditorNode && event == vtkCommand::ModifiedEvent)
    {
    this->UpdateDynamicModelerTool(surfaceEditorNode);
    if (surfaceEditorNode->GetContinuousUpdate() && this->HasCircularReference(surfaceEditorNode))
      {
      vtkWarningMacro("Circular reference detected. Disabling continuous update for: " << surfaceEditorNode->GetName());
      surfaceEditorNode->SetContinuousUpdate(false);
      return;
      }
    }

  if (surfaceEditorNode && surfaceEditorNode->GetContinuousUpdate())
    {
    vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = this->GetDynamicModelerTool(surfaceEditorNode);
    if (tool)
      {
      this->RunDynamicModelerTool(surfaceEditorNode);
      }
    }
}

//---------------------------------------------------------------------------
bool vtkCjyxDynamicModelerLogic::HasCircularReference(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!surfaceEditorNode)
    {
    vtkErrorMacro("Invalid input node!");
    return false;
    }
  vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = this->GetDynamicModelerTool(surfaceEditorNode);
  if (!tool)
    {
    return false;
    }

  std::vector<vtkDMMLNode*> inputNodes;
  for (int i = 0; i < tool->GetNumberOfInputNodes(); ++i)
    {
    vtkDMMLNode* inputNode = tool->GetNthInputNode(i, surfaceEditorNode);
    if (inputNode)
      {
      inputNodes.push_back(inputNode);
      }
    }

  for (int i = 0; i < tool->GetNumberOfOutputNodes(); ++i)
    {
    vtkDMMLNode* outputNode = tool->GetNthOutputNode(i, surfaceEditorNode);
    if (!outputNode)
      {
      continue;
      }
    std::vector<vtkDMMLNode*>::iterator inputNodeIt = std::find(inputNodes.begin(), inputNodes.end(), outputNode);
    if (inputNodeIt != inputNodes.end())
      {
      return true;
      }
    }

  return false;
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::UpdateDynamicModelerTool(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!surfaceEditorNode)
    {
    vtkErrorMacro("Invalid input node!");
    return;
    }

  DMMLNodeModifyBlocker blocker(surfaceEditorNode);

  vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = this->GetDynamicModelerTool(surfaceEditorNode);
  if (!tool || strcmp(tool->GetName(), surfaceEditorNode->GetToolName()) != 0)
    {
    // We are changing tool types, and should remove observers to the previous tool
    if (tool)
      {
      for (int i = 0; i < tool->GetNumberOfInputNodes(); ++i)
        {
        std::string referenceRole = tool->GetNthInputNodeReferenceRole(i);
        std::vector<const char*> referenceNodeIds;
        surfaceEditorNode->GetNodeReferenceIDs(referenceRole.c_str(), referenceNodeIds);
        int referenceIndex = 0;
        for (const char* referenceId : referenceNodeIds)
          {
          // Current behavior is to add back references without observers
          // This preserves the selected nodes for each tool
          surfaceEditorNode->SetNthNodeReferenceID(referenceRole.c_str(), referenceIndex, referenceId);
          ++referenceIndex;
          }
        }
      }

    tool = nullptr;
    if (surfaceEditorNode->GetToolName())
      {
      tool = vtkSmartPointer<vtkCjyxDynamicModelerTool>::Take(
        vtkCjyxDynamicModelerToolFactory::GetInstance()->CreateToolByName(surfaceEditorNode->GetToolName()));
      }
    this->Tools[surfaceEditorNode->GetID()] = tool;
    }

  if (tool)
    {
    // Update node observers to ensure that all input nodes are observed
    for (int i = 0; i < tool->GetNumberOfInputNodes(); ++i)
      {
      std::string referenceRole = tool->GetNthInputNodeReferenceRole(i);
      std::vector<const char*> referenceNodeIds;
      surfaceEditorNode->GetNodeReferenceIDs(referenceRole.c_str(), referenceNodeIds);
      vtkIntArray* events = tool->GetNthInputNodeEvents(i);
      int referenceIndex = 0;
      for (const char* referenceId : referenceNodeIds)
        {
        // Current behavior is to add back references without observers
        // This preserves the selected nodes for each tool
        surfaceEditorNode->SetAndObserveNthNodeReferenceID(referenceRole.c_str(), referenceIndex, referenceId, events);
        ++referenceIndex;
        }
      }
    }
}

//---------------------------------------------------------------------------
vtkCjyxDynamicModelerTool* vtkCjyxDynamicModelerLogic::GetDynamicModelerTool(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!surfaceEditorNode || !surfaceEditorNode->GetID())
    {
    return nullptr;
    }

  vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = nullptr;
  DynamicModelerToolList::iterator toolIt = this->Tools.find(surfaceEditorNode->GetID());
  if (toolIt == this->Tools.end())
    {
    return nullptr;
    }
  return toolIt->second;
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerLogic::RunDynamicModelerTool(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!surfaceEditorNode)
    {
    vtkErrorMacro("Invalid parameter node!");
    return;
    }
  if (!surfaceEditorNode->GetToolName())
    {
    return;
    }

  vtkSmartPointer<vtkCjyxDynamicModelerTool> tool = this->GetDynamicModelerTool(surfaceEditorNode);
  if (!tool)
    {
    vtkErrorMacro("Could not find tool with name: " << surfaceEditorNode->GetToolName());
    return;
    }
  if (!tool->HasRequiredInputs(surfaceEditorNode))
    {
    return;
    }

  tool->Run(surfaceEditorNode);
}
