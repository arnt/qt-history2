#ifndef DIGITALCLOCK_H
#define DIGITALCLOCK_H

#include <QLCDNumber>

class DigitalClock : public QLCDNumber
{
public:
    DigitalClock(QWidget *parent = 0);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void showTime();
    bool showingColon;
};

#endif
