/*
  clock.cpp
*/

#include <qdatetime.h>

#include "clock.h"

Clock::Clock( QWidget *parent, const char *name )
    : QLCDNumber( parent, name ), showingColon( TRUE )
{
    showTime();
    startTimer( 1000 );
}

void Clock::timerEvent( QTimerEvent * )
{
    showTime();
}

void Clock::showTime()
{
    QString time = QTime::currentTime().toString().left( 5 );
    if ( !showingColon )
	time[2] = ' ';
    display( time );
    showingColon = !showingColon;
}
