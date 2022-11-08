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

// DynamicModeler Logic includes
#include "vtkCjyxDynamicModelerPlaneCutTool.h"
#include "vtkCjyxDynamicModelerROICutTool.h"

// DynamicModeler DMML includes
#include "vtkDMMLDynamicModelerNode.h"

// DMML includes
#include <vtkDMMLMarkupsROINode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkClipPolyData.h>
#include <vtkCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkImplicitBoolean.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPlaneCollection.h>
#include <vtkReverseSense.h>
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>

//----------------------------------------------------------------------------
vtkToolNewMacro(vtkCjyxDynamicModelerROICutTool);

// Input references
const char* ROI_CUT_INPUT_MODEL_REFERENCE_ROLE = "ROICut.InputModel";
const char* ROI_CUT_INPUT_ROI_REFERENCE_ROLE = "ROICut.InputROI";

// Output references
const char* ROI_CUT_OUTPUT_INSIDE_MODEL_REFERENCE_ROLE = "ROICut.OutputPositiveModel";
const char* ROI_CUT_OUTPUT_OUTSIDE_MODEL_REFERENCE_ROLE = "ROICut.OutputNegativeModel";

// Parameters
const char* ROI_CUT_CAP_SURFACE_ATTRIBUTE_NAME = "ROICut.CapSurface";

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerROICutTool::vtkCjyxDynamicModelerROICutTool()
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
    "Model node",
    "Model node to be cut.",
    inputModelClassNames,
    this->GetInputModelReferenceRole(),
    true,
    false,
    inputModelEvents
  );
  this->InputNodeInfo.push_back(inputModel);

  vtkNew<vtkIntArray> inputROIEvents;
  inputROIEvents->InsertNextTuple1(vtkCommand::ModifiedEvent);
  inputROIEvents->InsertNextTuple1(vtkDMMLMarkupsNode::PointModifiedEvent);
  inputROIEvents->InsertNextTuple1(vtkDMMLTransformableNode::TransformModifiedEvent);
  vtkNew<vtkStringArray> inputROIClassNames;
  inputROIClassNames->InsertNextValue("vtkDMMLMarkupsROINode");
  NodeInfo inputROI(
    "ROI node",
    "ROI node to cut the model node.",
    inputROIClassNames,
    this->GetInputROIReferenceRole(),
    true,
    false,
    inputROIEvents
    );
  this->InputNodeInfo.push_back(inputROI);

  /////////
  // Outputs
  NodeInfo outputPositiveModel(
    "Clipped output model (inside)",
    "Portion of the cut model that is inside the ROI.",
    inputModelClassNames,
    this->GetOutputInsideModelReferenceRole(),
    false,
    false
    );
  this->OutputNodeInfo.push_back(outputPositiveModel);

  NodeInfo outputNegativeModel(
    "Clipped output model (outside)",
    "Portion of the cut model that is outside the ROI.",
    inputModelClassNames,
    this->GetOutputOutsideModelReferenceRole(),
    false,
    false
    );
  this->OutputNodeInfo.push_back(outputNegativeModel);

  /////////
  // Parameters

  ParameterInfo parameterCapSurface(
    "Cap surface",
    "Create a closed surface by triangulating the clipped region",
    this->GetCapSurfaceAttributeName(),
    PARAMETER_BOOL,
    true);
  this->InputParameterInfo.push_back(parameterCapSurface);

  this->InputModelToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->InputModelNodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->InputModelToWorldTransformFilter->SetTransform(this->InputModelNodeToWorldTransform);

  this->ROIClipper = vtkSmartPointer<vtkClipPolyData>::New();
  this->ROIClipper->SetInputConnection(this->InputModelToWorldTransformFilter->GetOutputPort());

  this->OutputInsideWorldToModelTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->OutputInsideWorldToModelTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->OutputInsideWorldToModelTransformFilter->SetInputConnection(this->ROIClipper->GetOutputPort());
  this->OutputInsideWorldToModelTransformFilter->SetTransform(this->OutputInsideWorldToModelTransform);

  this->OutputOutsideWorldToModelTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->OutputOutsideWorldToModelTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->OutputOutsideWorldToModelTransformFilter->SetInputConnection(this->ROIClipper->GetClippedOutputPort());
  this->OutputOutsideWorldToModelTransformFilter->SetTransform(this->OutputOutsideWorldToModelTransform);
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerROICutTool::~vtkCjyxDynamicModelerROICutTool()
= default;

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerROICutTool::GetName()
{
  return "ROI cut";
}

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerROICutTool::GetInputModelReferenceRole()
{
  return ROI_CUT_INPUT_MODEL_REFERENCE_ROLE;
}

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerROICutTool::GetInputROIReferenceRole()
{
  return ROI_CUT_INPUT_ROI_REFERENCE_ROLE;
}

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerROICutTool::GetOutputInsideModelReferenceRole()
{
  return ROI_CUT_OUTPUT_INSIDE_MODEL_REFERENCE_ROLE;
}

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerROICutTool::GetOutputOutsideModelReferenceRole()
{
  return ROI_CUT_OUTPUT_OUTSIDE_MODEL_REFERENCE_ROLE;
}

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerROICutTool::GetCapSurfaceAttributeName()
{
  return ROI_CUT_CAP_SURFACE_ATTRIBUTE_NAME;
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerROICutTool::RunInternal(vtkDMMLDynamicModelerNode* dynamicModelerNode)
{
  if (!this->HasRequiredInputs(dynamicModelerNode))
    {
    vtkErrorMacro("Invalid number of inputs");
    return false;
    }

  vtkDMMLModelNode* outputInsideModelNode = vtkDMMLModelNode::SafeDownCast(dynamicModelerNode->GetNodeReference(this->GetOutputInsideModelReferenceRole()));
  vtkDMMLModelNode* outputOutsideModelNode = vtkDMMLModelNode::SafeDownCast(dynamicModelerNode->GetNodeReference(this->GetOutputOutsideModelReferenceRole()));
  if (!outputInsideModelNode && !outputOutsideModelNode)
    {
    // Nothing to output
    return true;
    }

  vtkDMMLModelNode* inputModelNode = vtkDMMLModelNode::SafeDownCast(dynamicModelerNode->GetNodeReference(this->GetInputModelReferenceRole()));
  if (!inputModelNode)
    {
    vtkErrorMacro("Invalid input model node!");
    return false;
    }
  if (!inputModelNode->GetMesh() || inputModelNode->GetMesh()->GetNumberOfPoints() == 0)
    {
    if (outputInsideModelNode && outputInsideModelNode->GetPolyData())
      {
      outputInsideModelNode->GetPolyData()->Initialize();
      }
    if (outputOutsideModelNode && outputOutsideModelNode->GetPolyData())
      {
      outputOutsideModelNode->GetPolyData()->Initialize();
      }
    return true;
    }

  vtkDMMLMarkupsROINode* roiNode = vtkDMMLMarkupsROINode::SafeDownCast(dynamicModelerNode->GetNodeReference(this->GetInputROIReferenceRole()));
  if (!roiNode)
    {
    if (outputInsideModelNode)
      {
      outputInsideModelNode->SetAndObservePolyData(vtkNew<vtkPolyData>());
      }
    if (outputOutsideModelNode)
      {
      outputOutsideModelNode->SetAndObservePolyData(vtkNew<vtkPolyData>());
      }
    return true;
    }

  vtkNew<vtkPlanes> planes;
  roiNode->GetPlanesWorld(planes);

  vtkNew<vtkImplicitBoolean> planeFunction;
  planeFunction->SetOperationTypeToUnion();

  vtkNew<vtkPlaneCollection> planeCollection;
  for (int i = 0; i < planes->GetNumberOfPlanes(); ++i)
    {
    vtkPlane* currentPlane = planes->GetPlane(i);

    // vtkClipPolyData will remove the inside of the ROI (negative implicit function), we need to invert the planes so that the correct
    // region is preserved.
    double normal[3] = { 1,0,0 };
    currentPlane->GetNormal(normal);
    vtkMath::MultiplyScalar(normal, -1.0);

    double origin[3] = { 0.0, 0.0, 0.0 };
    currentPlane->GetOrigin(origin);

    // vtkPlaneCollection always returns a pointer to the same vtkPlane, so we need to make a copy.
    vtkNew<vtkPlane> plane;
    plane->SetNormal(normal);
    plane->SetOrigin(origin);

    planeFunction->AddFunction(plane);
    planeCollection->AddItem(plane);
    }

  this->ROIClipper->SetClipFunction(planeFunction);
  this->ROIClipper->SetGenerateClippedOutput(outputOutsideModelNode != nullptr);

  vtkDMMLTransformNode::GetTransformBetweenNodes(inputModelNode->GetParentTransformNode(), nullptr, this->InputModelNodeToWorldTransform);

  this->InputModelToWorldTransformFilter->SetInputConnection(inputModelNode->GetMeshConnection());
  this->InputModelToWorldTransformFilter->Update();

  bool capSurface = vtkVariant(dynamicModelerNode->GetAttribute(ROI_CUT_CAP_SURFACE_ATTRIBUTE_NAME)).ToInt() != 0;
  int roiType = roiNode->GetROIType();
  if (roiType != vtkDMMLMarkupsROINode::ROITypeBox && roiType != vtkDMMLMarkupsROINode::ROITypeBoundingBox)
    {
    capSurface = false;
    }

  vtkNew<vtkPolyData> endCapPolyData;
  if (capSurface)
    {
    vtkCjyxDynamicModelerPlaneCutTool::CreateEndCap(planeCollection, this->InputModelToWorldTransformFilter->GetOutput(), planeFunction, endCapPolyData);
    }

  if (outputInsideModelNode)
    {
    vtkDMMLTransformNode::GetTransformBetweenNodes(nullptr, outputInsideModelNode->GetParentTransformNode(), this->OutputInsideWorldToModelTransform);
    this->OutputInsideWorldToModelTransformFilter->Update();

    DMMLNodeModifyBlocker blocker(outputInsideModelNode);
    vtkSmartPointer<vtkPolyData> outputMesh = outputInsideModelNode->GetPolyData();
    if (!outputMesh)
      {
      outputMesh = vtkSmartPointer<vtkPolyData>::New();
      outputInsideModelNode->SetAndObservePolyData(outputMesh);
      }
    outputMesh->DeepCopy(this->OutputInsideWorldToModelTransformFilter->GetOutput());
    if (capSurface)
      {
      vtkNew<vtkAppendPolyData> appendEndCap;
      appendEndCap->AddInputData(outputMesh);
      appendEndCap->AddInputData(endCapPolyData);
      appendEndCap->Update();
      outputMesh->ShallowCopy(appendEndCap->GetOutput());
      }
    outputInsideModelNode->InvokeCustomModifiedEvent(vtkDMMLModelNode::MeshModifiedEvent);
    }

  if (outputOutsideModelNode)
    {
    vtkDMMLTransformNode::GetTransformBetweenNodes(nullptr, outputOutsideModelNode->GetParentTransformNode(), this->OutputOutsideWorldToModelTransform);
    this->OutputOutsideWorldToModelTransformFilter->Update();

    DMMLNodeModifyBlocker blocker(outputOutsideModelNode);
    vtkSmartPointer<vtkPolyData> outputMesh = outputOutsideModelNode->GetPolyData();
    if (!outputMesh)
      {
      outputMesh = vtkSmartPointer<vtkPolyData>::New();
      outputOutsideModelNode->SetAndObservePolyData(outputMesh);
      }
    outputMesh->DeepCopy(this->OutputOutsideWorldToModelTransformFilter->GetOutput());
    if (capSurface)
      {
      vtkNew<vtkReverseSense> reverseSense;
      reverseSense->SetInputData(endCapPolyData);
      reverseSense->ReverseCellsOn();
      reverseSense->ReverseNormalsOn();

      vtkNew<vtkAppendPolyData> appendEndCap;
      appendEndCap->AddInputData(outputMesh);
      appendEndCap->AddInputConnection(reverseSense->GetOutputPort());
      appendEndCap->Update();
      outputMesh->ShallowCopy(appendEndCap->GetOutput());
      }
    outputOutsideModelNode->InvokeCustomModifiedEvent(vtkDMMLModelNode::MeshModifiedEvent);
    }

  return true;
}
