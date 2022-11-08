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

#ifndef __vtkDMMLDynamicModelerNode_h
#define __vtkDMMLDynamicModelerNode_h

// DMML includes
#include <vtkDMMLNode.h>

// DynamicModeler includes
#include "vtkCjyxDynamicModelerModuleDMMLExport.h"

/// \ingroup DynamicModeler
/// \brief Parameter node for DynamicModeler
///
/// Stores the tool name, update status and input/output node references required for running dynamic modelling.
/// The tool name is used by the logic to determine what input/output nodes are required to process the dynamic modelling tool,
/// and runs the tool on the input if requested.
/// If ContinuousUpdate and Updating are both true, then the output nodes will automatically be updated when the input nodes
/// are changed.
class VTK_CJYX_DYNAMICMODELER_MODULE_DMML_EXPORT vtkDMMLDynamicModelerNode : public vtkDMMLNode
{
public:
  static vtkDMMLDynamicModelerNode *New();
  vtkTypeMacro(vtkDMMLDynamicModelerNode, vtkDMMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create instance of a DynamicModeler node.
  vtkDMMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a DMML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkDMMLNode *node) override;

  /// Get unique node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "DynamicModeler"; }

public:
  enum
  {
    InputNodeModifiedEvent = 18000, // Event that is invoked when one of the input nodes have been modified
  };

  /// The name of the vtkCjyxDynamicModelerTool that should be used for this node
  vtkGetStringMacro(ToolName);
  vtkSetStringMacro(ToolName);

  /// If continuous update is enabled, the specified tool will be run each time that any of the input nodes are modified.
  vtkGetMacro(ContinuousUpdate, bool);
  vtkSetMacro(ContinuousUpdate, bool);
  vtkBooleanMacro(ContinuousUpdate, bool);

protected:
  vtkDMMLDynamicModelerNode();
  ~vtkDMMLDynamicModelerNode() override;
  vtkDMMLDynamicModelerNode(const vtkDMMLDynamicModelerNode&);
  void operator=(const vtkDMMLDynamicModelerNode&);

  void ProcessDMMLEvents(vtkObject* caller, unsigned long eventID, void* callData) override;

  char* ToolName{ nullptr };
  bool ContinuousUpdate{ false };
};

#endif // __vtkDMMLDynamicModelerNode_h
