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
#include "qdesigner_command.h"
#include "qtundo.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractwidgetfactory.h>

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QInputDialog>
#include <QtGui/QMainWindow>
#include <QtGui/QDockWindow>
#include <QtGui/QVariant>

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
    AbstractFormWindow *formWindow = AbstractFormWindow::findFormWindow(widget());
    Q_ASSERT(formWindow);

    QList<QAction*> actions;

    actions.append(m_changeObjectNameAction);

    if (qt_cast<QMainWindow*>(formWindow->mainContainer()) != 0) {
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
    AbstractFormWindow *formWindow = AbstractFormWindow::findFormWindow(widget());
    Q_ASSERT(formWindow != 0);

    QMainWindow *mainWindow = qt_cast<QMainWindow*>(formWindow->mainContainer());
    Q_ASSERT(mainWindow != 0);

    formWindow->beginCommand(tr("Create Dock Window"));

    AbstractWidgetFactory *widgetFactory = formWindow->core()->widgetFactory();
    QDockWindow *dockWindow = (QDockWindow *) widgetFactory->createWidget(QLatin1String("QDockWindow"), formWindow->mainContainer());
    Q_ASSERT(dockWindow);

    InsertWidgetCommand *cmd = new InsertWidgetCommand(formWindow);
    cmd->init(dockWindow);
    formWindow->commandHistory()->push(cmd);

    ReparentWidgetCommand *reparentCmd = new ReparentWidgetCommand(formWindow);
    reparentCmd->init(widget(), dockWindow);
    formWindow->commandHistory()->push(reparentCmd);

    SetDockWindowWidgetCommand *setDockWidgetCmd = new SetDockWindowWidgetCommand(formWindow);
    setDockWidgetCmd->init(dockWindow, m_widget);
    formWindow->commandHistory()->push(setDockWidgetCmd);

    AddDockWindowCommand *addDockWindowCmd = new AddDockWindowCommand(formWindow);
    addDockWindowCmd->init(mainWindow, dockWindow);
    formWindow->commandHistory()->push(addDockWindowCmd);

    formWindow->endCommand();
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
