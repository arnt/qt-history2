#include <QtCore>

#include "digitalclock.h"

DigitalClock::DigitalClock(QWidget *parent)
    : QLCDNumber(parent)
{
    setSegmentStyle(Filled);

    showingColon = true;
    timerId = startTimer(1000);
    showTime();

    setWindowTitle(tr("Digital Clock"));
    resize(150, 60);
}

void DigitalClock::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId)
        showTime();
    else
        QLCDNumber::timerEvent(event);
}

void DigitalClock::showTime()
{
    QString time = QTime::currentTime().toString().left(5);
    if (!showingColon)
        time[2] = ' ';
    display(time);
    showingColon = !showingColon;
}
