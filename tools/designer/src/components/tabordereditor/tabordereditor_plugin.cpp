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

#include <QtGui/QAction>

#include "tabordereditor_plugin.h"
#include "tabordereditor_tool.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>


TabOrderEditorPlugin::TabOrderEditorPlugin()
    : m_initialized(false)
{
}

TabOrderEditorPlugin::~TabOrderEditorPlugin()
{
}

bool TabOrderEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void TabOrderEditorPlugin::initialize(AbstractFormEditor *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Tab Order"), this);

    setParent(core);
    m_core = core;
    m_initialized = true;

    connect(core->formWindowManager(), SIGNAL(formWindowAdded(AbstractFormWindow*)),
            this, SLOT(addFormWindow(AbstractFormWindow*)));

    connect(core->formWindowManager(), SIGNAL(formWindowRemoved(AbstractFormWindow*)),
            this, SLOT(removeFormWindow(AbstractFormWindow*)));
}

AbstractFormEditor *TabOrderEditorPlugin::core() const
{
    return m_core;
}

void TabOrderEditorPlugin::addFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    TabOrderEditorTool *tool = new TabOrderEditorTool(formWindow, this);
    m_tools[formWindow] = tool;
    connect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    formWindow->registerTool(tool);
}

void TabOrderEditorPlugin::removeFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    TabOrderEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    // ### FIXME disable the tool

    delete tool;
}

QAction *TabOrderEditorPlugin::action() const
{
    return m_action;
}

