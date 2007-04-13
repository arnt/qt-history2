/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "scanitem.h"
#include "colors.h"

#define ITEM_WIDTH 16
#define ITEM_HEIGHT 16

ScanItem::ScanItem(QGraphicsScene *scene, QGraphicsItem *parent)
    : DemoItem(scene, parent)
{
    useSharedImage(QString(__FILE__));
}

ScanItem::~ScanItem()
{
}

QImage *ScanItem::createImage(const QMatrix &matrix) const
{
    QRect scaledRect = matrix.mapRect(QRect(0, 0, ITEM_WIDTH, ITEM_HEIGHT));
    QImage *image = new QImage(scaledRect.width(), scaledRect.height(), QImage::Format_ARGB32_Premultiplied);
    image->fill(QColor(0, 0, 0, 0).rgba());
    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing);

    if (Colors::useEightBitPalette){
        painter.setPen(QPen(QColor(100, 100, 100), 2));
        painter.setBrush(QColor(206, 246, 117));
        painter.drawEllipse(1, 1, scaledRect.width()-2, scaledRect.height()-2);
    }
    else {
        painter.setPen(QPen(QColor(0, 0, 0, 250), 1));
        painter.setBrush(QColor(206, 246, 117, 150));
        painter.drawEllipse(1, 1, scaledRect.width()-2, scaledRect.height()-2);
    }
    return image;
}


