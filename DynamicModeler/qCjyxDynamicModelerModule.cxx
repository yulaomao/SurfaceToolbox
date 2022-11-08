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

// DynamicModeler Logic includes
#include <vtkCjyxDynamicModelerLogic.h>

// DynamicModeler includes
#include "qCjyxDynamicModelerModule.h"
#include "qCjyxDynamicModelerModuleWidget.h"

// DynamicModeler subject hierarchy includes
#include "qCjyxSubjectHierarchyDynamicModelerPlugin.h"

// Subject hierarchy includes
#include <qCjyxSubjectHierarchyPluginHandler.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_ExtensionTemplate
class qCjyxDynamicModelerModulePrivate
{
public:
  qCjyxDynamicModelerModulePrivate();
};

//-----------------------------------------------------------------------------
// qCjyxDynamicModelerModulePrivate methods

//-----------------------------------------------------------------------------
qCjyxDynamicModelerModulePrivate::qCjyxDynamicModelerModulePrivate()
= default;

//-----------------------------------------------------------------------------
// qCjyxDynamicModelerModule methods

//-----------------------------------------------------------------------------
qCjyxDynamicModelerModule::qCjyxDynamicModelerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qCjyxDynamicModelerModulePrivate)
{
}

//-----------------------------------------------------------------------------
qCjyxDynamicModelerModule::~qCjyxDynamicModelerModule()
= default;

//-----------------------------------------------------------------------------
QString qCjyxDynamicModelerModule::helpText() const
{
  return "This module allows surface mesh editing using dynamic modelling tools and operations";
}

//-----------------------------------------------------------------------------
QString qCjyxDynamicModelerModule::acknowledgementText() const
{
  return "This work was partially funded by CANARIE's Research Software Program,"
    "OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.";
}

//-----------------------------------------------------------------------------
QStringList qCjyxDynamicModelerModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Kyle Sunderland (PerkLab, Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qCjyxDynamicModelerModule::icon() const
{
  return QIcon(":/Icons/DynamicModeler.png");
}

//-----------------------------------------------------------------------------
QStringList qCjyxDynamicModelerModule::categories() const
{
  return QStringList() << "Surface Models";
}

//-----------------------------------------------------------------------------
QStringList qCjyxDynamicModelerModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModule::setup()
{
  this->Superclass::setup();

  vtkCjyxDynamicModelerLogic* dynamicModelerLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());

  // Register Subject Hierarchy core plugins
  qCjyxSubjectHierarchyDynamicModelerPlugin* dynamicModelerPlugin = new qCjyxSubjectHierarchyDynamicModelerPlugin();
  dynamicModelerPlugin->setDynamicModelerLogic(dynamicModelerLogic);
  qCjyxSubjectHierarchyPluginHandler::instance()->registerPlugin(dynamicModelerPlugin);
}

//-----------------------------------------------------------------------------
qCjyxAbstractModuleRepresentation* qCjyxDynamicModelerModule
::createWidgetRepresentation()
{
  return new qCjyxDynamicModelerModuleWidget;
}

//-----------------------------------------------------------------------------
vtkDMMLAbstractLogic* qCjyxDynamicModelerModule::createLogic()
{
  return vtkCjyxDynamicModelerLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qCjyxDynamicModelerModule::associatedNodeTypes() const
{
  return QStringList()
    << "vtkDMMLAnnotationFiducialNode";
}