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

#include "signalsloteditor_plugin.h"
#include "signalsloteditor_tool.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>

SignalSlotEditorPlugin::SignalSlotEditorPlugin()
    : m_initialized(false), m_action(0)
{
}

SignalSlotEditorPlugin::~SignalSlotEditorPlugin()
{
}

bool SignalSlotEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void SignalSlotEditorPlugin::initialize(AbstractFormEditor *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Signals/Slots"), this);
    m_action->setShortcut(tr("F4"));
    QIcon icon(QIcon(core->resourceLocation() + QLatin1String("/signalslottool.png")));
    m_action->setIcon(icon);
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

AbstractFormEditor *SignalSlotEditorPlugin::core() const
{
    return m_core;
}

void SignalSlotEditorPlugin::addFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    SignalSlotEditorTool *tool = new SignalSlotEditorTool(formWindow, this);
    connect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    m_tools[formWindow] = tool;
    formWindow->registerTool(tool);
}

void SignalSlotEditorPlugin::removeFormWindow(AbstractFormWindow *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    SignalSlotEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    // ### FIXME disable the tool

    delete tool;
}

QAction *SignalSlotEditorPlugin::action() const
{
    return m_action;
}

void SignalSlotEditorPlugin::activeFormWindowChanged(AbstractFormWindow *formWindow)
{
    m_action->setEnabled(formWindow != 0);
}
