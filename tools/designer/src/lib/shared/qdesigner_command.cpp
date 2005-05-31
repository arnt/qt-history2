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

#include <QtDesigner/QtDesigner>

#include "qdesigner_command_p.h"
#include "layout_p.h"
#include "qlayout_widget_p.h"
#include "qdesigner_widget_p.h"
#include "qdesigner_promotedwidget_p.h"

#include <QtCore/qdebug.h>

#include <QtGui/QToolBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QSplitter>
#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>

// ---- QDesignerFormEditorCommand ----
QDesignerFormEditorCommand::QDesignerFormEditorCommand(const QString &description, QDesignerFormEditorInterface *core)
    : QtCommand(description),
      m_core(core)
{
}

QDesignerFormEditorInterface *QDesignerFormEditorCommand::core() const
{
    return m_core;
}

// ---- QDesignerFormWindowManagerCommand ----
QDesignerFormWindowManagerCommand::QDesignerFormWindowManagerCommand(const QString &description, QDesignerFormWindowManagerInterface *formWindowManager)
    : QtCommand(description),
      m_formWindowManager(formWindowManager)
{
}

QDesignerFormWindowManagerInterface *QDesignerFormWindowManagerCommand::formWindowManager() const
{
    return m_formWindowManager;
}

// ---- QDesignerFormWindowCommand ----
QDesignerFormWindowCommand::QDesignerFormWindowCommand(const QString &description, QDesignerFormWindowInterface *formWindow)
    : QtCommand(description),
      m_formWindow(formWindow)
{
}

QDesignerFormWindowInterface *QDesignerFormWindowCommand::formWindow() const
{
    return m_formWindow;
}

bool QDesignerFormWindowCommand::hasLayout(QWidget *widget) const
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    if (widget && LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout) {
        QDesignerMetaDataBaseItemInterface *item = core->metaDataBase()->item(widget);
        return item != 0;
    }

    return false;
}

void QDesignerFormWindowCommand::checkObjectName(QWidget *widget)
{
    if (widget->objectName().isEmpty())
        qWarning("invalid object name");

    QDesignerFormEditorInterface *core = formWindow()->core();
    if (QDesignerMetaDataBaseItemInterface *item = core->metaDataBase()->item(widget)) {
        item->setName(widget->objectName());
    }
}

void QDesignerFormWindowCommand::updateBuddies(const QString &old_name,
                                                const QString &new_name)
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    QList<QDesignerLabel*> label_list = qFindChildren<QDesignerLabel*>(formWindow());
    foreach (QDesignerLabel *label, label_list) {
        QDesignerPropertySheetExtension* propertySheet
            = qt_extension<QDesignerPropertySheetExtension*>
                (core->extensionManager(), label);
        if (propertySheet == 0)
            continue;
        int idx = propertySheet->indexOf(QLatin1String("buddy"));
        if (idx == -1)
            continue;
        if (propertySheet->property(idx).toString() == old_name)
            propertySheet->setProperty(idx, new_name);
    }
}

void QDesignerFormWindowCommand::checkSelection(QWidget *widget)
{
    Q_UNUSED(widget);

#if 0 // ### port me
    QDesignerFormEditorInterface *core = formWindow()->core();

    formWindow()->updateSelection(widget);

    if (LayoutInfo::layoutType(core, widget) != LayoutInfo::NoLayout)
        formWindow()->updateChildSelections(widget);
#endif
}

void QDesignerFormWindowCommand::checkParent(QWidget *widget, QWidget *parentWidget)
{
    Q_ASSERT(widget);

    if (widget->parentWidget() != parentWidget)
        widget->setParent(parentWidget);
}

// ---- SetPropertyCommand ----
SetPropertyCommand::SetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
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

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), widget);
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
        updateBuddies(m_oldValue.toString(), m_newValue.toString());
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
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
        updateBuddies(m_newValue.toString(), m_oldValue.toString());
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
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
ResetPropertyCommand::ResetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
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

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), widget);
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
            QDesignerWidgetDataBaseItemInterface *item
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

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
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

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == widget())
            propertyEditor->setPropertyValue(propertyName(), m_oldValue, m_changed);
    }
}

// ---- InsertWidgetCommand ----
InsertWidgetCommand::InsertWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void InsertWidgetCommand::init(QWidget *widget)
{
    m_widget = widget;

    setDescription(tr("Insert '%1'").arg(widget->objectName()));

    QWidget *parentWidget = m_widget->parentWidget();
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

    m_insertMode = deco ? deco->currentInsertMode() : QDesignerLayoutDecorationExtension::InsertWidgetMode;
    m_cell = deco ? deco->currentCell() : qMakePair(0, 0);
}

void InsertWidgetCommand::redo()
{
    checkObjectName(m_widget);

    QWidget *parentWidget = m_widget->parentWidget();

    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

    if (deco != 0) {
        if (LayoutInfo::layoutType(core, parentWidget) == LayoutInfo::Grid) {
            switch (m_insertMode) {
                case QDesignerLayoutDecorationExtension::InsertRowMode: {
                    deco->insertRow(m_cell.first);
                } break;

                case QDesignerLayoutDecorationExtension::InsertColumnMode: {
                    deco->insertColumn(m_cell.second);
                } break;

                default: break;
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

    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parentWidget);

    if (deco) {
        deco->removeWidget(m_widget);
        deco->simplify();
    }

    formWindow()->unmanageWidget(m_widget);
    m_widget->hide();
}

// ---- RaiseWidgetCommand ----
RaiseWidgetCommand::RaiseWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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
LowerWidgetCommand::LowerWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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
DeleteWidgetCommand::DeleteWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), m_parentWidget);

    if (deco)
        deco->removeWidget(m_widget);

    formWindow()->unmanageWidget(m_widget);
    m_widget->setParent(formWindow());
    m_widget->hide();

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
ReparentWidgetCommand::ReparentWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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

// ---- PromoteToCustomWidgetCommand ----

static void replace_widget_item(QDesignerFormWindowInterface *fw, QWidget *wgt, QWidget *promoted)
{
    QDesignerFormEditorInterface *core = fw->core();
    QWidget *parent = wgt->parentWidget();

    QRect info;
    if (QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parent)) {
        QLayout *layout = LayoutInfo::managedLayout(core, parent);
        Q_ASSERT(layout != 0);

        int old_index = layout->indexOf(wgt);
        Q_ASSERT(old_index != -1);

        info = deco->itemInfo(old_index);

        QLayoutItem *item = layout->takeAt(old_index);
        delete item;
        layout->activate();
    }

    if (qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parent)) {
        QLayout *layout = LayoutInfo::managedLayout(core, parent);
        Q_ASSERT(layout != 0);

        // ### check if `info' is valid!

        switch (LayoutInfo::layoutType(core, layout)) {
            default: Q_ASSERT(0); break;

            case LayoutInfo::VBox:
                insert_into_box_layout(static_cast<QBoxLayout*>(layout), info.top(), promoted);
                break;

            case LayoutInfo::HBox:
                insert_into_box_layout(static_cast<QBoxLayout*>(layout), info.left(), promoted);
                break;

            case LayoutInfo::Grid:
                add_to_grid_layout(static_cast<QGridLayout*>(layout), promoted, info.top(), info.left(), info.height(), info.width());
                break;
        }
    }
}

PromoteToCustomWidgetCommand::PromoteToCustomWidgetCommand
                                (QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(tr("Promote to custom widget"), formWindow)
{
    m_widget = 0;
    m_promoted = 0;
}

void PromoteToCustomWidgetCommand::init(QDesignerWidgetDataBaseItemInterface *item,
                                        QWidget *widget)
{
    m_widget = widget;
    m_promoted = new QDesignerPromotedWidget(item, widget->parentWidget());
}

void PromoteToCustomWidgetCommand::redo()
{
    m_promoted->setObjectName(QLatin1String("__qt__promoted_") + m_widget->objectName());
    m_promoted->setGeometry(m_widget->geometry());

    replace_widget_item(formWindow(), m_widget, m_promoted);

    m_promoted->setChildWidget(m_widget);
    formWindow()->manageWidget(m_promoted);

    formWindow()->clearSelection();
    formWindow()->selectWidget(m_promoted);
    m_promoted->show();
}

void PromoteToCustomWidgetCommand::undo()
{
    m_promoted->setChildWidget(0);
    m_widget->setParent(m_promoted->parentWidget());
    m_widget->setGeometry(m_promoted->geometry());

    replace_widget_item(formWindow(), m_promoted, m_widget);

    formWindow()->manageWidget(m_widget);
    formWindow()->unmanageWidget(m_promoted);

    m_promoted->hide();
    m_widget->show();

    formWindow()->clearSelection();
    formWindow()->selectWidget(m_promoted);
}

// ---- DemoteFromCustomWidgetCommand ----

DemoteFromCustomWidgetCommand::DemoteFromCustomWidgetCommand
                                    (QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(tr("Demote from custom widget"), formWindow)
{
    m_promote_cmd = new PromoteToCustomWidgetCommand(formWindow);
}

void DemoteFromCustomWidgetCommand::init(QDesignerPromotedWidget *promoted)
{
    m_promote_cmd->m_widget = promoted->child();
    m_promote_cmd->m_promoted = promoted;
}

void DemoteFromCustomWidgetCommand::redo()
{
    m_promote_cmd->undo();
    m_promote_cmd->m_widget->show();
}

void DemoteFromCustomWidgetCommand::undo()
{
    m_promote_cmd->redo();
}

// ---- LayoutCommand ----
LayoutCommand::LayoutCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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
    formWindow()->simplifySelection(&m_widgets);
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
    QDesignerFormEditorInterface *core = formWindow()->core();

    QWidget *lb = m_layout->layoutBaseWidget();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), lb);

    QWidget *p = m_layout->parentWidget();
    if (!deco && hasLayout(p)) {
        deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), p);
    }

    delete deco; // release the extension

    m_layout->undoLayout();

    if (!m_layoutBase && lb != 0 && !qobject_cast<QLayoutWidget*>(lb)) {
        core->metaDataBase()->add(lb);
        lb->show();
    }

    checkSelection(m_parentWidget);
}

// ---- BreakLayoutCommand ----
BreakLayoutCommand::BreakLayoutCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(tr("Break layout"), formWindow)
{
}

BreakLayoutCommand::~BreakLayoutCommand()
{
}

void BreakLayoutCommand::init(const QList<QWidget*> &widgets, QWidget *layoutBase)
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    m_widgets = widgets;
    m_layoutBase = core->widgetFactory()->containerOfWidget(layoutBase);
    m_layout = 0;

    QPoint grid = formWindow()->grid();

    LayoutInfo::Type lay = LayoutInfo::layoutType(core, m_layoutBase);
    if (lay == LayoutInfo::HBox)
        m_layout = new HorizontalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qobject_cast<QSplitter*>(m_layoutBase) != 0);
    else if (lay == LayoutInfo::VBox)
        m_layout = new VerticalLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, qobject_cast<QSplitter*>(m_layoutBase) != 0);
    else if (lay == LayoutInfo::Grid)
        m_layout = new GridLayout(widgets, m_layoutBase, formWindow(), m_layoutBase, QSize(qMax(5, grid.x()), qMax(5, grid.y())));
    // ### StackedLayout

    Q_ASSERT(m_layout != 0);

    m_layout->sort();

    m_margin = m_layout->margin();
    m_spacing = m_layout->spacing();
}

void BreakLayoutCommand::redo()
{
    if (!m_layout)
        return;

    QDesignerFormEditorInterface *core = formWindow()->core();
    QWidget *lb = m_layout->layoutBaseWidget();
    QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), lb);
    QWidget *p = m_layout->parentWidget();
    if (!deco && hasLayout(p))
        deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), p);

    delete deco; // release the extension

    formWindow()->clearSelection(false);
    m_layout->breakLayout();

    core->metaDataBase()->add(lb);

    foreach (QWidget *widget, m_widgets) {
        widget->resize(widget->size().expandedTo(QSize(16, 16)));
    }
}

void BreakLayoutCommand::undo()
{
    if (!m_layout)
        return;

    formWindow()->clearSelection(false);
    m_layout->doLayout();

    if (m_layoutBase && m_layoutBase->layout()) {
        m_layoutBase->layout()->setSpacing(m_spacing);
        m_layoutBase->layout()->setMargin(m_margin);
    }
}

// ---- ToolBoxCommand ----
ToolBoxCommand::ToolBoxCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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
DeleteToolBoxPageCommand::DeleteToolBoxPageCommand(QDesignerFormWindowInterface *formWindow)
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
AddToolBoxPageCommand::AddToolBoxPageCommand(QDesignerFormWindowInterface *formWindow)
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
    m_widget->setObjectName(tr("page"));
    formWindow()->ensureUniqueObjectName(m_widget);

    setDescription(tr("Add Page"));

    QDesignerFormEditorInterface *core = formWindow()->core();
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
TabWidgetCommand::TabWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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
    m_tabWidget->setCurrentIndex(qMin(m_index, m_tabWidget->count()));
}

void TabWidgetCommand::addPage()
{
    m_widget->setParent(0);
    m_tabWidget->insertTab(m_index, m_widget, m_itemIcon, m_itemText);
    m_widget->show();
    m_tabWidget->setCurrentIndex(m_index);
}

// ---- DeleteTabPageCommand ----
DeleteTabPageCommand::DeleteTabPageCommand(QDesignerFormWindowInterface *formWindow)
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
AddTabPageCommand::AddTabPageCommand(QDesignerFormWindowInterface *formWindow)
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
    m_widget->setObjectName(tr("tab"));
    formWindow()->ensureUniqueObjectName(m_widget);

    setDescription(tr("Add Page"));

    QDesignerFormEditorInterface *core = formWindow()->core();
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
MoveTabPageCommand::MoveTabPageCommand(QDesignerFormWindowInterface *formWindow)
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
StackedWidgetCommand::StackedWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
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
    m_stackedWidget->insertWidget(m_index, m_widget);

    m_widget->show();
}

// ---- DeleteStackedWidgetPageCommand ----
DeleteStackedWidgetPageCommand::DeleteStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
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
AddStackedWidgetPageCommand::AddStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
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
    m_widget->setObjectName(tr("page"));
    formWindow()->ensureUniqueObjectName(m_widget);

    setDescription(tr("Add Page"));

    QDesignerFormEditorInterface *core = formWindow()->core();
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
TabOrderCommand::TabOrderCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(tr("Change Tab order"), formWindow),
      m_widgetItem(0)
{
}

void TabOrderCommand::init(const QList<QWidget*> &newTabOrder)
{
    QDesignerFormEditorInterface *core = formWindow()->core();
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
DockWidgetCommand::DockWidgetCommand(const QString &description, QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(description, formWindow)
{
}

DockWidgetCommand::~DockWidgetCommand()
{
}

void DockWidgetCommand::init(QDockWidget *dockWidget)
{
    m_dockWidget = dockWidget;
}

// ---- SetDockWidgetCommand ----
SetDockWidgetCommand::SetDockWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : DockWidgetCommand(tr("Set Dock Window Widget"), formWindow)
{
}

void SetDockWidgetCommand::init(QDockWidget *dockWidget, QWidget *widget)
{
    DockWidgetCommand::init(dockWidget);
    m_widget = widget;
    m_oldWidget = dockWidget->widget();
}

void SetDockWidgetCommand::undo()
{
    m_dockWidget->setWidget(m_oldWidget);
}

void SetDockWidgetCommand::redo()
{
    formWindow()->unmanageWidget(m_widget);
    formWindow()->core()->metaDataBase()->add(m_widget);
    m_dockWidget->setWidget(m_widget);
}

// ---- AddDockWidgetCommand ----
AddDockWidgetCommand::AddDockWidgetCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(tr("Add Dock Window"), formWindow)
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
AdjustWidgetSizeCommand::AdjustWidgetSizeCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void AdjustWidgetSizeCommand::init(QWidget *widget)
{
    m_widget = widget;
    setDescription(tr("Adjust Size of '%1'").arg(widget->objectName()));
}

void AdjustWidgetSizeCommand::redo()
{
    if (formWindow()->mainContainer() == m_widget && formWindow()->parentWidget()) {
        formWindow()->parentWidget()->resize(m_widget->sizeHint());
    } else {
        m_widget->adjustSize();
    }

    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == m_widget)
            propertyEditor->setPropertyValue(QLatin1String("geometry"), m_widget->geometry(), true);
    }
}

void AdjustWidgetSizeCommand::undo()
{
}

// ---- ChangeLayoutItemGeometry ----
ChangeLayoutItemGeometry::ChangeLayoutItemGeometry(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(tr("Change Layout Item Geometry"), formWindow)
{
}

void ChangeLayoutItemGeometry::init(QWidget *widget, int row, int column, int rowspan, int colspan)
{
    m_widget = widget;
    Q_ASSERT(m_widget->parentWidget() != 0);

    QLayout *layout = LayoutInfo::managedLayout(formWindow()->core(), m_widget->parentWidget());
    Q_ASSERT(layout != 0);

    QGridLayout *grid = qobject_cast<QGridLayout*>(layout);
    Q_ASSERT(grid != 0);

    int itemIndex = grid->indexOf(m_widget);
    Q_ASSERT(itemIndex != -1);

    int current_row, current_column, current_rowspan, current_colspan;
    grid->getItemPosition(itemIndex, &current_row, &current_column, &current_rowspan, &current_colspan);

    m_oldInfo.setRect(current_column, current_row, current_colspan, current_rowspan);
    m_newInfo.setRect(column, row, colspan, rowspan);
}

void ChangeLayoutItemGeometry::changeItemPosition(const QRect &g)
{
    QLayout *layout = LayoutInfo::managedLayout(formWindow()->core(), m_widget->parentWidget());
    Q_ASSERT(layout != 0);

    QGridLayout *grid = qobject_cast<QGridLayout*>(layout);
    Q_ASSERT(grid != 0);

    int itemIndex = grid->indexOf(m_widget);
    Q_ASSERT(itemIndex != -1);

    QLayoutItem *item = grid->takeAt(itemIndex);
    delete item;

    add_to_grid_layout(grid, m_widget, g.top(), g.left(), g.height(), g.width());

    grid->invalidate();
    grid->activate();

    QLayoutSupport::createEmptyCells(grid);

    formWindow()->clearSelection(false);
    formWindow()->selectWidget(m_widget, true);
}

void ChangeLayoutItemGeometry::redo()
{
    changeItemPosition(m_newInfo);
}

void ChangeLayoutItemGeometry::undo()
{
    changeItemPosition(m_oldInfo);
}

// ---- InsertRowCommand ----
InsertRowCommand::InsertRowCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(tr("Insert Row"), formWindow)
{
}

void InsertRowCommand::init(QWidget *widget, int row)
{
    m_widget = widget;
    m_row = row;
}

void InsertRowCommand::redo()
{
}

void InsertRowCommand::undo()
{
}
