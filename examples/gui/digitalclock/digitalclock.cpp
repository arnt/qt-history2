#include <QtCore>

#include "digitalclock.h"

DigitalClock::DigitalClock(QWidget *parent)
    : QLCDNumber(parent)
{
    showingColon = true;
    showTime();
    startTimer(1000);
    setWindowTitle(tr("Digital Clock"));
}

void DigitalClock::timerEvent(QTimerEvent *)
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
