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

#ifndef PLASMADELEGATE_H
#define PLASMADELEGATE_H

#include <qabstractitemdelegate.h>

class PlasmaDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    PlasmaDelegate(QObject *parent = 0);
    ~PlasmaDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QAbstractItemModel *model, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // PLASMADELEGATE_H
