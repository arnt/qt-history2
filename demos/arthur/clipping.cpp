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
    lastStep = 0;
    pressPoint = QPoint(-1, -1);

    const int rectCount = 10;

    for (int i=0; i<rectCount; ++i) {
        int width  = 100;
        int height = 100;

        int x = i*7;
        int y = i*13;

        rects.append(QRectF(x, y, width, height));
        rectDirection.append(QPointF(xfunc(i*113)*5 + 1, yfunc(i*113)*5 + 1));
    }
}

void Clipping::timerEvent(QTimerEvent *e)
{
    int w = width(), h = height();

    int currentStep = animationStep();
    for (int i=0; i<rects.size(); ++i) {
        QRectF r = rects.at(i);
        QPointF dt = rectDirection.at(i);
        double factor = (currentStep - lastStep) / 2.0;
        QPointF d = QPointF(dt.x() * factor, dt.y() * factor);
        r.translate(d);

        // Move rect horizontally
        if (r.left() < 0) {
            r.setRect(0, r.y(), r.width(), r.height());
            dt.setX(-dt.x());
        } else if (r.right() > w) {
            r.setRect(w-r.width(), r.y(), r.width(), r.height());
            dt.setX(-dt.x());
        }

        // Move rect vertically
        if (r.top() < 0) {
            r.setRect(r.x(), 0, r.width(), r.height());
            dt.setY(-dt.y());
        } else if (r.bottom() > h) {
            r.setRect(r.x(), h-r.height(), r.width(), r.height());
            dt.setY(-dt.y());
        }

        // Store the moved rect and direction
        rects[i] = r;
        rectDirection[i] = dt;
    }
    lastStep = currentStep;

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
            region ^= QRegion(rects.at(i).toRect(), QRegion::Ellipse);
        else
            region ^= rects.at(i).toRect();
    }

    // If the mouse is pressed
    if (pressPoint != QPoint(-1, -1)) {
        QRect mouseRect(pressPoint, currentPoint);
        region ^= mouseRect.normalized();
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
