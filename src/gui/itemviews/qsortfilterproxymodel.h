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

#ifndef QSORTFILTERPROXYMODEL_H
#define QSORTFILTERPROXYMODEL_H

#include <QtGui/qabstractproxymodel.h>

#ifndef QT_NO_SORTFILTERPROXYMODEL

#include <QtCore/qregexp.h>

QT_MODULE(Gui)

class QSortFilterProxyModelPrivate;
class QSortFilterProxyModelLessThan;
class QSortFilterProxyModelGreaterThan;

class Q_GUI_EXPORT QSortFilterProxyModel : public QAbstractProxyModel
{
    friend class QSortFilterProxyModelLessThan;
    friend class QSortFilterProxyModelGreaterThan;

    Q_OBJECT
    Q_PROPERTY(QRegExp filterRegExp READ filterRegExp WRITE setFilterRegExp)
    Q_PROPERTY(int filterKeyColumn READ filterKeyColumn WRITE setFilterKeyColumn)
    Q_PROPERTY(Qt::CaseSensitivity filterCaseSensitivity READ filterCaseSensitivity WRITE setFilterCaseSensitivity)
    Q_PROPERTY(bool dynamicSortFilter READ dynamicSortFilter WRITE setDynamicSortFilter)

public:
    QSortFilterProxyModel(QObject *parent = 0);
    ~QSortFilterProxyModel();

    void setSourceModel(QAbstractItemModel *sourceModel);

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    QItemSelection mapSelectionToSource(const QItemSelection &proxySelection) const;
    QItemSelection mapSelectionFromSource(const QItemSelection &sourceSelection) const;

    QRegExp filterRegExp() const;
    void setFilterRegExp(const QRegExp &regExp);

    int filterKeyColumn() const;
    void setFilterKeyColumn(int column);

    Qt::CaseSensitivity filterCaseSensitivity() const;
    void setFilterCaseSensitivity(Qt::CaseSensitivity cs);

    bool dynamicSortFilter() const;
    void setDynamicSortFilter(bool enable);

public Q_SLOTS:
    void setFilterRegExp(const QString &pattern);
    void setFilterWildcard(const QString &pattern);
    void setFilterFixedString(const QString &pattern);
    void clear();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

public:
#ifdef Q_NO_USING_KEYWORD
    inline QObject *parent() const { return QObject::parent(); }
#else
    using QObject::parent;
#endif

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

    void fetchMore(const QModelIndex &parent);
    bool canFetchMore(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex buddy(const QModelIndex &index) const;
    QModelIndexList match(const QModelIndex &start, int role,
                          const QVariant &value, int hits,
                          Qt::MatchFlags flags) const;
    QSize span(const QModelIndex &index) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
    Q_DECLARE_PRIVATE(QSortFilterProxyModel)
    Q_DISABLE_COPY(QSortFilterProxyModel)

    Q_PRIVATE_SLOT(d_func(), void sourceDataChanged(const QModelIndex &source_top_left, const QModelIndex &source_bottom_right))
    Q_PRIVATE_SLOT(d_func(), void sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceReset())
    Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeInserted(const QModelIndex &source_parent, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsInserted(const QModelIndex &source_parent, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeRemoved(const QModelIndex &source_parent, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsRemoved(const QModelIndex &source_parent, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsAboutToBeInserted(const QModelIndex &source_parent, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsInserted(const QModelIndex &source_parent, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsAboutToBeRemoved(const QModelIndex &source_parent, int start, int end))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsRemoved(const QModelIndex &source_parent, int start, int end))
};

#endif // QT_NO_SORTFILTERPROXYMODEL
#endif // QSORTFILTERPROXYMODEL_H
