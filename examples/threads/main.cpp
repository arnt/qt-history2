/****************************************************************************
** $Id: //depot/qt/main/examples/threads/main.cpp#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/


#include <qthread.h>

QMutex mutex;

class PingPong : public QThread
{
protected:
    virtual void run();
};

void PingPong::run()
{
    for ( int i = 0; i < 5; i++ ) {
	mutex.lock();
	qWarning("Ping");
	qWarning("Pong");
	mutex.unlock();
	
	sleep( 1 );
    }

    mutex.lock();
    qWarning("Thread finished!");
    mutex.unlock();
}

int main()
{
    PingPong first;
    PingPong second;

    first.start();
    second.start();

    first.wait();
    second.wait();
}
