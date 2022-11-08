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

// SubjectHierarchy DMML includes
#include "vtkDMMLSubjectHierarchyNode.h"
#include "vtkDMMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qCjyxSubjectHierarchyPluginHandler.h"
#include "qCjyxSubjectHierarchyDynamicModelerPlugin.h"
#include "qCjyxSubjectHierarchyDefaultPlugin.h"

// DynamicModeler logic includes
#include "vtkCjyxDynamicModelerAppendTool.h"
#include "vtkCjyxDynamicModelerBoundaryCutTool.h"
#include "vtkCjyxDynamicModelerCurveCutTool.h"
#include "vtkCjyxDynamicModelerHollowTool.h"
#include "vtkCjyxDynamicModelerLogic.h"
#include "vtkCjyxDynamicModelerMarginTool.h"
#include "vtkCjyxDynamicModelerMirrorTool.h"
#include "vtkCjyxDynamicModelerPlaneCutTool.h"
#include "vtkCjyxDynamicModelerROICutTool.h"
#include "vtkCjyxDynamicModelerSelectByPointsTool.h"

// DynamicModeler DMML includes
#include <vtkDMMLDynamicModelerNode.h>

// Terminologies includes
#include "qCjyxTerminologyItemDelegate.h"
#include "vtkCjyxTerminologiesModuleLogic.h"

// DMML widgets includes
#include "qDMMLNodeComboBox.h"

// DMML includes
#include <vtkDMMLDisplayableNode.h>
#include <vtkDMMLDisplayNode.h>
#include <vtkDMMLScene.h>

// vtkSegmentationCore includes
#include <vtkSegment.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QInputDialog>
#include <QStandardItem>
#include <QAction>

// CjyxQt includes
#include "qCjyxAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_SubjectHierarchy_Plugins
class qCjyxSubjectHierarchyDynamicModelerPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qCjyxSubjectHierarchyDynamicModelerPlugin);
protected:
  qCjyxSubjectHierarchyDynamicModelerPlugin* const q_ptr;
public:
  qCjyxSubjectHierarchyDynamicModelerPluginPrivate(qCjyxSubjectHierarchyDynamicModelerPlugin& object);
  ~qCjyxSubjectHierarchyDynamicModelerPluginPrivate() override;
  void init();

public:
  QAction* ContinuousUpdateAction { nullptr };
  QAction* UpdateAction{ nullptr };

  vtkCjyxDynamicModelerLogic* DynamicModelerLogic{ nullptr };
};

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyDynamicModelerPluginPrivate methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyDynamicModelerPluginPrivate::qCjyxSubjectHierarchyDynamicModelerPluginPrivate(qCjyxSubjectHierarchyDynamicModelerPlugin& object)
: q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qCjyxSubjectHierarchyDynamicModelerPluginPrivate::init()
{
  Q_Q(qCjyxSubjectHierarchyDynamicModelerPlugin);

  this->ContinuousUpdateAction = new QAction("Continuous update", q);
  this->ContinuousUpdateAction->setCheckable(true);
  QObject::connect(this->ContinuousUpdateAction, SIGNAL(triggered(bool)), q, SLOT(continuousUpdateChanged()));

  this->UpdateAction = new QAction("Update", q);
  QObject::connect(this->UpdateAction, SIGNAL(triggered(bool)), q, SLOT(updateTriggered()));
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyDynamicModelerPluginPrivate::~qCjyxSubjectHierarchyDynamicModelerPluginPrivate()
= default;

//-----------------------------------------------------------------------------
// qCjyxSubjectHierarchyDynamicModelerPlugin methods

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyDynamicModelerPlugin::qCjyxSubjectHierarchyDynamicModelerPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qCjyxSubjectHierarchyDynamicModelerPluginPrivate(*this) )
{
  this->m_Name = QString("DynamicModeler");

  Q_D(qCjyxSubjectHierarchyDynamicModelerPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qCjyxSubjectHierarchyDynamicModelerPlugin::~qCjyxSubjectHierarchyDynamicModelerPlugin()
= default;

//----------------------------------------------------------------------------
double qCjyxSubjectHierarchyDynamicModelerPlugin::canAddNodeToSubjectHierarchy(
  vtkDMMLNode* node, vtkIdType parentItemID/*=vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
    }
  else if (node->IsA("vtkDMMLDynamicModelerNode"))
    {
    return 0.5;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qCjyxSubjectHierarchyDynamicModelerPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  vtkDMMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkDMMLDynamicModelerNode"))
    {
    return 0.5;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qCjyxSubjectHierarchyDynamicModelerPlugin::roleForPlugin()const
{
  return "DynamicModeler";
}

//---------------------------------------------------------------------------
QIcon qCjyxSubjectHierarchyDynamicModelerPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qCjyxSubjectHierarchyDynamicModelerPlugin);

  if (!this->canOwnSubjectHierarchyItem(itemID))
    {
    // Item unknown by plugin
    return QIcon();
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QIcon();
    }

  vtkDMMLDynamicModelerNode* associatedNode = vtkDMMLDynamicModelerNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!associatedNode || !associatedNode->GetToolName())
    {
    return QIcon();
    }

  vtkNew<vtkCjyxDynamicModelerPlaneCutTool> planeCutTool;
  if (strcmp(associatedNode->GetToolName(), planeCutTool->GetName()) == 0)
    {
    return QIcon(":Icons/PlaneCut.png");
    }

  vtkNew<vtkCjyxDynamicModelerCurveCutTool> curveCutTool;
  if (strcmp(associatedNode->GetToolName(), curveCutTool->GetName()) == 0)
    {
    return QIcon(":Icons/CurveCut.png");
    }

  vtkNew<vtkCjyxDynamicModelerBoundaryCutTool> boundaryCutTool;
  if (strcmp(associatedNode->GetToolName(), boundaryCutTool->GetName()) == 0)
    {
    return QIcon(":Icons/BoundaryCut.png");
    }

  vtkNew<vtkCjyxDynamicModelerMirrorTool> mirrorTool;
  if (strcmp(associatedNode->GetToolName(), mirrorTool->GetName()) == 0)
    {
    return QIcon(":Icons/Mirror.png");
    }

  vtkNew<vtkCjyxDynamicModelerHollowTool> hollowTool;
  if (strcmp(associatedNode->GetToolName(), hollowTool->GetName()) == 0)
    {
    return QIcon(":Icons/Hollow.png");
    }

  vtkNew<vtkCjyxDynamicModelerMarginTool> marginTool;
  if (strcmp(associatedNode->GetToolName(), marginTool->GetName()) == 0)
    {
    return QIcon(":Icons/Margin.png");
    }

  vtkNew<vtkCjyxDynamicModelerAppendTool> appendTool;
  if (strcmp(associatedNode->GetToolName(), appendTool->GetName()) == 0)
    {
      return QIcon(":Icons/Append.png");
    }

  vtkNew<vtkCjyxDynamicModelerROICutTool> roiCutTool;
  if (strcmp(associatedNode->GetToolName(), roiCutTool->GetName()) == 0)
    {
      return QIcon(":Icons/ROICut.png");
    }
  
  vtkNew<vtkCjyxDynamicModelerSelectByPointsTool> selectByPointsTool;
  if (strcmp(associatedNode->GetToolName(), selectByPointsTool->GetName()) == 0)
    {
      return QIcon(":Icons/SelectByPoints.png");
    }

  return QIcon();
}

//-----------------------------------------------------------------------------
QList<QAction*> qCjyxSubjectHierarchyDynamicModelerPlugin::itemContextMenuActions()const
{
  Q_D(const qCjyxSubjectHierarchyDynamicModelerPlugin);

  QList<QAction*> actions;
  actions << d->ContinuousUpdateAction << d->UpdateAction;
  return actions;
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyDynamicModelerPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qCjyxSubjectHierarchyDynamicModelerPlugin);

  if (itemID == vtkDMMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
    }
  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkDMMLDynamicModelerNode* associatedNode = vtkDMMLDynamicModelerNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (associatedNode)
    {
    d->ContinuousUpdateAction->setVisible(true);
    d->ContinuousUpdateAction->setChecked(associatedNode->GetContinuousUpdate());

    d->UpdateAction->setVisible(true);
    }
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyDynamicModelerPlugin::continuousUpdateChanged()
{
  Q_D(qCjyxSubjectHierarchyDynamicModelerPlugin);

  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  vtkDMMLNode* node = shNode->GetItemDataNode(currentItemID);
  vtkDMMLDynamicModelerNode* dynamicModelerNode = vtkDMMLDynamicModelerNode::SafeDownCast(node);
  if (!dynamicModelerNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get DynamicModeler node by item ID " << currentItemID;
    return;
    }

  dynamicModelerNode->SetContinuousUpdate(!dynamicModelerNode->GetContinuousUpdate());
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyDynamicModelerPlugin::updateTriggered()
{
  Q_D(qCjyxSubjectHierarchyDynamicModelerPlugin);

  vtkDMMLSubjectHierarchyNode* shNode = qCjyxSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkDMMLScene* scene = qCjyxSubjectHierarchyPluginHandler::instance()->dmmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid DMML scene";
    return;
    }
  vtkIdType currentItemID = qCjyxSubjectHierarchyPluginHandler::instance()->currentItem();
  vtkDMMLNode* node = shNode->GetItemDataNode(currentItemID);
  vtkDMMLDynamicModelerNode* dynamicModelerNode = vtkDMMLDynamicModelerNode::SafeDownCast(node);
  if (!dynamicModelerNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid dynamic modeler node";
    return;
    }
  if (!d->DynamicModelerLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid dynamic modeler logic";
    return;
    }

  d->DynamicModelerLogic->RunDynamicModelerTool(dynamicModelerNode);
}

//-----------------------------------------------------------------------------
void qCjyxSubjectHierarchyDynamicModelerPlugin::setDynamicModelerLogic(vtkCjyxDynamicModelerLogic* dynamicModelerLogic)
{
  Q_D(qCjyxSubjectHierarchyDynamicModelerPlugin);
  d->DynamicModelerLogic = dynamicModelerLogic;
}
