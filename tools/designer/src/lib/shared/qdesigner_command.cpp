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

#include "qdesigner_command.h"
#include "layout.h"
#include "qdesigner_widget.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractformwindowmanager.h>
#include <abstractmetadatabase.h>
#include <abstractwidgetfactory.h>
#include <abstractpropertyeditor.h>
#include <qextensionmanager.h>
#include <propertysheet.h>
#include <qdesigner_promotedwidget.h>

#include <QtCore/qdebug.h>

#include <QtGui/QToolBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QSplitter>
#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>

// ---- AbstractFormEditorCommand ----
AbstractFormEditorCommand::AbstractFormEditorCommand(const QString &description, AbstractFormEditor *core)
    : QtCommand(description),
      m_core(core)
{
}

AbstractFormEditor *AbstractFormEditorCommand::core() const
{
    return m_core;
}

// ---- AbstractFormWindowManagerCommand ----
AbstractFormWindowManagerCommand::AbstractFormWindowManagerCommand(const QString &description, AbstractFormWindowManager *formWindowManager)
    : QtCommand(description),
      m_formWindowManager(formWindowManager)
{
}

AbstractFormWindowManager *AbstractFormWindowManagerCommand::formWindowManager() const
{
    return m_formWindowManager;
}

// ---- AbstractFormWindowCommand ----
AbstractFormWindowCommand::AbstractFormWindowCommand(const QString &description, AbstractFormWindow *formWindow)
    : QtCommand(description),
      m_formWindow(formWindow)
{
}

AbstractFormWindow *AbstractFormWindowCommand::formWindow() const
{
    return m_formWindow;
}

bool AbstractFormWindowCommand::hasLayout(QWidget *widget) const
{
    AbstractFormEditor *core = formWindow()->core();
    if (widget && LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout) {
        AbstractMetaDataBaseItem *item = core->metaDataBase()->item(widget);
        return item != 0;
    }

    return false;
}

void AbstractFormWindowCommand::checkObjectName(QWidget *widget)
{
    if (widget->objectName().isEmpty())
        qWarning("invalid object name");

    AbstractFormEditor *core = formWindow()->core();
    if (AbstractMetaDataBaseItem *item = core->metaDataBase()->item(widget)) {
        item->setName(widget->objectName());
    }
}

void AbstractFormWindowCommand::checkSelection(QWidget *widget)
{
    Q_UNUSED(widget);

#if 0 // ### port me
    AbstractFormEditor *core = formWindow()->core();

    formWindow()->updateSelection(widget);

    if (LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout)
        formWindow()->updateChildSelections(widget);
#endif
}

void AbstractFormWindowCommand::checkParent(QWidget *widget, QWidget *parentWidget)
{
    Q_ASSERT(widget);

    if (widget->parentWidget() != parentWidget)
        widget->setParent(parentWidget);
}

// ---- SetPropertyCommand ----
SetPropertyCommand::SetPropertyCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow),
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
            propertyEditor->setPropertyValue(propertyName(), m_newValue, true);
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
            propertyEditor->setPropertyValue(propertyName(), m_oldValue, m_changed);
    }
}

bool SetPropertyCommand::mergeMeWith(QtCommand *other)
{
    if (SetPropertyCommand *cmd = qobject_cast<SetPropertyCommand*>(other)) {
        if (cmd->propertyName() == propertyName() && cmd->widget() == widget()) {
            m_newValue = cmd->newValue();
            return true;
        }
    }

    return false;
}

// ---- ResetPropertyCommand ----
ResetPropertyCommand::ResetPropertyCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow),
      m_index(-1),
      m_propertySheet(0),
      m_changed(false)
{
    setCanMerge(true);
}

QWidget *ResetPropertyCommand::widget() const
{
    return m_widget;
}

QWidget *ResetPropertyCommand::parentWidget() const
{
    return m_parentWidget;
}

void ResetPropertyCommand::init(QWidget *widget, const QString &propertyName)
{
    Q_ASSERT(widget);

    m_widget = widget;
    m_parentWidget = widget->parentWidget();
    m_propertyName = propertyName;

    AbstractFormEditor *core = formWindow()->core();
    m_propertySheet = qt_extension<IPropertySheet*>(core->extensionManager(), widget);
    Q_ASSERT(m_propertySheet);

    m_index = m_propertySheet->indexOf(m_propertyName);
    Q_ASSERT(m_index != -1);

    m_changed = m_propertySheet->isChanged(m_index);
    m_oldValue = m_propertySheet->property(m_index);

    setDescription(tr("reset '%1' of '%2'").arg(m_propertyName).arg(m_widget->objectName()));
}

void ResetPropertyCommand::redo()
{
    Q_ASSERT(m_propertySheet);
    Q_ASSERT(m_index != -1);

    QObject *obj = m_widget;
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(obj))
        obj = promoted->child();

    QVariant new_value;

    if (m_propertySheet->reset(m_index)) {
        new_value = m_propertySheet->property(m_index);
    } else {
        int item_idx =  formWindow()->core()->widgetDataBase()->indexOfObject(obj);
        if (item_idx == -1) {
            new_value = m_oldValue; // We simply don't know the value in this case
        } else {
            AbstractWidgetDataBaseItem *item
                = formWindow()->core()->widgetDataBase()->item(item_idx);
            QList<QVariant> default_prop_values = item->defaultPropertyValues();
            if (m_index < default_prop_values.size())
                new_value = default_prop_values.at(m_index);
            else
                new_value = m_oldValue; // Again, we just don't know
        }

        m_propertySheet->setProperty(m_index, new_value);
    }

    m_propertySheet->setChanged(m_index, false);

    if (m_propertyName == QLatin1String("geometry")) {
        checkSelection(m_widget);
        checkParent(m_widget, m_parentWidget);
    } else if (m_propertyName == QLatin1String("objectName")) {
        checkObjectName(m_widget);
    }

    if (AbstractPropertyEditor *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == widget())
            propertyEditor->setPropertyValue(propertyName(), new_value, false);
    }
}

void ResetPropertyCommand::undo()
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
            propertyEditor->setPropertyValue(propertyName(), m_oldValue, m_changed);
    }
}

// ---- InsertWidgetCommand ----
InsertWidgetCommand::InsertWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
RaiseWidgetCommand::RaiseWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
LowerWidgetCommand::LowerWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
DeleteWidgetCommand::DeleteWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
                m_index = static_cast<QVBoxLayout*>(m_parentWidget->layout())->indexOf(m_widget);
                break;
            case LayoutInfo::HBox:
                m_index = static_cast<QHBoxLayout*>(m_parentWidget->layout())->indexOf(m_widget);
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

    m_formItem = formWindow()->core()->metaDataBase()->item(formWindow());
    m_tabOrderIndex = m_formItem->tabOrder().indexOf(widget);

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

    if (m_tabOrderIndex != -1) {
        QList<QWidget*> tab_order = m_formItem->tabOrder();
        tab_order.removeAt(m_tabOrderIndex);
        m_formItem->setTabOrder(tab_order);
    }

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

    if (m_tabOrderIndex != -1) {
        QList<QWidget*> tab_order = m_formItem->tabOrder();
        tab_order.insert(m_tabOrderIndex, m_widget);
        m_formItem->setTabOrder(tab_order);
    }

    formWindow()->emitSelectionChanged();
}

// ---- ReparentWidgetCommand ----
ReparentWidgetCommand::ReparentWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
LayoutCommand::LayoutCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
BreakLayoutCommand::BreakLayoutCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(tr("Break layout"), formWindow)
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
        m_layout = new HorizontalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qobject_cast<QSplitter*>(m_layoutBase) != 0);
    else if (lay == LayoutInfo::VBox)
        m_layout = new VerticalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qobject_cast<QSplitter*>(m_layoutBase) != 0);
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
ToolBoxCommand::ToolBoxCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
DeleteToolBoxPageCommand::DeleteToolBoxPageCommand(AbstractFormWindow *formWindow)
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
AddToolBoxPageCommand::AddToolBoxPageCommand(AbstractFormWindow *formWindow)
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
TabWidgetCommand::TabWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
DeleteTabPageCommand::DeleteTabPageCommand(AbstractFormWindow *formWindow)
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
AddTabPageCommand::AddTabPageCommand(AbstractFormWindow *formWindow)
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
MoveTabPageCommand::MoveTabPageCommand(AbstractFormWindow *formWindow)
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
StackedWidgetCommand::StackedWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
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
DeleteStackedWidgetPageCommand::DeleteStackedWidgetPageCommand(AbstractFormWindow *formWindow)
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
AddStackedWidgetPageCommand::AddStackedWidgetPageCommand(AbstractFormWindow *formWindow)
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
TabOrderCommand::TabOrderCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(tr("Change Tab order"), formWindow),
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
}

void TabOrderCommand::undo()
{
    m_widgetItem->setTabOrder(m_oldTabOrder);
}

// ---- DockWidgetCommand:: ----
DockWidgetCommand::DockWidgetCommand(const QString &description, AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(description, formWindow)
{
}

DockWidgetCommand::~DockWidgetCommand()
{
}

void DockWidgetCommand::init(QDockWidget *dockWidget)
{
    m_dockWidget = dockWidget;
}

// ---- SetDockWidgetWidgetCommand ----
SetDockWidgetWidgetCommand::SetDockWidgetWidgetCommand(AbstractFormWindow *formWindow)
    : DockWidgetCommand(tr("Set Dock Window Widget"), formWindow)
{
}

void SetDockWidgetWidgetCommand::init(QDockWidget *dockWidget, QWidget *widget)
{
    DockWidgetCommand::init(dockWidget);
    m_widget = widget;
    m_oldWidget = dockWidget->widget();
}

void SetDockWidgetWidgetCommand::undo()
{
    m_dockWidget->setWidget(m_oldWidget);
}

void SetDockWidgetWidgetCommand::redo()
{
    m_dockWidget->setWidget(m_widget);
}

// ---- AddDockWidgetCommand ----
AddDockWidgetCommand::AddDockWidgetCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(tr("Add Dock Window"), formWindow)
{
}

void AddDockWidgetCommand::init(QMainWindow *mainWindow, QDockWidget *dockWidget)
{
    m_mainWindow = mainWindow;
    m_dockWidget = dockWidget;
}

void AddDockWidgetCommand::redo()
{
    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_dockWidget);
}

void AddDockWidgetCommand::undo()
{
    m_mainWindow->removeDockWidget(m_dockWidget);
}

// ---- AdjustWidgetSizeCommand ----
AdjustWidgetSizeCommand::AdjustWidgetSizeCommand(AbstractFormWindow *formWindow)
    : AbstractFormWindowCommand(QString(), formWindow)
{
}

void AdjustWidgetSizeCommand::init(QWidget *widget)
{
    m_widget = widget;
    setDescription(tr("Adjust Size of '%1'").arg(widget->objectName()));
}

void AdjustWidgetSizeCommand::redo()
{
    m_widget->adjustSize();
}

void AdjustWidgetSizeCommand::undo()
{
}
