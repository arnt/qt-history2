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

#include "paths.h"

#include <qpainter.h>
#include <qpainterpath.h>

Paths::Paths(QWidget *parent)
    : DemoWidget(parent), step(0)
{
    setAttribute(Qt::WA_PaintOnScreen);
}


void Paths::paintEvent(QPaintEvent *)
{
    QPainter p(&dblBuffer);

    if (attributes->antialias)
        p.setRenderHint(QPainter::Antialiasing);

    if (!attributes->alpha)
        drawBackground(&p);

    p.setPen(QPen(QColor(63, 63, 127, attributes->alpha ? 159 : 255), 5));
    p.setBrush(QColor(191, 191, 255, attributes->alpha ? 63 : 255));

    int w = width(), h = height();

    // Define some semi random points.
    int bezierCount = 2;
    QPolygon a;
    for (int i=0; i<bezierCount*3+1; ++i) {
        a.append(QPoint(int(xfunc(step*0.7 + i*20) * w/2 + w/2),
                        int(yfunc(step*0.7 + i*20) * h/2 + h/2)));
    }
    ++step;

    // Create the path
    QPainterPath path;

    // Add the bezier curves to it
    path.moveTo(a.at(0));
    for (int bez=0; bez<bezierCount; ++bez) {
        path.cubicTo(a.at(bez*3+1), a.at(bez*3+2), a.at(bez*3+3));
    }
    path.closeSubpath();

    // Add a rect in the center of the widget
    path.addRect(100, 100, w-200, h-200);

    // Draw the path
    p.setClipRect(100-p.pen().width()/2, 100-p.pen().width()/2,
                  w-200+p.pen().width(), h-200+p.pen().width());
    p.drawPath(path);
    p.end();

    p.begin(this);
    p.drawPixmap(0, 0, dblBuffer);
}

void Paths::resizeEvent(QResizeEvent *event)
{
    dblBuffer = QPixmap(width(), height());
    QPainter p(&dblBuffer);
    drawBackground(&p);
    DemoWidget::resizeEvent(event);
}

void Paths::resetState()
{
    QPainter p(&dblBuffer);
    drawBackground(&p);
}
