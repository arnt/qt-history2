#include "paths.h"

#include <qpainter.h>
#include <qpainterpath.h>

Paths::Paths(QWidget *parent)
    : DemoWidget(parent)
{
    animationLoopStep = -1;
}


void Paths::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    fillBackground(&p);

    if (attributes->antialias)
        p.setRenderHints(QPainter::LineAntialiasing);

    p.setPen(QPen(QColor(63, 63, 127, attributes->alpha ? 191 : 255), 5));
    p.setBrush(QColor(191, 191, 255, attributes->alpha ? 127 : 255));

    int w = width(), h = height();

    QRect r(200, 200, w - 400, h - 400);

    QPainterPath path;
    path.beginSubpath();

    int bezierCount = 2;

    QPointArray a;
    for (int i=0; i<bezierCount*3+1; ++i) {
        a.append(QPoint(int(xfunc(animationStep + i*20) * w/2 + w/2),
                        int(yfunc(animationStep + i*20) * h/2 + h/2)));
    }

    for (int bez=0; bez<bezierCount; ++bez) {
        path.addBezier(a.at(bez*3), a.at(bez*3+1), a.at(bez*3+2), a.at(bez*3+3));
    }

    path.closeSubpath();

    path.addRect(r);

    p.drawPath(path);
}
