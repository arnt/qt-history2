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

#ifndef QMAPPINGPROXYMODEL_H
#define QMAPPINGPROXYMODEL_H

#include <QtGui/qabstractproxymodel.h>

class QMappingProxyModelPrivate;

class Q_GUI_EXPORT QMappingProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    QMappingProxyModel(QObject *parent = 0);
    ~QMappingProxyModel();

    void setSourceModel(QAbstractItemModel *sourceModel);

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

    QModelIndex buddy(const QModelIndex &index) const;
    QModelIndexList match(const QModelIndex &start, int role,
                          const QVariant &value, int hits,
                          Qt::MatchFlags flags) const;
    QSize span(const QModelIndex &index) const;

    virtual void clear();

protected Q_SLOTS:
    virtual void sourceDataChanged(const QModelIndex &source_top_left,
                                   const QModelIndex &source_bottom_right);
    virtual void sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end);
    virtual void sourceRowsAboutToBeInserted(const QModelIndex &source_parent, int start, int end);
    virtual void sourceRowsInserted();    
    virtual void sourceColumnsAboutToBeInserted(const QModelIndex &source_parent,
                                                int start, int end);
    virtual void sourceColumnsInserted();
    virtual void sourceRowsAboutToBeRemoved(const QModelIndex &source_parent, int start, int end);
    virtual void sourceRowsRemoved();
    virtual void sourceColumnsAboutToBeRemoved(const QModelIndex &source_parent,
                                               int start, int end);
    virtual void sourceColumnsRemoved();
    virtual void sourceLayoutChanged();

protected:
    QMappingProxyModel(QMappingProxyModelPrivate &, QObject *parent);

    QModelIndex proxyIndex(const QModelIndex &source_index) const;
    QModelIndex sourceIndex(const QModelIndex &proxy_index) const;

    void insertMapping(const QModelIndex &proxy_index, const QModelIndex &source_index);
    void removeMapping(const QModelIndex &proxy_index);

private:
    Q_DECLARE_PRIVATE(QMappingProxyModel)
    Q_DISABLE_COPY(QMappingProxyModel)
};

#endif // QMAPPINGPROXYMODEL_H
