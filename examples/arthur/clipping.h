#ifndef CLIPPING_H
#define CLIPPING_H

#include "demowidget.h"

#include <qpixmap.h>

class QMouseEvent;

class Clipping : public DemoWidget
{
public:
    Clipping(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

    void resizeEvent(QResizeEvent *e);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    QPixmap bgFill;

    QList<QRect> rects;
    QList<QPoint> rectDirection;

    QPoint pressPoint;
    QPoint currentPoint;
};

#endif // CLIPPING_H
