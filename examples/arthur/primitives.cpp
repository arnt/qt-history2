#include "primitives.h"

#include <qpainter.h>

#include <math.h>

static uint *colorTable = 0;
#define TABLESIZE 30

Primitives::Primitives(QWidget *parent)
    : DemoWidget(parent)
{
    animationLoopStep = -1;
    animationStep = 100;

    polygon.setPoints(9,
                      -100,0,
                      -25,25,
                      0,100,
                      25,25,
                      100,0,
                      25,-25,
                      0,-100,
                      -25,-25,
                      -100,0);

    if (!colorTable) {
        colorTable = new uint[TABLESIZE];
        for (int i=0; i<TABLESIZE; ++i) {
            colorTable[i] = ((i*11)%0xff) << 16
                            | ((i*37)%0xff) << 8
                            | (i*83)%0xff;
        }
    }
}

void Primitives::drawPrimitives(QPainter *p, PrimitiveType type, int funcOffset)
{
    const int count = 50;
    const int size = 100;
    const double distance = .2;
    const int rotationSpeed = 2;

    int w = width(), h = height();

    for (int i=0; i<count; ++i) {
        double x = xfunc(funcOffset + animationStep + i*distance);
        double y = yfunc(funcOffset + animationStep + i*distance);

        p->save();
        p->translate(w/2 + w/2 * x, h/2 + h/2 * y);
        p->rotate(animationStep + i * rotationSpeed);
        uint pixel = colorTable[i%TABLESIZE];
        pixel |= attributes->alpha ? 63 << 24 : 0xff << 24;
        QColor c(pixel);
        p->setBrush(c);

        switch (type) {
        case Rect:
            p->drawRect(0, 0, size*x, size*y);
            break;
        case Ellipse:
            p->drawEllipse(0, 0, size*x, size*y);
            break;
        case Polygon:
            p->scale(x, y);
            p->drawPolygon(polygon);
            break;
        }

        p->restore();
    }
}

void Primitives::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    fillBackground(&p);

    if (attributes->antialias)
        p.setRenderHints(QPainter::LineAntialiasing);

    if (attributes->alpha)
        p.setPen(QColor(0, 0, 0, 63));

    drawPrimitives(&p, Rect, 0);
    drawPrimitives(&p, Ellipse, 1000);
    drawPrimitives(&p, Polygon, 2000);
}
