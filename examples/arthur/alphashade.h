#ifndef ALPHASHADE_H
#define ALPHASHADE_H

#include "demowidget.h"

#include <qpixmap.h>
#include <qpointarray.h>

class AlphaShade : public DemoWidget
{
public:
    AlphaShade(QWidget *parent=0);
    void paintEvent(QPaintEvent *e);
    void drawPrimitives(QPainter *p);

private:
    int iterations;
    int spread;
    QPointArray polygon;
};

#endif // ALPHASHADE_H
