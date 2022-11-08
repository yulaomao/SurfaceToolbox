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

#ifndef __vtkCjyxDynamicModelerBoundaryCutTool_h
#define __vtkCjyxDynamicModelerBoundaryCutTool_h

#include "vtkCjyxDynamicModelerModuleLogicExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// STD includes
#include <map>
#include <string>
#include <vector>

class vtkCleanPolyData;
class vtkClipPolyData;
class vtkConnectivityFilter;
class vtkDataObject;
class vtkGeneralTransform;
class vtkGeometryFilter;
class vtkImplicitBoolean;
class vtkDMMLDynamicModelerNode;
class vtkPlane;
class vtkPointLocator;
class vtkPolyData;
class vtkPolyDataConnectivityFilter;
class vtkReverseSense;
class vtkThreshold;
class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkSelectPolyData;

#include "vtkCjyxDynamicModelerTool.h"

/// \brief Dynamic modeler tool for cutting a single surface mesh with planes.
///
/// Has two node inputs (Plane and Surface), and two outputs (Positive/Negative direction surface segments).
class VTK_CJYX_DYNAMICMODELER_MODULE_LOGIC_EXPORT vtkCjyxDynamicModelerBoundaryCutTool : public vtkCjyxDynamicModelerTool
{
public:
  static vtkCjyxDynamicModelerBoundaryCutTool* New();
  vtkCjyxDynamicModelerTool* CreateToolInstance() override;
  vtkTypeMacro(vtkCjyxDynamicModelerBoundaryCutTool, vtkCjyxDynamicModelerTool);

  /// Human-readable name of the mesh modification tool
  const char* GetName() override;

  /// Run the plane cut on the input model node
  bool RunInternal(vtkDMMLDynamicModelerNode* surfaceEditorNode) override;

protected:
  vtkCjyxDynamicModelerBoundaryCutTool();
  ~vtkCjyxDynamicModelerBoundaryCutTool() override;
  void operator=(const vtkCjyxDynamicModelerBoundaryCutTool&);

  /// Populates the list of seed points corresponding to the regions to be extracted.
  virtual void GetSeedPoints(vtkDMMLDynamicModelerNode* surfaceEditorNode, vtkPoints* points);

  /// The default seed point location. Calculated from the center of all input control points
  virtual void GetDefaultSeedPoint(vtkDMMLDynamicModelerNode* surfaceEditorNode, double seedPoint[3]);

  /// Sets the CellData scalars according to which region each cell belongs to.
  /// Seed scalars start at 1 and are incremented by 1 for each seed.
  virtual void ColorOutputRegions(vtkPoints* seedPoints);

protected:
  vtkSmartPointer<vtkCleanPolyData>              InputCleanFilter;

  vtkSmartPointer<vtkGeneralTransform>           InputModelToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter>    InputModelToWorldTransformFilter;

  vtkSmartPointer<vtkClipPolyData>               ClipPolyData;
  vtkSmartPointer<vtkPolyDataConnectivityFilter> Connectivity;
  vtkSmartPointer<vtkConnectivityFilter>         ColorConnectivity;

  vtkSmartPointer<vtkCleanPolyData>              OutputCleanFilter;

  vtkSmartPointer<vtkGeneralTransform>           OutputWorldToModelTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter>    OutputWorldToModelTransformFilter;

  vtkSmartPointer<vtkPointLocator>               ClippedModelPointLocator;

private:
  vtkCjyxDynamicModelerBoundaryCutTool(const vtkCjyxDynamicModelerBoundaryCutTool&) = delete;
};

#endif // __vtkCjyxDynamicModelerBoundaryCutTool_h
