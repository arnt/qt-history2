#include <QTime>

#include "digitalclock.h"

DigitalClock::DigitalClock(QWidget *parent)
    : QLCDNumber(parent)
{
    setWindowTitle(tr("Qt Example - Digital Clock"));
    showingColon = true;
    showTime();
    startTimer(1000);
}

void DigitalClock::timerEvent(QTimerEvent * /*event*/)
{
    showTime();
}

void DigitalClock::showTime()
{
    QString time = QTime::currentTime().toString().left(5);
    if (!showingColon)
        time[2] = ' ';
    display(time);
    showingColon = !showingColon;
}
