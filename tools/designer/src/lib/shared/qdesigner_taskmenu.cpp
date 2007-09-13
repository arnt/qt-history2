/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_taskmenu_p.h"
#include "qdesigner_command_p.h"
#include "richtexteditor_p.h"
#include "stylesheeteditor_p.h"
#include "qlayout_widget_p.h"
#include "layout_p.h"
#include "spacer_widget_p.h"
#include "textpropertyeditor_p.h"
#include "promotiontaskmenu_p.h"
#include "metadatabase_p.h"
#include "scriptdialog_p.h"
#include "scriptcommand_p.h"
#include "signalslotdialog_p.h"
#include "qdesigner_membersheet_p.h"

#include <shared_enums_p.h>

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerLanguageExtension>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QMainWindow>
#include <QtGui/QStatusBar>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

static QMenuBar *findMenuBar(const QWidget *widget)
{
    const QList<QObject*> children = widget->children();
    foreach (QObject *obj, widget->children()) {
        if (QMenuBar *mb = qobject_cast<QMenuBar*>(obj)) {
            return mb;
        }
    }

    return 0;
}

static QStatusBar *findStatusBar(const QWidget *widget)
{
    const QList<QObject*> children = widget->children();
    foreach (QObject *obj, widget->children()) {
        if (QStatusBar *sb = qobject_cast<QStatusBar*>(obj)) {
            return sb;
        }
    }

    return 0;
}

static inline QAction *createSeparatorHelper(QObject *parent) {
    QAction *rc = new QAction(parent);
    rc->setSeparator(true);
    return rc;
}

namespace  {
// --------------- ObjectNameDialog
class ObjectNameDialog : public QDialog
{
     public:
         ObjectNameDialog(QWidget *parent, const QString &oldName);
         QString newObjectName() const;

     private:
         qdesigner_internal::TextPropertyEditor *m_editor;
};

ObjectNameDialog::ObjectNameDialog(QWidget *parent, const QString &oldName)
    : QDialog(parent),
      m_editor( new qdesigner_internal::TextPropertyEditor(this, qdesigner_internal::TextPropertyEditor::EmbeddingNone,
                                                           qdesigner_internal::ValidationObjectName))
{
    setWindowTitle(QObject::tr("Change Object Name"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->addWidget(new QLabel(QObject::tr("Object Name")));

    m_editor->setText(oldName);
    m_editor->selectAll();
    m_editor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    vboxLayout->addWidget(m_editor);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                       Qt::Horizontal, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    vboxLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QString ObjectNameDialog::newObjectName() const
{
    return m_editor->text();
}

}

namespace qdesigner_internal {
// -------------- QDesignerTaskMenuPrivate
class QDesignerTaskMenuPrivate : public  QObjectPrivate {
public:
    QDesignerTaskMenuPrivate(QWidget *widget, QObject *parent);
    QPointer<QWidget> m_widget;
    QAction *m_separator;
    QAction *m_separator2;
    QAction *m_separator3;
    QAction *m_separator4;
    QAction *m_separator5;
    QAction *m_changeObjectNameAction;
    QAction *m_changeToolTip;
    QAction *m_changeWhatsThis;
    QAction *m_changeStyleSheet;

    QAction *m_addMenuBar;
    QAction *m_addToolBar;
    QAction *m_addStatusBar;
    QAction *m_removeStatusBar;
    QAction *m_changeScript;
    QAction *m_containerFakeMethods;
    PromotionTaskMenu* m_promotionTaskMenu;
};

QDesignerTaskMenuPrivate::QDesignerTaskMenuPrivate(QWidget *widget, QObject *parent) :
    m_widget(widget),
    m_separator(createSeparatorHelper(parent)),
    m_separator2(createSeparatorHelper(parent)),
    m_separator3(createSeparatorHelper(parent)),
    m_separator4(createSeparatorHelper(parent)),
    m_separator5(createSeparatorHelper(parent)),
    m_changeObjectNameAction(new QAction(QDesignerTaskMenu::tr("Change objectName..."), parent)),
    m_changeToolTip(new QAction(QDesignerTaskMenu::tr("Change toolTip..."), parent)),
    m_changeWhatsThis(new QAction(QDesignerTaskMenu::tr("Change whatsThis..."), parent)),
    m_changeStyleSheet(new QAction(QDesignerTaskMenu::tr("Change styleSheet..."), parent)),
    m_addMenuBar(new QAction(QDesignerTaskMenu::tr("Create Menu Bar"), parent)),
    m_addToolBar(new QAction(QDesignerTaskMenu::tr("Add Tool Bar"), parent)),
    m_addStatusBar(new QAction(QDesignerTaskMenu::tr("Create Status Bar"), parent)),
    m_removeStatusBar(new QAction(QDesignerTaskMenu::tr("Remove Status Bar"), parent)),
    m_changeScript(new QAction(QDesignerTaskMenu::tr("Change script..."), parent)),
    m_containerFakeMethods(new QAction(QDesignerTaskMenu::tr("Change signals/slots..."), parent)),
    m_promotionTaskMenu(new PromotionTaskMenu(widget, PromotionTaskMenu::ModeMultiSelection, parent))
{
}
// --------- QDesignerTaskMenu
QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent) :
    QObject(*(new QDesignerTaskMenuPrivate(widget, parent)), parent)
{
    Q_ASSERT(qobject_cast<QDesignerFormWindowInterface*>(widget) == 0);

    Q_D(QDesignerTaskMenu);
    connect(d->m_changeObjectNameAction, SIGNAL(triggered()), this, SLOT(changeObjectName()));
    connect(d->m_changeToolTip, SIGNAL(triggered()), this, SLOT(changeToolTip()));
    connect(d->m_changeWhatsThis, SIGNAL(triggered()), this, SLOT(changeWhatsThis()));
    connect(d->m_changeStyleSheet, SIGNAL(triggered()), this,  SLOT(changeStyleSheet()));
    connect(d->m_addMenuBar, SIGNAL(triggered()), this, SLOT(createMenuBar()));
    connect(d->m_addToolBar, SIGNAL(triggered()), this, SLOT(addToolBar()));
    connect(d->m_addStatusBar, SIGNAL(triggered()), this, SLOT(createStatusBar()));
    connect(d->m_removeStatusBar, SIGNAL(triggered()), this, SLOT(removeStatusBar()));
    connect(d->m_changeScript, SIGNAL(triggered()), this, SLOT(changeScript()));
    connect(d->m_containerFakeMethods, SIGNAL(triggered()), this, SLOT(containerFakeMethods()));
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
}

QAction *QDesignerTaskMenu::createSeparator()
{
    return createSeparatorHelper(this);
}

QWidget *QDesignerTaskMenu::widget() const
{
    Q_D(const QDesignerTaskMenu);
    return d->m_widget;
}

QDesignerFormWindowInterface *QDesignerTaskMenu::formWindow() const
{
    QDesignerFormWindowInterface *result = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(result != 0);
    return result;
}

void QDesignerTaskMenu::createMenuBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        CreateMenuBarCommand *cmd = new CreateMenuBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::addToolBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        AddToolBarCommand *cmd = new AddToolBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::createStatusBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        CreateStatusBarCommand *cmd = new CreateStatusBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTaskMenu::removeStatusBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());
        if (!mw) {
            // ### warning message
            return;
        }

        DeleteStatusBarCommand *cmd = new DeleteStatusBarCommand(fw);
        cmd->init(findStatusBar(mw));
        fw->commandHistory()->push(cmd);
    }
}

QList<QAction*> QDesignerTaskMenu::taskActions() const
{
    Q_D(const QDesignerTaskMenu);
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(formWindow);

    const bool isMainContainer = formWindow->mainContainer() == widget();

    QList<QAction*> actions;

    if (const QMainWindow *mw = qobject_cast<const QMainWindow*>(formWindow->mainContainer()))  {
        if (isMainContainer || mw->centralWidget() == widget()) {
            if (!findMenuBar(mw)) {
                actions.append(d->m_addMenuBar);
            }

            actions.append(d->m_addToolBar);
            // ### create the status bar
            if (!findStatusBar(mw))
                actions.append(d->m_addStatusBar);
            else
                actions.append(d->m_removeStatusBar);
            actions.append(d->m_separator);
        }
    }
    actions.append(d->m_changeObjectNameAction);
    actions.append(d->m_separator2);
    actions.append(d->m_changeToolTip);
    actions.append(d->m_changeWhatsThis);
    actions.append(d->m_changeStyleSheet);

    d->m_promotionTaskMenu->addActions(formWindow, PromotionTaskMenu::LeadingSeparator, actions);

#ifdef WANT_SCRIPT_OPTION
    if (!isMainContainer) {
        actions.append(d->m_separator4);
        actions.append(d->m_changeScript);
    }
#endif
    if (isMainContainer && !qt_extension<QDesignerLanguageExtension*>(formWindow->core()->extensionManager(), formWindow->core())) {
        actions.append(d->m_separator5);
        actions.append(d->m_containerFakeMethods);
    }
    return actions;
}

void QDesignerTaskMenu::changeObjectName()
{
    QDesignerFormWindowInterface *fw = formWindow();
    Q_ASSERT(fw != 0);

    QDesignerFormEditorInterface *core = fw->core();
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), widget());
    Q_ASSERT(sheet != 0);

    const QString objectNameProperty = QLatin1String("objectName");
    ObjectNameDialog dialog(fw, sheet->property(sheet->indexOf(objectNameProperty)).toString());
    if (dialog.exec() == QDialog::Accepted) {
        const QString newObjectName = dialog.newObjectName();
        if (!newObjectName.isEmpty())
            fw->cursor()->setProperty(objectNameProperty, newObjectName);
    }
}

void QDesignerTaskMenu::changeRichTextProperty(const QString &propertyName)
{
    Q_D(QDesignerTaskMenu);
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        Q_ASSERT(d->m_widget->parentWidget() != 0);

        RichTextEditorDialog dlg(fw);
        RichTextEditor *editor = dlg.editor();

        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(fw->core()->extensionManager(), d->m_widget);
        Q_ASSERT(sheet != 0);

        editor->setDefaultFont(d->m_widget->font());
        editor->setText(sheet->property(sheet->indexOf(propertyName)).toString());
        editor->selectAll();
        editor->setFocus();

        if (dlg.exec()) {
            const QString text = editor->text(Qt::RichText);
            fw->cursor()->setWidgetProperty(d->m_widget, propertyName, QVariant(text));
        }
    }
}

void QDesignerTaskMenu::changeToolTip()
{
    changeRichTextProperty(QLatin1String("toolTip"));
}

void QDesignerTaskMenu::changeWhatsThis()
{
    changeRichTextProperty(QLatin1String("whatsThis"));
}

void QDesignerTaskMenu::changeStyleSheet()
{
    Q_D(QDesignerTaskMenu);
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        StyleSheetPropertyEditorDialog dlg(fw, fw, d->m_widget);
        dlg.exec();
    }
}

void QDesignerTaskMenu::changeScript()
{
    Q_D(QDesignerTaskMenu);
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw)
        return;

    MetaDataBase *metaDataBase = qobject_cast<MetaDataBase*>(fw->core()->metaDataBase());
    if (!metaDataBase)
        return;

    const MetaDataBaseItem* item = metaDataBase->metaDataBaseItem(d->m_widget);
    if (!item)
        return;

    const QString oldScript = item->script();
    QString newScript = oldScript;

    ScriptDialog scriptDialog(fw->core()->dialogGui(), fw);
    if (!scriptDialog.editScript(newScript))
        return;

    // compile list of selected objects
    ScriptCommand::ObjectList objects;
    objects += (QWidget *)d->m_widget;

    const QDesignerFormWindowCursorInterface *cursor = fw->cursor();
    const int selectionCount =  cursor->selectedWidgetCount();
    for (int i = 0; i < selectionCount; i++) {
        QWidget *w = cursor->selectedWidget(i);
        if (w != d->m_widget)
             objects += w;
    }
    ScriptCommand *scriptCommand = new ScriptCommand(fw);
    if (!scriptCommand->init(objects, newScript)) {
        delete scriptCommand;
        return;
    }

    fw->commandHistory()->push(scriptCommand);
}

void QDesignerTaskMenu::containerFakeMethods()
{
    Q_D(QDesignerTaskMenu);
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw)
        return;
    SignalSlotDialog::editMetaDataBase(fw, d->m_widget, fw);
}

// ---------------- QDesignerTaskMenuFactory
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
    if (qobject_cast<const QLayoutWidget*>(widget) || qobject_cast<const Spacer*>(widget))
        return 0;

    return new QDesignerTaskMenu(widget, parent);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
