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

#include "buddyeditor_tool.h"

#include <abstractformwindow.h>
#include <abstractformeditor.h>

BuddyEditorTool::BuddyEditorTool(AbstractFormWindow *formWindow, QObject *parent)
    : AbstractFormWindowTool(parent),
      m_formWindow(formWindow)
{
}

BuddyEditorTool::~BuddyEditorTool()
{
}

AbstractFormEditor *BuddyEditorTool::core() const
{
    return m_formWindow->core();
}

AbstractFormWindow *BuddyEditorTool::formWindow() const
{
    return m_formWindow;
}

bool BuddyEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
}

