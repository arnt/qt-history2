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

#include "resourceeditor_tool.h"
#include "resourceeditor.h"

#include <abstractformwindow.h>
#include <abstractformeditor.h>

#include <QtGui/QAction>

ResourceEditorTool::ResourceEditorTool(AbstractFormWindow *formWindow, QObject *parent)
    : AbstractFormWindowTool(parent),
      m_formWindow(formWindow)
{
    m_action = new QAction(tr("Edit Resources"), this);
}

ResourceEditorTool::~ResourceEditorTool()
{
}

AbstractFormEditor *ResourceEditorTool::core() const
{
    return m_formWindow->core();
}

AbstractFormWindow *ResourceEditorTool::formWindow() const
{
    return m_formWindow;
}

bool ResourceEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);
    Q_UNUSED(event);

    return false;
}

QWidget *ResourceEditorTool::editor() const
{
    if (!m_editor) {
        Q_ASSERT(formWindow() != 0);
        m_editor = new ResourceEditor(formWindow(), 0);
    }

    return m_editor;
}

void ResourceEditorTool::activated()
{
}

void ResourceEditorTool::deactivated()
{
}

QAction *ResourceEditorTool::action() const
{
    return m_action;
}

