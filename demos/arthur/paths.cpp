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
    : DemoWidget(parent)
{
}


void Paths::paintEvent(QPaintEvent *)
{
    int stepped = animationStep();

    QPainter p(this);

    if (attributes->antialias)
        p.setRenderHint(QPainter::Antialiasing);

    drawBackground(&p);

    p.setPen(QPen(QColor(63, 63, 127, attributes->alpha ? 191 : 255), 5));
    p.setBrush(QColor(191, 191, 255, attributes->alpha ? 127 : 255));

    int w = width(), h = height();

    // Define some semi random points.
    int bezierCount = 2;
    QPolygon a;
    for (int i=0; i<bezierCount*3+1; ++i) {
        a.append(QPoint(int(xfunc(stepped*0.031415 + i*20) * w/2 + w/2),
                        int(yfunc(stepped*0.031415 + i*20) * h/2 + h/2)));
    }
    ++stepped;

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
    p.drawPath(path);
}

void Paths::resizeEvent(QResizeEvent *event)
{
    dblBuffer = QPixmap(width(), height());
    QPainter p(&dblBuffer);
    drawBackground(&p);
    DemoWidget::resizeEvent(event);
}
