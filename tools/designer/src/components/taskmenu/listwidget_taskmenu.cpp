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

#include "listwidget_taskmenu.h"

#include <abstractformwindow.h>

#include <QListWidget>
#include <QAction>

class ListWidgetEditor: public QListWidget
{
    Q_OBJECT
public:
    ListWidgetEditor(QListWidget *listWidget);
};

ListWidgetEditor::ListWidgetEditor(QListWidget *listWidget)
    : QListWidget(listWidget)
{
    setParent(listWidget, Qt::WStyle_Tool | Qt::WType_TopLevel);

    setDragEnabled(true);
    setAcceptDrops(false);
    viewport()->setAcceptDrops(false);

    QListWidgetItem *item = 0;

    item = new QListWidgetItem(tr("Item"), this);
    item->setFlags(item->flags() | QAbstractItemModel::ItemIsDragEnabled);

    item = new QListWidgetItem(tr("Checked item"), this);
    item->setCheckState(Qt::Checked);
    item->setFlags(item->flags() | QAbstractItemModel::ItemIsDragEnabled);

    item = new QListWidgetItem(tr("Unchecked item"), this);
    item->setCheckState(Qt::Unchecked);
    item->setFlags(item->flags() | QAbstractItemModel::ItemIsDragEnabled);

    item = new QListWidgetItem(tr("Item with an icon"), this);
    item->setIcon(QIcon(":/trolltech/formeditor/images/qtlogo.png"));
    item->setFlags(item->flags() | QAbstractItemModel::ItemIsDragEnabled);
}


ListWidgetTaskMenu::ListWidgetTaskMenu(QListWidget *listWidget, QObject *parent)
    : QDesignerTaskMenu(listWidget, parent),
      m_listWidget(listWidget)
{
    QAction *editItemsAction = new QAction(tr("Edit Items"), this);
    connect(editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_actions.append(editItemsAction);
}

ListWidgetTaskMenu::~ListWidgetTaskMenu()
{
}

QListWidget *ListWidgetTaskMenu::listWidget() const
{
    return m_listWidget;
}

QList<QAction*> ListWidgetTaskMenu::taskActions() const
{
    QList<QAction*> actions;
    actions += QDesignerTaskMenu::taskActions();
    actions += m_actions;
    return actions;
}

void ListWidgetTaskMenu::editItems()
{
    Q_ASSERT(m_editor == 0);

    if (AbstractFormWindow *formWindow = AbstractFormWindow::findFormWindow(listWidget())) {
        m_editor = new ListWidgetEditor(listWidget());
        connect(formWindow, SIGNAL(selectionChanged()), m_editor, SLOT(deleteLater()));
        m_editor->show();
    }
}

// ---- ListWidgetTaskMenuFactory ----
ListWidgetTaskMenuFactory::ListWidgetTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *ListWidgetTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(ITaskMenu))
        return 0;

    if (QListWidget *listWidget = qt_cast<QListWidget*>(object)) {
        return new ListWidgetTaskMenu(listWidget, parent);
    }

    return 0;
}


#include "listwidget_taskmenu.moc"
