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

#ifndef __qCjyxDynamicModelerModule_h
#define __qCjyxDynamicModelerModule_h

// CjyxQt includes
#include "qCjyxLoadableModule.h"

#include "qCjyxDynamicModelerModuleExport.h"

class qCjyxDynamicModelerModulePrivate;

/// \ingroup Cjyx_QtModules_ExtensionTemplate
class Q_CJYX_QTMODULES_DYNAMICMODELER_EXPORT
qCjyxDynamicModelerModule
  : public qCjyxLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.cjyx.modules.loadable.qCjyxLoadableModule/1.0");
  Q_INTERFACES(qCjyxLoadableModule);

public:

  typedef qCjyxLoadableModule Superclass;
  explicit qCjyxDynamicModelerModule(QObject *parent=0);
  ~qCjyxDynamicModelerModule() override;

  qCjyxGetTitleMacro(QTMODULE_TITLE);

  QString helpText() const override;
  QString acknowledgementText() const override;
  QStringList contributors() const override;

  QIcon icon() const override;

  QStringList categories() const override;

  QStringList dependencies() const override;

  /// Specify editable node types
  QStringList associatedNodeTypes() const override;

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;
  /// Create and return the widget representation associated to this module
  qCjyxAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkDMMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qCjyxDynamicModelerModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qCjyxDynamicModelerModule);
  Q_DISABLE_COPY(qCjyxDynamicModelerModule);

};

#endif
