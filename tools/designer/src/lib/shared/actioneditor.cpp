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
#include <QtGui/QStandardItemModel>

#include <qdebug.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

#define ICON_SIZE QSize(32, 32)

namespace qdesigner_internal {

static QString fixActionText(QString text)
{
    return text.replace(QLatin1String("&"), QString());
}

static QIcon fixActionIcon(QIcon icon)
{
    static const QIcon empty_icon(":/trolltech/formeditor/images/emptyicon.png");
    if (icon.isNull())
        return empty_icon;
    return icon;
}

/******************************************************************************
** ActionEditorModel
*/

class ActionEditorModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum { ActionRole = Qt::UserRole + 1000 };

    ActionEditorModel(QDesignerFormWindowInterface *form, QObject *parent = 0);
    QAction *action(int idx) const;

private slots:
    void updateActions();
    void actionChanged();
private:
    QPointer<QDesignerFormWindowInterface> m_form;
    void insertAction(QAction *action, int idx);
    void removeAction(int idx);
    int findAction(QAction *action) const;
};

ActionEditorModel::ActionEditorModel(QDesignerFormWindowInterface *form, QObject *parent)
    : QStandardItemModel(parent)
{
    m_form = form;
    connect(m_form, SIGNAL(formActionsChanged()), this, SLOT(updateActions()));
    insertColumn(0);
    updateActions();
}

void ActionEditorModel::updateActions()
{
    if (m_form == 0)
        return;
    QList<QAction*> newActionList = m_form->formActionList();

    for (int i = 0; i < newActionList.size(); ++i) {
        QAction *action = newActionList.at(i);
        int idx = findAction(action);
        if (idx == -1)
            insertAction(action, i);
    }

    int cnt = rowCount();
    for (int i = 0; i < cnt; ++i) {
        QAction *action = this->action(i);
        if (!newActionList.contains(action))
            removeAction(i);
        ++i;
    }
}

void ActionEditorModel::actionChanged()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action == 0)
        return;
    int idx = findAction(action);
    if (idx == -1)
        return;
    QModelIndex model_idx = index(idx, 0);
    setData(model_idx, fixActionText(action->text()), Qt::DisplayRole);
    setData(model_idx, fixActionIcon(action->icon()), Qt::DecorationRole);
}

int ActionEditorModel::findAction(QAction *action) const
{
    int cnt = rowCount();
    for (int i = 0; i < cnt; ++i) {
        if (this->action(i) == action)
            return i;
    }
    return -1;
}

QAction *ActionEditorModel::action(int idx) const
{
    if (idx >= rowCount())
        return 0;
    QVariant actionData = data(index(idx, 0), ActionRole);
    if (!qVariantCanConvert<QAction*>(actionData))
        return 0;
    return qvariant_cast<QAction*>(actionData);
}

void ActionEditorModel::insertAction(QAction *action, int idx)
{
    insertRow(idx);
    QModelIndex model_idx = index(idx, 0);

    setData(model_idx, QSize(ICON_SIZE.width()*3, ICON_SIZE.height()*2), Qt::SizeHintRole);
    setData(model_idx, fixActionText(action->text()), Qt::DisplayRole);
    setData(model_idx, fixActionIcon(action->icon()), Qt::DecorationRole);

    QVariant actionData;
    qVariantSetValue(actionData, action);
    setData(model_idx, actionData, ActionRole);

    connect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
}

void ActionEditorModel::removeAction(int idx)
{
    QAction *action = this->action(idx);
    if (action == 0)
        return;
    disconnect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
    removeRow(idx);
}

/******************************************************************************
** ActionEditorModelCache
*/

class ActionEditorModelCache : public QObject
{
    Q_OBJECT
public:
    ActionEditorModelCache(QDesignerFormEditorInterface *core);
    ActionEditorModel *model(QDesignerFormWindowInterface *form);
private slots:
    void formWindowRemoved(QDesignerFormWindowInterface *form);
private:
    QMap<QDesignerFormWindowInterface*, ActionEditorModel*> m_modelMap;
};

ActionEditorModelCache::ActionEditorModelCache(QDesignerFormEditorInterface *core)
{
    connect(core->formWindowManager(),
            SIGNAL(formWindowRemoved(QDesignerFormWindowInterface*)),
            this, SLOT(formWindowRemoved(QDesignerFormWindowInterface*)));
}

void ActionEditorModelCache::formWindowRemoved(QDesignerFormWindowInterface *form)
{
    ActionEditorModel *model = this->model(form);
    if (model == 0)
        return;
    m_modelMap.remove(form);
    delete model;
}

ActionEditorModel *ActionEditorModelCache::model(QDesignerFormWindowInterface *form)
{
    if (form == 0)
        return 0;
    ActionEditorModel *result = m_modelMap.value(form, 0);
    if (result == 0) {
        result = new ActionEditorModel(form, this);
        m_modelMap.insert(form, result);
    }
    return result;
}

/******************************************************************************
** ActionEditorView
*/

class ActionEditorView : public QListView
{
    Q_OBJECT
public:
    ActionEditorView(QWidget *parent = 0);
};

ActionEditorView::ActionEditorView(QWidget *parent)
    : QListView(parent)
{
    setViewMode(IconMode);
    setMovement(Static);
    setResizeMode(Adjust);
    setIconSize(ICON_SIZE);
    setSpacing(iconSize().width() / 3);
    setTextElideMode(Qt::ElideRight);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    setDragEnabled(true);
    setAcceptDrops(false);
}

/******************************************************************************
** ActionEditor
*/

ActionEditorModelCache *ActionEditor::m_modelCache = 0;

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

    connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(slotDeleteAction()));

    m_actionView = new ActionEditorView(this);
    l->addWidget(m_actionView);
    connect(m_actionView, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotEditAction(QModelIndex)));

    if (m_modelCache == 0)
        m_modelCache = new ActionEditorModelCache(core);
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
    if (formWindow == m_formWindow)
        return;

    QItemSelectionModel *sel_model = m_actionView->selectionModel();
    if (sel_model != 0) {
        disconnect(sel_model, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                    this, SLOT(updatePropertyEditor(QModelIndex)));
    }

    ActionEditorModel *model = m_modelCache->model(formWindow);
    qDebug() << "ActionEditor::setFormWindow():" << model;
    m_formWindow = formWindow;
    m_actionView->setModel(model);

    sel_model = m_actionView->selectionModel();
    if (sel_model != 0) {
        connect(sel_model, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                    this, SLOT(updatePropertyEditor(QModelIndex)));
    }

    bool b = formWindow != 0 && formWindow->mainContainer() != 0;
    m_actionNew->setEnabled(b);
    m_actionDelete->setEnabled(b);
}

QDesignerFormEditorInterface *ActionEditor::core() const
{
    return m_core;
}

void ActionEditor::slotNewAction()
{
    NewActionDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        QAction *action = new QAction(m_formWindow);
        action->setObjectName(dlg.actionName());
        action->setText(dlg.actionText());
        action->setIcon(dlg.actionIcon());
        formWindow()->addFormAction(action);
        core()->propertyEditor()->setObject(action);
    }
}

ActionEditorModel *ActionEditor::actionEditorModel() const
{
    return qobject_cast<ActionEditorModel*>(m_actionView->model());
}

void ActionEditor::slotEditAction(const QModelIndex &index)
{
    ActionEditorModel *model = actionEditorModel();
    if (model == 0)
        return;

    QAction *action = model->action(index.row());

    if (action == 0)
        return;

    NewActionDialog dlg(this);
    dlg.setActionData(action->text(), action->objectName(), action->icon());

    if (!dlg.exec())
        return;

    action->setObjectName(dlg.actionName());
    action->setText(dlg.actionText());
    action->setIcon(dlg.actionIcon());

    core()->propertyEditor()->setObject(action);
}

void ActionEditor::updatePropertyEditor(const QModelIndex &index)
{
    ActionEditorModel *model = actionEditorModel();
    if (model == 0)
        return;

    QAction *action = model->action(index.row());

    if (action == 0)
        return;

    core()->propertyEditor()->setObject(action);
}

void ActionEditor::slotDeleteAction()
{
}

} // namespace qdesigner_internal

#include "actioneditor.moc"
