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

#ifndef __vtkCjyxDynamicModelerPlaneCutTool_h
#define __vtkCjyxDynamicModelerPlaneCutTool_h

#include "vtkCjyxDynamicModelerModuleLogicExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// STD includes
#include <map>
#include <string>
#include <vector>

class vtkClipPolyData;
class vtkDataObject;
class vtkGeneralTransform;
class vtkGeometryFilter;
class vtkImplicitBoolean;
class vtkImplicitFunction;
class vtkDMMLDynamicModelerNode;
class vtkPlane;
class vtkPlaneCollection;
class vtkPolyData;
class vtkThreshold;
class vtkTransformPolyDataFilter;

#include "vtkCjyxDynamicModelerTool.h"

/// \brief Dynamic modeler tool for cutting a single surface mesh with planes.
///
/// Has two node inputs (Plane and Surface), and two outputs (Positive/Negative direction surface segments).
class VTK_CJYX_DYNAMICMODELER_MODULE_LOGIC_EXPORT vtkCjyxDynamicModelerPlaneCutTool : public vtkCjyxDynamicModelerTool
{
public:
  static vtkCjyxDynamicModelerPlaneCutTool* New();
  vtkCjyxDynamicModelerTool* CreateToolInstance() override;
  vtkTypeMacro(vtkCjyxDynamicModelerPlaneCutTool, vtkCjyxDynamicModelerTool);

  /// Human-readable name of the mesh modification tool
  const char* GetName() override;

  /// Run the plane cut on the input model node
  bool RunInternal(vtkDMMLDynamicModelerNode* surfaceEditorNode) override;

  /// Create an end cap on the clipped surface
  static void CreateEndCap(vtkPlaneCollection* planes, vtkPolyData* originalPolyData, vtkImplicitBoolean* cutFunction, vtkPolyData* outputEndCap);

protected:
  vtkCjyxDynamicModelerPlaneCutTool();
  ~vtkCjyxDynamicModelerPlaneCutTool() override;
  void operator=(const vtkCjyxDynamicModelerPlaneCutTool&);

protected:
  vtkSmartPointer<vtkTransformPolyDataFilter> InputModelToWorldTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        InputModelNodeToWorldTransform;

  vtkSmartPointer<vtkClipPolyData>            PlaneClipper;

  vtkSmartPointer<vtkTransformPolyDataFilter> OutputPositiveWorldToModelTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        OutputPositiveWorldToModelTransform;

  vtkSmartPointer<vtkTransformPolyDataFilter> OutputNegativeWorldToModelTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        OutputNegativeWorldToModelTransform;

private:
  vtkCjyxDynamicModelerPlaneCutTool(const vtkCjyxDynamicModelerPlaneCutTool&) = delete;
};

#endif // __vtkCjyxDynamicModelerPlaneCutTool_h
