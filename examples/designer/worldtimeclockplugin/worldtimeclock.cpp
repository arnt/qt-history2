#include <QtGui>

#include "worldtimeclock.h"

WorldTimeClock::WorldTimeClock(QWidget *parent)
    : QWidget(parent)
{
    timeZoneOffset = 0;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);

    setWindowTitle(tr("World Time Clock"));
    resize(200, 200);
}

void WorldTimeClock::paintEvent(QPaintEvent *)
{
    static const int hourHand[6] = { 7, 8, -7, 8, 0, -40 };
    static const int minuteHand[6] = { 7, 8, -7, 8, 0, -70 };
    QColor hourColor(127, 0, 127);
    QColor minuteColor(0, 127, 127, 191);

    int side = qMin(width(), height());
    QTime time = QTime::currentTime();
    time = time.addSecs(timeZoneOffset);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);

    painter.save();
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.drawConvexPolygon(QPolygon(3, hourHand));
    painter.restore();

    painter.setPen(hourColor);

    for (int i = 0; i < 12; ++i) {
        painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColor);

    painter.save();
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.drawConvexPolygon(QPolygon(3, minuteHand));
    painter.restore();

    painter.setPen(minuteColor);

    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0)
            painter.drawLine(92, 0, 96, 0);
        painter.rotate(6.0);
    }

    emit updated(time);
}

void WorldTimeClock::setTimeZone(int hourOffset)
{
    timeZoneOffset = qMin(qMax(-12, hourOffset), 12) * 3600;
    update();
}
