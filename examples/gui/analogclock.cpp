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

#include <QTimer>
#include <QPainter>

#include "analogclock.h"

/*
    Constructs an analog clock widget that uses an internal QTimer to
    ensure that it is updated at regular intervals.
*/

AnalogClock::AnalogClock(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Qt Example - Analog Clock"));
    time = QTime::currentTime();
    QTimer *internalTimer = new QTimer(this);
    connect(internalTimer, SIGNAL(timeout()), SLOT(timeout()));
    internalTimer->start(5000);
}

/*
    When the internal timer emits the timeout() signal, we update the
    clock if the minute hand should move.
*/

void AnalogClock::timeout()
{
    QTime new_time = QTime::currentTime();
    if (new_time.minute() != time.minute()) {
        time = new_time;
        update();
    }
}

/*
    The clock is drawn in a 1000x1000 square coordinate system, and is
    rendered to the widget in a centered square area that is as big as
    possible. The widget's standard foreground color is used to draw the
    clock face.
*/

void AnalogClock::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setBrush(palette().foreground());
    paint.save();

    paint.setWindow(-500,-500, 1000,1000);

    QRect viewport = paint.viewport();
    int size = qMin(viewport.width(), viewport.height());
    paint.setViewport(viewport.left() + viewport.width()/2 -size/2,
                      viewport.top() + viewport.height()/2 -size/2,
                      size, size);

    QPointArray pts;

    paint.save();
    paint.rotate(30*(time.hour()%12 - 3) + time.minute()/2);
    pts.setPoints(4, -20,0,  0,-20, 300,0, 0,20);
    paint.drawPolygon(pts);
    paint.restore();

    paint.save();
    paint.rotate((time.minute()-15)*6);
    pts.setPoints(4, -10,0, 0,-10, 400,0, 0,10);
    paint.drawPolygon(pts);
    paint.restore();

    for ( int i=0; i<12; i++ ) {
        paint.drawLine(440,0, 460,0);
        paint.rotate(30);
    }

    paint.restore();
}
