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

#include "qdesigner.h"
#include "qdesigner_mainwindow.h"
#include "qdesigner_workbench.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_actions.h"

// sdk
#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>

// Qt
#include <QtCore/QEvent>

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>

#include <qdebug.h>

QDesignerMainWindow::QDesignerMainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      m_workbench(0)
{
    initialize();

    // ### from settings
    m_actionManager->editWidgets()->setChecked(true);
}

QDesignerMainWindow::~QDesignerMainWindow()
{
}

void QDesignerMainWindow::initialize()
{
    m_workbench = new QDesignerWorkbench(this);
    m_workbench->setMainWindow(this);

    m_toolActions = new QActionGroup(this);
    m_toolActions->setExclusive(false);

    m_windowActions = new QActionGroup(this);
    m_windowActions->setExclusive(true);

    m_actionManager = new QDesignerActions(this);

    m_fileMenu = menuBar()->addMenu(tr("&File"));
    foreach (QAction *action, m_actionManager->fileActions()->actions()) {
        m_fileMenu->addAction(action);
    }

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    foreach (QAction *action, m_actionManager->editActions()->actions()) {
        m_editMenu->addAction(action);
    }

    m_editMenu->addSeparator();

    foreach (QAction *action, m_actionManager->editModeActions()->actions()) {
        m_editMenu->addAction(action);
    }

    m_formMenu = menuBar()->addMenu(tr("F&orm"));
    foreach (QAction *action, m_actionManager->formActions()->actions()) {
        m_formMenu->addAction(action);
    }

    m_toolMenu = menuBar()->addMenu(tr("&Tool"));

    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    foreach (QAction *action, m_actionManager->windowActions()->actions()) {
        m_windowMenu->addAction(action);
    }

    // create the toolbars
    QToolBar *editToolBar = addToolBar("Edit");
    foreach (QAction *action, m_actionManager->editActions()->actions()) {
        if (action->icon().isNull() == false)
            editToolBar->addAction(action);
    }

    QToolBar *formToolBar = addToolBar("Form");
    foreach (QAction *action, m_actionManager->formActions()->actions()) {
        if (action->icon().isNull() == false)
            formToolBar->addAction(action);
    }

    connect(workbench(), SIGNAL(formWindowAdded(QDesignerFormWindow*)),
            this, SLOT(addFormWindow(QDesignerFormWindow*)));

    connect(workbench(), SIGNAL(formWindowRemoved(QDesignerFormWindow*)),
                this, SLOT(removeFormWindow(QDesignerFormWindow*)));

    connect(workbench(), SIGNAL(toolWindowAdded(QDesignerToolWindow*)),
            this, SLOT(addToolWindow(QDesignerToolWindow*)));

    connect(workbench(), SIGNAL(toolWindowRemoved(QDesignerToolWindow*)),
            this, SLOT(removeToolWindow(QDesignerToolWindow*)));

    m_windowMenu->addSeparator();

    for (int i=0; i<workbench()->toolWindowCount(); ++i) {
        QDesignerToolWindow *tw = workbench()->toolWindow(i);
        addToolWindow(tw);
    }

    m_workbench->switchToTopLevelMode(); // ### remove me
}

void QDesignerMainWindow::addFormWindow(QDesignerFormWindow *formWindow)
{
    if (QAction *action = workbench()->actionForFormWindow(formWindow)) {
        m_windowActions->addAction(action);
        m_windowMenu->addAction(action);
    }
}

void QDesignerMainWindow::removeFormWindow(QDesignerFormWindow *formWindow)
{
    if (QAction *action = workbench()->actionForFormWindow(formWindow)) {
        m_windowActions->removeAction(action);
        m_windowMenu->removeAction(action);
    }
}

void QDesignerMainWindow::addToolWindow(QDesignerToolWindow *toolWindow)
{
    if (QAction *action = workbench()->actionForToolWindow(toolWindow)) {
        m_toolActions->addAction(action);
        m_toolMenu->addAction(action);
    }
}

void QDesignerMainWindow::removeToolWindow(QDesignerToolWindow *toolWindow)
{
    if (QAction *action = workbench()->actionForToolWindow(toolWindow)) {
        m_toolActions->removeAction(action);
        m_toolMenu->removeAction(action);
    }
}

QDesignerWorkbench *QDesignerMainWindow::workbench() const
{
    return m_workbench;
}

AbstractFormEditor *QDesignerMainWindow::core() const
{
    return workbench()->core();
}

void QDesignerMainWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::WindowStateChange:
            updateWindowState();
            break;

        default:
            break;
    }

    QMainWindow::changeEvent(event);
}

void QDesignerMainWindow::updateWindowState()
{
#if 0
    if ((windowState() & Qt::WindowMaximized) || (windowState() & Qt::WindowFullScreen)) {
        workbench()->switchToWorkspaceMode();
    } else if (windowState() & Qt::WindowNoState) {
        workbench()->switchToTopLevelMode();
    }
#endif
}

