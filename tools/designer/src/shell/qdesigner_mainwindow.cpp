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
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QSignal>
#include <qdebug.h>

QDesignerMainWindow::QDesignerMainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      m_workbench(0)
{
    initialize();
}

QDesignerMainWindow::~QDesignerMainWindow()
{
}

void QDesignerMainWindow::initialize()
{
    m_workbench = new QDesignerWorkbench(this);
    m_workbench->setMainWindow(this);

    m_actionManager = new QDesignerActions(this);

    m_fileMenu = menuBar()->addMenu(tr("&File"));
    foreach (QAction *action, m_actionManager->fileActions()->actions()) {
        m_fileMenu->addAction(action);
    }

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    foreach (QAction *action, m_actionManager->editActions()->actions()) {
        m_editMenu->addAction(action);
    }

    m_formMenu = menuBar()->addMenu(tr("F&orm"));
    foreach (QAction *action, m_actionManager->formActions()->actions()) {
        m_formMenu->addAction(action);
    }

    m_toolMenu = menuBar()->addMenu(tr("&Tool"));

    m_windowMenu = menuBar()->addMenu(tr("&Window"));

    connect(workbench(), SIGNAL(formWindowAdded(QDesignerFormWindow*)),
            this, SLOT(addFormWindow(QDesignerFormWindow*)));

    connect(workbench(), SIGNAL(formWindowRemoved(QDesignerFormWindow*)),
                this, SLOT(removeFormWindow(QDesignerFormWindow*)));

    connect(workbench(), SIGNAL(toolWindowAdded(QDesignerToolWindow*)),
            this, SLOT(addToolWindow(QDesignerToolWindow*)));

    connect(workbench(), SIGNAL(toolWindowRemoved(QDesignerToolWindow*)),
            this, SLOT(removeToolWindow(QDesignerToolWindow*)));

    // initialization
    foreach (QAction *action, workbench()->modeActionGroup()->actions()) {
        m_windowMenu->addAction(action);
    }

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
        m_windowMenu->addAction(action);
    }
}

void QDesignerMainWindow::removeFormWindow(QDesignerFormWindow *formWindow)
{
    if (QAction *action = workbench()->actionForFormWindow(formWindow)) {
        m_windowMenu->removeAction(action);
    }
}

void QDesignerMainWindow::addToolWindow(QDesignerToolWindow *toolWindow)
{
    if (QAction *action = workbench()->actionForToolWindow(toolWindow)) {
        m_toolMenu->addAction(action);
    }
}

void QDesignerMainWindow::removeToolWindow(QDesignerToolWindow *toolWindow)
{
    if (QAction *action = workbench()->actionForToolWindow(toolWindow)) {
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

