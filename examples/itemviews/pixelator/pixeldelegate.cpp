/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "pixeldelegate.h"

PixelDelegate::PixelDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
    pixelSize = 12;
}

void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    int size = qMin(option.rect.width(), option.rect.height());
    int brightness = index.model()->data(index, Qt::DisplayRole).toInt();
    double radius = (size/2.0) - (brightness/255.0 * size/2.0);
    if (radius == 0)
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    if (option.state & QStyle::State_Selected)
        painter->setBrush(option.palette.highlightedText());
    else
        painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(QRectF(option.rect.x() + option.rect.width()/2 - radius,
                            option.rect.y() + option.rect.height()/2 - radius,
                            2*radius, 2*radius));
    painter->restore();
}

QSize PixelDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                              const QModelIndex & /* index */) const
{
    return QSize(pixelSize, pixelSize);
}

void PixelDelegate::setPixelSize(int size)
{
    pixelSize = size;
}
