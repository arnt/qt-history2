#include "alphashade.h"

#include <qpainter.h>

static uint *colorTable = 0;
#define TABLESIZE 30

AlphaShade::AlphaShade(QWidget *parent)
    : DemoWidget(parent),
      iterations(2),
      spread(5)
{
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

void AlphaShade::drawPrimitives(QPainter *p)
{
    const int count = 50;
    const int size = 100;
    const double distance = .3;
    const int rotationSpeed = 2;

    int w = width(), h = height();

    p->setPen(Qt::NoPen);

    for (int i=0; i<count; ++i) {
        double x = xfunc(animationStep + i*distance);
        double y = yfunc(animationStep + i*distance);

        p->save();
        p->translate(w/2 + w/2 * x, h/2 + h/2 * y);
        p->rotate(animationStep + i * rotationSpeed);
        uint pixel = colorTable[i%TABLESIZE];
        pixel |= attributes->alpha ? 63 << 24 : 0xff << 24;
        QColor c(pixel);
        p->setBrush(c);

        enum PrimitiveType {
            Rect, Ellipse, Polygon
        };

        switch (PrimitiveType(((animationStep+i)/200)%3)) {
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

void AlphaShade::paintEvent(QPaintEvent *)
{
    if (!attributes)
        return;

    QPainter p(this);
    fillBackground(&p);


    if (attributes->antialias)
        p.setRenderHints(QPainter::LineAntialiasing);

    if (attributes->alpha)
        p.setPen(QColor(0, 0, 0, 63));

    int w = width(), h = height();

    p.save();

    // Get painter into position...
    p.translate(w/2-10, h/2-20);
    p.rotate(animationStep);
    p.translate(-w/4, -h/4);
    p.setPen(Qt::NoPen);

    int offset = 30;

    // Transparent shadow...
    p.setBrush(QColor(0, 0, 0, attributes->alpha ? 31 : 255));
    for (int x=0; x<iterations*spread; x+=spread) {
        for (int y=0; y<iterations*spread; y+=spread) {
            p.save();
            p.rotate(-animationStep);
            p.translate(offset + x, offset * 1.5 + y);
            p.rotate(animationStep);
            p.drawRect(0, 0, w/2, h/2);
            p.restore();
        }
    }

    // The solid fill on top...
    p.setPen(Qt::black);
    p.setBrush(QColor(255, 255, 255, attributes->alpha ? 127 : 255 ));
    p.drawRect(0, 0, w/2, h/2);

    p.restore();

    // The snake of primitives...
    drawPrimitives(&p);

}
