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

#include "qdesigner_taskmenu.h"
#include "qtundo.h"

#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QInputDialog>
#include <QtGui/QMainWindow>
#include <QtGui/QVariant>

class CreatDockWindowCommand: public QtCommand
{
public:
    CreatDockWindowCommand();

    void init(QWidget *widget);

    virtual void redo();
    virtual void undo();

private:
    QPointer<QWidget> m_widget;
    QPointer<QWidget> m_parentWidget;
    QPointer<QDockWindow> m_dockWindow;
};

CreatDockWindowCommand::CreatDockWindowCommand()
{
}

void CreatDockWindowCommand::init(QWidget *widget)
{
    Q_ASSERT(widget != 0);

    m_widget = widget;
    m_parentWidget = widget->parentWidget();
}

void CreatDockWindowCommand::redo()
{
}

void CreatDockWindowCommand::undo()
{
}


QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
    m_changeObjectNameAction = new QAction(tr("Change Object Name"), this);
    connect(m_changeObjectNameAction, SIGNAL(triggered()), this, SLOT(changeObjectName()));

    m_createDockWindowAction = new QAction(tr("Create Dock Window"), this);
    connect(m_createDockWindowAction, SIGNAL(triggered()), this, SLOT(createDockWindow()));
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
}

QWidget *QDesignerTaskMenu::widget() const
{
    return m_widget;
}

QList<QAction*> QDesignerTaskMenu::taskActions() const
{
    QList<QAction*> actions;

    actions.append(m_changeObjectNameAction);

    if (qt_cast<QMainWindow*>(widget()->parentWidget()) != 0) {
        actions.append(m_createDockWindowAction);
    }

    return actions;
}

void QDesignerTaskMenu::changeObjectName()
{
    AbstractFormWindow *formWindow = AbstractFormWindow::findFormWindow(widget());
    Q_ASSERT(formWindow);

    QString newObjectName = QInputDialog::getText(widget(), tr("Change Object Name"),
            tr("Object Name"), QLineEdit::Normal, widget()->objectName());

    if (!newObjectName.isEmpty()) {
        formWindow->cursor()->setProperty("objectName", newObjectName);
    }
}

void QDesignerTaskMenu::createDockWindow()
{
}

QDesignerTaskMenuFactory::QDesignerTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *QDesignerTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QWidget *widget = qt_cast<QWidget*>(object)) {
        if (iid == Q_TYPEID(ITaskMenu)) {
            return new QDesignerTaskMenu(widget, parent);
        }
    }

    return 0;
}
