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
#include "qdesigner_promotedwidget.h"
#include "qtundo.h"
#include "ui_promotetocustomwidgetdialog.h"

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

#include <qdebug.h>

QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
    m_changeObjectNameAction = new QAction(tr("Change Object Name"), this);
    connect(m_changeObjectNameAction, SIGNAL(triggered()), this, SLOT(changeObjectName()));

    m_createDockWindowAction = new QAction(tr("Create Dock Window"), this);
    connect(m_createDockWindowAction, SIGNAL(triggered()), this, SLOT(createDockWindow()));

    m_promoteToCustomWidgetAction = new QAction(tr("Promote to Custom Widget"), this);
    connect(m_promoteToCustomWidgetAction, SIGNAL(triggered()), this, SLOT(promoteToCustomWidget()));
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
}

QWidget *QDesignerTaskMenu::widget() const
{
    return m_widget;
}

AbstractFormWindow *QDesignerTaskMenu::formWindow() const
{
    AbstractFormWindow *result = AbstractFormWindow::findFormWindow(widget());
    Q_ASSERT(result != 0);
    return result;
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

    actions.append(m_promoteToCustomWidgetAction);
    
    return actions;
}

void QDesignerTaskMenu::changeObjectName()
{
    QString newObjectName = QInputDialog::getText(widget(), tr("Change Object Name"),
            tr("Object Name"), QLineEdit::Normal, widget()->objectName());

    if (!newObjectName.isEmpty()) {
        formWindow()->cursor()->setProperty("objectName", newObjectName);
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

void QDesignerTaskMenu::promoteToCustomWidget()
{
    QDialog *dialog = new QDialog(0);
    
    Ui::PromoteToCustomWidgetDialog ui;
    ui.setupUi(dialog);

    connect(ui.m_ok_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(ui.m_cancel_button, SIGNAL(clicked()), dialog, SLOT(reject()));
    ui.m_base_class_name_label->setText(QLatin1String(widget()->metaObject()->className()));
    
    if (!dialog->exec()) {
        delete dialog;
        return;
    }

    QWidget *wgt = widget();
    QWidget *parent = wgt->parentWidget();
    AbstractFormWindow *fw = formWindow();

    fw->beginCommand(tr("Promote to custom widget"));

    QDesignerPromotedWidget *promoted = new QDesignerPromotedWidget(wgt, parent);
    promoted->setGeometry(wgt->geometry());
    InsertWidgetCommand *insert_cmd = new InsertWidgetCommand(fw);
    insert_cmd->init(promoted);
    fw->commandHistory()->push(insert_cmd);

    ReparentWidgetCommand *reparent_cmd = new ReparentWidgetCommand(fw);
    reparent_cmd->init(wgt, promoted);
    fw->commandHistory()->push(reparent_cmd);
    
    fw->endCommand();

    fw->clearSelection();
    fw->selectWidget(promoted);
    
    delete dialog;
}

