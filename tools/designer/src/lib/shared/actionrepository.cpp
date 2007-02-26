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

#include "actionrepository_p.h"
#include "resourcemimedata_p.h"

#include <QtGui/QDrag>
#include <QtGui/QContextMenuEvent>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

static inline QAction *actionOfItem(const QListWidgetItem* item)
{
    return qvariant_cast<QAction*>(item->data(qdesigner_internal::ActionRepository::ActionRole));
}

// Check for image
static inline const qdesigner_internal::ResourceMimeData *imageResourceMimeData(const QMimeData *data)
{
    const qdesigner_internal::ResourceMimeData *resourceMimeData = qobject_cast<const qdesigner_internal::ResourceMimeData *>(data);
    if (!resourceMimeData || resourceMimeData->type() != qdesigner_internal::ResourceMimeData::Image)
        return 0;
    return resourceMimeData;
}

namespace qdesigner_internal {

ActionRepository::ActionRepository(QWidget *parent)
    : QListWidget(parent)
{
    setViewMode(IconMode);
    setMovement(Static);
    setResizeMode(Adjust);
    setIconSize(QSize(24, 24));
    setSpacing(iconSize().width() / 3);
    setTextElideMode(Qt::ElideMiddle);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode (DragDrop);
}

void ActionRepository::startDrag(Qt::DropActions supportedActions)
{
    if (!selectionModel())
        return;

    const QModelIndexList indexes = selectionModel()->selectedIndexes();

    if (indexes.count() > 0) {
        QDrag *drag = new QDrag(this);
        const QIcon icon = qvariant_cast<QIcon>(model()->data(indexes.front(), Qt::DecorationRole));
        drag->setPixmap(icon.pixmap(QSize(22, 22)));
        drag->setMimeData(model()->mimeData(indexes));
        drag->start(supportedActions);
    }
}

bool ActionRepository::event (QEvent * event )
{
    if (movement() != Static)
        return  QListWidget::event(event);

    // Static movement turns off drag handling, unfortunately.
    // We need to dispatch ourselves.
    switch (event->type()) {
    case QEvent::DragEnter:
        dragEnterEvent(static_cast<QDragEnterEvent *>(event));
        return true;
    case QEvent::DragMove:
        dragMoveEvent(static_cast<QDragMoveEvent *>(event));
        return true;
    case QEvent::Drop:
        dropEvent(static_cast<QDropEvent *>(event));
        return true;
    default:
        break;
    }
    return QListWidget::event(event);
}


void ActionRepository::dragEnterEvent(QDragEnterEvent *event)
{
    // We can not override the mime types of the model,
    // so we do our own checking
    if (imageResourceMimeData(event->mimeData())) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void  ActionRepository::dragMoveEvent(QDragMoveEvent *event)
{
    if (imageResourceMimeData(event->mimeData()))
        event->acceptProposedAction();
    else 
        event->ignore();
}

bool ActionRepository::dropMimeData(int index, const QMimeData * data, Qt::DropAction action )
{
    if (action != Qt::CopyAction)
        return false;

    QListWidgetItem *droppedItem = item(index);
    if (!droppedItem)
        return false;

    const ResourceMimeData *resourceMimeData = imageResourceMimeData(data);
    if (!resourceMimeData)
        return false;
    emit resourceImageDropped(resourceMimeData, actionOfItem(droppedItem));
    return true;
}

QMimeData *ActionRepository::mimeData(const QList<QListWidgetItem*> items) const
{
    ActionRepositoryMimeData::ActionList actionList;

    foreach (QListWidgetItem *item, items)
         actionList += actionOfItem(item);

    return new ActionRepositoryMimeData(actionList);
}

void ActionRepository::filter(const QString &text)
{
    const QSet<QListWidgetItem*> visibleItems = QSet<QListWidgetItem*>::fromList(findItems(text, Qt::MatchContains));
    for (int index=0; index<count(); ++index) {
        QListWidgetItem *i = item(index);
        setItemHidden(i, !visibleItems.contains(i));
    }
}

void ActionRepository::focusInEvent(QFocusEvent *event)
{
    QListWidget::focusInEvent(event);
    if (currentItem()) {
        emit currentItemChanged(currentItem(), currentItem());
    }
}

void ActionRepository::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenuRequested(event, itemAt(event->pos()));
}

// ----------     ActionRepositoryMimeData
ActionRepositoryMimeData::ActionRepositoryMimeData(QAction *a)
{
    m_actionList += a;
}

ActionRepositoryMimeData::ActionRepositoryMimeData(const ActionList &al) :
    m_actionList(al)
{
}

QStringList ActionRepositoryMimeData::formats() const
{
    return QStringList(QLatin1String("action-repository/actions"));
}
} // namespace qdesigner_internal
