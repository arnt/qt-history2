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

#include "qdesigner_formwindow.h"
#include "qdesigner_workbench.h"

// sdk
#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>

// shared
#include <qtundo.h>

#include <QtCore/QEvent>
#include <QtCore/QFile>

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

QDesignerFormWindow::QDesignerFormWindow(QDesignerWorkbench *workbench, QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      m_workbench(workbench)
{
    Q_ASSERT(workbench);

    m_editor = workbench->core()->formWindowManager()->createFormWindow(this);
    setCentralWidget(m_editor);

    connect(m_editor, SIGNAL(fileNameChanged(const QString&)),
            this, SLOT(updateWindowTitle(const QString&)));

    m_action = new QAction(this);
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(checked(bool)), this, SLOT(activated(bool)));

    updateWindowTitle(m_editor->fileName());
    connect(m_editor->commandHistory(), SIGNAL(commandExecuted()), this, SLOT(updateChanged()));
}

QDesignerFormWindow::QDesignerFormWindow(AbstractFormWindow *editor, QDesignerWorkbench *workbench, QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      m_editor(editor),
      m_workbench(workbench)
{
    Q_ASSERT(editor);
    Q_ASSERT(workbench);

    m_editor->setParent(this);
    setCentralWidget(m_editor);

    m_action = new QAction(this);
    m_action->setText(windowTitle());
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(checked(bool)), this, SLOT(activated(bool)));
    connect(m_editor->commandHistory(), SIGNAL(commandExecuted()), this, SLOT(updateChanged()));
}

QDesignerFormWindow::~QDesignerFormWindow()
{
    if (workbench())
        workbench()->removeFormWindow(this);
}

void QDesignerFormWindow::activated(bool active)
{
    if (active == true) {
        raise();
        activateWindow();
    }
}

QAction *QDesignerFormWindow::action() const
{
    return m_action;
}

void QDesignerFormWindow::changeEvent(QEvent *e)
{
    switch (e->type()) {
        case QEvent::WindowTitleChange:
            m_action->setText(windowTitle());
            break;
        case QEvent::WindowIconChange:
            m_action->setIcon(windowIcon());
            break;
        default:
            break;
    }
    QMainWindow::changeEvent(e);
}

QRect QDesignerFormWindow::geometryHint() const
{
    return QRect(0, 0, 400, 300);
}

AbstractFormWindow *QDesignerFormWindow::editor() const
{
    return m_editor;
}

QDesignerWorkbench *QDesignerFormWindow::workbench() const
{
    return m_workbench;
}

void QDesignerFormWindow::updateWindowTitle(const QString &fileName)
{
    QString fn = fileName;

    if (fn.isEmpty())
        fn = tr("Untitled");

    setWindowTitle(tr("%1 - [Design]").arg(fn));
}

void QDesignerFormWindow::closeEvent(QCloseEvent *ev)
{
    if (m_editor->isDirty()) {
        raise();
        QMessageBox box(tr("Save Form?"),
                tr("Do you want to save the changes you made to \"%1\" before closing?")
                .arg(m_editor->fileName().isEmpty() ? m_editor->windowTitle() : m_editor->fileName()),
                QMessageBox::Information,
                QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                QMessageBox::Cancel | QMessageBox::Escape, m_editor, Qt::Sheet);
        box.setButtonText(QMessageBox::Yes, m_editor->fileName().isEmpty() ? tr("Save...") : tr("Save"));
        box.setButtonText(QMessageBox::No, tr("Don't Save"));
        switch (box.exec()) {
            case QMessageBox::Yes:
                ev->setAccepted(workbench()->saveForm(m_editor));
                m_editor->setDirty(false);
                break;
            case QMessageBox::No:
                m_editor->setDirty(false); // Not really necessary, but stops problems if we get close again.
                ev->accept();
                break;
            case QMessageBox::Cancel:
                ev->ignore();
                break;
        }
    }

    /*
    if (m_formWindowManager->formWindowCount() == 1 && *accept
            && QSettings().value("newFormDialog/ShowOnStartup", true).toBool())
        QTimer::singleShot(200, this, SLOT(newForm()));  // Use timer in case we are quitting.
        */
}

void QDesignerFormWindow::updateChanged()
{
    setWindowModified(m_editor->isDirty());
}
