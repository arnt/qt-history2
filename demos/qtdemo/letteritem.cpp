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

#include <cmath>
#include "letteritem.h"
#include "colors.h"

LetterItem::LetterItem(char letter, QGraphicsScene *scene, QGraphicsItem *parent) : DemoItem(scene, parent), letter(letter)
{
    useSharedImage(QString(__FILE__) + letter);
}

LetterItem::~LetterItem()
{
}

QImage *LetterItem::createImage(const QMatrix &matrix) const
{
    QRect scaledRect = matrix.mapRect(QRect(0, 0, 25, 25));
    QImage *image = new QImage(scaledRect.width(), scaledRect.height(), QImage::Format_ARGB32_Premultiplied);
    image->fill(0);
    QPainter painter(image);
    painter.scale(matrix.m11(), matrix.m22());
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    if (Colors::useEightBitPalette){
        painter.setBrush(QColor(170, 160, 160));
        painter.drawEllipse(0, 0, 25, 25);
        painter.setFont(Colors::tickerFont());
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(10, 15, QString(this->letter));
    }
    else {
        QLinearGradient brush(0, 0, 0, 25);
        brush.setSpread(QLinearGradient::PadSpread);
        brush.setColorAt(0.0, QColor(206, 246, 117, 200));
        brush.setColorAt(1.0, QColor(206, 246, 117, 60));
        painter.setBrush(brush);       
        painter.drawEllipse(0, 0, 25, 25);
        painter.setFont(Colors::tickerFont());
        painter.setPen(QColor(255, 255, 255, 255));
        painter.drawText(10, 15, QString(this->letter));
    }
    return image;
}


