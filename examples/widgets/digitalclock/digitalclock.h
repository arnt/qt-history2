#ifndef DIGITALCLOCK_H
#define DIGITALCLOCK_H

#include <QBasicTimer>
#include <QLCDNumber>

class DigitalClock : public QLCDNumber
{
public:
    DigitalClock(QWidget *parent = 0);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void showTime();

    QBasicTimer timer;
    bool showingColon;
};

#endif
