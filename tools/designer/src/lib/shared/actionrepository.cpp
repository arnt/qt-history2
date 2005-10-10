#include "actionrepository_p.h"

#include <QtGui/QDrag>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

namespace qdesigner_internal {

ActionRepository::ActionRepository(QWidget *parent)
    : QListWidget(parent)
{
    setViewMode(IconMode);
    setMovement(Static);
    setResizeMode(Adjust);
    setIconSize(QSize(24, 24));
    setSpacing(iconSize().width() / 3);
    setTextElideMode(Qt::ElideRight);

    setDragEnabled(true);
    setAcceptDrops(false);
}

ActionRepository::~ActionRepository()
{
}

void ActionRepository::startDrag(Qt::DropActions supportedActions)
{
    if (!selectionModel())
        return;

    QModelIndexList indexes = selectionModel()->selectedIndexes();

    if (indexes.count() > 0) {
        QDrag *drag = new QDrag(this);
        QIcon icon = qvariant_cast<QIcon>(model()->data(indexes.front(), Qt::DecorationRole));
        drag->setPixmap(icon.pixmap(QSize(22, 22)));
        drag->setMimeData(model()->mimeData(indexes));
        drag->start(supportedActions);
    }
}

QMimeData *ActionRepository::mimeData(const QList<QListWidgetItem*> items) const
{
    ActionRepositoryMimeData *data = new ActionRepositoryMimeData();
    foreach (QListWidgetItem *item, items) {
        QAction *action = qvariant_cast<QAction*>(item->data(ActionRole));
        data->items.append(action);
    }
    return data;
}

void ActionRepository::filter(const QString &text)
{
    QSet<QListWidgetItem*> visibleItems = QSet<QListWidgetItem*>::fromList(findItems(text, Qt::MatchContains));
    for (int index=0; index<count(); ++index) {
        QListWidgetItem *i = item(index);
        setItemHidden(i, !visibleItems.contains(i));
    }
}


} // namespace qdesigner_internal
