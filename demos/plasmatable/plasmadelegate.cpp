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

#include "plasmadelegate.h"
#include <qabstractitemmodel.h>
#include <qpainter.h>

PlasmaDelegate::PlasmaDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

PlasmaDelegate::~PlasmaDelegate()
{

}

void PlasmaDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    unsigned char s = option.state & QStyle::State_Selected ? 63 : 0;
    unsigned int color = index.model()->data(index, Qt::DisplayRole).toInt();
    unsigned char r = ((color & 0x00FF0000) >> 16) + s;
    unsigned char g = ((color & 0x0000FF00) >> 8) + s;
    unsigned char b = (color & 0x000000FF) + s;

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(r, g, b));
    painter->drawRect(option.rect);
}

QSize PlasmaDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(4, 4);
}
