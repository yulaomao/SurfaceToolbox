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

#include "vtkCjyxDynamicModelerPlaneCutTool.h"

#include "vtkDMMLDynamicModelerNode.h"

// DMML includes
#include <vtkDMMLMarkupsPlaneNode.h>
#include <vtkDMMLModelNode.h>
#include <vtkDMMLSliceNode.h>
#include <vtkDMMLTransformNode.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkClipClosedSurface.h>
#include <vtkClipPolyData.h>
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkContourTriangulator.h>
#include <vtkCutter.h>
#include <vtkDataSetAttributes.h>
#include <vtkFeatureEdges.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImplicitBoolean.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolygon.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkReverseSense.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkStripper.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

//----------------------------------------------------------------------------
vtkToolNewMacro(vtkCjyxDynamicModelerPlaneCutTool);

const char* PLANE_CUT_INPUT_MODEL_REFERENCE_ROLE = "PlaneCut.InputModel";
const char* PLANE_CUT_INPUT_PLANE_REFERENCE_ROLE = "PlaneCut.InputPlane";
const char* PLANE_CUT_OUTPUT_POSITIVE_MODEL_REFERENCE_ROLE = "PlaneCut.OutputPositiveModel";
const char* PLANE_CUT_OUTPUT_NEGATIVE_MODEL_REFERENCE_ROLE = "PlaneCut.OutputNegativeModel";

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerPlaneCutTool::vtkCjyxDynamicModelerPlaneCutTool()
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
    PLANE_CUT_INPUT_MODEL_REFERENCE_ROLE,
    true,
    false,
    inputModelEvents
  );
  this->InputNodeInfo.push_back(inputModel);

  vtkNew<vtkIntArray> inputPlaneEvents;
  inputPlaneEvents->InsertNextTuple1(vtkCommand::ModifiedEvent);
  inputPlaneEvents->InsertNextTuple1(vtkDMMLMarkupsNode::PointModifiedEvent);
  inputPlaneEvents->InsertNextTuple1(vtkDMMLTransformableNode::TransformModifiedEvent);
  vtkNew<vtkStringArray> inputPlaneClassNames;
  inputPlaneClassNames->InsertNextValue("vtkDMMLMarkupsPlaneNode");
  inputPlaneClassNames->InsertNextValue("vtkDMMLSliceNode");
  NodeInfo inputPlane(
    "Plane node",
    "Plane node to cut the model node.",
    inputPlaneClassNames,
    PLANE_CUT_INPUT_PLANE_REFERENCE_ROLE,
    true,
    true,
    inputPlaneEvents
    );
  this->InputNodeInfo.push_back(inputPlane);

  /////////
  // Outputs
  NodeInfo outputPositiveModel(
    "Clipped output model (positive side)",
    "Portion of the cut model that is on the same side of the plane as the normal.",
    inputModelClassNames,
    PLANE_CUT_OUTPUT_POSITIVE_MODEL_REFERENCE_ROLE,
    false,
    false
    );
  this->OutputNodeInfo.push_back(outputPositiveModel);

  NodeInfo outputNegativeModel(
    "Clipped output model (negative side)",
    "Portion of the cut model that is on the opposite side of the plane as the normal.",
    inputModelClassNames,
    PLANE_CUT_OUTPUT_NEGATIVE_MODEL_REFERENCE_ROLE,
    false,
    false
    );
  this->OutputNodeInfo.push_back(outputNegativeModel);

  /////////
  // Parameters
  ParameterInfo parameterCapSurface(
    "Cap surface",
    "Create a closed surface by triangulating the clipped region",
    "CapSurface",
    PARAMETER_BOOL,
    true);
  this->InputParameterInfo.push_back(parameterCapSurface);

  ParameterInfo parameterOperationType(
    "Operation type",
    "Method used for combining the planes",
    "OperationType",
    PARAMETER_STRING_ENUM,
    "Union");

  vtkNew<vtkStringArray> possibleValues;
  parameterOperationType.PossibleValues = possibleValues;
  parameterOperationType.PossibleValues->InsertNextValue("Union");
  parameterOperationType.PossibleValues->InsertNextValue("Intersection");
  parameterOperationType.PossibleValues->InsertNextValue("Difference");
  this->InputParameterInfo.push_back(parameterOperationType);

  this->InputModelToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->InputModelNodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->InputModelToWorldTransformFilter->SetTransform(this->InputModelNodeToWorldTransform);

  this->PlaneClipper = vtkSmartPointer<vtkClipPolyData>::New();
  this->PlaneClipper->SetInputConnection(this->InputModelToWorldTransformFilter->GetOutputPort());
  this->PlaneClipper->SetValue(0.0);

  this->OutputPositiveWorldToModelTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->OutputPositiveWorldToModelTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->OutputPositiveWorldToModelTransformFilter->SetTransform(this->OutputPositiveWorldToModelTransform);

  this->OutputNegativeWorldToModelTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->OutputNegativeWorldToModelTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  this->OutputNegativeWorldToModelTransformFilter->SetTransform(this->OutputNegativeWorldToModelTransform);
}

//----------------------------------------------------------------------------
vtkCjyxDynamicModelerPlaneCutTool::~vtkCjyxDynamicModelerPlaneCutTool()
= default;

//----------------------------------------------------------------------------
const char* vtkCjyxDynamicModelerPlaneCutTool::GetName()
{
  return "Plane cut";
}

//----------------------------------------------------------------------------
void vtkCjyxDynamicModelerPlaneCutTool::CreateEndCap(vtkPlaneCollection* planes, vtkPolyData* originalPolyData, vtkImplicitBoolean* cutFunction, vtkPolyData* outputEndCap)
{
  int operationType = cutFunction->GetOperationType();
  vtkNew<vtkAppendPolyData> appendFilter;
  for (int i = 0; i < planes->GetNumberOfItems(); ++i)
    {
    vtkPlane* plane = planes->GetItem(i);
    vtkNew<vtkCutter> cutter;
    cutter->SetCutFunction(plane);
    cutter->SetInputData(originalPolyData);

    vtkNew<vtkContourTriangulator> contourTriangulator;
    contourTriangulator->SetInputConnection(cutter->GetOutputPort());
    contourTriangulator->Update();

    // Create a seam along the intersection of each plane with the triangulated contour
    // This allows the contour to be split correctly later.
    vtkNew<vtkPolyData> endCapPolyData;
    endCapPolyData->ShallowCopy(contourTriangulator->GetOutput());
    for (int j = 0; j < planes->GetNumberOfItems(); ++j)
      {
      if (i == j)
        {
        continue;
        }
      vtkPlane* plane2 = planes->GetItem(j);
      vtkNew<vtkClipPolyData> clipper;
      clipper->SetInputData(endCapPolyData);
      clipper->SetClipFunction(plane2);
      clipper->SetValue(0.0);
      clipper->GenerateClippedOutputOn();
      vtkNew<vtkAppendPolyData> appendCut;
      appendCut->AddInputConnection(clipper->GetOutputPort());
      appendCut->AddInputConnection(clipper->GetClippedOutputPort());
      appendCut->Update();
      endCapPolyData->ShallowCopy(appendCut->GetOutput());
      }

    // Remove all triangles that do not lie at 0.0
    double epsilon = 1e-4;
    vtkNew<vtkClipPolyData> clipper;
    clipper->SetInputData(endCapPolyData);
    clipper->SetClipFunction(cutFunction);
    clipper->InsideOutOff();
    clipper->SetValue(-epsilon);
    vtkNew<vtkClipPolyData> clipper2;
    clipper2->SetInputConnection(clipper->GetOutputPort());
    clipper2->SetClipFunction(cutFunction);
    clipper2->InsideOutOn();
    clipper2->SetValue(epsilon);
    clipper2->Update();
    endCapPolyData->ShallowCopy(clipper2->GetOutput());

    double planeNormal[3] = { 0.0 };
    plane->GetNormal(planeNormal);
    if (operationType != vtkImplicitBoolean::VTK_DIFFERENCE || i == 0)
      {
      vtkMath::MultiplyScalar(planeNormal, -1.0);
      }

    vtkCellArray* endCapPolys = endCapPolyData->GetPolys();
    if (endCapPolys && endCapPolyData->GetNumberOfPolys() > 0)
      {
      vtkNew<vtkIdList> polyPointIds;
      endCapPolys->GetCell(0, polyPointIds);
      double polyNormal[3] = { 0.0 };

      // The normal of the triangles generated by vtkContourTriangulator are based on the clockwise/counter-clockwise direction of the largest contour.
      // This normal will not always line up with the desired normal as defined by the plane, so if the normal generated by vtkContourTriangulator faces the wrong
      // direction, then we need to flip the normals of the polys so that it matches the expected normal.
      vtkPolygon::ComputeNormal(endCapPolyData->GetPoints(), polyPointIds->GetNumberOfIds(), polyPointIds->GetPointer(0), polyNormal);
      if (vtkMath::Dot(polyNormal, planeNormal) < 0.0)
        {
        vtkNew<vtkReverseSense> reverseSense;
        reverseSense->SetInputData(endCapPolyData);
        reverseSense->ReverseCellsOn();
        reverseSense->Update();
        endCapPolyData->ShallowCopy(reverseSense->GetOutput());
        }
      }

    vtkNew<vtkFloatArray> normals;
    normals->SetName("Normals");
    normals->SetNumberOfComponents(3);
    normals->SetNumberOfTuples(endCapPolyData->GetNumberOfPoints());
    for (int i = 0; i < endCapPolyData->GetNumberOfPoints(); ++i)
      {
      normals->SetTuple3(i, planeNormal[0], planeNormal[1], planeNormal[2]);
      }
    endCapPolyData->GetPointData()->SetNormals(normals);
    appendFilter->AddInputData(endCapPolyData);
    }
  appendFilter->Update();
  outputEndCap->ShallowCopy(appendFilter->GetOutput());
}

//----------------------------------------------------------------------------
bool vtkCjyxDynamicModelerPlaneCutTool::RunInternal(vtkDMMLDynamicModelerNode* surfaceEditorNode)
{
  if (!this->HasRequiredInputs(surfaceEditorNode))
    {
    vtkErrorMacro("Invalid number of inputs");
    return false;
    }

  vtkDMMLModelNode* outputPositiveModelNode = vtkDMMLModelNode::SafeDownCast(surfaceEditorNode->GetNodeReference(PLANE_CUT_OUTPUT_POSITIVE_MODEL_REFERENCE_ROLE));
  vtkDMMLModelNode* outputNegativeModelNode = vtkDMMLModelNode::SafeDownCast(surfaceEditorNode->GetNodeReference(PLANE_CUT_OUTPUT_NEGATIVE_MODEL_REFERENCE_ROLE));
  if (!outputPositiveModelNode && !outputNegativeModelNode)
    {
    // Nothing to output
    return true;
    }

  vtkNew<vtkImplicitBoolean> planes;
  std::string operationType = this->GetNthInputParameterValue(1, surfaceEditorNode).ToString();
  if (operationType == "Intersection")
    {
    planes->SetOperationTypeToIntersection();
    }
  else if (operationType == "Difference")
    {
    planes->SetOperationTypeToDifference();
    }
  else
    {
    planes->SetOperationTypeToUnion();
    }

  std::vector<vtkDMMLNode*> planeNodes;
  surfaceEditorNode->GetNodeReferences(PLANE_CUT_INPUT_PLANE_REFERENCE_ROLE, planeNodes);
  vtkNew<vtkPlaneCollection> planeCollection;
  int planeIndex = 0;
  for (vtkDMMLNode* planeNode : planeNodes)
    {
    vtkDMMLMarkupsPlaneNode* inputPlaneNode = vtkDMMLMarkupsPlaneNode::SafeDownCast(planeNode);
    vtkDMMLSliceNode* inputSliceNode = vtkDMMLSliceNode::SafeDownCast(planeNode);
    if (!inputPlaneNode && !inputSliceNode)
      {
      vtkErrorMacro("Invalid input plane nodes!");
      return false;
      }

    double origin_World[3] = { 0.0, 0.0, 0.0 };
    double normal_World[3] = { 0.0, 0.0, 1.0 };
    if (inputPlaneNode)
      {
      inputPlaneNode->GetOriginWorld(origin_World);
      inputPlaneNode->GetNormalWorld(normal_World);
      }
    if (inputSliceNode)
      {
      vtkMatrix4x4* sliceToRAS = inputSliceNode->GetSliceToRAS();
      vtkNew<vtkTransform> sliceToRASTransform;
      sliceToRASTransform->SetMatrix(sliceToRAS);
      sliceToRASTransform->TransformPoint(origin_World, origin_World);
      sliceToRASTransform->TransformVector(normal_World, normal_World);
      }

    vtkNew<vtkPlane> currentPlane;
    currentPlane->SetNormal(normal_World);
    currentPlane->SetOrigin(origin_World);
    planeCollection->AddItem(currentPlane);
    planes->AddFunction(currentPlane);
    ++planeIndex;
    }
  this->PlaneClipper->SetClipFunction(planes);

  vtkDMMLModelNode* inputModelNode = vtkDMMLModelNode::SafeDownCast(surfaceEditorNode->GetNodeReference(PLANE_CUT_INPUT_MODEL_REFERENCE_ROLE));
  if (!inputModelNode)
    {
    vtkErrorMacro("Invalid input model node!");
    return false;
    }

  if (!inputModelNode->GetMesh() || inputModelNode->GetMesh()->GetNumberOfPoints() == 0)
    {
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
  if (outputPositiveModelNode && outputPositiveModelNode->GetParentTransformNode())
    {
    outputPositiveModelNode->GetParentTransformNode()->GetTransformFromWorld(this->OutputPositiveWorldToModelTransform);
    }
  if (outputNegativeModelNode && outputNegativeModelNode->GetParentTransformNode())
    {
    outputNegativeModelNode->GetParentTransformNode()->GetTransformFromWorld(this->OutputNegativeWorldToModelTransform);
    }

  this->InputModelToWorldTransformFilter->SetInputConnection(inputModelNode->GetMeshConnection());

  if (outputNegativeModelNode)
    {
    this->PlaneClipper->GenerateClippedOutputOn();
    }
  this->PlaneClipper->Update();

  bool capSurface = this->GetNthInputParameterValue(0, surfaceEditorNode).ToInt() != 0;
  vtkNew<vtkPolyData> endCapPolyData;
  if (capSurface)
    {
    this->CreateEndCap(planeCollection, this->InputModelToWorldTransformFilter->GetOutput(), planes, endCapPolyData);
    }

  if (outputPositiveModelNode)
    {
    vtkNew<vtkPolyData> outputMesh;
    outputMesh->ShallowCopy(this->PlaneClipper->GetOutput());
    if (capSurface)
      {
      vtkNew<vtkAppendPolyData> appendEndCap;
      appendEndCap->AddInputData(outputMesh);
      appendEndCap->AddInputData(endCapPolyData);
      appendEndCap->Update();
      outputMesh->ShallowCopy(appendEndCap->GetOutput());
      }

    this->OutputPositiveWorldToModelTransformFilter->SetInputData(outputMesh);
    this->OutputPositiveWorldToModelTransformFilter->Update();
    outputMesh->DeepCopy(this->OutputPositiveWorldToModelTransformFilter->GetOutput());

    DMMLNodeModifyBlocker blocker(outputPositiveModelNode);
    outputPositiveModelNode->SetAndObserveMesh(outputMesh);
    outputPositiveModelNode->InvokeCustomModifiedEvent(vtkDMMLModelNode::MeshModifiedEvent);
    }

  if (outputNegativeModelNode)
    {
    vtkNew<vtkPolyData> outputMesh;
    outputMesh->ShallowCopy(this->PlaneClipper->GetClippedOutput());
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

    this->OutputNegativeWorldToModelTransformFilter->SetInputData(outputMesh);
    this->OutputNegativeWorldToModelTransformFilter->Update();
    outputMesh->DeepCopy(this->OutputNegativeWorldToModelTransformFilter->GetOutput());

    DMMLNodeModifyBlocker blocker(outputNegativeModelNode);
    outputNegativeModelNode->SetAndObserveMesh(outputMesh);
    outputNegativeModelNode->InvokeCustomModifiedEvent(vtkDMMLModelNode::MeshModifiedEvent);
    }

  return true;
}
