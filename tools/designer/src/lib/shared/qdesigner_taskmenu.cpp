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
#include "promotetocustomwidgetdialog_p.h"
#include "widgetfactory_p.h"
#include "widgetdatabase_p.h"
#include "metadatabase_p.h"
#include "qlayout_widget_p.h"
#include "layout_p.h"
#include "spacer_widget_p.h"
#include "textpropertyeditor_p.h"

#include <shared_enums_p.h>

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QMenuBar>
#include <QtGui/QMainWindow>
#include <QtGui/QStatusBar>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtCore/QSignalMapper>
#include <QtCore/qdebug.h>

namespace  {

QMenuBar *findMenuBar(const QWidget *widget)
{
    const QList<QObject*> children = widget->children();
    foreach (QObject *obj, widget->children()) {
        if (QMenuBar *mb = qobject_cast<QMenuBar*>(obj)) {
            return mb;
        }
    }

    return 0;
}

QStatusBar *findStatusBar(const QWidget *widget)
{
    const QList<QObject*> children = widget->children();
    foreach (QObject *obj, widget->children()) {
        if (QStatusBar *sb = qobject_cast<QStatusBar*>(obj)) {
            return sb;
        }
    }

    return 0;
}

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
      m_editor( new qdesigner_internal::TextPropertyEditor(qdesigner_internal::TextPropertyEditor::EmbeddingNone, 
                                                           qdesigner_internal::ValidationObjectName, this))
{
    setWindowTitle(tr("Change Object Name"));

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->addWidget(new QLabel(tr("Object Name")));

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

QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent) : 
    QObject(parent),
    m_widget(widget),
    m_separator(createSeparator()),
    m_separator2(createSeparator()),
    m_separator3(createSeparator()),
    m_changeObjectNameAction(createAction(tr("Change objectName..."), this, SLOT(changeObjectName()))),
    m_changeToolTip(createAction(tr("Change toolTip..."), this, SLOT(changeToolTip()))),
    m_changeWhatsThis(createAction(tr("Change whatsThis..."), this, SLOT(changeWhatsThis()))),
    m_changeStyleSheet(createAction(tr("Change styleSheet..."), this,  SLOT(changeStyleSheet()))),
    m_addMenuBar(createAction(tr("Create Menu Bar"), this, SLOT(createMenuBar()))),
    m_addToolBar(createAction(tr("Add Tool Bar"), this, SLOT(addToolBar()))),
    m_addStatusBar(createAction(tr("Create Status Bar"), this, SLOT(createStatusBar()))),
    m_removeStatusBar(createAction(tr("Remove Status Bar"), this, SLOT(removeStatusBar()))),
    m_promotionMapper(0)
{
    Q_ASSERT(qobject_cast<QDesignerFormWindowInterface*>(widget) == 0);
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
}
    
QAction *QDesignerTaskMenu::createSeparator() {
    QAction *rc = new QAction(this);
    rc->setSeparator(true);
    return rc;
}
    
QAction *QDesignerTaskMenu::createAction(const QString &text, QObject *receiver, const char *receiverSlot)
{
    QAction *rc = new QAction(text, this);
    connect(rc, SIGNAL(triggered()), receiver, receiverSlot);
    return rc;        
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
    
void QDesignerTaskMenu::createPromotionActions(QDesignerFormWindowInterface *formWindow) const
{
    // clear out old
    if (!m_promotionActions.empty()) {
        foreach (QAction *a, m_promotionActions) 
            a->deleteLater();
        m_promotionActions.clear();        
    }
    // No promotion of main container
    if (formWindow->mainContainer() == m_widget)
        return;

    // Check for a homogenous selection
    const PromotionSelectionList promotionSelection = promotionSelectionList(formWindow);
    if (promotionSelection.empty())
        return;   
    
    // Ugly, but we need to create actions
    QDesignerTaskMenu *taskMenu = const_cast<QDesignerTaskMenu *>(this);
    QDesignerFormEditorInterface *core = formWindow->core();
    // demote
    if (isPromoted(formWindow->core(), m_widget)) {
        QString demoteText = tr("Demote to ");
        demoteText  +=  promotedExtends(core , m_widget);
        m_promotionActions.push_back(taskMenu->createAction(demoteText, taskMenu, SLOT(demoteFromCustomWidget())));
    } else {
        // figure out candidates
        const QString baseClassName = WidgetFactory::classNameOf(core,  m_widget);
        const WidgetDataBaseItemList candidates = promotionCandidates(core->widgetDataBase(), baseClassName );
        if (!candidates.empty()) {
            // Set up a signal mapper to associate class names
            if (!m_promotionMapper) {
                m_promotionMapper = new QSignalMapper(taskMenu);
                connect(m_promotionMapper, SIGNAL(mapped(QString)), taskMenu, SLOT(promoteToCustomWidget(QString)));
            }
            const WidgetDataBaseItemList::const_iterator cend = candidates.constEnd();
            // Set up actions and map class names
            for (WidgetDataBaseItemList::const_iterator it = candidates.constBegin(); it != cend; ++it) {
                const QString customClassName = (*it)->name();
                QString text = tr("Promote to ");
                text +=  customClassName;
                QAction *action = taskMenu->createAction(text, taskMenu->m_promotionMapper, SLOT(map()));
                m_promotionMapper->setMapping(action, customClassName);
                m_promotionActions.push_back(action);
            }
        }
        // promote to new
        QAction *promoteAction = taskMenu->createAction(tr("Promote to Custom Widget"), taskMenu, SLOT(promoteToNewCustomWidget()));
        m_promotionActions.push_back(promoteAction);
    }
}

QList<QAction*> QDesignerTaskMenu::taskActions() const
{
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget());
    Q_ASSERT(formWindow);

    const bool isMainContainer = formWindow->mainContainer() == widget();

    QList<QAction*> actions;

    if (const QMainWindow *mw = qobject_cast<const QMainWindow*>(formWindow->mainContainer()))  {
        if (isMainContainer || mw->centralWidget() == widget()) {
            if (!findMenuBar(mw)) {
                actions.append(m_addMenuBar);
            }

            actions.append(m_addToolBar);
            // ### create the status bar
            if (!findStatusBar(mw))
                actions.append(m_addStatusBar);
            else
                actions.append(m_removeStatusBar);
            actions.append(m_separator);
        }
    }
    actions.append(m_changeObjectNameAction);
    actions.append(m_separator2);
    actions.append(m_changeToolTip);
    actions.append(m_changeWhatsThis);
    actions.append(m_changeStyleSheet);

    createPromotionActions(formWindow );
    if (!m_promotionActions.empty()) {
        actions.append(m_separator3);
        actions += m_promotionActions;
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

    ObjectNameDialog dialog(widget(), sheet->property(sheet->indexOf(QLatin1String("objectName"))).toString());
    if (dialog.exec() == QDialog::Accepted) {
        const QString newObjectName = dialog.newObjectName();
        if (!newObjectName.isEmpty())
            fw->cursor()->setProperty(QLatin1String("objectName"), newObjectName);
    }
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
    if (qobject_cast<const QLayoutWidget*>(widget) || qobject_cast<const Spacer*>(widget))
        return 0;

    return new QDesignerTaskMenu(widget, parent);
}
    
void QDesignerTaskMenu::promoteTo(QDesignerFormWindowInterface *fw, const QString &customClassName)
{
    PromoteToCustomWidgetCommand *cmd = new PromoteToCustomWidgetCommand(fw);
    cmd->init(promotionSelectionList(fw), customClassName);
    fw->commandHistory()->push(cmd);
}
 
    
void  QDesignerTaskMenu::promoteToCustomWidget(const QString &customClassName)
{
    promoteTo(formWindow(), customClassName);
}

void QDesignerTaskMenu::promoteToNewCustomWidget()
{
    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerFormEditorInterface *core = fw->core();
    QWidget *wgt = widget();
    QDesignerWidgetDataBaseInterface *db = core->widgetDataBase();

    Q_ASSERT(!isPromoted(core,wgt));

    const QString base_class_name = WidgetFactory::classNameOf(core, wgt);

    PromoteToCustomWidgetDialog dialog(db, promotionCandidates(db, base_class_name), base_class_name);
    if (!dialog.exec())
        return;

    const QString custom_class_name = dialog.customClassName();
    const QString include_file = dialog.includeFile();
    const QDesignerWidgetDataBaseItemInterface::IncludeType 
        includeType = dialog.isGlobalInclude() ?
                      QDesignerWidgetDataBaseItemInterface::IncludeGlobal : 
                      QDesignerWidgetDataBaseItemInterface::IncludeLocal;
        
    QDesignerWidgetDataBaseItemInterface *item = 
        appendDerived(db,custom_class_name, tr("Promoted Widgets"),
                      base_class_name, include_file, includeType,
                      true,true);
    Q_ASSERT(item);
    // To be a 100% sure, if item already exists.
    item->setIncludeFile(include_file);
    item->setIncludeType(includeType);

    // ### use the undo stack
    promoteTo(fw, custom_class_name);
}

void QDesignerTaskMenu::demoteFromCustomWidget()
{
    QDesignerFormWindowInterface *fw = formWindow();
    const PromotionSelectionList promotedWidgets = promotionSelectionList(fw);
    Q_ASSERT(!promotedWidgets.empty() && isPromoted(fw->core(), promotedWidgets.front()));

    // ### use the undo stack
    DemoteFromCustomWidgetCommand *cmd = new DemoteFromCustomWidgetCommand(fw);
    cmd->init(promotedWidgets);
    fw->commandHistory()->push(cmd);
}

void QDesignerTaskMenu::changeRichTextProperty(const QString &propertyName)
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        Q_ASSERT(m_widget->parentWidget() != 0);
        
        RichTextEditorDialog dlg(fw);
        RichTextEditor *editor = dlg.editor();

        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(fw->core()->extensionManager(), m_widget);
        Q_ASSERT(sheet != 0);

        editor->setDefaultFont(m_widget->font());
        editor->setText(sheet->property(sheet->indexOf(propertyName)).toString());
        editor->selectAll();
        editor->setFocus();

        if (dlg.exec()) {
            const QString text = editor->text(Qt::RichText);
            fw->cursor()->setWidgetProperty(m_widget, propertyName, QVariant(text));
        }
    }
}

QDesignerTaskMenu::PromotionSelectionList QDesignerTaskMenu::promotionSelectionList(QDesignerFormWindowInterface *formWindow) const
{
    // Check for a homogenous selection (same class, same promotion state)
    // and return the list if this is the case. Also make sure m_widget
    // is the last widget in the list so that it is re-selected as the last
    // widget by the promotion commands.
    const QString className = m_widget->metaObject()->className();
    const bool promoted = isPromoted(formWindow->core(), m_widget);
    
    PromotionSelectionList rc;
    
    if (QDesignerFormWindowCursorInterface *cursor = formWindow->cursor()) {
        const int selectedWidgetCount = cursor->selectedWidgetCount();
        for (int i=0; i < selectedWidgetCount; i++) {
            QWidget *w = cursor->selectedWidget(i);
            // Check, put  m_widget last
            if (w != m_widget) {
                if (w->metaObject()->className() != className || isPromoted(formWindow->core(), w) !=  promoted) {
                    return PromotionSelectionList();
                }
                rc.push_back(w);
            }
        }
    }
    rc.push_back(m_widget);
    return rc;
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
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        StyleSheetEditorDialog dlg(fw, m_widget);
        dlg.exec();       
    }
}

} // namespace qdesigner_internal
