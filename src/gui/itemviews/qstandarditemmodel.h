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

#ifndef QSTANDARDITEMMODEL_H
#define QSTANDARDITEMMODEL_H

#include <QtGui/qabstractitemmodel.h>

class QStandardItemModelPrivate;

class QStandardItemModel : public QAbstractItemModel
{
public:
    QStandardItemModel(QObject *parent = 0);
    QStandardItemModel(int rows, int columns, QObject *parent = 0);
    ~QStandardItemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    bool insertRows(int row, int count, const QModelIndex &parent);
    bool insertColumns(int column, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool removeColumns(int column, int count, const QModelIndex &parent);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

private:
    Q_DECLARE_PRIVATE(QStandardItemModel)
    Q_DISABLE_COPY(QStandardItemModel)
};

#endif //QSTANDARDITEMMODEL_H
