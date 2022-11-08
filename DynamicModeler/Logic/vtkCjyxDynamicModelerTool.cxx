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

#include "vtkCjyxDynamicModelerTool.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

/// DynamicModeler DMML includes
#include "vtkDMMLDynamicModelerNode.h"

/// DMML includes
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerTool::vtkCjyxDynamicModelerTool()
= default;

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerTool::~vtkCjyxDynamicModelerTool()
= default;

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerTool* vtkCjyxDynamicModelerTool::Clone()
{
  vtkCjyxDynamicModelerTool* clone = this->CreateToolInstance();
  return clone;
}

//----------------------------------------------------------------------------
int vtkCjyxDynamicModelerTool::GetNumberOfInputNodes()
{
  return this->InputNodeInfo.size();
}

//----------------------------------------------------------------------------
int vtkCjyxDynamicModelerTool::GetNumberOfInputParameters()
{
  return this->InputParameterInfo.size();
}

//----------------------------------------------------------------------------
int vtkCjyxDynamicModelerTool::GetNumberOfOutputNodes()
{
  return this->OutputNodeInfo.size();
}

//----------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthInputNodeName(int n)
{
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return "";
    }
  return this->InputNodeInfo[n].Name;
}

//----------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthInputNodeDescription(int n)
{
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return "";
    }
  return this->InputNodeInfo[n].Description;
}

//----------------------------------------------------------------------------
vtkStringArray* vtkCjyxDynamicModelerTool::GetNthInputNodeClassNames(int n)
{
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return nullptr;
    }
  return this->InputNodeInfo[n].ClassNames;
}

//----------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthInputNodeReferenceRole(int n)
{
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return "";
    }
  return this->InputNodeInfo[n].ReferenceRole;
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerTool::GetNthInputNodeRequired(int n)
{
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return false;
    }
  return this->InputNodeInfo[n].Required;
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerTool::GetNthInputNodeRepeatable(int n)
{
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return false;
    }
  return this->InputNodeInfo[n].Repeatable;
}

//----------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthOutputNodeName(int n)
{
  if (n >= this->GetNumberOfOutputNodes())
    {
    vtkErrorMacro("Output node " << n << " is out of range!");
    return "";
    }
  return this->OutputNodeInfo[n].Name;
}

//----------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthOutputNodeDescription(int n)
{
  if (n >= this->GetNumberOfOutputNodes())
    {
    vtkErrorMacro("Output node " << n << " is out of range!");
    return "";
    }
  return this->OutputNodeInfo[n].Description;
}

//----------------------------------------------------------------------------
vtkStringArray* vtkCjyxDynamicModelerTool::GetNthOutputNodeClassNames(int n)
{
  if (n >= this->GetNumberOfOutputNodes())
    {
    vtkErrorMacro("Output node " << n << " is out of range!");
    return nullptr;
    }
  return this->OutputNodeInfo[n].ClassNames;
}

//----------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthOutputNodeReferenceRole(int n)
{
  if (n >= this->GetNumberOfOutputNodes())
    {
    vtkErrorMacro("Output node " << n << " is out of range!");
    return "";
    }
  return this->OutputNodeInfo[n].ReferenceRole;
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerTool::GetNthOutputNodeRequired(int n)
{
  if (n >= this->GetNumberOfOutputNodes())
    {
    vtkErrorMacro("Output node " << n << " is out of range!");
    return false;
    }
  return this->OutputNodeInfo[n].Required;
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerTool::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Name:\t" << this->GetName() << std::endl;
}

//---------------------------------------------------------------------------
vtkIntArray* vtkCjyxDynamicModelerTool::GetNthInputNodeEvents(int n)
{
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return nullptr;
    }
  return this->InputNodeInfo[n].Events;
}

//---------------------------------------------------------------------------
vtkDMMLNode* vtkCjyxDynamicModelerTool::GetNthInputNode(int n, vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if(!surfaceEditorNode)
    {
    vtkErrorMacro("Invalid parameter node");
    return nullptr;
    }
  if (n >= this->GetNumberOfInputNodes())
    {
    vtkErrorMacro("Input node " << n << " is out of range!");
    return nullptr;
    }
  std::string referenceRole = this->GetNthInputNodeReferenceRole(n);
  return surfaceEditorNode->GetNodeReference(referenceRole.c_str());
}

//---------------------------------------------------------------------------
vtkDMMLNode* vtkCjyxDynamicModelerTool::GetNthOutputNode(int n, vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if(!surfaceEditorNode)
    {
    vtkErrorMacro("Invalid parameter node");
    return nullptr;
    }
  if (n >= this->GetNumberOfOutputNodes())
    {
    vtkErrorMacro("Output node " << n << " is out of range!");
    return nullptr;
    }
  std::string referenceRole = this->GetNthOutputNodeReferenceRole(n);
  return surfaceEditorNode->GetNodeReference(referenceRole.c_str());
}

//---------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthInputParameterName(int n)
{
  if (n >= this->GetNumberOfInputParameters())
    {
    vtkErrorMacro("Parameter " << n << " is out of range!");
    return "";
    }
  return this->InputParameterInfo[n].Name;
}

//---------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthInputParameterDescription(int n)
{
  if (n >= this->GetNumberOfInputParameters())
    {
    vtkErrorMacro("Parameter " << n << " is out of range!");
    return "";
    }
  return this->InputParameterInfo[n].Description;
}

//---------------------------------------------------------------------------
std::string vtkCjyxDynamicModelerTool::GetNthInputParameterAttributeName(int n)
{
  if (n >= this->GetNumberOfInputParameters())
    {
    vtkErrorMacro("Parameter " << n << " is out of range!");
    return "";
    }
  return this->InputParameterInfo[n].AttributeName;
}

//---------------------------------------------------------------------------
int vtkCjyxDynamicModelerTool::GetNthInputParameterType(int n)
{
  if (n >= this->GetNumberOfInputParameters())
    {
    vtkErrorMacro("Parameter " << n << " is out of range!");
    return PARAMETER_STRING;
    }
  return this->InputParameterInfo[n].Type;
}

//---------------------------------------------------------------------------
vtkVariant vtkCjyxDynamicModelerTool::GetNthInputParameterValue(int n, vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (n >= this->GetNumberOfInputParameters())
    {
    vtkErrorMacro("Parameter " << n << " is out of range!");
    return PARAMETER_STRING;
    }
  std::string attributeName = this->GetNthInputParameterAttributeName(n);
  const char* parameterValue = surfaceEditorNode->GetAttribute(attributeName.c_str());
  if (!parameterValue)
    {
    return this->InputParameterInfo[n].DefaultValue;
    }
  return vtkVariant(parameterValue);
}

//---------------------------------------------------------------------------
vtkStringArray* vtkCjyxDynamicModelerTool::GetNthInputParameterPossibleValues(int n)
{
  if (n >= this->GetNumberOfInputParameters())
    {
    vtkErrorMacro("Parameter " << n << " is out of range!");
    return nullptr;
    }
  return this->InputParameterInfo[n].PossibleValues;
}

//---------------------------------------------------------------------------
bool vtkCjyxDynamicModelerTool::HasRequiredInputs(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  for (int i = 0; i < this->GetNumberOfInputNodes(); ++i)
    {
    if (!this->GetNthInputNodeRequired(i))
      {
      continue;
      }

    std::string referenceRole = this->GetNthInputNodeReferenceRole(i);
    if (surfaceEditorNode->GetNodeReference(referenceRole.c_str()) == nullptr)
      {
      return false;
      }
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkCjyxDynamicModelerTool::HasOutput(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  for (int i = 0; i < this->GetNumberOfOutputNodes(); ++i)
    {
    std::string referenceRole = this->GetNthOutputNodeReferenceRole(i);
    if (surfaceEditorNode->GetNodeReference(referenceRole.c_str()) != nullptr)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerTool::GetInputNodes(vtkDMMLDynamicModelerNode* surfaceEditorNode, std::vector<vtkDMMLNode*>& inputNodes)
{
  for (int inputIndex = 0; inputIndex < this->GetNumberOfInputNodes(); ++inputIndex)
    {
    std::string referenceRole = this->GetNthInputNodeReferenceRole(inputIndex);
    int numberOfNodeReferences = surfaceEditorNode->GetNumberOfNodeReferences(referenceRole.c_str());
    for (int referenceIndex = 0; referenceIndex < numberOfNodeReferences; ++referenceIndex)
      {
      vtkDMMLNode* inputNode = surfaceEditorNode->GetNthNodeReference(referenceRole.c_str(), referenceIndex);
      if (inputNode)
        {
        inputNodes.push_back(inputNode);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerTool::GetOutputNodes(vtkDMMLDynamicModelerNode* surfaceEditorNode, std::vector<vtkDMMLNode*>& outputNodes)
{
  for (int outputIndex = 0; outputIndex < this->GetNumberOfOutputNodes(); ++outputIndex)
    {
    std::string referenceRole = this->GetNthOutputNodeReferenceRole(outputIndex);
    int numberOfNodeReferences = surfaceEditorNode->GetNumberOfNodeReferences(referenceRole.c_str());
    for (int referenceIndex = 0; referenceIndex < numberOfNodeReferences; ++referenceIndex)
      {
      vtkDMMLNode* outputNode = surfaceEditorNode->GetNthNodeReference(referenceRole.c_str(), referenceIndex);
      if (outputNode)
        {
        outputNodes.push_back(outputNode);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkCjyxDynamicModelerTool::CreateOutputDisplayNodes(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!surfaceEditorNode)
    {
    vtkErrorMacro("Invalid surfaceEditorNode!");
    return;
    }

  std::vector<vtkDMMLNode*> inputNodes;
  this->GetInputNodes(surfaceEditorNode, inputNodes);

  std::vector<vtkDMMLNode*> outputNodes;
  this->GetOutputNodes(surfaceEditorNode, outputNodes);

  for (vtkDMMLNode* outputNode : outputNodes)
    {
    vtkDMMLDisplayableNode* outputDisplayableNode = vtkDMMLDisplayableNode::SafeDownCast(outputNode);
    if (outputDisplayableNode == nullptr || outputDisplayableNode->GetDisplayNode())
      {
      continue;
      }

    outputDisplayableNode->CreateDefaultDisplayNodes();
    vtkDMMLDisplayNode* outputDisplayNode = outputDisplayableNode->GetDisplayNode();
    if (outputDisplayNode == nullptr)
      {
      continue;
      }

    for (vtkDMMLNode* inputNode : inputNodes)
      {
      vtkDMMLDisplayableNode* inputDisplayableNode = vtkDMMLDisplayableNode::SafeDownCast(inputNode);
      if (inputDisplayableNode == nullptr || !inputDisplayableNode->IsA(outputNode->GetClassName()))
        {
        continue;
        }

      vtkDMMLDisplayNode* inputDisplayNode = inputDisplayableNode->GetDisplayNode();
      if (!inputDisplayNode)
        {
        continue;
        }
      outputDisplayNode->CopyContent(inputDisplayNode);
      break;
      }
    }
}

//---------------------------------------------------------------------------
bool vtkCjyxDynamicModelerTool::Run(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!this->HasRequiredInputs(surfaceEditorNode))
    {
    vtkErrorMacro("Input node missing!");
    return false;
    }
  if (!this->HasOutput(surfaceEditorNode))
    {
    vtkErrorMacro("Output node missing!");
    return false;
    }

  this->CreateOutputDisplayNodes(surfaceEditorNode);
  return this->RunInternal(surfaceEditorNode);
}
