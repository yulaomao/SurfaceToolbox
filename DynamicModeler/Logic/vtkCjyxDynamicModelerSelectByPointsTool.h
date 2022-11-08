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

  This file was originally developed by Mauro I. Dominguez.

==============================================================================*/

#ifndef __vtkCjyxDynamicModelerSelectByPointsTool_h
#define __vtkCjyxDynamicModelerSelectByPointsTool_h

#include "vtkCjyxDynamicModelerModuleLogicExport.h"

// VTK includes
#include <vtkSmartPointer.h>

class vtkFastMarchingGeodesicDistance;
class vtkGeneralTransform;
class vtkDMMLMarkupsFiducialNode;
class vtkPointLocator;
class vtkPolyData;
class vtkTransformPolyDataFilter;
class vtkUnsignedCharArray;

#include "vtkCjyxDynamicModelerTool.h"

/// \brief Dynamic modelling tool to select surface patches on a model using markups fiducials.
///
/// The tool has two input nodes (Surface and Fiducials) and two outputs
/// (surface with "Selection" point scalar values and surface cropped to this selection).
class VTK_CJYX_DYNAMICMODELER_MODULE_LOGIC_EXPORT vtkCjyxDynamicModelerSelectByPointsTool : public vtkCjyxDynamicModelerTool
{
public:
  static vtkCjyxDynamicModelerSelectByPointsTool* New();
  vtkCjyxDynamicModelerTool* CreateToolInstance() override;
  vtkTypeMacro(vtkCjyxDynamicModelerSelectByPointsTool, vtkCjyxDynamicModelerTool);

  /// Human-readable name of the mesh modification tool
  const char* GetName() override;

  /// Run the faces selection on the input model node
  bool RunInternal(vtkDMMLDynamicModelerNode* surfaceEditorNode) override;

  void CreateOutputDisplayNodes(vtkDMMLDynamicModelerNode* surfaceEditorNode) override;
  
protected:

  // Uses cached locator
  bool UpdateUsingSphereRadius(vtkPolyData* inputMesh_World, vtkDMMLMarkupsFiducialNode* fiducialNode,
    double selectionDistance, bool computeSelectionScalarsModel, bool computeSelectedFacesModel,
    vtkUnsignedCharArray* outputSelectionArray, vtkSmartPointer<vtkPolyData>& selectedFacesMesh_World);

  // Uses cached locator and geodesic distance filter
  bool UpdateUsingGeodesicDistance(vtkPolyData* inputMesh_World, vtkDMMLMarkupsFiducialNode* fiducialNode,
    double selectionDistance, bool computeSelectionScalarsModel, bool computeSelectedFacesModel,
    vtkUnsignedCharArray* outputSelectionArray, vtkSmartPointer<vtkPolyData>& selectedFacesMesh_World);

  vtkCjyxDynamicModelerSelectByPointsTool();
  ~vtkCjyxDynamicModelerSelectByPointsTool() override;
  void operator=(const vtkCjyxDynamicModelerSelectByPointsTool&);

protected:
  vtkSmartPointer<vtkTransformPolyDataFilter> InputModelToWorldTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        InputModelNodeToWorldTransform;

  vtkSmartPointer<vtkTransformPolyDataFilter> OutputSelectionScalarsModelTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        OutputSelectionScalarsModelTransform;

  vtkSmartPointer<vtkTransformPolyDataFilter> OutputSelectedFacesModelTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        OutputSelectedFacesModelTransform;

  // Cache output meshes to minimize need for memory reallocation.
  vtkSmartPointer<vtkPolyData> SelectionScalarsOutputMesh;
  vtkSmartPointer<vtkPolyData> SelectedFacesOutputMesh;

  // Cache filters that are expensive to initialize
  vtkSmartPointer<vtkPointLocator> InputMeshLocator_World;
  vtkSmartPointer<vtkFastMarchingGeodesicDistance> GeodesicDistance;

  // Value is 1 for points that are closer to input fiducials than the selection distance, 0 for others.
  vtkSmartPointer<vtkUnsignedCharArray> SelectionArray;

private:
  vtkCjyxDynamicModelerSelectByPointsTool(const vtkCjyxDynamicModelerSelectByPointsTool&) = delete;
};

#endif // __vtkCjyxDynamicModelerSelectByPointsTool_h
