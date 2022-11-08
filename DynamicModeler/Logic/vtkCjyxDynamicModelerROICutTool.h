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

#ifndef __vtkCjyxDynamicModelerROICutTool_h
#define __vtkCjyxDynamicModelerROICutTool_h

#include "vtkCjyxDynamicModelerModuleLogicExport.h"

// VTK includes
#include <vtkSmartPointer.h>

class vtkClipPolyData;
class vtkGeneralTransform;
class vtkDMMLDynamicModelerNode;
class vtkTransformPolyDataFilter;

#include "vtkCjyxDynamicModelerTool.h"

/// \brief Dynamic modeler tool for cutting a single surface mesh with ROIs.
///
/// Has two node inputs (ROI and Surface), and two outputs (Positive/Negative direction surface segments).
class VTK_CJYX_DYNAMICMODELER_MODULE_LOGIC_EXPORT vtkCjyxDynamicModelerROICutTool : public vtkCjyxDynamicModelerTool
{
public:
  static vtkCjyxDynamicModelerROICutTool* New();
  vtkCjyxDynamicModelerTool* CreateToolInstance() override;
  vtkTypeMacro(vtkCjyxDynamicModelerROICutTool, vtkCjyxDynamicModelerTool);

  /// Human-readable name of the mesh modification tool
  const char* GetName() override;

  /// Run the ROI cut on the input model node
  bool RunInternal(vtkDMMLDynamicModelerNode* dynamicModelerNode) override;


  // Inputs

  /// Reference role used for the input model (vtkDMMLModelNode)
  static const char* GetInputModelReferenceRole();
  /// Reference role used for the input ROI (vtkDMMLMarkupsROINode)
  static const char* GetInputROIReferenceRole();


  // Outputs

  /// Reference role used for the output model that is inside the ROI (vtkDMMLModelNode)
  static const char* GetOutputInsideModelReferenceRole();
  /// Reference role used for the output model that is outside the ROI (vtkDMMLModelNode)
  static const char* GetOutputOutsideModelReferenceRole();


  // Parameters

    /// Node attribute that is used to indicate if the output models should be capped (vtkDMMLModelNode)
  static const char* GetCapSurfaceAttributeName();

protected:
  vtkCjyxDynamicModelerROICutTool();
  ~vtkCjyxDynamicModelerROICutTool() override;
  void operator=(const vtkCjyxDynamicModelerROICutTool&);

protected:
  vtkSmartPointer<vtkTransformPolyDataFilter> InputModelToWorldTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        InputModelNodeToWorldTransform;

  vtkSmartPointer<vtkClipPolyData>            ROIClipper;

  vtkSmartPointer<vtkTransformPolyDataFilter> OutputInsideWorldToModelTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        OutputInsideWorldToModelTransform;

  vtkSmartPointer<vtkTransformPolyDataFilter> OutputOutsideWorldToModelTransformFilter;
  vtkSmartPointer<vtkGeneralTransform>        OutputOutsideWorldToModelTransform;

private:
  vtkCjyxDynamicModelerROICutTool(const vtkCjyxDynamicModelerROICutTool&) = delete;
};

#endif // __vtkCjyxDynamicModelerROICutTool_h
