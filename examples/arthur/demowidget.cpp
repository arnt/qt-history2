#include "demowidget.h"

#include <qpainter.h>

#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14259
#endif


DemoWidget::DemoWidget(QWidget *parent)
    : QWidget(parent),
      timeoutRate(50),
      animationStep(0),
      animationLoopStep(360)
{
    srand((uint) time(0));
    a = rand() / (double)RAND_MAX;
    b = rand() / (double)RAND_MAX;
    c = rand() / (double)RAND_MAX;
    d = rand() / (double)RAND_MAX;

    setAttribute(Qt::WA_NoBackground);
    setAttribute(Qt::WA_NoSystemBackground);
}

void DemoWidget::startAnimation()
{
    timerId = startTimer(timeoutRate);
}

void DemoWidget::stopAnimation()
{
    killTimer(timerId);
}

void DemoWidget::timerEvent(QTimerEvent *)
{
    ++animationStep;
    if (animationStep == animationLoopStep)
        animationStep = 0;
    update();
}

QSize DemoWidget::sizeHint() const
{
    return QSize(400, 400);
}

void DemoWidget::fillBackground(QPainter *p)
{
    if (!attributes)
        return;
    switch (attributes->fillMode) {
    case Attributes::Solid:
        p->fillRect(0, 0, width(), height(), attributes->color);
        break;
    case Attributes::Gradient:
        p->fillRect(0, 0, width(), height(),
                    QBrush(QPoint(0, 0), attributes->color,
                           QPoint(0, height()), attributes->secondaryColor));
        break;
    case Attributes::Tiles:
        p->drawTiledPixmap(0, 0, width(), height(), attributes->tile);
        break;
    case Attributes::Pixmap:
        p->drawPixmap(0, 0, width(), height(), attributes->pattern);
        break;
    }

    p->setPen(Qt::black);
    p->setBrush(Qt::NoBrush);
    p->drawRect(0, 0, width(), height());
}

/*!
  Returns the x value of a random continuous function.
*/
double DemoWidget::xfunc(double t)
{
    return (sin(a*t/M_PI) + cos(b*t/M_PI) + sin(c*t/M_PI)) / 3.0;
}

/*!
  Returns the x value of a random continuous function.
*/
double DemoWidget::yfunc(double t)
{
    return (sin(b*t/M_PI) + cos(c*t/M_PI) + sin(d*t/M_PI)) / 3.0;
}
