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

#include "actioneditor_p.h"
#include "actionrepository_p.h"
#include "iconloader_p.h"
#include "newactiondialog_p.h"
#include "qdesigner_menu_p.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidget>
#include <QtGui/QToolBar>
#include <QtGui/QSplitter>
#include <QtGui/QAction>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>

#include <qdebug.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

namespace qdesigner_internal {

class ActionFilterWidget: public QWidget
{
public:
    ActionFilterWidget(ActionEditor *actionEditor, QToolBar *parent)
        : QWidget(parent),
          m_actionEditor(actionEditor)
    {
        QHBoxLayout *l = new QHBoxLayout(this);
        l->setMargin(0);
        l->setSpacing(0);

        l->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

        QLabel *label = new QLabel(tr("Filter: "), this);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->addWidget(label);

        m_editor = new QLineEdit(this);
        l->addWidget(m_editor);

        connect(m_editor, SIGNAL(textChanged(QString)), actionEditor, SLOT(setFilter(QString)));
    }

private:
    QLineEdit *m_editor;
    ActionEditor *m_actionEditor;
};

class ActionGroupDelegate: public QItemDelegate
{
public:
    ActionGroupDelegate(QObject *parent)
        : QItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        QItemDelegate::paint(painter, option, index);
    }

    virtual void drawFocus(QPainter * /*painter*/, const QStyleOptionViewItem &/*option*/, const QRect &/*rect*/) const {}
};

ActionEditor::ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags)
    : QDesignerActionEditorInterface(parent, flags),
      m_core(core)
{
    setWindowTitle(tr("Actions"));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    QToolBar *toolbar = new QToolBar(this);
//    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // ### style
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    l->addWidget(toolbar);

    m_actionNew = toolbar->addAction(tr("New..."));
    m_actionNew->setIcon(createIconSet("filenew.png"));
    m_actionNew->setEnabled(false);
    connect(m_actionNew, SIGNAL(triggered()), this, SLOT(slotNewAction()));

    m_actionDelete = toolbar->addAction(tr("Delete"));
    m_actionDelete->setIcon(createIconSet("editdelete.png"));
    m_actionDelete->setEnabled(false);

    m_filterWidget = new ActionFilterWidget(this, toolbar);
    m_filterWidget->setEnabled(false);
    toolbar->addWidget(m_filterWidget);

    connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(slotDeleteAction()));

    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    l->addWidget(splitter);

#if 0 // ### implement me
    m_actionGroups = new QListWidget(splitter);
    splitter->addWidget(m_actionGroups);
    m_actionGroups->setItemDelegate(new ActionGroupDelegate(m_actionGroups));
    m_actionGroups->setMovement(QListWidget::Static);
    m_actionGroups->setResizeMode(QListWidget::Fixed);
    m_actionGroups->setIconSize(QSize(48, 48));
    m_actionGroups->setFlow(QListWidget::TopToBottom);
    m_actionGroups->setViewMode(QListWidget::IconMode);
    m_actionGroups->setWrapping(false);
#endif

    m_actionRepository = new ActionRepository(splitter);
    splitter->addWidget(m_actionRepository);

    connect(m_actionRepository, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(slotItemChanged(QListWidgetItem*)));
    connect(m_actionRepository, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(editAction(QListWidgetItem*)));
}

ActionEditor::~ActionEditor()
{
}

QAction *ActionEditor::actionNew() const
{
    return m_actionNew;
}

QAction *ActionEditor::actionDelete() const
{
    return m_actionDelete;
}

QDesignerFormWindowInterface *ActionEditor::formWindow() const
{
    return m_formWindow;
}

void ActionEditor::setFormWindow(QDesignerFormWindowInterface *formWindow)
{
    m_formWindow = formWindow;

    m_actionRepository->clear();

    if (!formWindow || !formWindow->mainContainer()) {
        m_actionNew->setEnabled(false);
        m_actionDelete->setEnabled(false);
        m_filterWidget->setEnabled(false);
        return;
    }

    m_actionNew->setEnabled(true);
    m_actionDelete->setEnabled(true);
    m_filterWidget->setEnabled(true);

    QList<QAction*> actionList = qFindChildren<QAction*>(formWindow->mainContainer());
    foreach (QAction *action, actionList) {
        if (!core()->metaDataBase()->item(action)
            || action->isSeparator()
            // ### || action->menu()
            )
            continue;

        createListWidgetItem(action);
    }

    setFilter(m_filter);
}

QString fixActionText(QString text)
{
    return text.replace(QLatin1String("&"), QString());
}

QIcon fixActionIcon(QIcon icon)
{
    static const QIcon empty_icon(":/trolltech/formeditor/images/emptyicon.png");
    if (icon.isNull())
        return empty_icon;
    return icon;
}

QListWidgetItem *ActionEditor::createListWidgetItem(QAction *action)
{
    if (action->menu())
        return 0;

    QListWidgetItem *item = new QListWidgetItem(m_actionRepository);
    QSize s = m_actionRepository->iconSize();
    item->setSizeHint(QSize(s.width()*3, s.height()*2));
    item->setText(fixActionText(action->text()));
    item->setIcon(fixActionIcon(action->icon()));

    QVariant itemData;
    qVariantSetValue(itemData, action);
    item->setData(ActionRepository::ActionRole, itemData);

    QVariant actionData;
    qVariantSetValue(actionData, item);
    action->setData(actionData);

    connect(action, SIGNAL(changed()), this, SLOT(slotActionChanged()));

    return item;
}

void ActionEditor::updatePropertyEditor(QAction *action)
{
    if (!action || !core()->propertyEditor())
        return;

    QObject *sel = action;
    if (action->menu())
        sel = action->menu();

    core()->propertyEditor()->setObject(sel);
}

void ActionEditor::slotItemChanged(QListWidgetItem *item)
{
    if (!item)
        return;

    if (QAction *action = qvariant_cast<QAction*>(item->data(ActionRepository::ActionRole))) {
        updatePropertyEditor(action);
    }
}

void ActionEditor::slotActionChanged()
{
    QIcon empty_icon(":/trolltech/formeditor/images/emptyicon.png");

    QAction *action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action != 0);

    QListWidgetItem *item = qvariant_cast<QListWidgetItem*>(action->data());
    Q_ASSERT(item != 0);

    item->setText(fixActionText(action->text()));
    item->setIcon(fixActionIcon(action->icon()));
}

QDesignerFormEditorInterface *ActionEditor::core() const
{
    return m_core;
}

QString ActionEditor::filter() const
{
    return m_filter;
}

void ActionEditor::setFilter(const QString &f)
{
    m_filter = f;
    m_actionRepository->filter(m_filter);
}

void ActionEditor::slotNewAction()
{
    NewActionDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        QWidget *form = formWindow()->mainContainer();

        QAction *action = new QAction(form);
        action->setObjectName(dlg.actionName());
        action->setText(dlg.actionText());
        action->setIcon(dlg.actionIcon());

        core()->metaDataBase()->add(action);

        QDesignerPropertySheetExtension *sheet = 0;
        sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), action);
        sheet->setChanged(sheet->indexOf("objectName"), true);
        sheet->setChanged(sheet->indexOf("text"), true);
        sheet->setChanged(sheet->indexOf("icon"), !action->icon().isNull());

        // formWindow()->emitSelectionChanged();
        m_actionRepository->clearSelection();
        QListWidgetItem *item = createListWidgetItem(action);
        m_actionRepository->setItemSelected(item, true);
        updatePropertyEditor(action);
    }
}

void ActionEditor::editAction(QListWidgetItem *item)
{
    if (!item)
        return;

    QAction *action = qvariant_cast<QAction*>(item->data(ActionRepository::ActionRole));
    if (action == 0)
        return;


    NewActionDialog dlg(this);
    dlg.setActionData(action->text(), action->objectName(), action->icon());

    if (!dlg.exec())
        return;

    action->setObjectName(dlg.actionName());
    action->setText(dlg.actionText());
    action->setIcon(dlg.actionIcon());

    QDesignerPropertySheetExtension *sheet = 0;
    sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), action);
    sheet->setChanged(sheet->indexOf("icon"), !action->icon().isNull());

    updatePropertyEditor(action);
}

void ActionEditor::slotDeleteAction()
{
    QListWidgetItem *item = m_actionRepository->currentItem();
    if (item == 0)
        return;

    QAction *action = qvariant_cast<QAction*>(item->data(ActionRepository::ActionRole));
    if (action == 0)
        return;

    core()->metaDataBase()->remove(action);
    action->setParent(0);

    delete item;

    if (m_actionRepository->count() == 0)
        core()->propertyEditor()->setObject(formWindow()->mainContainer());
}

void ActionEditor::slotNotImplemented()
{
    QMessageBox::information(this, tr("Designer"), tr("Feature not implemented!"));
}

} // namespace qdesigner_internal
