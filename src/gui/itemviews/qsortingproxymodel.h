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

#ifndef QSORTINGPROXYMODEL_H
#define QSORTINGPROXYMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qmap.h>

class QSortingProxyModelPrivate;

class Q_GUI_EXPORT QSortingProxyModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    QSortingProxyModel(QObject *parent = 0);
    ~QSortingProxyModel();

    virtual void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

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

    void sort(int column, Qt::SortOrder order);

    QModelIndex buddy(const QModelIndex &index) const;
    QModelIndexList match(const QModelIndex &start, int role,
                          const QVariant &value, int hits,
                          Qt::MatchFlags flags) const;
    QSize span(const QModelIndex &index) const;

    void clear();

protected:
    QSortingProxyModel(QSortingProxyModelPrivate &, QObject *parent);

    static bool lessThan(const QModelIndex &left, const QModelIndex &right);
    static bool greaterThan(const QModelIndex &left, const QModelIndex &right);

    QModelIndex proxyIndex(const QModelIndex &source_index) const;
    QModelIndex sourceIndex(int row, int column, const QModelIndex &parent) const;

protected slots:
    void sourceDataChanged(const QModelIndex &source_top_left,
                           const QModelIndex &source_bottom_right);

    void sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end);

    void sourceRowsAboutToBeInserted(const QModelIndex &source_parent, int start, int end);
    void sourceRowsInserted();
    
    void sourceColumnsAboutToBeInserted(const QModelIndex &sourceparent, int start, int end);
    void sourceColumnsInserted();
    
    void sourceRowsAboutToBeRemoved(const QModelIndex &source_parent, int start, int end);
    void sourceRowsRemoved();
    
    void sourceColumnsAboutToBeRemoved(const QModelIndex &source_parent, int start, int end);
    void sourceColumnsRemoved();

    void clearAndSort();

    QModelIndex createMappedProxyIndex(const QModelIndex &proxy_parent,
                                       const QModelIndex &source_index) const;

protected:
    mutable QMap<QModelIndex, QModelIndex> proxy_to_source; // mapping

    int sort_column;
    Qt::SortOrder sort_order;

private:
    Q_DECLARE_PRIVATE(QSortingProxyModel)
    Q_DISABLE_COPY(QSortingProxyModel)
};

#endif // QSORTINGPROXYMODEL_H
