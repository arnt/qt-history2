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

#include "qdesigner_stackedbox.h"
#include "abstractformwindow.h"

#include <abstractformeditor.h>
#include <abstractpropertyeditor.h>
#include <abstractmetadatabase.h>

#include <qextensionmanager.h>
#include <propertysheet.h>
#include <qdesigner_command.h>
#include <qtundo.h>

#include <qevent.h>
#include <qtoolbutton.h>
#include <qaction.h>

QDesignerStackedWidget::QDesignerStackedWidget(QWidget *parent)
    : QStackedWidget(parent)
{
    prev = new QToolButton(this);
    prev->setObjectName("designer_wizardstack_button");
    prev->setArrowType(Qt::LeftArrow);
    prev->setAutoRaise(true);
    prev->setAutoRepeat(true);
    prev->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(prev, SIGNAL(clicked()), this, SLOT(prevPage()));
    removeWidget(prev);

    next = new QToolButton(this);
    next->setObjectName("designer_wizardstack_button");
    next->setArrowType(Qt::RightArrow);
    next->setAutoRaise(true);
    next->setAutoRepeat(true);
    next->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(next, SIGNAL(clicked()), this, SLOT(nextPage()));
    removeWidget(next);

    updateButtons();

    m_actionPreviousPage = new QAction(tr("Previous Page"));
    connect(m_actionPreviousPage, SIGNAL(triggered()), this, SLOT(prevPage()));

    m_actionNextPage = new QAction(tr("Next Page"));
    connect(m_actionNextPage, SIGNAL(triggered()), this, SLOT(nextPage()));

    m_actionDeletePage = new QAction(tr("Delete Page"));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));

    m_actionInsertPage = new QAction(tr("Add Page"));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));
}

void QDesignerStackedWidget::removeCurrentPage()
{
    if (currentIndex() == -1)
        return;

    if (AbstractFormWindow *fw = AbstractFormWindow::findFormWindow(this)) {
        DeleteStackedWidgetPageCommand *cmd = new DeleteStackedWidgetPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerStackedWidget::addPage()
{
    if (AbstractFormWindow *fw = AbstractFormWindow::findFormWindow(this)) {
        AddStackedWidgetPageCommand *cmd = new AddStackedWidgetPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerStackedWidget::updateButtons()
{
    if (prev) {
        prev->setGeometry(width() - 31, 1, 15, 15);
        prev->show();
        prev->raise();
    }

    if (next) {
        next->setGeometry(width() - 16, 1, 15, 15);
        next->show();
        next->raise();
    }
}

void QDesignerStackedWidget::prevPage()
{
    if (AbstractFormWindow *fw = AbstractFormWindow::findFormWindow(this)) {
        int newIndex = currentIndex() - 1;
        if (newIndex < 0)
            newIndex = count() -1;

        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(this, "currentIndex", newIndex);
        fw->commandHistory()->push(cmd);
        updateButtons();
    }
}

void QDesignerStackedWidget::nextPage()
{
    if (AbstractFormWindow *fw = AbstractFormWindow::findFormWindow(this)) {
        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(this, "currentIndex", (currentIndex() + 1) % count());
        fw->commandHistory()->push(cmd);
        updateButtons();
    }
}

void QDesignerStackedWidget::childEvent(QChildEvent *e)
{
    QStackedWidget::childEvent(e);

    if (e->added() && (e->child() == prev || e->child() == next)) {
        removeWidget(static_cast<QWidget*>(e->child()));
        updateButtons();
    }
}

void QDesignerStackedWidget::resizeEvent(QResizeEvent *e)
{
    QStackedWidget::resizeEvent(e);
    updateButtons();
}

void QDesignerStackedWidget::showEvent(QShowEvent *e)
{
    QStackedWidget::showEvent(e);
    updateButtons();
}
