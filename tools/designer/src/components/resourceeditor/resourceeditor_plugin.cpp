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

#include "resourceeditor_plugin.h"
#include "resourceeditor_tool.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>


ResourceEditorPlugin::ResourceEditorPlugin()
    : m_initialized(false)
{
}

ResourceEditorPlugin::~ResourceEditorPlugin()
{
}

bool ResourceEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void ResourceEditorPlugin::initialize(AbstractFormEditor *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Resources"), this);
//    m_action->setIcon(QIcon(core->resourceLocation() + QLatin1String("/buddytool.png")));
    m_action->setEnabled(false);

    setParent(core);
    m_core = core;
    m_initialized = true;

    connect(core->formWindowManager(), SIGNAL(formWindowAdded(AbstractFormWindow*)),
            this, SLOT(addFormWindow(AbstractFormWindow*)));

    connect(core->formWindowManager(), SIGNAL(formWindowRemoved(AbstractFormWindow*)),
            this, SLOT(removeFormWindow(AbstractFormWindow*)));

    connect(core->formWindowManager(), SIGNAL(activeFormWindowChanged(AbstractFormWindow*)),
                this, SLOT(activeFormWindowChanged(AbstractFormWindow *)));
}

AbstractFormEditor *ResourceEditorPlugin::core() const
{
    return m_core;
}

void ResourceEditorPlugin::addFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    ResourceEditorTool *tool = new ResourceEditorTool(formWindow, this);
    m_tools[formWindow] = tool;
    connect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    formWindow->registerTool(tool);
}

void ResourceEditorPlugin::removeFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    ResourceEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    // ### FIXME disable the tool

    delete tool;
}

QAction *ResourceEditorPlugin::action() const
{
    return m_action;
}

void ResourceEditorPlugin::activeFormWindowChanged(AbstractFormWindow *formWindow)
{
    m_action->setEnabled(formWindow != 0);
}
