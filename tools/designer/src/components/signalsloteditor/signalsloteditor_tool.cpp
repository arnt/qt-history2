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

#include "signalsloteditor_tool.h"
#include "signalsloteditor.h"

#include <abstractformwindow.h>
#include <abstractformeditor.h>
#include <ui4.h>

#include <QtGui/QAction>
#include <QtCore/qdebug.h>

SignalSlotEditorTool::SignalSlotEditorTool(AbstractFormWindow *formWindow, QObject *parent)
    : AbstractFormWindowTool(parent),
      m_formWindow(formWindow)
{
    m_action = new QAction(tr("Edit Signals/Slots"), this);
}

SignalSlotEditorTool::~SignalSlotEditorTool()
{
}

AbstractFormEditor *SignalSlotEditorTool::core() const
{
    return m_formWindow->core();
}

AbstractFormWindow *SignalSlotEditorTool::formWindow() const
{
    return m_formWindow;
}

bool SignalSlotEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);
    Q_UNUSED(event);

    return false;
}

QWidget *SignalSlotEditorTool::editor() const
{
    if (!m_editor) {
        Q_ASSERT(formWindow() != 0);
        m_editor = new SignalSlotEditor(formWindow(), 0);
        connect(formWindow(), SIGNAL(mainContainerChanged(QWidget*)), m_editor, SLOT(setBackground(QWidget*)));
    }

    return m_editor;
}

QAction *SignalSlotEditorTool::action() const
{
    return m_action;
}

void SignalSlotEditorTool::activated()
{
    connect(formWindow(), SIGNAL(changed()),
                m_editor, SLOT(updateBackground()));
    m_editor->updateBackground();
}

void SignalSlotEditorTool::deactivated()
{
    disconnect(formWindow(), SIGNAL(changed()),
                m_editor, SLOT(updateBackground()));
}

void SignalSlotEditorTool::saveToDom(DomUI *ui, QWidget*)
{
    ui->setElementConnections(m_editor->toUi());    
}

void SignalSlotEditorTool::loadFromDom(DomUI *ui, QWidget *mainContainer)
{
    m_editor->fromUi(ui->elementConnections(), mainContainer);
}
