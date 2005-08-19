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

#include "qmappingproxymodel.h"
#include <qsize.h>
#include <qdebug.h>
#include <private/qmappingproxymodel_p.h>

/*!
  \class QMappingProxyModel
  \brief The QMappingProxyModel class provides support for mapping data
  that is passed between another model and a view.

*/

/*!
    Constructs a sorting proxy model with the given \a parent.
*/

QMappingProxyModel::QMappingProxyModel(QObject *parent)
    :QAbstractItemModel(*new QMappingProxyModelPrivate, parent)
{
    Q_D(QMappingProxyModel);
    setModel(&d->empty);
}

/*!
    \internal
*/

QMappingProxyModel::QMappingProxyModel(QMappingProxyModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(QMappingProxyModel);
    setModel(&d->empty);
}

/*!
    Destroys the sorting proxy model.
*/
QMappingProxyModel::~QMappingProxyModel()
{

}

/*!
    Sets the given \a model to be processed by the proxy model.
*/
void QMappingProxyModel::setModel(QAbstractItemModel *model)
{
    Q_D(QMappingProxyModel);
    if (d->model && d->model != &d->empty) {
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        disconnect(d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                   this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

        disconnect(d->model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceRowsInserted()));

        disconnect(d->model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsInserted()));

        disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceRowsRemoved()));

        disconnect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceColumnsRemoved()));

        disconnect(d->model, SIGNAL(modelReset()), this, SLOT(sourceLayoutChanged()));
        disconnect(d->model, SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));
    }

    if (model) {
        d->model = model;
        
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        connect(d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

        connect(d->model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted()));

        connect(d->model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                this, SLOT(sourceColumnsInserted()));

        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved()));

        connect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceColumnsRemoved()));

        connect(d->model, SIGNAL(modelReset()), this, SLOT(sourceLayoutChanged()));
        connect(d->model, SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));

    } else {
        d->model = &d->empty;
    }

    clear();
}

/*!
    Returns the model that contains the data that is available through the
    sorting proxy model.
*/
QAbstractItemModel *QMappingProxyModel::model() const
{
    Q_D(const QMappingProxyModel);
    return d->model;
}

/*!
    \reimp
*/
QModelIndex QMappingProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    void *parent_node = 0;
    if (parent.isValid())
        parent_node = proxy_to_source.find(parent); // ### slow
    QModelIndex proxy_index = createIndex(row, column, parent_node);

    if (!proxy_to_source.contains(proxy_index)) { // not mapped
        QModelIndex source_parent = proxy_to_source.value(parent);
        QModelIndex source_index = model()->index(row, column, source_parent);
        Q_ASSERT(proxy_index.isValid());
        Q_ASSERT(source_index.isValid());
        Q_ASSERT(!proxy_to_source.contains(proxy_index));
        proxy_to_source.insert(proxy_index, source_index);
    }

    return proxy_index;
}

/*!
  \reimp
*/
QModelIndex QMappingProxyModel::parent(const QModelIndex &child) const
{
    if (child.isValid() && child.internalPointer() != 0) { // is valid and not toplevel
#if 1
        QModelIndex source_child = proxy_to_source.value(child);
        QModelIndex source_parent = model()->parent(source_child);
        return proxyIndex(source_parent);
#else
        QMap<QModelIndex,QModelIndex>::iterator it =
            reinterpret_cast<QMap<QModelIndex,QModelIndex>::iterator>(child.internalPointer());
        return it.key(); // the parent index
#endif
    }
    return QModelIndex();
}

/*!
  \reimp
*/
int QMappingProxyModel::rowCount(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    return model()->rowCount(source_parent);
}

/*!
  \reimp
*/
int QMappingProxyModel::columnCount(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    return model()->columnCount(source_parent);
}

/*!
  \reimp
*/
bool QMappingProxyModel::hasChildren(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    return model()->hasChildren(source_parent);
}

/*!
  \reimp
*/
QVariant QMappingProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        QModelIndex source_index = proxy_to_source.value(index);
        Q_ASSERT(source_index.isValid());
        return model()->data(source_index, role);
    }
    return QVariant();
}

/*!
  \reimp
*/
bool QMappingProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        QModelIndex source_index = proxy_to_source.value(index);
        return model()->setData(source_index, value, role);
    }
    return false;
}

/*!
  \reimp
*/
QMimeData *QMappingProxyModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList source_indexes;
    for (int i = 0; i < indexes.count(); ++i)
        source_indexes << proxy_to_source.value(indexes.at(i));
    return model()->mimeData(source_indexes);
}

/*!
  \reimp
*/
bool QMappingProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent)
{
    QModelIndex proxy_index = index(row, column, parent); // will insert the proxy_index in the map
    QModelIndex source_index = proxy_to_source.value(proxy_index);
    return model()->dropMimeData(data, action, source_index.row(), source_index.column(),
                                 source_index.parent());
}

/*!
  \reimp
*/
bool QMappingProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    QModelIndex source_parent = proxy_to_source.value(parent);
    int source_row;
    int source_row_count = model()->rowCount(source_parent);
    if (row >= source_row_count) {
        source_row = source_row_count;
    } else {
        QModelIndex proxy_index = index(row, 0, parent); // will insert the proxy_index in the map
        QModelIndex source_index = proxy_to_source.value(proxy_index);
        source_row = source_index.row();
    }
    return model()->insertRows(source_row, count, source_parent);
}

/*!
  \reimp
*/
bool QMappingProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    QModelIndex source_parent = proxy_to_source.value(parent);
    int source_column;
    int source_column_count = model()->columnCount(source_parent);
    if (column >= source_column_count) {
        source_column = source_column_count;
    } else {
        QModelIndex proxy_index = index(0, column, parent); // will insert the proxy_index in the map
        QModelIndex source_index = proxy_to_source.value(proxy_index);
        source_column = source_index.column();
    }
    return model()->insertColumns(source_column, count, source_parent);
}

/*!
  \reimp
*/
bool QMappingProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    QModelIndex proxy_index = index(row, 0, parent); // will insert the proxy_index in the map
    QModelIndex source_index = proxy_to_source.value(proxy_index);
    return model()->removeRows(source_index.row(), count, source_index.parent());
}

/*!
  \reimp
*/
bool QMappingProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    QModelIndex proxy_index = index(0, column, parent); // will insert the proxy_index in the map
    QModelIndex source_index = proxy_to_source.value(proxy_index);
    return model()->removeColumns(source_index.column(), count, source_index.parent());
}

/*!
  \reimp
*/
void QMappingProxyModel::fetchMore(const QModelIndex &parent)
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    model()->fetchMore(source_parent);
}

/*!
  \reimp
*/
bool QMappingProxyModel::canFetchMore(const QModelIndex &parent) const
{
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = proxy_to_source.value(parent);
    return model()->canFetchMore(source_parent);
}

/*!
  \reimp
*/
Qt::ItemFlags QMappingProxyModel::flags(const QModelIndex &index) const
{
    QModelIndex source_index;
    if (index.isValid())
        source_index = proxy_to_source.value(index);
    return model()->flags(source_index);
}

/*!
  \reimp
*/
QModelIndex QMappingProxyModel::buddy(const QModelIndex &index) const
{
    if (index.isValid()) {
        QModelIndex source_index = proxy_to_source.value(index);
        QModelIndex source_buddy = model()->buddy(source_index);
        return proxyIndex(source_buddy);
    }
    return QModelIndex();
}

/*!
  \reimp
*/
QModelIndexList QMappingProxyModel::match(const QModelIndex &start, int role,
                                          const QVariant &value, int hits,
                                          Qt::MatchFlags flags) const
{
    QModelIndex source_start = proxy_to_source.value(start);
    QModelIndexList result = model()->match(source_start, role, value, hits, flags);
    for (int i = 0; i < result.count(); ++i)
        result[i] = proxyIndex(result.at(i));
    return result;
}

/*!
  \reimp
*/
QSize QMappingProxyModel::span(const QModelIndex &index) const
{
    QModelIndex source_index = proxy_to_source.value(index);
    return model()->span(source_index);
}

/*!
  Clears the mapping in the mapping proxy model.
*/
void QMappingProxyModel::clear()
{
    proxy_to_source.clear();
    reset();
}

// protected

// slots called by the source model

/*!
  \internal
*/
void QMappingProxyModel::sourceDataChanged(const QModelIndex &source_top_left,
                                           const QModelIndex &source_bottom_right)
{
    emit dataChanged(proxyIndex(source_top_left), proxyIndex(source_bottom_right));
}

/*!
  \internal
*/
void QMappingProxyModel::sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end)
{
    // FIXME: map rows and columns for headers too
    emit headerDataChanged(orientation, start, end);
}

/*!
  \internal
*/
void QMappingProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &source_parent,
                                                     int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    QModelIndex source_index = model()->index(start, 0, source_parent);
    QModelIndex proxy_index = proxyIndex(source_index);
    int proxy_start = proxy_index.row() > 0 ? proxy_index.row() : start;
    int proxy_end = proxy_start + (end - start);
    beginInsertRows(proxy_parent, proxy_start, proxy_end); // emits signal
}

/*!
  \internal
*/
void QMappingProxyModel::sourceRowsInserted()
{
    sourceLayoutChanged();
    endInsertRows(); // emits signal
}

/*!
  \internal
*/
void QMappingProxyModel::sourceColumnsAboutToBeInserted(const QModelIndex &source_parent,
                                                        int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    beginInsertColumns(proxy_parent, start, end); // emits signal
}

/*!
  \internal
*/
void QMappingProxyModel::sourceColumnsInserted()
{
    endInsertColumns(); // emits signal
}

/*!
  \internal
*/
void QMappingProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &source_parent,
                                                    int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    QModelIndex source_index = model()->index(start, 0, source_parent);
    QModelIndex proxy_index = proxyIndex(source_index);
    int proxy_start = proxy_index.row();
    int proxy_end = proxy_start + (end - start);
    beginRemoveRows(proxy_parent, proxy_start, proxy_end); // emits signal
    proxy_to_source.remove(proxy_index);
}

/*!
  \internal
*/
void QMappingProxyModel::sourceRowsRemoved()
{
    sourceLayoutChanged();
    endRemoveRows(); // emits signal
}

/*!
  \internal
*/
void QMappingProxyModel::sourceColumnsAboutToBeRemoved(const QModelIndex &source_parent,
                                                       int start, int end)
{
    QModelIndex proxy_parent = proxyIndex(source_parent);
    beginRemoveColumns(proxy_parent, start, end); // emits signal
}

/*!
  \internal
*/
void QMappingProxyModel::sourceColumnsRemoved()
{
    endRemoveColumns(); // emits signal
}

/*!
  \internal
*/
void QMappingProxyModel::sourceLayoutChanged()
{
    proxy_to_source.clear();
}

/*!
  Returns the model index in the QMappingProxyModel given
  the \a source_index from the source model.
*/
QModelIndex QMappingProxyModel::proxyIndex(const QModelIndex &source_index) const
{
    if (!source_index.isValid())
        return QModelIndex();

    QModelIndex proxy_index = proxy_to_source.key(source_index); // ### slow
    if (proxy_index.isValid())
        return proxy_index;

    // not found in the map; create the index and insert it into the map
    QModelIndex proxy_parent;
    QModelIndex source_parent = model()->parent(source_index);
    if (source_parent.isValid())
        proxy_parent = proxyIndex(source_parent);
    return index(source_index.row(), source_index.column(), proxy_parent);
}
