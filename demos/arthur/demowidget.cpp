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

#include "demowidget.h"

#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>

#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14259
#endif

/*!
  \class Attributes

  This class contains the attributes that should be used by the demo widget when
  painting.
*/

/*!
  \class DemoWidget

  The DemoWidget class provides conveninence functionality for the
  various demowidgets in the Arthur painter demo.
*/

DemoWidget::DemoWidget(QWidget *parent)
    : QWidget(parent),
      timeoutRate(50),
      animationStep(0)
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
    if (!animationTimer.isActive())
	animationTimer.start(timeoutRate, this);
}

void DemoWidget::stopAnimation()
{
    animationTimer.stop();
}

void DemoWidget::timerEvent(QTimerEvent * e)
{
    if (e->timerId() == animationTimer.timerId()) {
        ++animationStep;
        update();
        if ((animationStep % 10) == 0 )
            QApplication::syncX();
    }
}

void DemoWidget::showEvent(QShowEvent *)
{
    startAnimation();
}

void DemoWidget::hideEvent(QShowEvent *)
{
    stopAnimation();
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
