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

#include "buddyeditor_plugin.h"
#include "buddyeditor_tool.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>


BuddyEditorPlugin::BuddyEditorPlugin()
    : m_initialized(false)
{
}

BuddyEditorPlugin::~BuddyEditorPlugin()
{
}

bool BuddyEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void BuddyEditorPlugin::initialize(AbstractFormEditor *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Buddies"), this);
    m_action->setIcon(QIcon(core->resourceLocation() + QLatin1String("/buddytool.png")));
    m_action->setIcon(QIcon(core->resourceLocation() + QLatin1String("/buddytool.png")));
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

AbstractFormEditor *BuddyEditorPlugin::core() const
{
    return m_core;
}

void BuddyEditorPlugin::addFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    BuddyEditorTool *tool = new BuddyEditorTool(formWindow, this);
    m_tools[formWindow] = tool;
    connect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    formWindow->registerTool(tool);
}

void BuddyEditorPlugin::removeFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    BuddyEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    // ### FIXME disable the tool

    delete tool;
}

QAction *BuddyEditorPlugin::action() const
{
    return m_action;
}

void BuddyEditorPlugin::activeFormWindowChanged(AbstractFormWindow *formWindow)
{
    m_action->setEnabled(formWindow != 0);
}
