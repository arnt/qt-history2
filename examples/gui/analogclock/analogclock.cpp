#include <QtCore>
#include <QtGui>

#include "analogclock.h"

AnalogClock::AnalogClock(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Analog Clock"));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(5000);

    resize(100, 100);
}

void AnalogClock::paintEvent(QPaintEvent *)
{
    static QCOORD hourHand[8] = { 2, 0, 0, 2, -2, 0, 0, -25 };
    static QCOORD minuteHand[8] = { 1, 0, 0, 1, -1, 0, 0, -40 };

    QPainter painter(this);
    painter.setWindow(-50, -50, 100, 100);
    painter.setBrush(palette().foreground());

    for (int i = 0; i < 12; i++) {
        painter.drawLine(44, 0, 46, 0);
        painter.rotate(30);
    }

    QTime time = QTime::currentTime();

    painter.save();
    painter.rotate(30 * (time.hour() % 12) + time.minute() / 2);
    painter.drawConvexPolygon(QPointArray(4, hourHand));
    painter.restore();

    painter.save();
    painter.rotate(6 * time.minute());
    painter.drawConvexPolygon(QPointArray(4, minuteHand));
    painter.restore();
}
