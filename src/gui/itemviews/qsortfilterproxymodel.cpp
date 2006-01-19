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

#include "qsortfilterproxymodel.h"

#ifndef QT_NO_SORTFILTERPROXYMODEL

#include "qitemselectionmodel.h"
#include <qsize.h>
#include <qdebug.h>
#include <qdatetime.h>
#include <private/qabstractitemmodel_p.h>
#include <private/qabstractproxymodel_p.h>

class QSortFilterProxyModelLessThan
{
public:
    inline QSortFilterProxyModelLessThan(int column, const QModelIndex &parent,
                                       const QAbstractItemModel *source,
                                       const QSortFilterProxyModel *proxy)
        : sort_column(column), source_parent(parent), source_model(source), proxy_model(proxy) {}

    inline bool operator()(int r1, int r2) const
    {
        QModelIndex i1 = source_model->index(r1, sort_column, source_parent);
        QModelIndex i2 = source_model->index(r2, sort_column, source_parent);
        return proxy_model->lessThan(i1, i2);
    }

private:
    int sort_column;
    QModelIndex source_parent;
    const QAbstractItemModel *source_model;
    const QSortFilterProxyModel *proxy_model;
};

class QSortFilterProxyModelGreaterThan
{
public:
    inline QSortFilterProxyModelGreaterThan(int column, const QModelIndex &parent,
                                          const QAbstractItemModel *source,
                                          const QSortFilterProxyModel *proxy)
        : sort_column(column), source_parent(parent), source_model(source), proxy_model(proxy) {}

    inline bool operator()(int r1, int r2) const
    {
        QModelIndex i1 = source_model->index(r1, sort_column, source_parent);
        QModelIndex i2 = source_model->index(r2, sort_column, source_parent);
        return proxy_model->lessThan(i2, i1);
    }

private:
    int sort_column;
    QModelIndex source_parent;
    const QAbstractItemModel *source_model;
    const QSortFilterProxyModel *proxy_model;
};


class QSortFilterProxyModelPrivate : public QAbstractProxyModelPrivate
{
    Q_DECLARE_PUBLIC(QSortFilterProxyModel)

public:
    struct Mapping {
        QVector<int> source_rows;
        QVector<int> source_columns;
        QVector<int> proxy_rows;
        QVector<int> proxy_columns;
        QVector<QModelIndex> mapped_children;
    };

    mutable QMap<QModelIndex, Mapping*> source_index_mapping;

    int sort_column;
    Qt::SortOrder sort_order;

    int filter_column;
    QRegExp filter_regexp;

    QMap<QModelIndex, Mapping *>::const_iterator create_mapping(
        const QModelIndex &source_parent) const;
    QModelIndex proxy_to_source(const QModelIndex &proxyIndex) const;
    QModelIndex source_to_proxy(const QModelIndex &sourceIndex) const;

    void remove_from_mapping(const QModelIndex &source_parent);

    inline QMap<QModelIndex, Mapping *>::const_iterator index_to_iterator(
        const QModelIndex &proxy_index) const
    {
        Q_ASSERT(proxy_index.isValid());
        const void *p = proxy_index.internalPointer();
        Q_ASSERT(p);
        QMap<QModelIndex, Mapping *>::const_iterator it =
            reinterpret_cast<QMap<QModelIndex, Mapping *>::const_iterator & >(p);
        Q_ASSERT(it != source_index_mapping.end());
        return it;
    }

    inline QModelIndex create_index(int row, int column,
                                    QMap<QModelIndex, Mapping*>::const_iterator it) const
    {
        const void *p = static_cast<const void *>(it);
        return q_func()->createIndex(row, column, const_cast<void *>(p));
    }

    void sourceDataChanged(const QModelIndex &source_top_left,
                           const QModelIndex &source_bottom_right);
    void sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end);
    void sourceLayoutChanged();
    void clear();
};

typedef QMap<QModelIndex, QSortFilterProxyModelPrivate::Mapping *> IndexMap;

void QSortFilterProxyModelPrivate::remove_from_mapping(const QModelIndex &source_parent)
{
    if (Mapping *m = source_index_mapping.take(source_parent)) {
        for (int i = 0; i < m->mapped_children.size(); ++i)
            remove_from_mapping(m->mapped_children.at(i));
        delete m;
    }
}

void QSortFilterProxyModelPrivate::clear()
{
    // store the persistent indexes
    QModelIndexList source_indexes;
    int persistent_count = persistent.indexes.count();
    for (int i = 0; i < persistent_count; ++i) {
        QModelIndex proxy_index = persistent.indexes.at(i)->index;
        QModelIndex source_index = proxy_to_source(proxy_index);
        source_indexes.append(source_index);
    }

    qDeleteAll(source_index_mapping);
    source_index_mapping.clear();

    // update the persistent indexes
    for (int i = 0; i < persistent_count; ++i) {
        QModelIndex source_index = source_indexes.at(i);
        create_mapping(source_index.parent());
        QModelIndex proxy_index = source_to_proxy(source_index);
        persistent.indexes[i]->index = proxy_index;
    }
}

IndexMap::const_iterator QSortFilterProxyModelPrivate::create_mapping(
    const QModelIndex &source_parent) const
{
    Q_Q(const QSortFilterProxyModel);

    IndexMap::const_iterator it = source_index_mapping.find(source_parent);
    if (it != source_index_mapping.end()) // was mapped already
        return it;

    Mapping *m = new Mapping;

    int source_rows = model->rowCount(source_parent);
    for (int i = 0; i < source_rows; ++i) {
        if (q->filterAcceptsRow(i, source_parent))
            m->source_rows.append(i);
    }

    int source_cols = model->columnCount(source_parent);
    for (int i = 0; i < source_cols; ++i) {
        if (q->filterAcceptsColumn(i, source_parent))
            m->source_columns.append(i);
    }

    if (sort_column >= 0) { // only sorts rows
        if (sort_order == Qt::AscendingOrder) {
            QSortFilterProxyModelLessThan lt(sort_column, source_parent, model, q);
            qStableSort(m->source_rows.begin(), m->source_rows.end(), lt);
        } else {
            QSortFilterProxyModelGreaterThan gt(sort_column, source_parent, model, q);
            qStableSort(m->source_rows.begin(), m->source_rows.end(), gt);
        }
    }

    m->proxy_rows.fill(-1, source_rows);
    for (int i = 0; i < m->source_rows.size(); ++i)
        m->proxy_rows[m->source_rows.at(i)] = i;

    m->proxy_columns.fill(-1, source_cols);
    for (int i = 0; i < m->source_columns.size(); ++i)
        m->proxy_columns[m->source_columns.at(i)] = i;

    it = source_index_mapping.insert(source_parent, m);

    if (source_parent.isValid()) {
        QModelIndex source_grand_parent = source_parent.parent();
        IndexMap::const_iterator it2 = create_mapping(source_grand_parent);
        Q_ASSERT(it2 != source_index_mapping.end());
        it2.value()->mapped_children.append(source_parent);
    }

    return it;
}

QModelIndex QSortFilterProxyModelPrivate::proxy_to_source(const QModelIndex &proxy_index) const
{
    if (!proxy_index.isValid())
        return QModelIndex(); // for now; we may want to be able to set a root index later
    IndexMap::const_iterator it = index_to_iterator(proxy_index);
    Q_ASSERT(it != source_index_mapping.end());
    Mapping *m = it.value();
    Q_ASSERT(m);
    if (m->source_rows.isEmpty() || m->source_columns.isEmpty())
        return QModelIndex();
    int source_row = m->source_rows.at(proxy_index.row());
    int source_col = m->source_columns.at(proxy_index.column());
    return model->index(source_row, source_col, it.key());
}

QModelIndex QSortFilterProxyModelPrivate::source_to_proxy(const QModelIndex &source_index) const
{
    if (!source_index.isValid())
        return QModelIndex(); // for now; we may want to be able to set a root index later
    QModelIndex source_parent = source_index.parent();
    IndexMap::const_iterator it = create_mapping(source_parent);
    Q_ASSERT(it != source_index_mapping.end());
    Mapping *m = it.value();
    Q_ASSERT(m);
    if (m->proxy_rows.isEmpty() || m->proxy_columns.isEmpty())
        return QModelIndex();
    int proxy_row = m->proxy_rows.at(source_index.row());
    int proxy_column = m->proxy_columns.at(source_index.column());
    return create_index(proxy_row, proxy_column, it);
}

void QSortFilterProxyModelPrivate::sourceDataChanged(const QModelIndex &source_top_left,
                                                     const QModelIndex &source_bottom_right)
{
    Q_Q(QSortFilterProxyModel);
    QModelIndex proxy_top_left = source_to_proxy(source_top_left);
    QModelIndex proxy_bottom_right = source_to_proxy(source_bottom_right);
    emit q->dataChanged(proxy_top_left, proxy_bottom_right);
}

void QSortFilterProxyModelPrivate::sourceHeaderDataChanged(Qt::Orientation orientation,
                                                           int start, int end)
{
    Q_Q(QSortFilterProxyModel);
    Mapping *m = create_mapping(QModelIndex()).value();
    int proxy_start = (orientation == Qt::Vertical
                       ? m->proxy_rows.at(start)
                       : m->proxy_columns.at(start));
    int proxy_end = (orientation == Qt::Vertical
                     ? m->proxy_rows.at(end)
                     : m->proxy_columns.at(end));
    emit q->headerDataChanged(orientation, proxy_start, proxy_end);
}

void QSortFilterProxyModelPrivate::sourceLayoutChanged()
{
    Q_Q(QSortFilterProxyModel);
    // All internal structures are deleted in clear()
    q->reset();
}

/*!
  \since 4.1
  \class QSortFilterProxyModel
  \brief The QSortFilterProxyModel class provides support for sorting and filtering data passed
  between another model and a view.
  \ingroup model-view

  The sorting filter model transform the structure of a source model by mapping the model indexes
  it supplies to new indexes, corresponding to different locations, for views to use.
  This approach allows a given source model to be restructured as far as views are concerned
  without requiring any transformations on the underlying data.

  The default implementation of the filter and sorting functions use the data for the items
  Qt::DisplayRole compare or accept items.

  The default implementation of the lessThan() function used when sorting, can handle the
  following data types:
  \list
  \o QVariant::Int
  \o QVariant::UInt
  \o QVariant::LongLong
  \o QVariant::ULongLong
  \o QVariant::Double
  \o QVariant::Char
  \o QVariant::Date
  \o QVariant::Time
  \o QVariant::DateTime
  \o QVariant::String
  \endlist

  \sa QAbstractProxyModel, QAbstractItemModel, {Model/View Programming}
*/

/*!
    Constructs a sorting filter model with the given \a parent.
*/

QSortFilterProxyModel::QSortFilterProxyModel(QObject *parent)
    : QAbstractProxyModel(*new QSortFilterProxyModelPrivate, parent)
{
    Q_D(QSortFilterProxyModel);
    d->sort_column = -1;
    d->sort_order = Qt::AscendingOrder;
    d->filter_column = 0;
    connect(this, SIGNAL(modelReset()), this, SLOT(clear()));
}

/*!
    Destroys the sorting filter model.
*/
QSortFilterProxyModel::~QSortFilterProxyModel()
{
    Q_D(QSortFilterProxyModel);
    qDeleteAll(d->source_index_mapping);
    d->source_index_mapping.clear();
}

/*!
  \reimp
*/
void QSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    Q_D(QSortFilterProxyModel);

    if (d->model && d->model != &d->empty) {
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        disconnect(d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                   this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceLayoutChanged()));

        disconnect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                   this, SLOT(sourceLayoutChanged()));

        disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceLayoutChanged()));

        disconnect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   this, SLOT(sourceLayoutChanged()));

        disconnect(d->model, SIGNAL(modelReset()), this, SLOT(sourceLayoutChanged()));
        disconnect(d->model, SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        connect(d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceLayoutChanged()));

        connect(d->model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                this, SLOT(sourceLayoutChanged()));

        connect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceLayoutChanged()));

        connect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceLayoutChanged()));

        connect(d->model, SIGNAL(modelReset()), this, SLOT(sourceLayoutChanged()));
        connect(d->model, SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()));
    }

    d->clear();
}

/*!
    \reimp
*/
QModelIndex QSortFilterProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const QSortFilterProxyModel);
    if (row < 0 || column < 0)
        return QModelIndex();

    QModelIndex source_parent = d->proxy_to_source(parent); // parent is already mapped at this point
    IndexMap::const_iterator it = d->create_mapping(source_parent); // but make sure that the children are mapped
    if (it.value()->source_rows.count() <= row || it.value()->source_columns.count() <= column)
        return QModelIndex();

    return d->create_index(row, column, it);
}

/*!
  \reimp
*/
QModelIndex QSortFilterProxyModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    Q_D(const QSortFilterProxyModel);
    IndexMap::const_iterator it = d->index_to_iterator(child);
    Q_ASSERT(it != d->source_index_mapping.end());
    QModelIndex source_parent = it.key();
    QModelIndex proxy_parent = d->source_to_proxy(source_parent);
    return proxy_parent;
}

/*!
  \reimp
*/
int QSortFilterProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_parent = d->proxy_to_source(parent);
    IndexMap::const_iterator it = d->create_mapping(source_parent);
    return it.value()->source_rows.count();
}

/*!
  \reimp
*/
int QSortFilterProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_parent = d->proxy_to_source(parent);
    IndexMap::const_iterator it = d->create_mapping(source_parent);
    return it.value()->source_columns.count();
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_parent = d->proxy_to_source(parent);
    if (!d->model->hasChildren(source_parent))
        return false;
    QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(source_parent).value();
    return m->source_rows.count() != 0 && m->source_columns.count() != 0;
}

/*!
  \reimp
*/
QVariant QSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_index = d->proxy_to_source(index);
    return d->model->data(source_index, role);
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(QSortFilterProxyModel);
    QModelIndex source_index = d->proxy_to_source(index);
    return d->model->setData(source_index, value, role);
}

/*!
  \reimp
*/
QVariant QSortFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const QSortFilterProxyModel);
    IndexMap::const_iterator it = d->create_mapping(QModelIndex());
    int source_section = (orientation == Qt::Vertical
                          ? it.value()->source_rows.at(section)
                          : it.value()->source_columns.at(section));
    return d->model->headerData(source_section, orientation, role);
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::setHeaderData(int section, Qt::Orientation orientation,
                                          const QVariant &value, int role)
{
    Q_D(QSortFilterProxyModel);
    IndexMap::const_iterator it = d->create_mapping(QModelIndex());
    int source_section = (orientation == Qt::Vertical
                          ? it.value()->source_rows.at(section)
                          : it.value()->source_columns.at(section));
    return d->model->setHeaderData(source_section, orientation, value, role);
}

/*!
  \reimp
*/
QMimeData *QSortFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndexList source_indexes;
    for (int i = 0; i < indexes.count(); ++i)
        source_indexes << d->proxy_to_source(indexes.at(i));
    return d->model->mimeData(source_indexes);
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                         int row, int column, const QModelIndex &parent)
{
    Q_D(QSortFilterProxyModel);
    QModelIndex proxy_index = index(row, column, parent);
    QModelIndex source_index = d->proxy_to_source(proxy_index);
    return d->model->dropMimeData(data, action, source_index.row(), source_index.column(),
                                  source_index.parent());
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_D(QSortFilterProxyModel);
    if (row < 0)
        return false;
    QModelIndex source_parent = d->proxy_to_source(parent);
    QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(source_parent).value();
    if (row > m->source_rows.count())
        return false;
    int source_row = (row >= m->source_rows.count()
                      ? m->source_rows.count()
                      : m->source_rows.at(row));
    d->remove_from_mapping(source_parent);
    return d->model->insertRows(source_row, count, source_parent);
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(QSortFilterProxyModel);
    if (column < 0)
        return false;
    QModelIndex source_parent = d->proxy_to_source(parent);
    QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(source_parent).value();
    if (column > m->source_columns.count())
        return false;
    int source_column = (column >= m->source_columns.count()
                         ? m->source_columns.count()
                         : m->source_columns.at(column));
    d->remove_from_mapping(source_parent);
    return d->model->insertColumns(source_column, count, source_parent);
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_D(QSortFilterProxyModel);
    if (row < 0)
        return false;
    QModelIndex source_parent = d->proxy_to_source(parent);
    QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(source_parent).value();
    if (row >= m->source_rows.count())
        return false;
    int source_row = (row >= m->source_rows.count()
                      ? m->source_rows.at(m->source_rows.count()) + 1
                      : m->source_rows.at(row));
    d->remove_from_mapping(source_parent);
    return d->model->removeRows(source_row, count, source_parent);
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(QSortFilterProxyModel);
    if (column < 0)
        return false;
    QModelIndex source_parent = d->proxy_to_source(parent);
    QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(source_parent).value();
    if (column >= m->source_columns.count())
        return false;
    int source_column = (column >= m->source_columns.count()
                         ? m->source_columns.at(m->source_columns.count()) + 1
                         : m->source_columns.at(column));
    d->remove_from_mapping(source_parent);
    return d->model->removeColumns(source_column, count, source_parent);
}

/*!
  \reimp
*/
void QSortFilterProxyModel::fetchMore(const QModelIndex &parent)
{
    Q_D(QSortFilterProxyModel);
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = d->proxy_to_source(parent);
    d->model->fetchMore(source_parent);
}

/*!
  \reimp
*/
bool QSortFilterProxyModel::canFetchMore(const QModelIndex &parent) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_parent;
    if (parent.isValid())
        source_parent = d->proxy_to_source(parent);
    return d->model->canFetchMore(source_parent);
}

/*!
  \reimp
*/
Qt::ItemFlags QSortFilterProxyModel::flags(const QModelIndex &index) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_index;
    if (index.isValid())
        source_index = d->proxy_to_source(index);
    return d->model->flags(source_index);
}

/*!
  \reimp
*/
QModelIndex QSortFilterProxyModel::buddy(const QModelIndex &index) const
{
    Q_D(const QSortFilterProxyModel);
    if (!index.isValid())
        return QModelIndex();
    QModelIndex source_index = d->proxy_to_source(index);
    QModelIndex source_buddy = d->model->buddy(source_index);
    if (source_index == source_buddy)
        return index;
    return d->source_to_proxy(source_buddy);
}

/*!
  \reimp
*/
QModelIndexList QSortFilterProxyModel::match(const QModelIndex &start, int role,
                                             const QVariant &value, int hits,
                                             Qt::MatchFlags flags) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_start = d->proxy_to_source(start);
    QModelIndexList result = d->model->match(source_start, role, value, hits, flags);
    for (int i = 0; i < result.count(); ++i)
        result[i] = d->source_to_proxy(result.at(i));
    return result;
}

/*!
  \reimp
*/
QSize QSortFilterProxyModel::span(const QModelIndex &index) const
{
    Q_D(const QSortFilterProxyModel);
    QModelIndex source_index = d->proxy_to_source(index);
    return d->model->span(source_index);
}

/*!
  \reimp
*/
void QSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_D(QSortFilterProxyModel);
    d->sort_column = column;
    d->sort_order = order;
    clear();
}

/*!
    \property QSortFilterProxyModel::filterRegExp
    \brief the QRegExp used to filter the contents of the source model

    Setting this property overwrites the current \l caseSensitivity.

    \sa setCaseSensitivity(), setFilterWildcard(), setFilterFixedString()
*/
QRegExp QSortFilterProxyModel::filterRegExp() const
{
    Q_D(const QSortFilterProxyModel);
    return d->filter_regexp;
}

void QSortFilterProxyModel::setFilterRegExp(const QRegExp &regExp)
{
    Q_D(QSortFilterProxyModel);
    d->filter_regexp = regExp;
    clear();
}

/*!
  \property QSortFilterProxyModel::filterKeyColumn
  \brief the column where the key used to filter the contents
  of the source model is read from.
*/
int QSortFilterProxyModel::filterKeyColumn() const
{
    Q_D(const QSortFilterProxyModel);
    return d->filter_column;
}

void QSortFilterProxyModel::setFilterKeyColumn(int column)
{
    Q_D(QSortFilterProxyModel);
    Q_ASSERT(d->model == &d->empty || column < d->model->columnCount());
    d->filter_column = column;
    clear();
}

/*!
    Returns the case sensitivity of the QRegExp pattern used to filter the
    contents of the source model. By default, the filter is case sensistive.
*/
Qt::CaseSensitivity QSortFilterProxyModel::filterCaseSensitivity() const
{
    Q_D(const QSortFilterProxyModel);
    return d->filter_regexp.caseSensitivity();
}

/*!
    Sets the case sensitivity of the QRegExp pattern used to filter the contents
    of the source model to \a cs. By default, the filter is case sensitive.

    \sa setFilterRegExp(), setFilterWildcard(), setFilterFixedString()
*/
void QSortFilterProxyModel::setFilterCaseSensitivity(Qt::CaseSensitivity cs)
{
    Q_D(QSortFilterProxyModel);
    if (cs == d->filter_regexp.caseSensitivity())
        return;
    d->filter_regexp.setCaseSensitivity(cs);
    clear();
}

/*!
    \overload

    Sets the regular expression used to filter the contents
    of the source model to \a pattern.

    \sa setFilterCaseSensitivity(), setFilterWildcard(), setFilterFixedString()
*/
void QSortFilterProxyModel::setFilterRegExp(const QString &pattern)
{
    Q_D(QSortFilterProxyModel);
    d->filter_regexp.setPatternSyntax(QRegExp::RegExp);
    d->filter_regexp.setPattern(pattern);
    clear();
}

/*!
    Sets the wildcard expression used to filter the contents
    of the source model to \a pattern.

    \sa setFilterCaseSensitivity(), setFilterRegExp(), setFilterFixedString()
*/
void QSortFilterProxyModel::setFilterWildcard(const QString &pattern)
{
    Q_D(QSortFilterProxyModel);
    d->filter_regexp.setPatternSyntax(QRegExp::Wildcard);
    d->filter_regexp.setPattern(pattern);
    clear();
}

/*!
    Sets the fixed string used to filter the contents
    of the source model to \a pattern.

    \sa setFilterCaseSensitivity(), setFilterRegExp(), setFilterWildcard()
*/
void QSortFilterProxyModel::setFilterFixedString(const QString &pattern)
{
    Q_D(QSortFilterProxyModel);
    d->filter_regexp.setPatternSyntax(QRegExp::FixedString);
    d->filter_regexp.setPattern(pattern);
    clear();
}

/*!
  Clears the sorting filter model, removing all mapping.
*/
void QSortFilterProxyModel::clear()
{
    Q_D(QSortFilterProxyModel);
    d->clear();
    emit layoutChanged();
}

/*!
  Returns true if the value of the item referred to by the given index \a left
  is less than the value of the item referred to by the given index \a right,
  otherwise returns false.
  This function is used as the < operator when sorting, and handles several
  QVariant types:

  \list
  \o QVariant::Int
  \o QVariant::UInt
  \o QVariant::LongLong
  \o QVariant::ULongLong
  \o QVariant::Double
  \o QVariant::Char
  \o QVariant::Date
  \o QVariant::Time
  \o QVariant::DateTime
  \o QVariant::String
  \endlist
*/
bool QSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant l = (left.model() ? left.model()->data(left, Qt::DisplayRole) : QVariant());
    QVariant r = (right.model() ? right.model()->data(right, Qt::DisplayRole) : QVariant());
    switch (l.type()) {
    case QVariant::Int:
        return l.toInt() < r.toInt();
    case QVariant::UInt:
        return l.toUInt() < r.toUInt();
    case QVariant::LongLong:
        return l.toLongLong() < r.toLongLong();
    case QVariant::ULongLong:
        return l.toULongLong() < r.toULongLong();
    case QVariant::Double:
        return l.toDouble() < r.toDouble();
    case QVariant::Char:
        return l.toChar() < r.toChar();
    case QVariant::Date:
        return l.toDate() < r.toDate();
    case QVariant::Time:
            return l.toTime() < r.toTime();
    case QVariant::DateTime:
        return l.toDateTime() < r.toDateTime();
    case QVariant::String:
    default:
        return l.toString() < r.toString();
    }
    return false;
}

/*!
  Returns true if the value in the item in the row indicated by
  the given \a source_row and \a source_parent should be included in the model.
  The default implementation uses filterRegExp with the data returned for the Qt::DisplayRole
  to determine if the row should be accepted or not.
*/
bool QSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_D(const QSortFilterProxyModel);
    if (d->filter_regexp.isEmpty() || d->filter_column == -1)
        return true;
    QModelIndex source_index = d->model->index(source_row, d->filter_column, source_parent);
    Q_ASSERT(source_index.isValid());
    QString key = d->model->data(source_index, Qt::DisplayRole).toString();
    return key.contains(d->filter_regexp);
}

/*!
  Returns true if the value in the item in the column indicated by
  the given \a source_column and \a source_parent should be included in the model.
  The default implementation returns true.
*/
bool QSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_column);
    Q_UNUSED(source_parent);
    return true;
}

/*!
  Returns the source model index  corresponding to the
  given \a proxyIndex from the sorting filter  model.
*/
QModelIndex QSortFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    Q_D(const QSortFilterProxyModel);
    return d->proxy_to_source(proxyIndex);
}

/*!
  Returns the model index in the QSortFilterProxyModel given
  the \a sourceIndex from the source model.
*/
QModelIndex QSortFilterProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    Q_D(const QSortFilterProxyModel);
    return d->source_to_proxy(sourceIndex);
}

/*!
  \reimp
*/
QItemSelection QSortFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    return QAbstractProxyModel::mapSelectionToSource(proxySelection);
}

/*!
  \reimp
*/
QItemSelection QSortFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    return QAbstractProxyModel::mapSelectionFromSource(sourceSelection);
}

#include "moc_qsortfilterproxymodel.cpp"

#endif // QT_NO_SORTFILTERPROXYMODEL
