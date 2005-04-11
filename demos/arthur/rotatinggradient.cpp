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

#include "rotatinggradient.h"

#include <qpainter.h>

RotatingGradient::RotatingGradient(QWidget *parent)
    : DemoWidget(parent)
{
    timeoutRate = 25;
}

void RotatingGradient::timerEvent(QTimerEvent *e)
{
    matrix.rotate(1);
    DemoWidget::timerEvent(e);
}

void RotatingGradient::paintEvent(QPaintEvent *)
{
    if (!attributes)
        return;

    QPainter p(this);

    // We paint the whole widget so don't paint background if we are not transparent.
    if (attributes->alpha)
        drawBackground(&p);

    // Define a value that will move from 0 to 255 then down to 0
    // again. fade in, fade out
    int fade = animationStep() % 512;
    if (fade > 255)
        fade = 511 - fade;

    // Create two points that rotate around origo at radius w/4 and
    // move them to the center of the widget. We also add the fade
    // factor to the widgets to tilt them away from the static circle
    // motion.
    QPoint p1 = QPoint(-width()/4, 0)*matrix + QPoint(width()/2, height()/2);
    QPoint p2 = QPoint(width()/4, 0)*matrix + QPoint(width()/2, height()/2);

    int alpha1 = 255;
    int alpha2 = attributes->alpha ? fade : 255;

    // Define the gradient brush. The colors will fade from blue and green to
    // red and blue
    QLinearGradient lg(p1, p2);
    lg.setColorAt(0, QColor(fade, 0, 255-fade, alpha1));
    lg.setColorAt(1, QColor(0, 255-fade, fade, alpha2));
    p.setBrush(lg);

    p.drawRect(0, 0, width(), height());
}
