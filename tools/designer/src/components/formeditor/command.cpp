/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "command.h"
#include "formeditor.h"
#include "formwindowmanager.h"
#include "formwindow.h"
#include "formwindowcursor.h"
#include "layout.h"
#include "qdesigner_widget.h"

#include <abstractmetadatabase.h>
#include <abstractwidgetfactory.h>
#include <abstractpropertyeditor.h>
#include <qextensionmanager.h>
#include <propertysheet.h>

#include <QToolBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QSplitter>
#include <qdebug.h>

// ---- FormEditorCommand ----
FormEditorCommand::FormEditorCommand(const QString &description, FormEditor *core)
    : QtCommand(description),
      m_core(core)
{
}

FormEditor *FormEditorCommand::core() const
{
    return m_core;
}

// ---- FormWindowManagerCommand ----
FormWindowManagerCommand::FormWindowManagerCommand(const QString &description, FormWindowManager *formWindowManager)
    : QtCommand(description),
      m_formWindowManager(formWindowManager)
{
}

FormWindowManager *FormWindowManagerCommand::formWindowManager() const
{
    return m_formWindowManager;
}

// ---- FormWindowCommand ----
FormWindowCommand::FormWindowCommand(const QString &description, FormWindow *formWindow)
    : QtCommand(description),
      m_formWindow(formWindow)
{
}

FormWindow *FormWindowCommand::formWindow() const
{
    return m_formWindow;
}

bool FormWindowCommand::hasLayout(QWidget *widget) const
{
    AbstractFormEditor *core = formWindow()->core();
    if (widget && LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout) {
        AbstractMetaDataBaseItem *item = core->metaDataBase()->item(widget);
        return item != 0;
    }

    return false;
}

void FormWindowCommand::checkObjectName(QWidget *widget)
{
    if (widget->objectName().isEmpty())
        qWarning("invalid object name");

    AbstractFormEditor *core = formWindow()->core();
    if (AbstractMetaDataBaseItem *item = core->metaDataBase()->item(widget)) {
        item->setName(widget->objectName());
    }
}

void FormWindowCommand::checkSelection(QWidget *widget)
{
    AbstractFormEditor *core = formWindow()->core();

    formWindow()->updateSelection(widget);

    if (LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout)
        formWindow()->updateChildSelections(widget);
}

void FormWindowCommand::checkParent(QWidget *widget, QWidget *parentWidget)
{
    Q_ASSERT(widget);

    if (widget->parentWidget() != parentWidget)
        widget->setParent(parentWidget);
}

// ---- SetPropertyCommand ----
SetPropertyCommand::SetPropertyCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow),
      m_index(-1),
      m_propertySheet(0),
      m_changed(false)
{
    setCanMerge(true);
}

QWidget *SetPropertyCommand::widget() const
{
    return m_widget;
}

QWidget *SetPropertyCommand::parentWidget() const
{
    return m_parentWidget;
}

void SetPropertyCommand::init(QWidget *widget, const QString &propertyName, const QVariant &newValue)
{
    Q_ASSERT(widget);

    m_widget = widget;
    m_parentWidget = widget->parentWidget();
    m_propertyName = propertyName;
    m_newValue = newValue;

    AbstractFormEditor *core = formWindow()->core();
    m_propertySheet = qt_extension<IPropertySheet*>(core->extensionManager(), widget);
    Q_ASSERT(m_propertySheet);

    m_index = m_propertySheet->indexOf(m_propertyName);
    Q_ASSERT(m_index != -1);

    m_changed = m_propertySheet->isChanged(m_index);
    m_oldValue = m_propertySheet->property(m_index);

    setDescription(tr("changed '%1' of '%2'").arg(m_propertyName).arg(m_widget->objectName()));
}

void SetPropertyCommand::redo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    m_propertySheet->setProperty(m_index, m_newValue);
    m_changed = m_propertySheet->isChanged(m_index);
    m_propertySheet->setChanged(m_index, true);

    if (m_propertyName == QLatin1String("geometry")) {
        checkSelection(m_widget);
        checkParent(m_widget, m_parentWidget);
    } else if (m_propertyName == QLatin1String("objectName")) {
        checkObjectName(m_widget);
    }

    if (AbstractPropertyEditor *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == widget())
            propertyEditor->setPropertyValue(propertyName(), m_newValue);
    }
}

void SetPropertyCommand::undo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    m_propertySheet->setProperty(m_index, m_oldValue);
    m_propertySheet->setChanged(m_index, m_changed);

    if (m_propertyName == QLatin1String("geometry")) {
        checkSelection(m_widget);
        checkParent(m_widget, m_parentWidget);
    } else if (m_propertyName == QLatin1String("objectName")) {
        checkObjectName(m_widget);
    }

    if (AbstractPropertyEditor *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == widget())
            propertyEditor->setPropertyValue(propertyName(), m_oldValue);
    }
}

bool SetPropertyCommand::mergeMeWith(QtCommand *other)
{
    if (SetPropertyCommand *cmd = qt_cast<SetPropertyCommand*>(other)) {
        if (cmd->propertyName() == propertyName() && cmd->widget() == widget()) {
            m_newValue = cmd->newValue();
            return true;
        }
    }

    return false;
}

// ---- InsertWidgetCommand ----
InsertWidgetCommand::InsertWidgetCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

void InsertWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;

    setDescription(tr("Insert '%1'").arg(widget->objectName()));

    QWidget *parentWidget = m_widget->parentWidget();
    AbstractFormEditor *core = formWindow()->core();
    ILayoutDecoration *deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), parentWidget);

    if (!deco && hasLayout(parentWidget))
        deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), parentWidget);

    m_insertMode = deco ? deco->currentInsertMode() : ILayoutDecoration::InsertWidgetMode;
    m_cell = deco ? deco->currentCell() : qMakePair(0,0);
}

void InsertWidgetCommand::redo()
{
    checkObjectName(m_widget);

    QWidget *parentWidget = m_widget->parentWidget();

    AbstractFormEditor *core = formWindow()->core();
    ILayoutDecoration *deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), parentWidget);

    if (!deco && hasLayout(parentWidget))
        deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), parentWidget);

    if (deco) {
        if (LayoutInfo::layoutType(core, parentWidget) == LayoutInfo::Grid) {
            switch (m_insertMode) {
                case ILayoutDecoration::InsertRowMode:
                    deco->insertRow(m_cell.first);
                    break;

                case ILayoutDecoration::InsertColumnMode:
                    deco->insertColumn(m_cell.second);
                    break;

                default:
                    break;
            } // end switch
        }

        deco->insertWidget(m_widget, m_cell);
    }

    formWindow()->manageWidget(m_widget);
    m_widget->show();
}

void InsertWidgetCommand::undo()
{
    QWidget *parentWidget = m_widget->parentWidget();

    AbstractFormEditor *core = formWindow()->core();
    ILayoutDecoration *deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), parentWidget);
    if (!deco && hasLayout(parentWidget))
        deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), parentWidget);

    if (deco) {
        deco->removeWidget(m_widget);
        deco->simplify();
    }

    formWindow()->unmanageWidget(m_widget);
    m_widget->hide();
}

// ---- RaiseWidgetCommand ----
RaiseWidgetCommand::RaiseWidgetCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

void RaiseWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;
    setDescription(tr("Raise '%1'").arg(widget->objectName()));
}

void RaiseWidgetCommand::redo()
{
    m_widget->raise();
}

void RaiseWidgetCommand::undo()
{
}

// ---- LowerWidgetCommand ----
LowerWidgetCommand::LowerWidgetCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

void LowerWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;
    setDescription(tr("Lower '%1'").arg(widget->objectName()));
}

void LowerWidgetCommand::redo()
{
    m_widget->raise();
}

void LowerWidgetCommand::undo()
{
}

// ---- DeleteWidgetCommand ----
DeleteWidgetCommand::DeleteWidgetCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

void DeleteWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;
    m_parentWidget = widget->parentWidget();
    m_geometry = widget->geometry();

    m_layoutType = LayoutInfo::NoLayout;
    m_index = -1;
    if (hasLayout(m_parentWidget)) {
        m_layoutType = LayoutInfo::layoutType(formWindow()->core(), m_parentWidget);

        switch (m_layoutType) {
            case LayoutInfo::VBox:
                m_index = static_cast<QVBoxLayout*>(m_parentWidget->layout())->findWidget(m_widget);
                break;
            case LayoutInfo::HBox:
                m_index = static_cast<QHBoxLayout*>(m_parentWidget->layout())->findWidget(m_widget);
                break;
            case LayoutInfo::Grid: {
                m_index = 0;
                while (QLayoutItem *item = m_parentWidget->layout()->itemAt(m_index)) {
                    if (item->widget() == m_widget)
                        break;
                    ++m_index;
                }

                static_cast<QGridLayout*>(m_parentWidget->layout())->getItemPosition(m_index, &m_row, &m_col, &m_rowspan, &m_colspan);
            } break;

            default:
                break;
        } // end switch
    }

    setDescription(tr("Delete '%1'").arg(widget->objectName()));
}

void DeleteWidgetCommand::redo()
{
    AbstractFormEditor *core = formWindow()->core();
    ILayoutDecoration *deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), m_parentWidget);
    if (!deco && hasLayout(m_parentWidget))
        deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), m_parentWidget);

    if (deco)
        deco->removeWidget(m_widget);

    formWindow()->unmanageWidget(m_widget);
    m_widget->hide();
    m_widget->setParent(formWindow());

    formWindow()->emitSelectionChanged();
}

void DeleteWidgetCommand::undo()
{
    m_widget->setParent(m_parentWidget);
    m_widget->setGeometry(m_geometry);
    formWindow()->manageWidget(m_widget);

    // ### set up alignment
    switch (m_layoutType) {
        case LayoutInfo::VBox: {
            QVBoxLayout *vbox = static_cast<QVBoxLayout*>(m_parentWidget->layout());
            insert_into_box_layout(vbox, m_index, m_widget);
        } break;

        case LayoutInfo::HBox: {
            QHBoxLayout *hbox = static_cast<QHBoxLayout*>(m_parentWidget->layout());
            insert_into_box_layout(hbox, m_index, m_widget);
        } break;

        case LayoutInfo::Grid: {
            QGridLayout *grid = static_cast<QGridLayout*>(m_parentWidget->layout());
            add_to_grid_layout(grid, m_widget, m_row, m_col, m_rowspan, m_colspan);
        } break;

        default:
            break;
    } // end switch

    m_widget->show();
    formWindow()->emitSelectionChanged();
}

// ---- ReparentWidgetCommand ----
ReparentWidgetCommand::ReparentWidgetCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

void ReparentWidgetCommand::init(QWidget *widget, QWidget *parentWidget)
{
    Q_ASSERT(widget);

    m_widget = widget;
    m_oldParentWidget = widget->parentWidget();
    m_newParentWidget = parentWidget;

    m_oldPos = m_widget->pos();
    m_newPos = m_newParentWidget->mapFromGlobal(m_oldParentWidget->mapToGlobal(m_oldPos));

    setDescription(tr("Reparent '%1'").arg(widget->objectName()));
}

void ReparentWidgetCommand::redo()
{
    m_widget->setParent(m_newParentWidget);
    m_widget->move(m_newPos);
    m_widget->show();
}

void ReparentWidgetCommand::undo()
{
    m_widget->setParent(m_oldParentWidget);
    m_widget->move(m_oldPos);
    m_widget->show();
}

// ---- LayoutCommand ----
LayoutCommand::LayoutCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

LayoutCommand::~LayoutCommand()
{
    m_layout->deleteLater();
}

void LayoutCommand::init(QWidget *parentWidget, const QList<QWidget*> &widgets, LayoutInfo::Type layoutType,
        QWidget *layoutBase, bool splitter)
{
    m_parentWidget = parentWidget;
    m_widgets = widgets;
    QPoint grid = formWindow()->grid();
    QSize sz(qMax(5, grid.x()), qMax(5, grid.y()));

    switch (layoutType) {
        case LayoutInfo::Grid:
            m_layout = new GridLayout(widgets, m_parentWidget, formWindow(), layoutBase, sz);
            setDescription(tr("Lay out using grid"));
            break;
        case LayoutInfo::VBox:
            m_layout = new VerticalLayout(widgets, m_parentWidget, formWindow(), layoutBase, splitter);
            setDescription(tr("Lay out vertically"));
            break;
        case LayoutInfo::HBox:
            m_layout = new HorizontalLayout(widgets, m_parentWidget, formWindow(), layoutBase, splitter);
            setDescription(tr("Lay out horizontally"));
            break;
        default:
            Q_ASSERT(0);
    }

    m_layout->setup();
}

void LayoutCommand::redo()
{
    m_layout->doLayout();
    checkSelection(m_parentWidget);
}

void LayoutCommand::undo()
{
    m_layout->undoLayout();

    AbstractFormEditor *core = formWindow()->core();
    ILayoutDecoration *deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), m_layoutBase);

    delete deco; // release the extension

    checkSelection(m_parentWidget);
}

// ---- BreakLayoutCommand ----
BreakLayoutCommand::BreakLayoutCommand(FormWindow *formWindow)
    : FormWindowCommand(tr("Break layout"), formWindow)
{
}

BreakLayoutCommand::~BreakLayoutCommand()
{
}

void BreakLayoutCommand::init(const QList<QWidget*> &widgets, QWidget *layoutBase)
{
    m_widgets = widgets;
    m_layoutBase = layoutBase;

    AbstractFormEditor *core = formWindow()->core();
    LayoutInfo::Type lay = LayoutInfo::layoutType(core, m_layoutBase);

    AbstractMetaDataBase *metaDataBase = core->metaDataBase();
    if (AbstractMetaDataBaseItem *item = metaDataBase->item(m_layoutBase)) {
        m_margin = item->margin();
        m_spacing = item->spacing();
    }

    QPoint grid = formWindow()->grid();

    m_layout = 0;
    if (lay == LayoutInfo::HBox)
        m_layout = new HorizontalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qt_cast<QSplitter*>(m_layoutBase) != 0);
    else if (lay == LayoutInfo::VBox)
        m_layout = new VerticalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qt_cast<QSplitter*>(m_layoutBase) != 0);
    else if (lay == LayoutInfo::Grid)
        m_layout = new GridLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, QSize(qMax(5, grid.x()), qMax(5, grid.y())));
    // ### StackedLayout

}

void BreakLayoutCommand::redo()
{
    if (!m_layout)
        return;

    formWindow()->clearSelection(false);
    m_layout->breakLayout();
    for (int i = 0; i < m_widgets.size(); ++i) {
        QWidget *w = m_widgets.at(i);
        w->resize(qMax(16, w->width()), qMax(16, w->height()));
    }

    AbstractFormEditor *core = formWindow()->core();
    ILayoutDecoration *deco = qt_extension<ILayoutDecoration*>(core->extensionManager(), m_layoutBase);

    delete deco; // release the extension
}

void BreakLayoutCommand::undo()
{
    if (!m_layout)
        return;

    formWindow()->clearSelection(false);
    m_layout->doLayout();

    AbstractFormEditor *core = formWindow()->core();

    QWidget *container = core->widgetFactory()->containerOfWidget(m_layoutBase);

    if (AbstractMetaDataBaseItem *item = core->metaDataBase()->item(container)) {
        item->setSpacing(m_spacing);
        item->setMargin(m_margin);
    }
}

// ---- ToolBoxCommand ----
ToolBoxCommand::ToolBoxCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

ToolBoxCommand::~ToolBoxCommand()
{
}

void ToolBoxCommand::init(QToolBox *toolBox)
{
    m_toolBox = toolBox;
    m_index = m_toolBox->currentIndex();
    m_widget = m_toolBox->widget(m_index);
    m_itemText = m_toolBox->itemText(m_index);
    m_itemIcon = m_toolBox->itemIcon(m_index);
}

void ToolBoxCommand::removePage()
{
    m_toolBox->removeItem(m_index);

    m_widget->hide();
    m_widget->setParent(formWindow());
}

void ToolBoxCommand::addPage()
{
    m_widget->setParent(m_toolBox);
    m_toolBox->insertItem(m_index, m_widget, m_itemIcon, m_itemText);

    m_widget->show();
}

// ---- DeleteToolBoxPageCommand ----
DeleteToolBoxPageCommand::DeleteToolBoxPageCommand(FormWindow *formWindow)
    : ToolBoxCommand(formWindow)
{
}

DeleteToolBoxPageCommand::~DeleteToolBoxPageCommand()
{
}

void DeleteToolBoxPageCommand::init(QToolBox *toolBox)
{
    ToolBoxCommand::init(toolBox);
    setDescription(tr("Delete Page"));
}

void DeleteToolBoxPageCommand::redo()
{
    removePage();
}

void DeleteToolBoxPageCommand::undo()
{
    addPage();
}

// ---- AddToolBoxPageCommand ----
AddToolBoxPageCommand::AddToolBoxPageCommand(FormWindow *formWindow)
    : ToolBoxCommand(formWindow)
{
}

AddToolBoxPageCommand::~AddToolBoxPageCommand()
{
}

void AddToolBoxPageCommand::init(QToolBox *toolBox)
{
    m_toolBox = toolBox;

    m_index = m_toolBox->currentIndex();
    m_widget = new QDesignerWidget(formWindow(), m_toolBox);
    m_itemText = tr("Page");
    m_itemIcon = QIcon();

    setDescription(tr("Add Page"));

    AbstractFormEditor *core = formWindow()->core();
    core->metaDataBase()->add(m_widget);
}

void AddToolBoxPageCommand::redo()
{
    addPage();
}

void AddToolBoxPageCommand::undo()
{
    removePage();
}

// ---- TabWidgetCommand ----
TabWidgetCommand::TabWidgetCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

TabWidgetCommand::~TabWidgetCommand()
{
}

void TabWidgetCommand::init(QTabWidget *tabWidget)
{
    m_tabWidget = tabWidget;
    m_index = m_tabWidget->currentIndex();
    m_widget = m_tabWidget->widget(m_index);
    m_itemText = m_tabWidget->tabText(m_index);
    m_itemIcon = m_tabWidget->tabIcon(m_index);
}

void TabWidgetCommand::removePage()
{
    m_tabWidget->removeTab(m_index);

    m_widget->hide();
    m_widget->setParent(formWindow());
}

void TabWidgetCommand::addPage()
{
    m_widget->setParent(m_tabWidget);
    m_tabWidget->insertTab(m_index, m_widget, m_itemIcon, m_itemText);

    m_widget->show();
}

// ---- DeleteTabPageCommand ----
DeleteTabPageCommand::DeleteTabPageCommand(FormWindow *formWindow)
    : TabWidgetCommand(formWindow)
{
}

DeleteTabPageCommand::~DeleteTabPageCommand()
{
}

void DeleteTabPageCommand::init(QTabWidget *tabWidget)
{
    TabWidgetCommand::init(tabWidget);
    setDescription(tr("Delete Page"));
}

void DeleteTabPageCommand::redo()
{
    removePage();
}

void DeleteTabPageCommand::undo()
{
    addPage();
}

// ---- AddTabPageCommand ----
AddTabPageCommand::AddTabPageCommand(FormWindow *formWindow)
    : TabWidgetCommand(formWindow)
{
}

AddTabPageCommand::~AddTabPageCommand()
{
}

void AddTabPageCommand::init(QTabWidget *tabWidget)
{
    m_tabWidget = tabWidget;

    m_index = m_tabWidget->currentIndex();
    m_widget = new QDesignerWidget(formWindow(), m_tabWidget);
    m_itemText = tr("Page");
    m_itemIcon = QIcon();

    setDescription(tr("Add Page"));

    AbstractFormEditor *core = formWindow()->core();
    core->metaDataBase()->add(m_widget);
}

void AddTabPageCommand::redo()
{
    addPage();
}

void AddTabPageCommand::undo()
{
    removePage();
}

// ---- MoveTabPageCommand ----
MoveTabPageCommand::MoveTabPageCommand(FormWindow *formWindow)
    : TabWidgetCommand(formWindow)
{
}

MoveTabPageCommand::~MoveTabPageCommand()
{
}

void MoveTabPageCommand::init(QTabWidget *tabWidget, QWidget *page,
                      const QIcon &icon, const QString &label,
                      int index, int newIndex)
{
    TabWidgetCommand::init(tabWidget);
    setDescription(tr("Move Page"));

    m_page = page;
    m_newIndex = newIndex;
    m_oldIndex = index;
    m_label = label;
    m_icon = icon;
}

void MoveTabPageCommand::redo()
{
    m_tabWidget->removeTab(m_oldIndex);
    m_tabWidget->insertTab(m_newIndex, m_page, m_icon, m_label);
    m_tabWidget->setCurrentIndex(m_newIndex);
}

void MoveTabPageCommand::undo()
{
    m_tabWidget->removeTab(m_newIndex);
    m_tabWidget->insertTab(m_oldIndex, m_page, m_icon, m_label);
    m_tabWidget->setCurrentIndex(m_oldIndex);
}

// ---- StackedWidgetCommand ----
StackedWidgetCommand::StackedWidgetCommand(FormWindow *formWindow)
    : FormWindowCommand(QString::null, formWindow)
{
}

StackedWidgetCommand::~StackedWidgetCommand()
{
}

void StackedWidgetCommand::init(QStackedWidget *stackedWidget)
{
    m_stackedWidget = stackedWidget;
    m_index = m_stackedWidget->currentIndex();
    m_widget = m_stackedWidget->widget(m_index);
}

void StackedWidgetCommand::removePage()
{
    m_stackedWidget->removeWidget(m_stackedWidget->widget(m_index));

    m_widget->hide();
    m_widget->setParent(formWindow());
}

void StackedWidgetCommand::addPage()
{
    m_widget->setParent(m_stackedWidget);
    m_stackedWidget->insertWidget(m_index, m_widget);

    m_widget->show();
}

// ---- DeleteStackedWidgetPageCommand ----
DeleteStackedWidgetPageCommand::DeleteStackedWidgetPageCommand(FormWindow *formWindow)
    : StackedWidgetCommand(formWindow)
{
}

DeleteStackedWidgetPageCommand::~DeleteStackedWidgetPageCommand()
{
}

void DeleteStackedWidgetPageCommand::init(QStackedWidget *stackedWidget)
{
    StackedWidgetCommand::init(stackedWidget);
    setDescription(tr("Delete Page"));
}

void DeleteStackedWidgetPageCommand::redo()
{
    removePage();
}

void DeleteStackedWidgetPageCommand::undo()
{
    addPage();
}

// ---- AddStackedWidgetPageCommand ----
AddStackedWidgetPageCommand::AddStackedWidgetPageCommand(FormWindow *formWindow)
    : StackedWidgetCommand(formWindow)
{
}

AddStackedWidgetPageCommand::~AddStackedWidgetPageCommand()
{
}

void AddStackedWidgetPageCommand::init(QStackedWidget *stackedWidget)
{
    m_stackedWidget = stackedWidget;

    m_index = m_stackedWidget->currentIndex();
    m_widget = new QDesignerWidget(formWindow(), m_stackedWidget);

    setDescription(tr("Add Page"));

    AbstractFormEditor *core = formWindow()->core();
    core->metaDataBase()->add(m_widget);
}

void AddStackedWidgetPageCommand::redo()
{
    addPage();
}

void AddStackedWidgetPageCommand::undo()
{
    removePage();
}

// ---- TabOrderCommand ----
TabOrderCommand::TabOrderCommand(FormWindow *formWindow)
    : FormWindowCommand(tr("Change Tab order"), formWindow),
      m_widgetItem(0)
{
}

void TabOrderCommand::init(const QList<QWidget*> &newTabOrder)
{
    AbstractFormEditor *core = formWindow()->core();
    Q_ASSERT(core);

    m_widgetItem = core->metaDataBase()->item(formWindow());
    Q_ASSERT(m_widgetItem);
    m_oldTabOrder = m_widgetItem->tabOrder();
    m_newTabOrder = newTabOrder;
}

void TabOrderCommand::redo()
{
    m_widgetItem->setTabOrder(m_newTabOrder);
    formWindow()->updateOrderIndicators();
}

void TabOrderCommand::undo()
{
    m_widgetItem->setTabOrder(m_oldTabOrder);
    formWindow()->updateOrderIndicators();
}
