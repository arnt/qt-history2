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

#include <QAction>
#include <QEvent>

QDesignerFormWindow::QDesignerFormWindow(QDesignerWorkbench *workbench, QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      m_workbench(workbench)
{
    Q_ASSERT(workbench);

    m_editor = workbench->core()->formWindowManager()->createFormWindow(this);
    setCentralWidget(m_editor);

    m_action = new QAction(this);
    m_action->setText(windowTitle());
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(checked(bool)), this, SIGNAL(activated(bool)));
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
    connect(m_action, SIGNAL(checked(bool)), this, SIGNAL(activated(bool)));
}

QDesignerFormWindow::~QDesignerFormWindow()
{
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
    return QRect();
}

AbstractFormWindow *QDesignerFormWindow::editor() const
{
    return m_editor;
}
