#include <QtCore>
#include <QtGui>

#include "analogclock.h"

AnalogClock::AnalogClock(QWidget *parent)
    : QWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(5000);

    setWindowTitle(tr("Analog Clock"));
    resize(200, 200);
}

void AnalogClock::paintEvent(QPaintEvent *)
{
    static int hourHand[8] = { 2, 0, 0, 2, -2, 0, 0, -25 };
    static int minuteHand[8] = { 1, 0, 0, 1, -1, 0, 0, -40 };

    int side = qMin(width(), height());
    QTime time = QTime::currentTime();

    QPainter painter(this);
    painter.setBrush(painter.pen().color());
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 100.0, side / 100.0);

    painter.save();
    painter.rotate(30 * (time.hour() % 12) + time.minute() / 2);
    painter.drawConvexPolygon(QPolygon(4, hourHand));
    painter.restore();

    painter.save();
    painter.rotate(6 * time.minute());
    painter.drawConvexPolygon(QPolygon(4, minuteHand));
    painter.restore();

    for (int i = 0; i < 12; ++i) {
        painter.drawLine(44, 0, 46, 0);
        painter.rotate(30);
    }
}
