/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <qvector.h>

class LinearModel : public QAbstractListModel
{
    Q_OBJECT
public:
    LinearModel(QObject *parent = 0)
        : QAbstractListModel(parent) {}

    int rowCount() const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool isEditable(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int position, const QModelIndex &index, int rows);
    bool removeRows(int position, const QModelIndex &index, int rows);

private:
    QVector<int> values;
};

#endif
