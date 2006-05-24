/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BUBBLE_H
#define BUBBLE_H

class Bubble
{
public:
    Bubble(const QPointF &position, qreal radius, const QPointF &velocity);

    void drawBubble(QPainter *painter);
    void updateBrush();
    void move(const QRect &bbox);
    QRectF rect();

private:
    QColor randomColor();

    QBrush brush;
    QPointF position;
    QPointF vel;
    qreal radius;
    QColor innerColor;
    QColor outerColor;
};

#endif
