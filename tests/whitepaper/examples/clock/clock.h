/*
  clock.h
*/

#ifndef CLOCK_H
#define CLOCK_H

#include <qlcdnumber.h>

class Clock : public QLCDNumber
{
public:
    Clock( QWidget *parent = 0, const char *name = 0 );

protected:
    void timerEvent( QTimerEvent *event );

private:
    void showTime();

    bool showingColon;
};

#endif
