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

#include <QAbstractTableModel>
#include <qlist.h>
#include <QObject>
#include <QVariant>

class TableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TableModel(int rows = 1, int columns = 1, QObject *parent = 0);

    int rowCount() const;
    int columnCount() const;
    QVariant data(const QModelIndex &index, int role) const;

    bool isEditable(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int position, const QModelIndex &parent, int rows);
    bool insertColumns(int position, const QModelIndex &parent, int columns);
    bool removeRows(int position, const QModelIndex &parent, int rows);
    bool removeColumns(int position, const QModelIndex &parent, int columns);

private:
    QList<QStringList> rowList;
};

#endif
