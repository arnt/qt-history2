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

#include <QtCore/qabstractitemmodel.h>

QT_MODULE(Gui)

#ifndef QT_NO_STANDARDITEMMODEL

class QStandardItemModelPrivate;

class Q_GUI_EXPORT QStandardItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QStandardItemModel(QObject *parent = 0);
    QStandardItemModel(int rows, int columns, QObject *parent = 0);
    ~QStandardItemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role = Qt::EditRole);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void clear();

private:
    Q_DECLARE_PRIVATE(QStandardItemModel)
    Q_DISABLE_COPY(QStandardItemModel)
};

#endif // QT_NO_STANDARDITEMMODEL
#endif //QSTANDARDITEMMODEL_H
