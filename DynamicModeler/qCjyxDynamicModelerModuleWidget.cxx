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

// Qt includes
#include <QCheckBox>
#include <QDebug>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

// ctk includes
#include <ctkDoubleSpinBox.h>

// CjyxQt includes
#include <qDMMLNodeComboBox.h>

// DynamicModeler Module includes
#include "qCjyxDynamicModelerModuleWidget.h"
#include "ui_qCjyxDynamicModelerModuleWidget.h"

// DMML includes
#include <vtkDMMLScene.h>
#include <vtkDMMLNode.h>

// VTK includes
#include <vtkStringArray.h>

// DynamicModeler Logic includes
#include <vtkCjyxDynamicModelerAppendTool.h>
#include <vtkCjyxDynamicModelerBoundaryCutTool.h>
#include <vtkCjyxDynamicModelerCurveCutTool.h>
#include <vtkCjyxDynamicModelerHollowTool.h>
#include <vtkCjyxDynamicModelerMarginTool.h>
#include <vtkCjyxDynamicModelerLogic.h>
#include <vtkCjyxDynamicModelerMirrorTool.h>
#include <vtkCjyxDynamicModelerPlaneCutTool.h>
#include <vtkCjyxDynamicModelerROICutTool.h>
#include <vtkCjyxDynamicModelerSelectByPointsTool.h>
#include <vtkCjyxDynamicModelerToolFactory.h>

// DynamicModeler DMML includes
#include <vtkDMMLDynamicModelerNode.h>

// Subject hierarchy includes
#include <qDMMLSubjectHierarchyModel.h>

//-----------------------------------------------------------------------------
/// \ingroup Cjyx_QtModules_ExtensionTemplate
class qCjyxDynamicModelerModuleWidgetPrivate: public Ui_qCjyxDynamicModelerModuleWidget
{
public:
  qCjyxDynamicModelerModuleWidgetPrivate();
  vtkWeakPointer<vtkDMMLDynamicModelerNode> DynamicModelerNode{ nullptr };

  std::string CurrentToolName;
};

//-----------------------------------------------------------------------------
// qCjyxDynamicModelerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qCjyxDynamicModelerModuleWidgetPrivate::qCjyxDynamicModelerModuleWidgetPrivate()
= default;

//-----------------------------------------------------------------------------
// qCjyxDynamicModelerModuleWidget methods

//-----------------------------------------------------------------------------
qCjyxDynamicModelerModuleWidget::qCjyxDynamicModelerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qCjyxDynamicModelerModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qCjyxDynamicModelerModuleWidget::~qCjyxDynamicModelerModuleWidget()
= default;

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::setup()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->SubjectHierarchyTreeView->setMultiSelection(false);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->idColumn(), true);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->colorColumn(), true);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->transformColumn(), true);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->descriptionColumn(), true);

  int buttonPosition = 1;
  const int columns = 5;

  vtkNew<vtkCjyxDynamicModelerPlaneCutTool> planeCutTool;
  this->addToolButton(QIcon(":/Icons/PlaneCut.png"), planeCutTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerCurveCutTool> curveCutTool;
  this->addToolButton(QIcon(":/Icons/CurveCut.png"), curveCutTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerBoundaryCutTool> boundaryCutTool;
  this->addToolButton(QIcon(":/Icons/BoundaryCut.png"), boundaryCutTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerHollowTool> hollowTool;
  this->addToolButton(QIcon(":/Icons/Hollow.png"), hollowTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerMarginTool> marginTool;
  this->addToolButton(QIcon(":/Icons/Margin.png"), marginTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerMirrorTool> mirrorTool;
  this->addToolButton(QIcon(":/Icons/Mirror.png"), mirrorTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerAppendTool> appendTool;
  this->addToolButton(QIcon(":/Icons/Append.png"), appendTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerROICutTool> roiTool;
  this->addToolButton(QIcon(":/Icons/ROICut.png"), roiTool, buttonPosition / columns, (buttonPosition++) % columns);

  vtkNew<vtkCjyxDynamicModelerSelectByPointsTool> selectByPointsTool;
  this->addToolButton(QIcon(":/Icons/SelectByPoints.png"), selectByPointsTool, buttonPosition / columns, (buttonPosition++) % columns);

  connect(d->SubjectHierarchyTreeView, SIGNAL(currentItemChanged(vtkIdType)),
    this, SLOT(onParameterNodeChanged()));
  connect(d->ApplyButton, SIGNAL(checkStateChanged(Qt::CheckState)),
    this, SLOT(onApplyButtonClicked()));
  connect(d->ApplyButton, SIGNAL(clicked()),
    this, SLOT(onApplyButtonClicked()));
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::addToolButton(QIcon icon, vtkCjyxDynamicModelerTool* tool, int row, int column)
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  if (!tool)
    {
    qCritical() << "Invalid tool object!";
    }

  QPushButton* button = new QPushButton();
  button->setIcon(icon);
  if (tool->GetName())
    {
    button->setToolTip(tool->GetName());
    button->setProperty("ToolName", tool->GetName());
    }
  d->ButtonLayout->addWidget(button, row, column);

  connect(button, SIGNAL(clicked()), this, SLOT(onAddToolClicked()));
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::onAddToolClicked()
{
  Q_D(qCjyxDynamicModelerModuleWidget);

  if (!QObject::sender() || !this->dmmlScene())
    {
    return;
    }

  QString toolName = QObject::sender()->property("ToolName").toString();
  std::string nodeName = this->dmmlScene()->GenerateUniqueName(toolName.toStdString());

  vtkNew<vtkDMMLDynamicModelerNode> dynamicModelerNode;
  dynamicModelerNode->SetName(nodeName.c_str());
  dynamicModelerNode->SetToolName(toolName.toUtf8());
  this->dmmlScene()->AddNode(dynamicModelerNode);
  d->SubjectHierarchyTreeView->setCurrentNode(dynamicModelerNode);
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::onParameterNodeChanged()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  if (!this->dmmlScene() || this->dmmlScene()->IsBatchProcessing())
    {
    return;
    }

  vtkDMMLDynamicModelerNode* meshModifyNode = vtkDMMLDynamicModelerNode::SafeDownCast(d->SubjectHierarchyTreeView->currentNode());
  qvtkReconnect(d->DynamicModelerNode, meshModifyNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDMML()));

  d->DynamicModelerNode = meshModifyNode;
  this->updateWidgetFromDMML();
}

//-----------------------------------------------------------------------------
bool qCjyxDynamicModelerModuleWidget::isInputWidgetsRebuildRequired()
{
  Q_D(qCjyxDynamicModelerModuleWidget);

  vtkCjyxDynamicModelerLogic* dynamicModelerLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  vtkCjyxDynamicModelerTool* tool = nullptr;
  if (dynamicModelerLogic && d->DynamicModelerNode)
    {
    tool = dynamicModelerLogic->GetDynamicModelerTool(d->DynamicModelerNode);
    }
  if (!tool)
    {
    return true;
    }

  std::map<std::string, int> numberOfInputWidgetsByReferenceRole;
  QList<qDMMLNodeComboBox*> inputNodeSelectors = d->InputNodesCollapsibleButton->findChildren<qDMMLNodeComboBox*>();
  for (qDMMLNodeComboBox* inputNodeSelector : inputNodeSelectors)
    {
    QString referenceRole = inputNodeSelector->property("ReferenceRole").toString();
    numberOfInputWidgetsByReferenceRole[referenceRole.toStdString()] += 1;
    }

  for (int i = 0; i < tool->GetNumberOfInputNodes(); ++i)
    {
    std::string inputReferenceRole = tool->GetNthInputNodeReferenceRole(i);
    int expectedNumberOfInputWidgets = d->DynamicModelerNode->GetNumberOfNodeReferences(inputReferenceRole.c_str());
    if (tool->GetNthInputNodeRepeatable(i))
      {
      expectedNumberOfInputWidgets += 1;
      }
    int numberOfInputWidgets = numberOfInputWidgetsByReferenceRole[inputReferenceRole];
    if (numberOfInputWidgets != expectedNumberOfInputWidgets)
      {
      // We expected a different number of input widgets.
      // A rebuild is required.
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::rebuildInputWidgets()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  vtkCjyxDynamicModelerTool* tool = nullptr;
  vtkCjyxDynamicModelerLogic* meshModifyLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  if (meshModifyLogic && d->DynamicModelerNode)
    {
    tool = meshModifyLogic->GetDynamicModelerTool(d->DynamicModelerNode);
    }

  QList<QWidget*> widgets = d->InputNodesCollapsibleButton->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
  for (QWidget* widget : widgets)
    {
    widget->deleteLater();
    }

  if (tool == nullptr || tool->GetNumberOfInputNodes() == 0)
    {
    d->InputNodesCollapsibleButton->setEnabled(false);
    return;
    }
  d->InputNodesCollapsibleButton->setEnabled(true);

  QWidget* inputNodesWidget = new QWidget();
  QFormLayout* inputNodesLayout = new QFormLayout();
  inputNodesWidget->setLayout(inputNodesLayout);
  d->InputNodesCollapsibleButton->layout()->addWidget(inputNodesWidget);
  for (int inputIndex = 0; inputIndex < tool->GetNumberOfInputNodes(); ++inputIndex)
    {
    std::string name = tool->GetNthInputNodeName(inputIndex);
    std::string description = tool->GetNthInputNodeDescription(inputIndex);
    std::string referenceRole = tool->GetNthInputNodeReferenceRole(inputIndex);
    vtkStringArray* classNameArray = tool->GetNthInputNodeClassNames(inputIndex);
    QStringList classNames;
    for (int classNameIndex = 0; classNameIndex < classNameArray->GetNumberOfValues(); ++classNameIndex)
      {
      vtkStdString className = classNameArray->GetValue(classNameIndex);
      classNames << className.c_str();
      }

    int numberOfInputs = 1;
    if (tool->GetNthInputNodeRepeatable(inputIndex))
      {
      numberOfInputs = d->DynamicModelerNode->GetNumberOfNodeReferences(referenceRole.c_str()) + 1;
      }

    for (int inputSelectorIndex = 0; inputSelectorIndex < numberOfInputs; ++inputSelectorIndex)
      {
      QLabel* nodeLabel = new QLabel();
      std::stringstream labelTextSS;
      labelTextSS << name;
      if (tool->GetNthInputNodeRepeatable(inputIndex))
        {
        labelTextSS << " [" << inputSelectorIndex + 1 << "]"; // Start index at 1
        }
      labelTextSS << ":";

      std::string labelText = labelTextSS.str();
      nodeLabel->setText(labelText.c_str());
      nodeLabel->setToolTip(description.c_str());

      qDMMLNodeComboBox* nodeSelector = new qDMMLNodeComboBox();
      nodeSelector->setNodeTypes(classNames);
      nodeSelector->setToolTip(description.c_str());
      nodeSelector->setNoneEnabled(true);
      nodeSelector->setDMMLScene(this->dmmlScene());
      nodeSelector->setProperty("ReferenceRole", referenceRole.c_str());
      nodeSelector->setProperty("InputIndex", inputIndex);
      nodeSelector->setProperty("InputSelectorIndex", inputSelectorIndex);
      nodeSelector->setAddEnabled(false);
      nodeSelector->setRemoveEnabled(false);
      nodeSelector->setRenameEnabled(false);

      inputNodesLayout->addRow(nodeLabel, nodeSelector);

      connect(nodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
        this, SLOT(updateDMMLFromWidget()));
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::rebuildParameterWidgets()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  vtkCjyxDynamicModelerLogic* meshModifyLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  vtkCjyxDynamicModelerTool* tool = nullptr;
  if (meshModifyLogic && d->DynamicModelerNode)
    {
    tool = meshModifyLogic->GetDynamicModelerTool(d->DynamicModelerNode);
    }

  QList<QWidget*> widgets = d->ParametersCollapsibleButton->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
  for (QWidget* widget : widgets)
    {
    widget->deleteLater();
    }

  if (tool == nullptr || tool->GetNumberOfInputParameters() == 0)
    {
    d->ParametersCollapsibleButton->setEnabled(false);
    d->ParametersCollapsibleButton->setVisible(false);
    return;
    }
  d->ParametersCollapsibleButton->setEnabled(true);
  d->ParametersCollapsibleButton->setVisible(true);

  QWidget* inputParametersWidget = new QWidget();
  QFormLayout* inputParametersLayout = new QFormLayout();
  inputParametersWidget->setLayout(inputParametersLayout);
  d->ParametersCollapsibleButton->layout()->addWidget(inputParametersWidget);
  for (int i = 0; i < tool->GetNumberOfInputParameters(); ++i)
    {
    std::string name = tool->GetNthInputParameterName(i);
    std::string description = tool->GetNthInputParameterDescription(i);
    std::string attributeName = tool->GetNthInputParameterAttributeName(i);
    int type = tool->GetNthInputParameterType(i);

    QLabel* parameterLabel = new QLabel();
    std::stringstream labelTextSS;
    labelTextSS << name << ":";
    std::string labelText = labelTextSS.str();
    parameterLabel->setText(labelText.c_str());
    parameterLabel->setToolTip(description.c_str());

    QWidget* parameterSelector = nullptr;
    if (type == vtkCjyxDynamicModelerTool::PARAMETER_BOOL)
      {
      QCheckBox* checkBox = new QCheckBox();
      parameterSelector = checkBox;
      connect(checkBox, SIGNAL(stateChanged(int)),
        this, SLOT(updateDMMLFromWidget()));
      }
    else if (type == vtkCjyxDynamicModelerTool::PARAMETER_INT)
      {
      QSpinBox* spinBox = new QSpinBox();
      connect(spinBox, SIGNAL(valueChanged(int)),
        this, SLOT(updateDMMLFromWidget()));
      parameterSelector = spinBox;
      }
    else if (type == vtkCjyxDynamicModelerTool::PARAMETER_DOUBLE)
      {
      ctkDoubleSpinBox* doubleSpinBox = new ctkDoubleSpinBox();
      doubleSpinBox->setMinimum(-doubleSpinBox->maximum()); // allow negative values
      connect(doubleSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(updateDMMLFromWidget()));
      parameterSelector = doubleSpinBox;
      }
    else if (type == vtkCjyxDynamicModelerTool::PARAMETER_STRING_ENUM)
      {
      QComboBox* enumComboBox = new QComboBox();
      vtkStringArray* possibleValues = tool->GetNthInputParameterPossibleValues(i);
      for (int valueIndex = 0; valueIndex < possibleValues->GetNumberOfValues(); ++valueIndex)
        {
        enumComboBox->addItem(QString::fromStdString(possibleValues->GetValue(valueIndex)));
        }
      connect(enumComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(updateDMMLFromWidget()));
      parameterSelector = enumComboBox;
      }
    else
      {
      QLineEdit* lineEdit = new QLineEdit();
      connect(lineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(updateDMMLFromWidget()));
      parameterSelector = lineEdit;
      }

    parameterSelector->setObjectName(attributeName.c_str());
    parameterSelector->setToolTip(description.c_str());
    parameterSelector->setProperty("AttributeName", attributeName.c_str());
    inputParametersLayout->addRow(parameterLabel, parameterSelector);
    }
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::rebuildOutputWidgets()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  vtkCjyxDynamicModelerLogic* meshModifyLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  vtkCjyxDynamicModelerTool* tool = nullptr;
  if (meshModifyLogic && d->DynamicModelerNode)
    {
    tool = meshModifyLogic->GetDynamicModelerTool(d->DynamicModelerNode);
    }

  QList<QWidget*> widgets = d->OutputNodesCollapsibleButton->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
  for (QWidget* widget : widgets)
    {
    widget->deleteLater();
    }

  if (tool == nullptr || tool->GetNumberOfOutputNodes() == 0)
    {
    d->OutputNodesCollapsibleButton->setEnabled(false);
    return;
    }
  d->OutputNodesCollapsibleButton->setEnabled(true);

  QWidget* outputNodesWidget = new QWidget();
  QFormLayout* outputNodesLayout = new QFormLayout();
  outputNodesWidget->setLayout(outputNodesLayout);
  d->OutputNodesCollapsibleButton->layout()->addWidget(outputNodesWidget);
  for (int i = 0; i < tool->GetNumberOfOutputNodes(); ++i)
    {
    std::string name = tool->GetNthOutputNodeName(i);
    std::string description = tool->GetNthOutputNodeDescription(i);
    std::string referenceRole = tool->GetNthOutputNodeReferenceRole(i);
    vtkStringArray* classNameArray = tool->GetNthOutputNodeClassNames(i);
    QStringList classNames;
    for (int i = 0; i < classNameArray->GetNumberOfValues(); ++i)
      {
      vtkStdString className = classNameArray->GetValue(i);
      classNames << className.c_str();
      }

    QLabel* nodeLabel = new QLabel();
    std::stringstream labelTextSS;
    labelTextSS << name << ":";
    std::string labelText = labelTextSS.str();
    nodeLabel->setText(labelText.c_str());
    nodeLabel->setToolTip(description.c_str());

    qDMMLNodeComboBox* nodeSelector = new qDMMLNodeComboBox();
    nodeSelector->setNodeTypes(classNames);
    nodeSelector->setToolTip(description.c_str());
    nodeSelector->setNoneEnabled(true);
    nodeSelector->setDMMLScene(this->dmmlScene());
    nodeSelector->setProperty("ReferenceRole", referenceRole.c_str());
    nodeSelector->setAddEnabled(true);
    nodeSelector->setRemoveEnabled(true);
    nodeSelector->setRenameEnabled(true);

    outputNodesLayout->addRow(nodeLabel, nodeSelector);

    connect(nodeSelector, SIGNAL(currentNodeChanged(vtkDMMLNode*)),
      this, SLOT(updateDMMLFromWidget()));
    }
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::updateInputWidgets()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  if (!d->DynamicModelerNode)
    {
    return;
    }

  QList<qDMMLNodeComboBox*> inputNodeSelectors = d->InputNodesCollapsibleButton->findChildren<qDMMLNodeComboBox*>();
  for (qDMMLNodeComboBox* inputNodeSelector : inputNodeSelectors)
    {
    QString referenceRole = inputNodeSelector->property("ReferenceRole").toString();
    int inputSelectorIndex = inputNodeSelector->property("InputSelectorIndex").toInt();
    vtkDMMLNode* referenceNode = d->DynamicModelerNode->GetNthNodeReference(referenceRole.toUtf8(), inputSelectorIndex);
    bool wasBlocking = inputNodeSelector->blockSignals(true);
    inputNodeSelector->setCurrentNode(referenceNode);
    inputNodeSelector->blockSignals(wasBlocking);
    }
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::updateParameterWidgets()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  vtkCjyxDynamicModelerLogic* meshModifyLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  vtkCjyxDynamicModelerTool* tool = nullptr;
  if (meshModifyLogic && d->DynamicModelerNode)
    {
    tool = meshModifyLogic->GetDynamicModelerTool(d->DynamicModelerNode);
    }
  if (!d->DynamicModelerNode || tool == nullptr || tool->GetNumberOfInputParameters() == 0)
    {
    return;
    }

  for (int i = 0; i < tool->GetNumberOfInputParameters(); ++i)
    {
    std::string name = tool->GetNthInputParameterName(i);
    std::string description = tool->GetNthInputParameterDescription(i);
    std::string attributeName = tool->GetNthInputParameterAttributeName(i);
    vtkVariant value = tool->GetNthInputParameterValue(i, d->DynamicModelerNode);
    int type = tool->GetNthInputParameterType(i);

    QWidget* parameterSelector = d->ParametersCollapsibleButton->findChild<QWidget*>(attributeName.c_str());
    if (type == vtkCjyxDynamicModelerTool::PARAMETER_BOOL)
      {
      QCheckBox* checkBox = qobject_cast<QCheckBox*>(parameterSelector);
      if (!checkBox)
        {
        qCritical() << "Could not find widget for parameter " << name.c_str();
        continue;
        }

      bool wasBlocking = checkBox->blockSignals(true);
      bool checked = value.ToInt() != 0;
      checkBox->setChecked(checked);
      checkBox->blockSignals(wasBlocking);
      }
    else if (type == vtkCjyxDynamicModelerTool::PARAMETER_INT)
      {
      QSpinBox* spinBox = qobject_cast<QSpinBox*>(parameterSelector);
      if (!spinBox)
        {
        qCritical() << "Could not find widget for parameter " << name.c_str();
        continue;
        }
      bool wasBlocking = spinBox->blockSignals(true);
      spinBox->setValue(value.ToInt());
      spinBox->blockSignals(wasBlocking);
      }
    else if (type == vtkCjyxDynamicModelerTool::PARAMETER_DOUBLE)
      {
      ctkDoubleSpinBox* doubleSpinBox = qobject_cast<ctkDoubleSpinBox*>(parameterSelector);
      if (!doubleSpinBox)
        {
        qCritical() << "Could not find widget for parameter " << name.c_str();
        continue;
        }
      bool wasBlocking = doubleSpinBox->blockSignals(true);
      doubleSpinBox->setValue(value.ToDouble());
      doubleSpinBox->blockSignals(wasBlocking);
      }
    else if (type == vtkCjyxDynamicModelerTool::PARAMETER_STRING_ENUM)
      {
      QComboBox* comboBox = qobject_cast<QComboBox*>(parameterSelector);
      if (!comboBox)
        {
        qCritical() << "Could not find widget for parameter " << name.c_str();
        continue;
        }
      bool wasBlocking = comboBox->blockSignals(true);
      int index = comboBox->findText(QString::fromStdString(value.ToString()));
      comboBox->setCurrentIndex(index);
      comboBox->blockSignals(wasBlocking);
      }
    else
      {
      QLineEdit* lineEdit = qobject_cast<QLineEdit*>(parameterSelector);
      if (!lineEdit)
        {
        qCritical() << "Could not find widget for parameter " << name.c_str();
        continue;
        }
      int cursorPosition = lineEdit->cursorPosition();
      bool wasBlocking = lineEdit->blockSignals(true);
      lineEdit->setText(QString::fromStdString(value.ToString()));
      lineEdit->setCursorPosition(cursorPosition);
      lineEdit->blockSignals(wasBlocking);
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::updateOutputWidgets()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  if (!d->DynamicModelerNode)
    {
    return;
    }

  QList< qDMMLNodeComboBox*> outputNodeSelectors = d->OutputNodesCollapsibleButton->findChildren<qDMMLNodeComboBox*>();
  for (qDMMLNodeComboBox* outputNodeSelector : outputNodeSelectors)
    {
    QString referenceRole = outputNodeSelector->property("ReferenceRole").toString();
    vtkDMMLNode* referenceNode = d->DynamicModelerNode->GetNodeReference(referenceRole.toUtf8());
    bool wasBlocking = outputNodeSelector->blockSignals(true);
    outputNodeSelector->setCurrentNode(referenceNode);
    outputNodeSelector->blockSignals(wasBlocking);
    }
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::updateWidgetFromDMML()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  vtkCjyxDynamicModelerLogic* meshModifyLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  vtkCjyxDynamicModelerTool* tool = nullptr;
  if (meshModifyLogic && d->DynamicModelerNode)
    {
    tool = meshModifyLogic->GetDynamicModelerTool(d->DynamicModelerNode);
    }
  d->ApplyButton->setEnabled(tool != nullptr && tool->HasRequiredInputs(d->DynamicModelerNode) &&
    tool->HasOutput(d->DynamicModelerNode));

  std::string toolName = "";
  if (d->DynamicModelerNode && d->DynamicModelerNode->GetToolName())
    {
    toolName = d->DynamicModelerNode->GetToolName();
    }

  if (toolName != d->CurrentToolName)
    {
    this->rebuildInputWidgets();
    this->rebuildParameterWidgets();
    this->rebuildOutputWidgets();
    d->CurrentToolName = toolName;
    }
  else if (this->isInputWidgetsRebuildRequired())
    {
    this->rebuildInputWidgets();
    }

  this->updateInputWidgets();
  this->updateParameterWidgets();
  this->updateOutputWidgets();

  bool wasBlocking = d->ApplyButton->blockSignals(true);
  if (d->DynamicModelerNode && d->DynamicModelerNode->GetContinuousUpdate())
    {
    d->ApplyButton->setCheckState(Qt::Checked);
    }
  else
    {
    d->ApplyButton->setCheckState(Qt::Unchecked);
    }
  d->ApplyButton->blockSignals(wasBlocking);
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::updateDMMLFromWidget()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  if (!d->DynamicModelerNode)
    {
    return;
    }

  DMMLNodeModifyBlocker blocker(d->DynamicModelerNode);

  // Continuous update
  d->DynamicModelerNode->SetContinuousUpdate(d->ApplyButton->checkState() == Qt::Checked);

  // If no tool is specified, there is nothing else to update
  vtkCjyxDynamicModelerLogic* dynamicModelerLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  vtkCjyxDynamicModelerTool* tool = nullptr;
  if (dynamicModelerLogic && d->DynamicModelerNode)
    {
    tool = dynamicModelerLogic->GetDynamicModelerTool(d->DynamicModelerNode);
    }
  if (!tool)
    {
    return;
    }

  for (int i = 0; i < tool->GetNumberOfInputNodes(); ++i)
    {
    std::string referenceRole = tool->GetNthInputNodeReferenceRole(i);
    d->DynamicModelerNode->RemoveNodeReferenceIDs(referenceRole.c_str());
    }

  // Update all input reference roles from the parameter node
  QList<qDMMLNodeComboBox*> inputNodeSelectors = d->InputNodesCollapsibleButton->findChildren<qDMMLNodeComboBox*>();
  for (qDMMLNodeComboBox* inputNodeSelector : inputNodeSelectors)
    {
    QString referenceRole = inputNodeSelector->property("ReferenceRole").toString();
    QString currentNodeID = inputNodeSelector->currentNodeID();
    d->DynamicModelerNode->AddNodeReferenceID(referenceRole.toUtf8(), currentNodeID.toUtf8());
    }

  QList<qDMMLNodeComboBox*> outputNodeSelectors = d->OutputNodesCollapsibleButton->findChildren<qDMMLNodeComboBox*>();
  for (qDMMLNodeComboBox* outputNodeSelector : outputNodeSelectors)
    {
    QString referenceRole = outputNodeSelector->property("ReferenceRole").toString();
    QString currentNodeID = outputNodeSelector->currentNodeID();
    d->DynamicModelerNode->SetNodeReferenceID(referenceRole.toUtf8(), currentNodeID.toUtf8());
    }

  d->ApplyButton->setToolTip("");
  d->ApplyButton->setCheckBoxUserCheckable(true);
  // If a node is selected in both the input and outputs, then disable continuous updates
  if (dynamicModelerLogic->HasCircularReference(d->DynamicModelerNode))
    {
    d->DynamicModelerNode->SetContinuousUpdate(false);
    d->ApplyButton->setToolTip("Output node detected in input. Continuous update is not availiable.");
    d->ApplyButton->setCheckBoxUserCheckable(false);
    }

  QList<QWidget*> parameterSelectors = d->ParametersCollapsibleButton->findChildren<QWidget*>();
  for (QWidget* parameterSelector : parameterSelectors)
    {
    vtkVariant value;
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(parameterSelector);
    if (checkBox)
      {
      value = checkBox->isChecked() ? 1 : 0;
      }

    QSpinBox* spinBox = qobject_cast<QSpinBox*>(parameterSelector);
    if (spinBox)
      {
      value = spinBox->value();
      }

    ctkDoubleSpinBox* doubleSpinBox = qobject_cast<ctkDoubleSpinBox*>(parameterSelector);
    if (doubleSpinBox)
      {
      value = doubleSpinBox->value();
      }

    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(parameterSelector);
    if (lineEdit)
      {
      std::string text = lineEdit->text().toStdString();
      value = text.c_str();
      }

    QComboBox* comboBox = qobject_cast<QComboBox*>(parameterSelector);
    if (comboBox)
      {
      std::string text = comboBox->currentText().toStdString();
      value = text.c_str();
      }

    std::string attributeName = parameterSelector->property("AttributeName").toString().toStdString();
    if (attributeName != "")
      {
      d->DynamicModelerNode->SetAttribute(attributeName.c_str(), value.ToString());
      }
    }
}

//-----------------------------------------------------------------------------
void qCjyxDynamicModelerModuleWidget::onApplyButtonClicked()
{
  Q_D(qCjyxDynamicModelerModuleWidget);
  if (!d->DynamicModelerNode)
    {
    return;
    }
  this->updateDMMLFromWidget();

  /// Checkbox is checked. Should be handled by continuous update in logic
  if (d->ApplyButton->checkState() == Qt::Checked)
    {
    return;
    }

  /// Continuous update is off, trigger manual update.
  vtkCjyxDynamicModelerLogic* meshModifyLogic = vtkCjyxDynamicModelerLogic::SafeDownCast(this->logic());
  meshModifyLogic->RunDynamicModelerTool(d->DynamicModelerNode);
}
