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

#include "alphashade.h"

#include <qpolygon.h>
#include <qpainter.h>

static QPolygon polygon;
static uint *colorTable = 0;
#define TABLESIZE 30

void initPrimitives()
{
    static int init = 0;

    if (init) return;
    init = 1;

    polygon.setPoints(9,
                        0, 100,
                       75, 125,
                      100, 200,
                      125, 125,
                      200, 100,
                      125,  75,
                      100,   0,
                       75,  75,
                        0, 100);

    if (!colorTable) {
        colorTable = new uint[TABLESIZE];
        for (int i=0; i<TABLESIZE; ++i) {
            colorTable[i] = ((i*11)%0xff) << 16
                            | ((i*23)%0xff) << 8
                            | (i*37)%0xff;
        }
    }
}

void drawPrimitives(DemoWidget *dw, QPainter *p, int count, double distance, int step)
{
    initPrimitives();

    const int size = 100;
    const int rotationSpeed = 2;

    int w = dw->width(), h = dw->height();

    p->setPen(Qt::NoPen);

    for (int i=0; i<count; ++i) {
        double x = dw->xfunc(step + i*distance);
        double y = dw->yfunc(step + i*distance);

        p->save();
        p->translate(w/2 + w/2 * x, h/2 + h/2 * y);
        p->rotate(step + i * rotationSpeed);
        QColor c(colorTable[i%TABLESIZE]);
        if (dw->attribs()->alpha)
            c.setAlpha(63);
        p->setBrush(c);

        enum PrimitiveType {
            Rect, Ellipse, Polygon
        };

        switch (PrimitiveType(((step+i)/200)%3)) {
        case Rect:
            p->drawRect(0, 0, int(size*x), int(size*y));
            break;
        case Ellipse:
            p->drawEllipse(0, 0, int(size*x), int(size*y));
            break;
        case Polygon:
            p->scale(x * size / 200, y * size / 200);
            p->drawPolygon(polygon);
            break;
        }

        p->restore();
    } // for (
}

void drawShadedCube(DemoWidget *dw, QPainter *p, int iterations, int spread, int step)
{
    if (dw->attribs()->antialias)
        p->setRenderHint(QPainter::Antialiasing);

    if (dw->attribs()->alpha)
        p->setPen(QColor(0, 0, 0, 63));

    int w = dw->width(), h = dw->height();

    p->save();

    // Get painter into position...
    p->translate(w/2-10, h/2-20);
    p->rotate(step/10.0);
    p->translate(-w/4, -h/4);
    p->setPen(Qt::NoPen);

    int offset = 30;

    // Transparent shadow...
    p->setBrush(QColor(0, 0, 0, dw->attribs()->alpha ? 31 : 255));
    for (int x=0; x<iterations*spread; x+=spread) {
        for (int y=0; y<iterations*spread; y+=spread) {
            p->save();
            p->rotate(-step/10.0);
            p->translate(offset + x, offset * 1.5 + y);
            p->rotate(step/10.0);
            p->drawRect(0, 0, w/2, h/2);
            p->restore();
        }
    }

    // The solid fill on top...
    p->setPen(Qt::black);
    p->setBrush(QColor(255, 255, 255, dw->attribs()->alpha ? 127 : 255 ));
    p->drawRect(0, 0, w/2, h/2);

    p->restore();
}

AlphaShade::AlphaShade(QWidget *parent)
    : DemoWidget(parent)
{
}

void AlphaShade::paintEvent(QPaintEvent *)
{
    if (!attributes)
        return;

    QPainter p(this);
    drawBackground(&p);
    drawShadedCube(this, &p, 1, 1, animationStep());
    drawPrimitives(this, &p, 50, .3, animationStep()/3);
}

void AlphaShade::mousePressEvent(QMouseEvent *)
{
    stopAnimation();
}

void AlphaShade::mouseReleaseEvent(QMouseEvent *)
{
    startAnimation();
}
