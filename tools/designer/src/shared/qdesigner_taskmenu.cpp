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
#include "promotetocustomwidgetdialog.h"
#include "widgetfactory.h"
#include "widgetdatabase.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>

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

    QString demote_string = tr("Demote from Custom Widget");
    if (QDesignerPromotedWidget *promoted = qt_cast<QDesignerPromotedWidget*>(widget))
        demote_string = tr("Demote to ") + promoted->item()->extends();
    m_demoteFromCustomWidgetAction = new QAction(demote_string, this);
    connect(m_demoteFromCustomWidgetAction, SIGNAL(triggered()),
            this, SLOT(demoteFromCustomWidget()));
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

    if (qt_cast<QDesignerPromotedWidget*>(m_widget) == 0)
        actions.append(m_promoteToCustomWidgetAction);
    else
        actions.append(m_demoteFromCustomWidgetAction);
            
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
    AbstractFormWindow *fw = formWindow();
    AbstractFormEditor *core = fw->core();
    QWidget *wgt = widget();
    QWidget *parent = wgt->parentWidget();
    AbstractWidgetDataBase *db = core->widgetDataBase();
    WidgetFactory *factory = qt_cast<WidgetFactory*>(core->widgetFactory());

    Q_ASSERT(qt_cast<QDesignerPromotedWidget*>(wgt) == 0);
        
    QString base_class_name = factory->classNameOf(wgt);

    PromoteToCustomWidgetDialog dialog(db, base_class_name);
    if (!dialog.exec())
        return;
    
    QString custom_class_name = dialog.customClassName();
    QString include_file = dialog.includeFile();

    AbstractWidgetDataBaseItem *item = 0;
    int idx = db->indexOfClassName(custom_class_name);
    if (idx == -1) {
        item = new WidgetDataBaseItem(custom_class_name, tr("Promoted Widgets"));
        item->setCustom(true);
        item->setPromoted(true);
        item->setExtends(base_class_name);
        db->append(item);
    } else {
        item = db->item(idx);
    }
    item->setIncludeFile(include_file);
    
    fw->beginCommand(tr("Promote to custom widget"));

    QDesignerPromotedWidget *promoted
        = new QDesignerPromotedWidget(item, wgt, parent);
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
}

void QDesignerTaskMenu::demoteFromCustomWidget()
{
    AbstractFormWindow *fw = formWindow();
    QWidget *wgt = widget();
    QWidget *parent = wgt->parentWidget();

    QDesignerPromotedWidget *promoted = qt_cast<QDesignerPromotedWidget*>(wgt);
    Q_ASSERT(promoted != 0);

    fw->beginCommand(tr("Demote to ") + promoted->item()->extends());
    
    ReparentWidgetCommand *reparent_cmd = new ReparentWidgetCommand(fw);
    reparent_cmd->init(promoted->child(), parent);
    fw->commandHistory()->push(reparent_cmd);
    
    DeleteWidgetCommand *delete_cmd = new DeleteWidgetCommand(fw);
    delete_cmd->init(promoted);
    fw->commandHistory()->push(delete_cmd);
    
    fw->endCommand();
    
    fw->clearSelection();
    fw->selectWidget(promoted);
}

