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

#ifndef QSTANDARDITEMMODELPRIVTE_H
#define QSTANDARDITEMMODELPRIVTE_H

#include <private/qabstractitemmodel_p.h>

class  ModelItem {
public:
    ModelItem() {};
    QVariant value(int role)
        {
            for (int i=0; i<roles.count(); ++i)
                if (roles.at(i).first == role)
                    return roles.at(i).second;
            return QVariant::Invalid;
        }
    void setValue(int role, QVariant value)
        {
            for (int i=0; i<roles.count(); ++i)
                if (roles.at(i).first == role) {
                    roles[i].second = value;
                    return;
                }
            roles.append(QPair<int, QVariant>(role, value));
            return;
        }

    QVector<QPair<int, QVariant> > roles;
};

class ModelRow {
public:
    ModelRow(ModelRow *parent) : p(parent), childrenColumns(0) {}
    ~ModelRow()
        {
            for (int i=0; i<items.count(); ++i)
                delete items.at(i);
            items.clear();
            for (int i=0; i<childrenRows.count(); ++i)
                delete childrenRows.at(i);
            childrenRows.clear();
        }

    ModelRow *p;
    QVector<ModelItem*> items;
    int childrenColumns;
    QVector<ModelRow*> childrenRows;
};

class QStandardItemModelPrivate : public QAbstractItemModelPrivate
{

    Q_DECLARE_PUBLIC(QStandardItemModel)

public:
    QStandardItemModelPrivate(int rows, int columns) : topLevelRows(rows), topLevelColumns(columns) {}
    virtual ~QStandardItemModelPrivate() {}

    ModelRow *containedRow(const QModelIndex &index, bool createIfMissing) const;

    QVector<ModelRow*> topLevelRows;
    int topLevelColumns;
};

#endif // QSTANDARDITEMMODELPRIVTE_H
