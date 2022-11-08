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

#ifndef __qCjyxDynamicModelerModuleWidget_h
#define __qCjyxDynamicModelerModuleWidget_h

// CjyxQt includes
#include "qCjyxAbstractModuleWidget.h"

#include "qCjyxDynamicModelerModuleExport.h"

class qCjyxDynamicModelerModuleWidgetPrivate;
class vtkDMMLNode;
class vtkCjyxDynamicModelerTool;

/// \ingroup Cjyx_QtModules_ExtensionTemplate
class Q_CJYX_QTMODULES_DYNAMICMODELER_EXPORT qCjyxDynamicModelerModuleWidget :
  public qCjyxAbstractModuleWidget
{
  Q_OBJECT;

public:
  typedef qCjyxAbstractModuleWidget Superclass;
  qCjyxDynamicModelerModuleWidget(QWidget *parent=0);
  ~qCjyxDynamicModelerModuleWidget() override;

public:
  /// Add a tool button to the top of the widget.
  /// The button will create a new node using the tool when clicked.
  void addToolButton(QIcon icon, vtkCjyxDynamicModelerTool* tool, int row, int column);

protected:
  QScopedPointer<qCjyxDynamicModelerModuleWidgetPrivate> d_ptr;

  void setup() override;

  bool isInputWidgetsRebuildRequired();

  void rebuildInputWidgets();
  void rebuildParameterWidgets();
  void rebuildOutputWidgets();

  void updateInputWidgets();
  void updateParameterWidgets();
  void updateOutputWidgets();

protected slots:
  void onAddToolClicked();
  void onParameterNodeChanged();
  void updateWidgetFromDMML();
  void updateDMMLFromWidget();
  void onApplyButtonClicked();

private:
  Q_DECLARE_PRIVATE(qCjyxDynamicModelerModuleWidget);
  Q_DISABLE_COPY(qCjyxDynamicModelerModuleWidget);
};

#endif
