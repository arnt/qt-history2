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

#include "tool_widgeteditor.h"
#include "formwindow.h"

// sdk
#include <abstractformwindowmanager.h>
#include <abstractwidgetbox.h>
#include <layoutinfo.h>

#include <QtGui/qevent.h>
#include <QtGui/QAction>
#include <QtCore/qdebug.h>

WidgetEditorTool::WidgetEditorTool(FormWindow *formWindow)
    : AbstractFormWindowTool(formWindow),
      m_formWindow(formWindow)
{
    m_action = new QAction(tr("Edit Widgets"), this);
}

QAction *WidgetEditorTool::action() const
{
    return m_action;
}

WidgetEditorTool::~WidgetEditorTool()
{
}

AbstractFormEditor *WidgetEditorTool::core() const
{
    return m_formWindow->core();
}

AbstractFormWindow *WidgetEditorTool::formWindow() const
{
    return m_formWindow;
}

bool WidgetEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move: {
        if (LayoutInfo::layoutType(core(), managedWidget->parentWidget()) != LayoutInfo::NoLayout) { // ### check me
            m_formWindow->updateSelection(widget);
            if (event->type() != QEvent::Resize)
                m_formWindow->updateChildSelections(widget);
        }
    } break;

    case QEvent::FocusOut:
    case QEvent::FocusIn: {
        if (widget == m_formWindow)
            break;
    } return true;

    case QEvent::KeyPress:
        return handleKeyPressEvent(widget, managedWidget, static_cast<QKeyEvent*>(event));

    case QEvent::KeyRelease:
        return handleKeyReleaseEvent(widget, managedWidget, static_cast<QKeyEvent*>(event));

    case QEvent::MouseMove:
        return handleMouseMoveEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonPress:
        return handleMousePressEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonRelease:
        return handleMouseReleaseEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonDblClick:
        return handleMouseButtonDblClickEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::ContextMenu:
        return handleContextMenu(widget, managedWidget, static_cast<QContextMenuEvent*>(event));

    default: break;

    } // end switch

    return false;
}

// ### remove me

bool WidgetEditorTool::handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e)
{
    Q_UNUSED(widget);

    m_formWindow->handleContextMenu(managedWidget, e);
    return true;
}

bool WidgetEditorTool::handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    Q_UNUSED(widget);

    m_formWindow->handleMouseButtonDblClickEvent(managedWidget, e);
    return true;
}

bool WidgetEditorTool::handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    Q_UNUSED(widget);

    m_formWindow->handleMousePressEvent(managedWidget, e);
    return true;
}

bool WidgetEditorTool::handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    Q_UNUSED(widget);

    m_formWindow->handleMouseMoveEvent(managedWidget, e);
    return true;
}

bool WidgetEditorTool::handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    Q_UNUSED(widget);

    m_formWindow->handleMouseReleaseEvent(managedWidget, e);
    return true;
}

bool WidgetEditorTool::handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e)
{
    Q_UNUSED(widget);

    m_formWindow->handleKeyPressEvent(managedWidget, e);
    return true;
}

bool WidgetEditorTool::handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e)
{
    Q_UNUSED(widget);

    m_formWindow->handleKeyReleaseEvent(managedWidget, e);
    return true;
}

bool WidgetEditorTool::handlePaintEvent(QWidget *widget, QWidget *managedWidget, QPaintEvent *e)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);
    Q_UNUSED(e);

    return false;
}

QWidget *WidgetEditorTool::editor() const
{
    Q_ASSERT(formWindow() != 0);
    return formWindow()->mainContainer();
}

void WidgetEditorTool::activated()
{
    if (core()->widgetBox())
        core()->widgetBox()->setEnabled(true);

    if (m_formWindow == 0)
        return;

    QList<QWidget*> sel = m_formWindow->selectedWidgets();
    foreach (QWidget *w, sel)
        m_formWindow->raiseSelection(w);
}

void WidgetEditorTool::deactivated()
{
    if (core()->widgetBox())
        core()->widgetBox()->setEnabled(false);

    if (m_formWindow == 0)
        return;

    m_formWindow->clearSelection();
}

