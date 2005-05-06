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
#include "richtexteditor.h"
#include "promotetocustomwidgetdialog.h"
#include "widgetfactory.h"
#include "widgetdatabase.h"
#include "qlayout_widget.h"
#include "spacer_widget.h"
#include "layout.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerLayoutDecorationExtension>

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QInputDialog>
#include <QtGui/QMainWindow>
#include <QtGui/QDockWidget>
#include <QtCore/QVariant>

#include <QtCore/qdebug.h>

QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
    m_separator = new QAction(this);
    m_separator->setSeparator(true);

    m_changeObjectNameAction = new QAction(tr("Change objectName"), this);
    connect(m_changeObjectNameAction, SIGNAL(triggered()), this, SLOT(changeObjectName()));

    m_changeStatusTip = new QAction(tr("Change statusTip"), this);
    connect(m_changeStatusTip, SIGNAL(triggered()), this, SLOT(changeStatusTip()));

    m_changeToolTip = new QAction(tr("Change toolTip"), this);
    connect(m_changeToolTip, SIGNAL(triggered()), this, SLOT(changeToolTip()));

    m_changeWhatsThis = new QAction(tr("Change whatsThis"), this);
    connect(m_changeWhatsThis, SIGNAL(triggered()), this, SLOT(changeWhatsThis()));

    m_createDockWidgetAction = new QAction(tr("Create Dock Window"), this);
    connect(m_createDockWidgetAction, SIGNAL(triggered()), this, SLOT(createDockWidget()));

    m_promoteToCustomWidgetAction = new QAction(tr("Promote to Custom Widget"), this);
    connect(m_promoteToCustomWidgetAction, SIGNAL(triggered()), this, SLOT(promoteToCustomWidget()));

    QString demote_string = tr("Demote from Custom Widget");
    if (const QDesignerPromotedWidget *promoted = qobject_cast<const QDesignerPromotedWidget*>(widget))
        demote_string = tr("Demote to ") + promoted->item()->extends();

    m_demoteFromCustomWidgetAction = new QAction(demote_string, this);
    connect(m_demoteFromCustomWidgetAction, SIGNAL(triggered()), this, SLOT(demoteFromCustomWidget()));
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
}

QWidget *QDesignerTaskMenu::widget() const
{
    return m_widget;
}

QDesignerFormWindowInterface *QDesignerTaskMenu::formWindow() const
{
    QDesignerFormWindowInterface *result = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(result != 0);
    return result;
}

QList<QAction*> QDesignerTaskMenu::taskActions() const
{
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(formWindow);

    QList<QAction*> actions;

    actions.append(m_changeObjectNameAction);
    actions.append(m_separator);
    actions.append(m_changeToolTip);
    actions.append(m_changeStatusTip);
    actions.append(m_changeWhatsThis);

#if 0
    if (qobject_cast<const QMainWindow*>(formWindow->mainContainer()) != 0 && qobject_cast<QDockWidget*>(widget()) == 0)
        actions.append(m_createDockWidgetAction);
#endif

    if (m_widget != formWindow) {
        actions.append(m_separator);
        if (qobject_cast<const QDesignerPromotedWidget*>(m_widget) == 0)
            actions.append(m_promoteToCustomWidgetAction);
        else
            actions.append(m_demoteFromCustomWidgetAction);
    }

    return actions;
}

void QDesignerTaskMenu::changeObjectName()
{
    QString newObjectName = QInputDialog::getText(widget(), tr("Change Object Name"),
            tr("Object Name"), QLineEdit::Normal, widget()->objectName());

    if (!newObjectName.isEmpty()) {
        formWindow()->cursor()->setProperty(QLatin1String("objectName"), newObjectName);
    }
}

void QDesignerTaskMenu::createDockWidget()
{
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(formWindow != 0);

    QMainWindow *mainWindow = qobject_cast<QMainWindow*>(formWindow->mainContainer());
    Q_ASSERT(mainWindow != 0);

    formWindow->beginCommand(tr("Create Dock Window"));

    QDesignerWidgetFactoryInterface *widgetFactory = formWindow->core()->widgetFactory();
    QDockWidget *dockWidget = (QDockWidget *) widgetFactory->createWidget(QLatin1String("QDockWidget"), formWindow->mainContainer());
    Q_ASSERT(dockWidget);

    InsertWidgetCommand *cmd = new InsertWidgetCommand(formWindow);
    cmd->init(dockWidget);
    formWindow->commandHistory()->push(cmd);

    ReparentWidgetCommand *reparentCmd = new ReparentWidgetCommand(formWindow);
    reparentCmd->init(widget(), dockWidget);
    formWindow->commandHistory()->push(reparentCmd);

    SetDockWidgetCommand *setDockWidgetCmd = new SetDockWidgetCommand(formWindow);
    setDockWidgetCmd->init(dockWidget, m_widget);
    formWindow->commandHistory()->push(setDockWidgetCmd);

    AddDockWidgetCommand *addDockWidgetCmd = new AddDockWidgetCommand(formWindow);
    addDockWidgetCmd->init(mainWindow, dockWidget);
    formWindow->commandHistory()->push(addDockWidgetCmd);

    formWindow->endCommand();
}

QDesignerTaskMenuFactory::QDesignerTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *QDesignerTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
        return 0;

    QWidget *widget = qobject_cast<QWidget*>(object);
    if (widget == 0)
        return 0;

    // check if is an internal widget (### generalize)
    if (qobject_cast<QLayoutWidget*>(widget) || qobject_cast<Spacer*>(widget))
        return 0;

    return new QDesignerTaskMenu(widget, parent);
}


static void replace_widget_item(QDesignerFormWindowInterface *fw, QWidget *wgt, QWidget *promoted)
{
    QDesignerFormEditorInterface *core = fw->core();
    QWidget *parent = wgt->parentWidget();

    QRect info;
    if (QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parent)) {
        QLayout *layout = LayoutInfo::managedLayout(core, parent);
        Q_ASSERT(layout != 0);

        int old_index = layout->indexOf(wgt);
        Q_ASSERT(old_index != -1);

        info = deco->itemInfo(old_index);

        QLayoutItem *item = layout->takeAt(old_index);
        delete item;
        layout->activate();
    }

    if (qt_extension<QDesignerLayoutDecorationExtension*>(core->extensionManager(), parent)) {
        QLayout *layout = LayoutInfo::managedLayout(core, parent);
        Q_ASSERT(layout != 0);

        // ### check if `info' is valid!

        switch (LayoutInfo::layoutType(core, layout)) {
            default: Q_ASSERT(0); break;

            case LayoutInfo::VBox:
                insert_into_box_layout(static_cast<QBoxLayout*>(layout), info.top(), promoted);
                break;

            case LayoutInfo::HBox:
                insert_into_box_layout(static_cast<QBoxLayout*>(layout), info.left(), promoted);
                break;

            case LayoutInfo::Grid:
                add_to_grid_layout(static_cast<QGridLayout*>(layout), promoted, info.top(), info.left(), info.height(), info.width());
                break;
        }
    }
}

void QDesignerTaskMenu::promoteToCustomWidget()
{
    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerFormEditorInterface *core = fw->core();
    QWidget *wgt = widget();
    QWidget *parent = wgt->parentWidget();
    QDesignerWidgetDataBaseInterface *db = core->widgetDataBase();
    WidgetFactory *factory = qobject_cast<WidgetFactory*>(core->widgetFactory());

    Q_ASSERT(qobject_cast<QDesignerPromotedWidget*>(wgt) == 0);

    QString base_class_name = QLatin1String(factory->classNameOf(wgt));

    PromoteToCustomWidgetDialog dialog(db, base_class_name);
    if (!dialog.exec())
        return;

    QString custom_class_name = dialog.customClassName();
    QString include_file = dialog.includeFile();

    QDesignerWidgetDataBaseItemInterface *item = 0;
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

    // ### use the undo stack
    // fw->beginCommand(tr("Promote to custom widget"));

    PromoteToCustomWidgetCommand *cmd = new PromoteToCustomWidgetCommand(fw);
    cmd->init(item, wgt);
    fw->commandHistory()->push(cmd);
}

void QDesignerTaskMenu::demoteFromCustomWidget()
{
    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget());
    Q_ASSERT(promoted != 0);

    // ### use the undo stack
    //fw->beginCommand(tr("Demote to ") + promoted->item()->extends());

    DemoteFromCustomWidgetCommand *cmd = new DemoteFromCustomWidgetCommand(fw);
    cmd->init(promoted);
    fw->commandHistory()->push(cmd);
}

void QDesignerTaskMenu::changeRichTextProperty(const QString &propertyName)
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        RichTextEditorDialog *dlg = new RichTextEditorDialog(fw);
        Q_ASSERT(m_widget->parentWidget() != 0);
        RichTextEditor *editor = dlg->editor();

        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(fw->core()->extensionManager(), m_widget);
        Q_ASSERT(sheet != 0);

        editor->setDefaultFont(m_widget->font());
        editor->setText(sheet->property(sheet->indexOf(propertyName)).toString());
        editor->selectAll();

        if (dlg->exec()) {
            QString text = editor->text(Qt::RichText);
            fw->cursor()->setWidgetProperty(m_widget, propertyName, QVariant(text));
        }

        delete dlg;
    }
}

void QDesignerTaskMenu::changeToolTip()
{
    changeRichTextProperty(QLatin1String("toolTip"));
}

void QDesignerTaskMenu::changeStatusTip()
{
    changeRichTextProperty(QLatin1String("statusTip"));
}

void QDesignerTaskMenu::changeWhatsThis()
{
    changeRichTextProperty(QLatin1String("whatsThis"));
}
