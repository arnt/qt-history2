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

#include "qproxymodel.h"
#include <private/qabstractitemmodel_p.h>
#include <qsize.h>

#define d d_func()
#define q q_func()

class QEmptyModel : public QAbstractItemModel
{
public:
    QEmptyModel(QObject *parent = 0) : QAbstractItemModel(parent) {}
    QModelIndex index(int, int, const QModelIndex &) const { return QModelIndex::Null; }
    QModelIndex parent(const QModelIndex &) const { return QModelIndex::Null; }
    int rowCount(const QModelIndex &) const { return 0; }
    int columnCount(const QModelIndex &) const { return 0; }
    QVariant data(const QModelIndex &, int) const { return QVariant(); }
};

class QProxyModelPrivate : private QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QProxyModel)

public:
    QProxyModelPrivate() : QAbstractItemModelPrivate(), model(0) {}
    QAbstractItemModel *model;
    QEmptyModel empty;
};

QProxyModel::QProxyModel(QObject *parent)
    : QAbstractItemModel(*new QProxyModelPrivate, parent)
{
    setModel(&d->empty);
    disconnect(this, SIGNAL(reset()), this, SLOT(resetPersistentIndexes()));
}

QProxyModel::QProxyModel(QProxyModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    setModel(&d->empty);
    disconnect(this, SIGNAL(reset()), this, SLOT(resetPersistentIndexes()));
}

QProxyModel::~QProxyModel()
{
}

void QProxyModel::setModel(QAbstractItemModel *model)
{
    if (d->model && d->model != &d->empty) {
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
        disconnect(d->model, SIGNAL(headerDataChanged(Orientation,int,int)),
                   this, SIGNAL(headerDataChanged(Orientation,int,int)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SIGNAL(rowsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SIGNAL(rowsRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                   this, SIGNAL(columnsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   this, SIGNAL(columnsRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(reset()), this, SIGNAL(reset()));
    }

    if (model) {
        d->model = model;
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(headerDataChanged(Orientation,int,int)),
                this, SIGNAL(headerDataChanged(Orientation,int,int)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SIGNAL(rowsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SIGNAL(rowsRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                this, SIGNAL(columnsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SIGNAL(columnsRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(reset()), this, SIGNAL(reset()));
    } else {
        d->model = &d->empty;
    }
}

QAbstractItemModel *QProxyModel::model() const
{
    return d->model;
}

QModelIndex QProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    return d->model->index(row, column, parent);    
}

QModelIndex QProxyModel::parent(const QModelIndex &child) const
{
    return d->model->parent(child);
}

int QProxyModel::rowCount(const QModelIndex &parent) const
{
    return d->model->rowCount(parent);
}

int QProxyModel::columnCount(const QModelIndex &parent) const
{
    return d->model->columnCount(parent);
}

bool QProxyModel::hasChildren(const QModelIndex &parent) const
{
    return d->model->hasChildren(parent);
}

bool QProxyModel::canDecode(QMimeSource *src) const
{
    return d->model->canDecode(src);
}

bool QProxyModel::decode(QDropEvent *e, const QModelIndex &parent)
{
    return d->model->decode(e, parent);
}

QDragObject *QProxyModel::dragObject(const QModelIndexList &indexes, QWidget *dragSource)
{
    return d->model->dragObject(indexes, dragSource);
}

QVariant QProxyModel::data(const QModelIndex &index, int role) const
{
    return d->model->data(index, role);
}

bool QProxyModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    return d->model->setData(index, role, value);
}

QVariant QProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return d->model->headerData(section, orientation, role);
}

bool QProxyModel::setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value)
{
    return d->model->setHeaderData(section, orientation, role, value);
}

bool QProxyModel::insertRows(int row, const QModelIndex &parent, int count)
{
    return d->model->insertRows(row, parent, count);
}

bool QProxyModel::insertColumns(int column, const QModelIndex &parent, int count)
{
    return d->model->insertColumns(column, parent, count);
}

void QProxyModel::fetchMore(const QModelIndex &parent)
{
    d->model->fetchMore(parent);
}

QAbstractItemModel::ItemFlags QProxyModel::flags(const QModelIndex &index) const
{
    return d->model->flags(index);
}

bool QProxyModel::isSortable() const
{
    return true;
}

void QProxyModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order)
{
    d->model->sort(column, parent, order);
}

bool QProxyModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    return d->model->equal(left, right);
}

bool QProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return d->model->lessThan(left, right);
}

QModelIndexList QProxyModel::match(const QModelIndex &start, int role,
                                   const QVariant &value,
                                   int hits, QAbstractItemModel::MatchFlags flags) const
{
    return d->model->match(start, role, value, hits, flags);
}

QSize QProxyModel::span(const QModelIndex &index) const
{
    return d->model->span(index);
}
