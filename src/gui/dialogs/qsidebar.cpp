/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsidebar_p.h"
#include "qfilesystemmodel_p.h"

#ifndef QT_NO_FILEDIALOG

#include <qaction.h>
#include <qurl.h>
#include <qmenu.h>
#include <qmimedata.h>
#include <qevent.h>
#include <qdebug.h>
#if QT_VERSION >= 0x040300
#include <qfileiconprovider.h>
#else
#include <qdirmodel.h>
#endif

QSidebar::QSidebar(QFileSystemModel *model, const QList<QUrl> &newUrls, QWidget *parent) : QListWidget(parent), fileSystemModel(model)
{
    // ### TODO make icon size dynamic
    setIconSize(QSize(24,24));
    setUniformItemSizes(true);
    connect(fileSystemModel, SIGNAL(layoutChanged()),
        this, SLOT(layoutChanged()), Qt::QueuedConnection);
    init();
    setUrls(newUrls);
    setCurrentIndex(this->model()->index(0,0));
}

QSidebar::~QSidebar()
{
}

/*!
    \reimp
*/
QStringList QSidebar::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

/*!
    \reimp
*/
QMimeData *QSidebar::mimeData ( const QList<QListWidgetItem *> items ) const
{
    QList<QUrl> list;
    for (int i = 0; i < items.count(); ++i) {
        if (indexFromItem(items.at(i)).column() == 0)
           list.append(indexFromItem(items.at(i)).data(UrlRole).toUrl());
    }
    QMimeData *data = new QMimeData();
    data->setUrls(list);
    return data;
}

/*!
    \ Don't allow drops from files.
*/
void QSidebar::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->formats().contains(mimeTypes().first()))
        return;

    const QList<QUrl> list = event->mimeData()->urls();
    for (int i = 0; i < list.count(); ++i) {
        QModelIndex idx = fileSystemModel->index(list.at(0).toLocalFile());
        if (!fileSystemModel->isDir(idx))
            return;
    }

    QListWidget::dragEnterEvent(event);
}

/*!
    \reimp
*/
bool QSidebar::dropMimeData(int row, const QMimeData *data, Qt::DropAction action)
{
    Q_UNUSED(action);
    if (!data->formats().contains(mimeTypes().first()))
        return false;
    addUrls(data->urls(), row);
    return true;
}

QSize QSidebar::sizeHint() const
{
    return QListView::sizeHintForIndex(model()->index(0, 0));
}

QList<QUrl> QSidebar::urls() const
{
    QList<QUrl> list;
    for (int i = 0; i < model()->rowCount(); ++i)
        list.append(model()->data(model()->index(i, 0), UrlRole).toUrl());
    return list;
}

void QSidebar::setUrls(const QList<QUrl> &list)
{
    model()->removeRows(0, model()->rowCount());
    addUrls(list, 0);
}

/*!
    Add urls \a list into the list at \a row

    Copied in QFileDialog
    \sa dropMimeData() addFullPath()
*/
void QSidebar::addUrls(const QList<QUrl> &list, int row)
{
    row = qMin(row, model()->rowCount());
    for (int i = list.count() - 1; i >= 0; --i) {
        QUrl url = list.at(i);
        if (!url.isValid() || url.scheme() != QLatin1String("file"))
            continue;
        for (int j = 0; j < model()->rowCount(); ++j) {
            if (model()->index(j, 0).data(UrlRole) == url) {
                model()->removeRow(j);
                if (j <= row)
                    row--;
                break;
            }
        }
        row = qMax(row, 0);
        QModelIndex idx = fileSystemModel->index(url.toLocalFile());
        if (!fileSystemModel->isDir(idx))
            continue;
        model()->insertRows(row, 1);
        setUrl(model()->index(row, 0), url);
        watching.append(url.toLocalFile());
    }
}

// Copied in QFileDialog
void QSidebar::setUrl(const QModelIndex &index, const QUrl &url)
{
    QModelIndex dirIndex = fileSystemModel->index(url.toLocalFile());
    model()->setData(index, url, UrlRole);
    if (url.path().isEmpty()) {
        model()->setData(index, fileSystemModel->myComputer());
        model()->setData(index, fileSystemModel->myComputer(Qt::DecorationRole), Qt::DecorationRole);
    } else {
        QString newName = dirIndex.data().toString();
        QIcon newIcon = qvariant_cast<QIcon>(dirIndex.data(Qt::DecorationRole));
        if (!dirIndex.isValid()) {
            newIcon = fileSystemModel->iconProvider()->icon(QFileIconProvider::Folder);
            newName = QFileInfo(url.toLocalFile()).fileName();
        }

        if (index.data().toString() != newName)
            model()->setData(index, newName);
        QIcon oldIcon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        if (oldIcon.serialNumber() != newIcon.serialNumber())
            model()->setData(index, newIcon, Qt::DecorationRole);
    }
}

void QSidebar::selectUrl(const QUrl &url)
{
    disconnect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
               this, SLOT(clicked(const QModelIndex &)));

    selectionModel()->clear();
    for (int i = 0; i < model()->rowCount(); ++i) {
        if (model()->index(i, 0).data(QSidebar::UrlRole).toUrl() == url) {
            selectionModel()->select(model()->index(i, 0), QItemSelectionModel::Select);
            break;
        }
    }

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(clicked(const QModelIndex &)));
}

void QSidebar::init()
{
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(clicked(const QModelIndex &)));
    setDragDropMode(QAbstractItemView::DragDrop);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

/*!
    \internal

    \sa removeEntry()
*/
void QSidebar::showContextMenu(const QPoint &position)
{
    QList<QAction *> actions;
    if (indexAt(position).isValid()) {
        QAction *action = new QAction(tr("Remove"), this);
        connect(action, SIGNAL(triggered()), this, SLOT(removeEntry()));
        actions.append(action);
    }
    if (actions.count() > 0)
        QMenu::exec(actions, mapToGlobal(position));
}

/*!
    \internal

    \sa showContextMenu()
*/
void QSidebar::removeEntry()
{
    QList<QModelIndex> idxs = selectionModel()->selectedIndexes();
    QList<QPersistentModelIndex> indexes;
    for (int i = 0; i < idxs.count(); i++)
        indexes.append(idxs.at(i));

    for (int i = 0; i < indexes.count(); ++i)
        model()->removeRow(indexes.at(i).row());
}

/*!
    \internal

    \sa goToUrl()
*/
void QSidebar::clicked(const QModelIndex &index)
{
    QUrl url = model()->index(index.row(), 0).data(QSidebar::UrlRole).toUrl();
    emit goToUrl(url);
    selectUrl(url);
}

/*!
    \reimp
    Don't automatically select something
 */
void QSidebar::focusInEvent(QFocusEvent *event)
{
    QAbstractScrollArea::focusInEvent(event);
    viewport()->update();
}

/*!
    Re-get all of our data, anything could have changed!

    Copied in QFileDialog
 */
void QSidebar::layoutChanged()
{
    QStringList paths;
    for (int i = 0; i < watching.count(); ++i)
        paths.append(watching.at(i));
    watching.clear();
    QMultiHash<QString, QModelIndex> lt;
    for (int i = 0; i < model()->rowCount(); ++i) {
        QModelIndex idx = model()->index(i, 0);
        lt.insert(idx.data(UrlRole).toUrl().toLocalFile(), idx);
    }

    for (int i = 0; i < paths.count(); ++i) {
        QString path = paths.at(i);
        QModelIndex newIndex = fileSystemModel->index(path);
        watching.append(path);
        if (!newIndex.isValid())
            continue;
        QList<QModelIndex> values = lt.values(path);
        for (int i = 0; i < values.size(); ++i) {
            QModelIndex idx = values.at(i);
            setUrl(idx, QUrl::fromLocalFile(path));
        }
    }
}

#endif
