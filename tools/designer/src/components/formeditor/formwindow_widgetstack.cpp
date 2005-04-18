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

#include <QtDesigner/abstractformwindowtool.h>
#include "formwindow_widgetstack.h"

#include <QtGui/QWidget>
#include <QtGui/qevent.h>
#include <QtGui/QAction>

#include <QtCore/qdebug.h>

using namespace qdesigner::components::formeditor;

FormWindowWidgetStack::FormWindowWidgetStack(QWidget *parent)
    : QWidget(parent),
      m_current_index(-1)
{
}

FormWindowWidgetStack::~FormWindowWidgetStack()
{
}

int FormWindowWidgetStack::count() const
{
    return m_tools.count();
}

QDesignerFormWindowToolInterface *FormWindowWidgetStack::currentTool() const
{
    return tool(m_current_index);
}

void FormWindowWidgetStack::setCurrentTool(int index)
{
    if (index < 0 || index >= count()) {
        qWarning("FormWindowWidgetStack::setCurrentTool(): invalid index: %d", index);
        return;
    }

    if (index == m_current_index)
        return;

    if (m_current_index != -1)
        m_tools.at(m_current_index)->deactivated();

    m_current_index = index;

    QDesignerFormWindowToolInterface *tool = m_tools.at(m_current_index);
    tool->activated();
    QWidget *w = tool->editor();
    if (w != 0) {
        if (w->rect() != rect())
            w->setGeometry(rect());
        w->raise();
    }

    emit currentToolChanged(index);
}

void FormWindowWidgetStack::setSenderAsCurrentTool()
{
    QDesignerFormWindowToolInterface *tool = 0;
    QAction *action = qobject_cast<QAction*>(sender());
    if (action == 0) {
        qWarning("FormWindowWidgetStack::setSenderAsCurrentTool(): sender is not a QAction");
        return;
    }

    foreach (QDesignerFormWindowToolInterface *t, m_tools) {
        if (action == t->action()) {
            tool = t;
            break;
        }
    }

    if (tool == 0) {
        qWarning("FormWindowWidgetStack::setSenderAsCurrentTool(): unknown tool");
        return;
    }

    setCurrentTool(tool);
}

int FormWindowWidgetStack::indexOf(QDesignerFormWindowToolInterface *tool) const
{
    for (int i = 0; i < m_tools.size(); ++i) {
        if (m_tools.at(i) == tool)
            return i;
    }

    return -1;
}

void FormWindowWidgetStack::setCurrentTool(QDesignerFormWindowToolInterface *tool)
{
    int index = indexOf(tool);
    if (index == -1) {
        qWarning("FormWindowWidgetStack::setCurrentTool(): unknown tool");
        return;
    }

    setCurrentTool(index);
}

void FormWindowWidgetStack::addTool(QDesignerFormWindowToolInterface *tool)
{
    QWidget *w = tool->editor();
    if (w != 0)
        w->setParent(this);

    m_tools.append(tool);

    connect(tool->action(), SIGNAL(triggered()), this, SLOT(setSenderAsCurrentTool()));
}

void FormWindowWidgetStack::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    QRect r = QRect(0, 0, event->size().width(), event->size().height());

    // We always resize the widget tool
    QDesignerFormWindowToolInterface *widget_tool = tool(0);
    if (widget_tool != 0 && widget_tool->editor() != 0)
        widget_tool->editor()->setGeometry(r);

    QDesignerFormWindowToolInterface *cur_tool = currentTool();
    if (cur_tool == widget_tool)
        return;

    if (cur_tool != 0 && cur_tool->editor() != 0)
        cur_tool->editor()->setGeometry(r);
}

QDesignerFormWindowToolInterface *FormWindowWidgetStack::tool(int index) const
{
    if (index < 0 || index >= count())
        return 0;

    return m_tools.at(index);
}

int FormWindowWidgetStack::currentIndex() const
{
    return m_current_index;
}
