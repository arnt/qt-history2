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

#ifndef QSTANDARDITEMMODEL_P_H
#define QSTANDARDITEMMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractitemmodel_p.h"

#ifndef QT_NO_STANDARDITEMMODEL

#include <private/qwidgetitemdata_p.h>
#include <QtCore/qpair.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>

class QStandardItemPrivate
{
    Q_DECLARE_PUBLIC(QStandardItem)
public:
    inline QStandardItemPrivate()
        : type(QStandardItem::Type),
          model(0),
          parent(0),
          flags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable
                |Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled),
          rows(0),
          columns(0)
        { }
    virtual ~QStandardItemPrivate();

    int childIndex(int row, int column) const;
    inline int childIndex(const QStandardItem *child) const {
        return children.indexOf(const_cast<QStandardItem*>(child));
    }
    QPair<int, int> itemPosition(const QStandardItem *item) const;
    inline int rowCount() const {
        return rows;
    }
    inline int columnCount() const {
        return columns;
    }
    void childDeleted(QStandardItem *child);

    int type;
    QStandardItemModel *model;
    QStandardItem *parent;
    QVector<QWidgetItemData> values;
    Qt::ItemFlags flags;
    QVector<QStandardItem*> children;
    int rows;
    int columns;

    QStandardItem *q_ptr;
};

class QStandardItemModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QStandardItemModel)

public:
    QStandardItemModelPrivate();
    virtual ~QStandardItemModelPrivate();

    QStandardItem *itemFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromItem(const QStandardItem *item) const;
    QStandardItem *createItem() const;

    QVector<QStandardItem*> columnHeaderItems;
    QVector<QStandardItem*> rowHeaderItems;
    QStandardItem *root;
    const QStandardItem *itemPrototype;
};

#endif // QT_NO_STANDARDITEMMODEL

#endif // QSTANDARDITEMMODEL_P_H
