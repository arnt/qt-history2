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

#include "clipping.h"

#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qevent.h>

Clipping::Clipping(QWidget *parent)
    : DemoWidget(parent)
{
    pressPoint = QPoint(-1, -1);

    const int rectCount = 10;

    for (int i=0; i<rectCount; ++i) {
        int width  = 100;
        int height = 100;

        int x = i*7;
        int y = i*13;

        rects.append(QRect(x, y, width, height));
        rectDirection.append(QPoint(int(xfunc(i*113)*5) + 5, int(yfunc(i*113)*5) + 5));
    }
}

void Clipping::timerEvent(QTimerEvent *e)
{
    int w = width(), h = height();

    for (int i=0; i<rects.size(); ++i) {
        QRect r = rects.at(i);
        QPoint d = rectDirection.at(i);
        r.translate(d);

        // Move rect horizontally
        if (r.left() < 0) {
            r.setRect(0, r.y(), r.width(), r.height());
            d.setX(-d.x());
        } else if (r.right() > w) {
            r.setRect(w-r.width(), r.y(), r.width(), r.height());
            d.setX(-d.x());
        }

        // Move rect vertically
        if (r.top() < 0) {
            r.setRect(r.x(), 0, r.width(), r.height());
            d.setY(-d.y());
        } else if (r.bottom() > h) {
            r.setRect(r.x(), h-r.height(), r.width(), r.height());
            d.setY(-d.y());
        }

        // Store the moved rect and direction
        rects[i] = r;
        rectDirection[i] = d;
    }

    // Call baseclass implementation
    DemoWidget::timerEvent(e);
}

void Clipping::paintEvent(QPaintEvent *)
{
    int w = width(), h = height();

    QPainter pt(this);

    drawBackground(&pt);

    // Start with an empty region
    QRegion region;

    for (int i=0; i<rects.size(); ++i) {
        // Make every fourth rect an ellipse and add them to the region
        if (i%4 == 0)
            region |= QRegion(rects.at(i), QRegion::Ellipse);
        else
            region |= rects.at(i);
    }

    // If the mouse is pressed
    if (pressPoint != QPoint(-1, -1)) {
        QRect mouseRect = QRect(pressPoint, currentPoint);
        region ^= mouseRect.normalize();
    }

    // Create the region used for clipping.
    QRegion clip(0, 0, w, h);
    clip ^= region;
    pt.setClipRegion(clip);

    QColor bg = palette().color(QPalette::Background);
    pt.setPen(Qt::NoPen);
    pt.setBrush(QColor(bg.red(), bg.green(), bg.blue(), attributes->alpha ? 191 : 255));

    pt.drawRect(rect());
}

void Clipping::mousePressEvent(QMouseEvent *e)
{
    pressPoint = e->pos();
    currentPoint = e->pos();
}

void Clipping::mouseMoveEvent(QMouseEvent *e)
{
    currentPoint = e->pos();
}
