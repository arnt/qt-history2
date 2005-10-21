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

#include "menueditor.h"
#include <findicondialog_p.h>
#include <iconloader_p.h>
#include <qdesigner_command_p.h>

#include <QtDesigner/QtDesigner>
#include <QtCore/QDir>
#include <QtCore/QQueue>
#include <QtGui/QMenu>

using namespace qdesigner_internal;

MenuEditor::MenuEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent), m_updating(false), m_widget(0)
{
    ui.setupUi(this);
    m_form = form;
    QIcon resetIcon = createIconSet(QString::fromUtf8("editdelete.png"));
    ui.deletePixmapItemButton->setIcon(resetIcon);
    ui.deletePixmapItemButton->setEnabled(false);

    QIcon upIcon = createIconSet(QString::fromUtf8("up.png"));
    QIcon downIcon = createIconSet(QString::fromUtf8("down.png"));
    QIcon minusIcon = createIconSet(QString::fromUtf8("minus.png"));
    QIcon plusIcon = createIconSet(QString::fromUtf8("plus.png"));
    ui.newItemButton->setIcon(plusIcon);
    ui.deleteItemButton->setIcon(minusIcon);
    ui.moveItemUpButton->setIcon(upIcon);
    ui.moveItemDownButton->setIcon(downIcon);
}

MenuEditor::~MenuEditor()
{
}

void MenuEditor::fillContentsFromWidget(QWidget *widget)
{
    m_widget = widget;

    ui.treeWidget->clear();
    m_actionToItem.clear();
    m_itemToAction.clear();

    m_updating = true;
    QList<QAction *> actions = m_widget->actions();

    QQueue<QPair<QAction *, QTreeWidgetItem *> > pendingList;
    QListIterator<QAction *> it(actions);
    while (it.hasNext())
        pendingList.enqueue(qMakePair(it.next(), (QTreeWidgetItem *)0));

    while (!pendingList.isEmpty()) {
        QPair<QAction *, QTreeWidgetItem *> pair = pendingList.dequeue();

        QAction *action = pair.first;
        QTreeWidgetItem *parentItem = pair.second;

        QTreeWidgetItem *item = 0;
        if (parentItem)
            item = new QTreeWidgetItem(parentItem);
        else
            item = new QTreeWidgetItem(ui.treeWidget);
        item->setText(0, action->text());
        item->setIcon(0, action->icon());
        ui.treeWidget->setItemExpanded(item, true);
        m_actionToItem[action] = item;
        m_itemToAction[item] = action;
        if (action->isSeparator())
            item->setText(0, tr("< S E P A R A T O R >"));
        else
            item->setFlags(item->flags() | Qt::ItemIsEditable);

        QMenu *menu = action->menu();
        if (menu) {
            QList<QAction *> menuActions = menu->actions();
            QListIterator<QAction *> itAction(menuActions);
            while (itAction.hasNext())
                pendingList.enqueue(qMakePair(itAction.next(), item));
        }
    }

    if (ui.treeWidget->topLevelItemCount() > 0)
        ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(0));

    m_updating = false;
    updateEditor();
}

void MenuEditor::on_newItemButton_clicked()
{
    m_updating = true;
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    QTreeWidgetItem *newItem = 0;
    QAction *parentAction = 0;
    QAction *beforeAction = 0;
    if (curItem) {
        if (curItem->parent()) {
            parentAction = m_itemToAction[curItem->parent()];
            QTreeWidgetItem *beforeItem = 0;
            int idx = curItem->parent()->indexOfChild(curItem);
            if (curItem->parent()->childCount() > idx + 1)
                beforeItem = curItem->parent()->child(idx + 1);
            if (beforeItem)
                beforeAction = m_itemToAction[beforeItem];

            newItem = new QTreeWidgetItem(curItem->parent(), curItem);
        }
        else {
            QTreeWidgetItem *beforeItem = 0;
            int idx = ui.treeWidget->indexOfTopLevelItem(curItem);
            if (ui.treeWidget->topLevelItemCount() > idx + 1)
                beforeItem = ui.treeWidget->topLevelItem(idx + 1);
            if (beforeItem)
                beforeAction = m_itemToAction[beforeItem];

            newItem = new QTreeWidgetItem(ui.treeWidget, curItem);
        }
    } else
        newItem = new QTreeWidgetItem(ui.treeWidget);
    QString newText = tr("New Menu");
    newItem->setText(0, newText);
    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

    // COMMAND BEGIN
    QDesignerFormEditorInterface *core = m_form->core();
    QMenu *menu = qobject_cast<QMenu*>(core->widgetFactory()->createWidget("QMenu", m_widget));
    menu->setTitle(newText);
    core->metaDataBase()->add(menu->menuAction());
    m_form->manageWidget(menu);
    menu->setObjectName(QLatin1String("menuAction"));
    m_form->ensureUniqueObjectName(menu);
    menu->menuAction()->setObjectName(menu->objectName());
    if (parentAction)
        parentAction->menu()->insertAction(beforeAction, menu->menuAction());
    else
        m_widget->insertAction(beforeAction, menu->menuAction());
    // COMMAND END


    m_actionToItem[menu->menuAction()] = newItem;
    m_itemToAction[newItem] = menu->menuAction();

    ui.treeWidget->setCurrentItem(newItem);
    m_updating = false;
    updateEditor();
}

void MenuEditor::on_newSubItemButton_clicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    m_updating = true;

    QAction *parentAction = m_itemToAction[curItem];
    if (!parentAction->menu()) // parent action is not a menu type
        return;

    QAction *beforeAction = 0;
    if (curItem->childCount() > 0)
        beforeAction = m_itemToAction[curItem->child(0)];

    QTreeWidgetItem *newItem = new QTreeWidgetItem(curItem);
    QString newText = tr("New Sub Menu");
    newItem->setText(0, newText);
    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

    // COMMAND BEGIN
    QDesignerFormEditorInterface *core = m_form->core();
    QMenu *menu = qobject_cast<QMenu*>(core->widgetFactory()->createWidget("QMenu",
                    parentAction->menu()));
    menu->setTitle(newText);
    core->metaDataBase()->add(menu->menuAction());
    m_form->manageWidget(menu);
    menu->setObjectName(QLatin1String("menuAction"));
    m_form->ensureUniqueObjectName(menu);
    menu->menuAction()->setObjectName(menu->objectName());
    parentAction->menu()->insertAction(beforeAction, menu->menuAction());
    // COMMAND END

    m_actionToItem[menu->menuAction()] = newItem;
    m_itemToAction[newItem] = menu->menuAction();

    ui.treeWidget->setCurrentItem(newItem);
    m_updating = false;
    updateEditor();
}

void MenuEditor::on_newSeparatorItemButton_clicked()
{
    m_updating = true;
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    QTreeWidgetItem *newItem = 0;
    QAction *parentAction = 0;
    QAction *beforeAction = 0;
    if (curItem) {
        if (curItem->parent()) {
            parentAction = m_itemToAction[curItem->parent()];
            QTreeWidgetItem *beforeItem = 0;
            int idx = curItem->parent()->indexOfChild(curItem);
            if (curItem->parent()->childCount() > idx + 1)
                beforeItem = curItem->parent()->child(idx + 1);
            if (beforeItem)
                beforeAction = m_itemToAction[beforeItem];

            newItem = new QTreeWidgetItem(curItem->parent(), curItem);
        }
        else {
            QTreeWidgetItem *beforeItem = 0;
            int idx = ui.treeWidget->indexOfTopLevelItem(curItem);
            if (ui.treeWidget->topLevelItemCount() > idx + 1)
                beforeItem = ui.treeWidget->topLevelItem(idx + 1);
            if (beforeItem)
                beforeAction = m_itemToAction[beforeItem];

            newItem = new QTreeWidgetItem(ui.treeWidget, curItem);
        }
    } else
        newItem = new QTreeWidgetItem(ui.treeWidget);
    QString newText = tr("< S E P A R A T O R >");
    newItem->setText(0, newText);

    // COMMAND BEGIN
    QDesignerFormEditorInterface *core = m_form->core();
    QAction *action = new QAction(m_widget);
    action->setSeparator(true);
    core->metaDataBase()->add(action);
    action->setObjectName(QLatin1String("menuAction"));
    m_form->ensureUniqueObjectName(action);
    if (parentAction)
        parentAction->menu()->insertAction(beforeAction, action);
    else
        m_widget->insertAction(beforeAction, action);
    // COMMAND END

    m_actionToItem[action] = newItem;
    m_itemToAction[newItem] = action;

    ui.treeWidget->setCurrentItem(newItem);
    m_updating = false;
    updateEditor();
}

void MenuEditor::on_deleteItemButton_clicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    m_updating = true;

    QAction *actionToRemove = m_itemToAction[curItem];
    QAction *parentAction = 0;
    QTreeWidgetItem *nextCurrent = 0;
    if (curItem->parent()) {
        parentAction = m_itemToAction[curItem->parent()];
        int idx = curItem->parent()->indexOfChild(curItem);
        if (idx == curItem->parent()->childCount() - 1)
            idx--;
        else
            idx++;
        if (idx < 0)
            nextCurrent = curItem->parent();
        else
            nextCurrent = curItem->parent()->child(idx);
    } else {
        int idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        if (idx == ui.treeWidget->topLevelItemCount() - 1)
            idx--;
        else
            idx++;
        if (idx >= 0)
            nextCurrent = ui.treeWidget->topLevelItem(idx);
    }

    // COMMAND BEGIN
    QDesignerFormEditorInterface *core = m_form->core();
    if (actionToRemove->menu()) {
        core->metaDataBase()->remove(actionToRemove->menu());
        m_form->unmanageWidget(actionToRemove->menu());
    }
    core->metaDataBase()->remove(actionToRemove);
    if (parentAction)
        parentAction->menu()->removeAction(actionToRemove);
    else
        m_widget->removeAction(actionToRemove);
    // COMMAND END

    QQueue<QTreeWidgetItem *> pendingQueue;
    pendingQueue.enqueue(curItem);
    while (!pendingQueue.isEmpty()) {
        QTreeWidgetItem *item = pendingQueue.dequeue();

        QAction *action = m_itemToAction[item];
        m_itemToAction.remove(item);
        m_actionToItem.remove(action);

        for (int i = 0; i < item->childCount(); i++)
            pendingQueue.enqueue(item->child(i));
    }
    delete curItem;

    if (nextCurrent)
        ui.treeWidget->setCurrentItem(nextCurrent);

    m_updating = false;
    updateEditor();
}

void MenuEditor::on_moveItemUpButton_clicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx;
    if (curItem->parent())
        idx = curItem->parent()->indexOfChild(curItem);
    else
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
    if (idx == 0)
        return;

    m_updating = true;

    QAction *parentAction = 0;
    QAction *action = m_itemToAction[curItem];
    QAction *oldBefore = 0;
    QAction *newBefore = 0;

    QTreeWidgetItem *takenItem = 0;
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent();
        parentAction = m_itemToAction[parentItem];
        if (parentItem->childCount() > idx + 1)
            oldBefore = m_itemToAction[parentItem->child(idx + 1)];
        newBefore = m_itemToAction[parentItem->child(idx - 1)];

        takenItem = parentItem->takeChild(idx);
        parentItem->insertChild(idx - 1, takenItem);
    } else {
        if (ui.treeWidget->topLevelItemCount() > idx + 1)
            oldBefore = m_itemToAction[ui.treeWidget->topLevelItem(idx + 1)];
        newBefore = m_itemToAction[ui.treeWidget->topLevelItem(idx - 1)];

        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        ui.treeWidget->insertTopLevelItem(idx - 1, takenItem);
    }
    ui.treeWidget->setCurrentItem(takenItem);

    // COMMAND BEGIN
    if (parentAction) {
        parentAction->menu()->removeAction(action);
        parentAction->menu()->insertAction(newBefore, action);
    } else {
        m_widget->removeAction(action);
        m_widget->insertAction(newBefore, action);
    }
    // COMMAND END

    m_updating = false;
    updateEditor();
}

void MenuEditor::on_moveItemDownButton_clicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx, idxCount;
    if (curItem->parent()) {
        idx = curItem->parent()->indexOfChild(curItem);
        idxCount = curItem->parent()->childCount();
    } else {
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        idxCount = ui.treeWidget->topLevelItemCount();
    }
    if (idx == idxCount - 1)
        return;

    m_updating = true;

    QAction *parentAction = 0;
    QAction *action = m_itemToAction[curItem];
    QAction *oldBefore = 0;
    QAction *newBefore = 0;

    QTreeWidgetItem *takenItem = 0;
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent();
        parentAction = m_itemToAction[parentItem];
        oldBefore = m_itemToAction[parentItem->child(idx + 1)];
        if (parentItem->childCount() > idx + 2)
            newBefore = m_itemToAction[parentItem->child(idx + 2)];

        takenItem = parentItem->takeChild(idx);
        parentItem->insertChild(idx + 1, takenItem);
    } else {
        oldBefore = m_itemToAction[ui.treeWidget->topLevelItem(idx + 1)];
        if (ui.treeWidget->topLevelItemCount() > idx + 2)
            newBefore = m_itemToAction[ui.treeWidget->topLevelItem(idx + 2)];

        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        ui.treeWidget->insertTopLevelItem(idx + 1, takenItem);
    }
    ui.treeWidget->setCurrentItem(takenItem);

    // COMMAND BEGIN
    if (parentAction) {
        parentAction->menu()->removeAction(action);
        parentAction->menu()->insertAction(newBefore, action);
    } else {
        m_widget->removeAction(action);
        m_widget->insertAction(newBefore, action);
    }
    // COMMAND END

    m_updating = false;
    updateEditor();
}

void MenuEditor::on_treeWidget_currentItemChanged(QTreeWidgetItem *,
            QTreeWidgetItem *)
{
    if (m_updating)
        return;
    updateEditor();
    /*
    if (ui.itemTextLineEdit->isEnabled()) {
        ui.itemTextLineEdit->selectAll();
        ui.itemTextLineEdit->setFocus();
    }
    */
}

void MenuEditor::on_treeWidget_itemChanged(QTreeWidgetItem *item)
{
    if (m_updating)
        return;

    m_updating = true;
    QAction *action = m_itemToAction[item];
    QString text = item->text(0);
    // COMMAND BEGIN
    action->setText(text);
    // COMMAND END
    m_updating = false;
    updateEditor();
}

void MenuEditor::updateEditor()
{
    QTreeWidgetItem *current = ui.treeWidget->currentItem();

    bool currentItemEnabled = false;
    bool subItemEnabled = false;
    bool moveItemUpEnabled = false;
    bool moveItemDownEnabled = false;
    bool currentItemEditEnabled = false;

    if (current) {
        currentItemEnabled = true;
        QAction *action = m_itemToAction[current];
        if (action->menu())
            subItemEnabled = true;
        if (!action->isSeparator())
            currentItemEditEnabled = true;
        int idx;
        int idxCount;
        if (current->parent()) {
            idx = current->parent()->indexOfChild(current);
            idxCount = current->parent()->childCount();
        } else {
            idx = ui.treeWidget->indexOfTopLevelItem(current);
            idxCount = ui.treeWidget->topLevelItemCount();
        }
        if (idx > 0)
            moveItemUpEnabled = true;
        if (idx < idxCount - 1)
            moveItemDownEnabled = true;
    }

    ui.textLabel->setEnabled(currentItemEditEnabled);
    ui.pixmapLabel->setEnabled(currentItemEditEnabled);
    ui.deletePixmapItemButton->setEnabled(currentItemEditEnabled);
    ui.previewPixmapItemButton->setEnabled(currentItemEditEnabled);
    ui.itemTextLineEdit->setEnabled(currentItemEditEnabled);
    ui.newSubItemButton->setEnabled(subItemEnabled);
    ui.deleteItemButton->setEnabled(currentItemEnabled);

    ui.moveItemUpButton->setEnabled(moveItemUpEnabled);
    ui.moveItemDownButton->setEnabled(moveItemDownEnabled);

    QString itemText;
    QIcon itemIcon;

    if (current) {
        itemText = current->text(0);
        itemIcon = current->icon(0);
    }

    ui.itemTextLineEdit->setText(itemText);
    ui.previewPixmapItemButton->setIcon(itemIcon);
    ui.deletePixmapItemButton->setEnabled(!itemIcon.isNull());
}

void MenuEditor::on_itemTextLineEdit_textEdited(const QString &text)
{
    if (m_updating)
        return;
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    m_updating = true;
    QAction *action = m_itemToAction[curItem];
    // COMMAND BEGIN
    action->setText(text);
    // COMMAND END
    curItem->setText(0, text);
    m_updating = false;
}

void MenuEditor::on_deletePixmapItemButton_clicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    curItem->setIcon(0, QIcon());

    QAction *action = m_itemToAction[curItem];

    // COMMAND BEGIN
    action->setIcon(QIcon());
    // COMMAND END

    ui.previewPixmapItemButton->setIcon(QIcon());
    ui.deletePixmapItemButton->setEnabled(false);
}

void MenuEditor::on_previewPixmapItemButton_clicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    FindIconDialog dialog(m_form, this);
    QString file_path;
    QString qrc_path;

    QIcon icon = curItem->icon(0);
    if (icon.isNull()) {
        file_path = m_form->absoluteDir().absolutePath();
    } else {
        file_path = m_form->core()->iconCache()->iconToFilePath(icon);
        qrc_path = m_form->core()->iconCache()->iconToQrcPath(icon);
    }

    dialog.setPaths(qrc_path, file_path);
    if (dialog.exec()) {
        file_path = dialog.filePath();
        qrc_path = dialog.qrcPath();
        if (!file_path.isEmpty()) {
            icon = m_form->core()->iconCache()->nameToIcon(file_path, qrc_path);
            curItem->setIcon(0, icon);
            ui.previewPixmapItemButton->setIcon(icon);
            ui.deletePixmapItemButton->setEnabled(!icon.isNull());

            QAction *action = m_itemToAction[curItem];

            // COMMAND BEGIN
            action->setIcon(icon);
            // COMMAND END
        }
    }
}

