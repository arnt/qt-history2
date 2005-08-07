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

#ifndef QSORTINGPROXYMODEL_H
#define QSORTINGPROXYMODEL_H

#include <QtGui/qmappingproxymodel.h>

class QSortingProxyModelPrivate;

class Q_GUI_EXPORT QSortingProxyModel : public QMappingProxyModel
{
    Q_OBJECT

public:
    QSortingProxyModel(QObject *parent = 0);
    ~QSortingProxyModel();

    void sort(int column, Qt::SortOrder order);
    void clear();

protected:
    QSortingProxyModel(QSortingProxyModelPrivate &, QObject *parent);

    static bool lessThan(const QModelIndex &left, const QModelIndex &right);
    static bool greaterThan(const QModelIndex &left, const QModelIndex &right);

    void sourceLayoutChanged();

private:
    Q_DECLARE_PRIVATE(QSortingProxyModel)
    Q_DISABLE_COPY(QSortingProxyModel)
};

#endif // QSORTINGPROXYMODEL_H
