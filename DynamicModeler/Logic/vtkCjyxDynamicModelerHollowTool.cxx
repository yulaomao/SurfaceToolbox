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

#include "vtkCjyxDynamicModelerHollowTool.h"

#include "vtkDMMLDynamicModelerNode.h"

// DMML includes
#include <vtkDMMLMarkupsPlaneNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkIntArray.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

//----------------------------------------------------------------------------
vtkToolNewMacro(vtkCjyxDynamicModelerHollowTool);

const char* HOLLOW_INPUT_MODEL_REFERENCE_ROLE = "Hollow.InputModel";
const char* HOLLOW_OUTPUT_MODEL_REFERENCE_ROLE = "Hollow.OutputModel";

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerHollowTool::vtkCjyxDynamicModelerHollowTool()
{
  /////////
  // Inputs
  vtkNew<vtkIntArray> inputModelEvents;
  inputModelEvents->InsertNextTuple1(vtkCommand::ModifiedEvent);
  inputModelEvents->InsertNextTuple1(vtkDMMLModelNode::MeshModifiedEvent);
  inputModelEvents->InsertNextTuple1(vtkDMMLTransformableNode::TransformModifiedEvent);
  vtkNew<vtkStringArray> inputModelClassNames;
  inputModelClassNames->InsertNextValue("vtkDMMLModelNode");
  NodeInfo inputModel(
    "Model",
    "Model to be hollowed.",
    inputModelClassNames,
    HOLLOW_INPUT_MODEL_REFERENCE_ROLE,
    true,
    false,
    inputModelEvents
  );
  this->InputNodeInfo.push_back(inputModel);

  /////////
  // Outputs
  NodeInfo outputModel(
    "Hollowed model",
    "Input model with its boundary surface converted to a shell. Surface normals must be computed for the model, for example using Surface Toolbox module.",
    inputModelClassNames,
    HOLLOW_OUTPUT_MODEL_REFERENCE_ROLE,
    false,
    false
    );
  this->OutputNodeInfo.push_back(outputModel);

  /////////
  // Parameters
  ParameterInfo parameterShellThickness(
    "Shell thickness",
    "Shell thickness of the generated hollow model. Keep the value low to avoid self-intersection.",
    "ShellThickness",
    PARAMETER_DOUBLE,
    1.0);
  this->InputParameterInfo.push_back(parameterShellThickness);

  this->InputModelToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->InputModelNodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->InputModelToWorldTransformFilter->SetTransform(this->InputModelNodeToWorldTransform);

  this->HollowFilter = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
  this->HollowFilter->SetInputConnection(this->InputModelToWorldTransformFilter->GetOutputPort());
  this->HollowFilter->SetExtrusionTypeToNormalExtrusion();
  this->HollowFilter->SetScaleFactor(1.0);

  this->TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  this->TriangleFilter->SetInputConnection(this->HollowFilter->GetOutputPort());

  this->NormalsFilter = vtkSmartPointer<vtkPolyDataNormals>::New();
  this->NormalsFilter->SetInputConnection(this->TriangleFilter->GetOutputPort());
  this->NormalsFilter->AutoOrientNormalsOn();

  this->OutputModelToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->OutputWorldToModelTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->OutputModelToWorldTransformFilter->SetTransform(this->OutputWorldToModelTransform);
  this->OutputModelToWorldTransformFilter->SetInputConnection(this->NormalsFilter->GetOutputPort());
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerHollowTool::~vtkCjyxDynamicModelerHollowTool()
= default;

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerHollowTool::GetName()
{
  return "Hollow";
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerHollowTool::RunInternal(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!this->HasRequiredInputs(surfaceEditorNode))
    {
    vtkErrorMacro("Invalid number of inputs");
    return false;
    }

  vtkDMMLModelNode* outputModelNode = vtkDMMLModelNode::SafeDownCast(surfaceEditorNode->GetNodeReference(HOLLOW_OUTPUT_MODEL_REFERENCE_ROLE));
  if (!outputModelNode)
    {
    // Nothing to output
    return true;
    }

  vtkDMMLModelNode* inputModelNode = vtkDMMLModelNode::SafeDownCast(surfaceEditorNode->GetNodeReference(HOLLOW_INPUT_MODEL_REFERENCE_ROLE));
  if (!inputModelNode)
    {
    vtkErrorMacro("Invalid input model node!");
    return false;
    }

  if (!inputModelNode->GetMesh() || inputModelNode->GetMesh()->GetNumberOfPoints() == 0)
    {
    vtkNew<vtkPolyData> outputPolyData;
    outputModelNode->SetAndObservePolyData(outputPolyData);
    return true;
    }

  if (inputModelNode->GetParentTransformNode())
    {
    inputModelNode->GetParentTransformNode()->GetTransformToWorld(this->InputModelNodeToWorldTransform);
    }
  else
    {
    this->InputModelNodeToWorldTransform->Identity();
    }
  if (outputModelNode && outputModelNode->GetParentTransformNode())
    {
    outputModelNode->GetParentTransformNode()->GetTransformFromWorld(this->OutputWorldToModelTransform);
    }
  else
    {
    this->OutputWorldToModelTransform->Identity();
    }

  double shellThickness = this->GetNthInputParameterValue(0, surfaceEditorNode).ToDouble();
  this->HollowFilter->SetScaleFactor(shellThickness);

  this->InputModelToWorldTransformFilter->SetInputConnection(inputModelNode->GetMeshConnection());

  this->OutputModelToWorldTransformFilter->Update();
  vtkNew<vtkPolyData> outputMesh;
  outputMesh->DeepCopy(this->OutputModelToWorldTransformFilter->GetOutput());

  DMMLNodeModifyBlocker blocker(outputModelNode);
  outputModelNode->SetAndObserveMesh(outputMesh);
  outputModelNode->InvokeCustomModifiedEvent(vtkDMMLModelNode::MeshModifiedEvent);

  return true;
}
