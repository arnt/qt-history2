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

#ifndef QPROXYMODEL_H
#define QPROXYMODEL_H

#include <qabstractitemmodel.h>

class QProxyModelPrivate;

class Q_GUI_EXPORT QProxyModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QProxyModel)

public:
    QProxyModel(QObject *parent = 0);
    ~QProxyModel();

    virtual void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    // implementing model interface

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;

    bool canDecode(QMimeSource *src) const;
    bool decode(QDropEvent *e, const QModelIndex &parent);
    QDragObject *dragObject(const QModelIndexList &indexes, QWidget *dragSource);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent, int count);
    bool insertColumns(int column, const QModelIndex &parent, int count);

    void fetchMore(const QModelIndex &parent);
    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);

    bool equal(const QModelIndex &left, const QModelIndex &right) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                          int hits, QAbstractItemModel::MatchFlags flags) const;

    QSize span(const QModelIndex &index) const;

public slots:
    bool submit();
    void revert();

protected:
    QProxyModel(QProxyModelPrivate &, QObject *parent = 0);
};

#endif
