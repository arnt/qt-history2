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

#ifndef HEXDELEGATE_H
#define HEXDELEGATE_H

#include <qitemdelegate.h>

class HexDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    HexDelegate(QObject *parent);
    ~HexDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QAbstractItemModel *model, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QAbstractItemModel *model, const QModelIndex &index) const;
private:
    mutable QString textHex;
    QSize sz;
};

#endif // HEXDELEGATE_H
