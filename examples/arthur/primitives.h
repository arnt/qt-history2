#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "demowidget.h"

#include <qpointarray.h>

class Primitives : public DemoWidget
{
public:
    enum PrimitiveType {
        Rect, Ellipse, Polygon
    };

    Primitives(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

    void drawPrimitives(QPainter *p, PrimitiveType type, int funcOffset);

private:
    QPointArray polygon;
};

#endif // PRIMITIVES_H
