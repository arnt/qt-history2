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

#include <QtCore/qabstractitemmodel.h>

QT_MODULE(Gui)

#ifndef QT_NO_PROXYMODEL

class QProxyModelPrivate;

class Q_GUI_EXPORT QProxyModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QProxyModel(QObject *parent = 0);
    ~QProxyModel();

    virtual void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    // implementing model interface

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    bool insertRows(int row, int count, const QModelIndex &parent);
    bool insertColumns(int column, int count, const QModelIndex &parent);

    void fetchMore(const QModelIndex &parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order);

    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                          int hits, Qt::MatchFlags flags) const;

    QSize span(const QModelIndex &index) const;

    bool submit();
    void revert();

#ifdef Q_NO_USING_KEYWORD
    inline QObject *parent() const { return QObject::parent(); }
#else
    using QObject::parent;
#endif

protected:
    QProxyModel(QProxyModelPrivate &, QObject *parent = 0);

    QModelIndex setProxyModel(const QModelIndex &source_index) const;
    QModelIndex setSourceModel(const QModelIndex &proxy_index) const;

    void connectToModel(const QAbstractItemModel *model) const;
    void disconnectFromModel(const QAbstractItemModel *model) const;

private:    
    Q_DECLARE_PRIVATE(QProxyModel)
    Q_DISABLE_COPY(QProxyModel)

    Q_PRIVATE_SLOT(d_func(), void sourceDataChanged(const QModelIndex&,const QModelIndex&))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeInserted(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsInserted(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeRemoved(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsRemoved(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsAboutToBeInserted(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsInserted(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsAboutToBeRemoved(const QModelIndex&,int,int))
    Q_PRIVATE_SLOT(d_func(), void sourceColumnsRemoved(const QModelIndex&,int,int))
};

#endif // QT_NO_PROXYMODEL

#endif // QPROXYMODEL_H
