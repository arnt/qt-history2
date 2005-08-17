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

#ifndef QFILTERINGPROXYMODEL_H
#define QFILTERINGPROXYMODEL_H

#include <QtGui/qmappingproxymodel.h>

class Q_GUI_EXPORT QFilteringProxyModel : public QMappingProxyModel
{
    Q_OBJECT

public:
    QFilteringProxyModel(QObject *parent = 0);
    ~QFilteringProxyModel();

    void clear();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;

protected:
    virtual bool filterRow(int source_row, const QModelIndex &source_parent) const = 0;

    void mapChildren(const QModelIndex &parent) const;
    void sourceLayoutChanged();

    mutable QMap<QModelIndex, int> filtered_row_count; // maps the parent to the  number of  rows filtered out
};

#endif // QFILTERINGPROXYMODEL_H
