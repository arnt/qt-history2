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

#include <qabstractitemmodel.h>
#include <qobject.h>
#include <qstringlist.h>

class StringListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    StringListModel(const QStringList &strings, QObject *parent = 0)
        : QAbstractListModel(parent), stringList(strings) {}

    int rowCount() const;
    QModelIndex index(int row, int column, const QModelIndex &parent, QModelIndex::Type type) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool isEditable(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int position, const QModelIndex &index, int rows);
    bool removeRows(int position, const QModelIndex &index, int rows);

private:
    QStringList stringList;
};

#endif
