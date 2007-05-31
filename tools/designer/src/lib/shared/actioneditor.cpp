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

/*
TRANSLATOR qdesigner_internal::ActionEditor
*/

#include "actioneditor_p.h"
#include "actionrepository_p.h"
#include "iconloader_p.h"
#include "newactiondialog_p.h"
#include "qdesigner_menu_p.h"
#include "qdesigner_command_p.h"
#include "qdesigner_propertycommand_p.h"
#include "resourcemimedata_p.h"
#include "qdesigner_objectinspector_p.h"
#include "qdesigner_utils_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertyEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerMetaDataBaseInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerIconCacheInterface>

#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QToolBar>
#include <QtGui/QSplitter>
#include <QtGui/QAction>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QContextMenuEvent>

#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtCore/QSignalMapper>

Q_DECLARE_METATYPE(QAction*)

namespace qdesigner_internal {
//-------- ActionFilterWidget
class ActionFilterWidget: public QWidget
{
    Q_OBJECT
public:
    ActionFilterWidget(ActionEditor *actionEditor, QToolBar *parent)
        : QWidget(parent),
          m_button(new QPushButton(this)),
          m_editor(new QLineEdit(this)),
          m_actionEditor(actionEditor)
    {
        QHBoxLayout *l = new QHBoxLayout(this);
        l->setMargin(0);
        l->setSpacing(0);

        l->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

        QLabel *label = new QLabel(tr("Filter: "), this);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->addWidget(label);

        l->addWidget(m_editor);

        connect(m_editor, SIGNAL(textChanged(QString)), actionEditor, SLOT(setFilter(QString)));

        m_button->setIcon(createIconSet(QLatin1String("resetproperty.png")));
        m_button->setIconSize(QSize(16, 16));
        m_button->setFlat(true);
        l->addWidget(m_button);
        l->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

        connect(m_button, SIGNAL(clicked()), m_editor, SLOT(clear()));
        connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(checkButton(QString)));
    }

private slots:
    void checkButton(const QString &text)
    {
        m_button->setEnabled(!text.isEmpty());
    }

private:
    QPushButton *m_button;
    QLineEdit *m_editor;
    ActionEditor *m_actionEditor;
};

//--------  ActionGroupDelegate
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

//--------  ActionEditor
ActionEditor::ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags) :
    QDesignerActionEditorInterface(parent, flags),
    m_core(core),
    m_actionGroups(0),
    m_actionView(new ActionView),
    m_actionNew(new QAction(tr("New..."), this)),
    m_actionEdit(new QAction(tr("Edit..."), this)),
    m_actionDelete(new QAction(tr("Delete"), this)),
    m_viewModeGroup(new  QActionGroup(this)),
    m_iconViewAction(0),
    m_listViewAction(0),
    m_filterWidget(0),
    m_selectAssociatedWidgetsMapper(0)
{
    m_actionView->initialize(m_core);
    setWindowTitle(tr("Actions"));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    QToolBar *toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(22, 22));
    toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    l->addWidget(toolbar);
    // edit actions
    m_actionNew->setIcon(createIconSet(QLatin1String("filenew.png")));
    m_actionNew->setEnabled(false);
    connect(m_actionNew, SIGNAL(triggered()), this, SLOT(slotNewAction()));
    toolbar->addAction(m_actionNew);

    m_actionDelete->setIcon(createIconSet(QLatin1String("editdelete.png")));
    m_actionDelete->setEnabled(false);
    connect(m_actionDelete, SIGNAL(triggered()), this, SLOT(slotDeleteAction()));
    toolbar->addAction(m_actionDelete);

    m_actionEdit->setEnabled(false);
    connect(m_actionEdit, SIGNAL(triggered()), this, SLOT(editCurrentAction()));
    // filter
    m_filterWidget = new ActionFilterWidget(this, toolbar);
    m_filterWidget->setEnabled(false);
    toolbar->addWidget(m_filterWidget);
    // Action group for detailed/icon view. Steal the icons from the file dialog.
    m_viewModeGroup->setExclusive(true);
    connect(m_viewModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotViewMode(QAction*)));

    m_iconViewAction = m_viewModeGroup->addAction(tr("Icon View"));
    m_iconViewAction->setData(QVariant(ActionView::IconView));
    m_iconViewAction->setCheckable(true);
    m_iconViewAction->setIcon(style()->standardIcon (QStyle::SP_FileDialogListView));
    toolbar->addAction(m_iconViewAction);

    m_listViewAction = m_viewModeGroup->addAction(tr("Detailed View"));
    m_listViewAction->setData(QVariant(ActionView::DetailedView));
    m_listViewAction->setCheckable(true);
    m_listViewAction->setIcon(style()->standardIcon (QStyle::SP_FileDialogDetailedView));
    toolbar->addAction(m_listViewAction);
    // main layout
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    splitter->addWidget(m_actionView);
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

    connect(m_actionView, SIGNAL(resourceImageDropped(ResourceMimeData,QAction*)),
            this, SLOT(resourceImageDropped(ResourceMimeData,QAction*)));

    connect(m_actionView, SIGNAL(currentChanged(QAction*)),this, SLOT(slotCurrentItemChanged(QAction*)));
    // make it possible for vs integration to reimplement edit action dialog
    connect(m_actionView, SIGNAL(activated(QAction*)), this, SIGNAL(itemActivated(QAction*)));

    connect(m_actionView, SIGNAL(contextMenuRequested(QContextMenuEvent*, QAction*)),
            this, SLOT(slotContextMenuRequested(QContextMenuEvent*, QAction*)));

    connect(this, SIGNAL(itemActivated(QAction*)), this, SLOT(editAction(QAction*)));

    updateViewModeActions();
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
    if (formWindow != 0 && formWindow->mainContainer() == 0)
        formWindow = 0;

    // we do NOT rely on this function to update the action editor
    if (m_formWindow == formWindow)
        return;

    if (m_formWindow != 0) {
        const QList<QAction*> actionList = qFindChildren<QAction*>(m_formWindow->mainContainer());
        foreach (QAction *action, actionList)
            disconnect(action, SIGNAL(changed()), this, SLOT(slotActionChanged()));
    }

    m_formWindow = formWindow;

    m_actionView->model()->clear();

    if (!formWindow || !formWindow->mainContainer()) {
        m_actionNew->setEnabled(false);
        m_actionEdit->setEnabled(false);
        m_actionDelete->setEnabled(false);
        m_filterWidget->setEnabled(false);
        return;
    }

    m_actionNew->setEnabled(true);
    m_filterWidget->setEnabled(true);

    const QList<QAction*> actionList = qFindChildren<QAction*>(formWindow->mainContainer());
    foreach (QAction *action, actionList) {
        if (!core()->metaDataBase()->item(action)
            || action->isSeparator()
            // ### || action->menu()
            ) {
            continue;
        }
        m_actionView->model()->addAction(action);
        connect(action, SIGNAL(changed()), this, SLOT(slotActionChanged()));
    }

    setFilter(m_filter);
}

void ActionEditor::slotCurrentItemChanged(QAction *action)
{
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw)
        return;

    const bool hasCurrentAction = action != 0;
    m_actionEdit->setEnabled(hasCurrentAction);
    m_actionDelete->setEnabled(hasCurrentAction);

    if (!action) {
        fw->clearSelection();
        return;
    }

    QDesignerObjectInspector *oi = qobject_cast<QDesignerObjectInspector *>(core()->objectInspector());

    if (action->associatedWidgets().empty()) {
        // Special case: action not in object tree. Unselect all and set in property editor
        fw->clearSelection(false);
        if (oi)
            oi->clearSelection();
        core()->propertyEditor()->setObject(action);
    } else {
        if (oi)
            oi->selectObject(action);
    }
}

void ActionEditor::slotActionChanged()
{
    QAction *action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action != 0);

    ActionModel *model = m_actionView->model();
    const int row = model->findAction(action);
    if (row == -1) {
        if (action->menu() == 0) // action got its menu deleted, create item
            model->addAction(action);
    } else if (action->menu() != 0) { // action got its menu created, remove item
        model->removeRow(row);
    } else {
        // action text or icon changed, update item
        model->update(row);
    }
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
    m_actionView->filter(m_filter);
}

// Set changed state of icon property,  reset when icon is cleared
static void refreshIconPropertyChanged(const QAction *action, QDesignerPropertySheetExtension *sheet)
{
    sheet->setChanged(sheet->indexOf(QLatin1String("icon")), !action->icon().isNull());
}

void ActionEditor::manageAction(QAction *action)
{
    action->setParent(formWindow()->mainContainer());
    core()->metaDataBase()->add(action);

    if (action->isSeparator() || action->menu() != 0)
        return;

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), action);
    sheet->setChanged(sheet->indexOf(QLatin1String("objectName")), true);
    sheet->setChanged(sheet->indexOf(QLatin1String("text")), true);
    refreshIconPropertyChanged(action, sheet);

    m_actionView->setCurrentIndex(m_actionView->model()->addAction(action));
    connect(action, SIGNAL(changed()), this, SLOT(slotActionChanged()));
}

void ActionEditor::unmanageAction(QAction *action)
{
    core()->metaDataBase()->remove(action);
    action->setParent(0);

    disconnect(action, SIGNAL(changed()), this, SLOT(slotActionChanged()));

    const int row = m_actionView->model()->findAction(action);
    if (row != -1)
        m_actionView->model()->remove(row);
}

void ActionEditor::slotNewAction()
{
    NewActionDialog dlg(this);
    dlg.setWindowTitle(tr("New action"));

    if (dlg.exec() == QDialog::Accepted) {
        QAction *action = new QAction(formWindow());
        action->setObjectName(dlg.actionName());
        formWindow()->ensureUniqueObjectName(action);
        action->setText(dlg.actionText());
        action->setIcon(dlg.actionIcon());

        AddActionCommand *cmd = new AddActionCommand(formWindow());
        cmd->init(action);
        formWindow()->commandHistory()->push(cmd);
    }
}

static inline bool isSameIcon(const QIcon &i1, const QIcon &i2)
{
    return i1.serialNumber() == i2.serialNumber();
}

// return a FormWindow command to apply an icon
static QDesignerFormWindowCommand *setIconPropertyCommand(const QIcon &newIcon, QAction *action, QDesignerFormWindowInterface *fw)
{
    const QString iconProperty = QLatin1String("icon");
    if (newIcon.isNull()) {
        ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
        cmd->init(action, iconProperty);
        return cmd;
    }
    SetPropertyCommand *cmd = new SetPropertyCommand(fw);
    cmd->init(action, iconProperty, newIcon);
    return cmd;
}

static QDesignerFormWindowCommand *setTextPropertyCommand(const QString &propertyName, const QString &text, QAction *action, QDesignerFormWindowInterface *fw)
{
    if (text.isEmpty()) {
        ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
        cmd->init(action, propertyName);
        return cmd;
    }
    SetPropertyCommand *cmd = new SetPropertyCommand(fw);
    cmd->init(action, propertyName, text);
    return cmd;
}

void ActionEditor::editAction(QAction *action)
{
    if (!action)
        return;

    NewActionDialog dlg(this);
    dlg.setWindowTitle(tr("Edit action"));
    dlg.setActionData(action->text(), action->objectName(), action->icon());

    if (!dlg.exec())
        return;

    // figure out changes and whether to start a macro
    enum ChangedMask { NameChanged = 1, TextChanged = 2 , IconChanged = 4 };
    const QString newName = dlg.actionName();
    const QString newText = dlg.actionText();
    const QIcon newIcon = dlg.actionIcon();

    int changedMask = 0;

    if (newName != action->objectName())
        changedMask |= NameChanged;
    if (newText != action->text())
        changedMask |= TextChanged;
    if (!isSameIcon(newIcon, action->icon()))
        changedMask |= IconChanged;

    if (!changedMask)
        return;

    const bool severalChanges = (changedMask != NameChanged) && (changedMask != TextChanged) && (changedMask != IconChanged);
    if (severalChanges)
        formWindow()->beginCommand(QLatin1String("Edit action"));

    if (changedMask & NameChanged)
        formWindow()->commandHistory()->push(setTextPropertyCommand(QLatin1String("objectName"), newName, action, formWindow()));

    if (changedMask & TextChanged)
        formWindow()->commandHistory()->push(setTextPropertyCommand(QLatin1String("text"), newText, action, formWindow()));

    if (changedMask & IconChanged)
        formWindow()->commandHistory()->push(setIconPropertyCommand(newIcon, action, formWindow()));

    if (severalChanges)
        formWindow()->endCommand();

}

void ActionEditor::editCurrentAction()
{
    if (QAction *a = m_actionView->currentAction())
        editAction(a);
}

void ActionEditor::slotDeleteAction()
{
    QAction *action = m_actionView->currentAction();
    if (!action)
        return;

    RemoveActionCommand *cmd = new RemoveActionCommand(formWindow());
    cmd->init(action);
    formWindow()->commandHistory()->push(cmd);
}

void ActionEditor::slotNotImplemented()
{
    QMessageBox::information(this, tr("Designer"), tr("Feature not implemented!"));
}

QString ActionEditor::actionTextToName(const QString &text, const QString &prefix)
{
    QString name = text;
    if (name.isEmpty())
        return QString();

    name[0] = name.at(0).toUpper();
    name.prepend(prefix);
    const QString underscore = QString(QLatin1Char('_'));
    name.replace(QRegExp(QString(QLatin1String("[^a-zA-Z_0-9]"))), underscore);
    name.replace(QRegExp(QLatin1String("__*")), underscore);
    if (name.endsWith(underscore.at(0)))
        name.truncate(name.size() - 1);

    return name;
}

void  ActionEditor::resourceImageDropped(const ResourceMimeData &data, QAction *action)
{
    QDesignerFormWindowInterface *fw =  formWindow();
    if (!fw)
        return;

    const QIcon icon = resourceMimeDataToIcon(data, fw);

    if (icon.isNull() || isSameIcon(icon, action->icon()))
        return;

    fw->commandHistory()->push(setIconPropertyCommand(icon , action, fw));
}

void ActionEditor::mainContainerChanged()
{
    // Invalidate references to objects kept in model
    if (sender() == formWindow())
        setFormWindow(0);
}

int ActionEditor::viewMode() const
{
    return m_actionView->viewMode();
}

void ActionEditor::setViewMode(int lm)
{
    m_actionView->setViewMode(lm);
    updateViewModeActions();
}

void ActionEditor::slotViewMode(QAction *a)
{
    setViewMode(a->data().toInt());
}

void ActionEditor::slotSelectAssociatedWidget(QWidget *w)
{
    QDesignerFormWindowInterface *fw = formWindow();
    if (!fw )
        return;

    QDesignerObjectInspector *oi = qobject_cast<QDesignerObjectInspector *>(core()->objectInspector());
    if (!oi)
        return;

    fw->clearSelection(); // Actually, there are no widgets selected due to focus in event handling. Just to be sure.
    oi->selectObject(w);
}

void ActionEditor::updateViewModeActions()
{
    switch (viewMode()) {
    case ActionView::IconView:
        m_iconViewAction->setChecked(true);
        break;
    case ActionView::DetailedView:
        m_listViewAction->setChecked(true);
        break;
    }
}


void ActionEditor::slotContextMenuRequested(QContextMenuEvent *e, QAction *item)
{
    // set up signal mapper
    if (!m_selectAssociatedWidgetsMapper) {
        m_selectAssociatedWidgetsMapper = new QSignalMapper(this);
        connect(m_selectAssociatedWidgetsMapper, SIGNAL(mapped(QWidget*)), this, SLOT(slotSelectAssociatedWidget(QWidget*)));
    }


    QMenu menu(this);

    menu.addAction(m_iconViewAction);
    menu.addAction(m_listViewAction);
    menu.addSeparator();
    menu.addAction(m_actionNew);
    menu.addAction(m_actionEdit);

    // Associated Widgets
    if (QAction *action = m_actionView->currentAction()) {
        const QWidgetList associatedWidgets = ActionModel::associatedWidgets(action);
        if (!associatedWidgets.empty()) {
            QMenu *associatedWidgetsSubMenu =  menu.addMenu(tr("Used In"));
            foreach (QWidget *w, associatedWidgets) {
                QAction *action = associatedWidgetsSubMenu->addAction(w->objectName());
                m_selectAssociatedWidgetsMapper->setMapping(action, w);
                connect(action, SIGNAL(triggered()), m_selectAssociatedWidgetsMapper, SLOT(map()));
            }
        }
    }

    menu.addAction(m_actionDelete);

    emit contextMenuRequested(&menu, item);

    menu.exec(e->globalPos());
    e->accept();
}

} // namespace qdesigner_internal

#include "actioneditor.moc"
