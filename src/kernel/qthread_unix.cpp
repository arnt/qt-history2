/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_unix.cpp#4 $
**
** QThread class for Unix
**
** Created : 931107
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qthread.h"
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <qlist.h>
#include <unistd.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qobject.h>

class QMutexPrivate {

public:

    pthread_mutex_t mymutex;
    QMutexPrivate();
    ~QMutexPrivate();

};

class QThreadPrivate {
    
public:
    
};

class QThreadEventPrivate {
    
public:
    
};

QMutexPrivate::QMutexPrivate()
{
    int ret = pthread_mutex_init( &mymutex, 0 );
    if( ret ) {
	qFatal( "Mutex init failure %s", strerror( ret ) );
    }
}

QMutexPrivate::~QMutexPrivate()
{
    int ret = pthread_mutex_destroy( &mymutex );
    if( ret ) {
	qWarning( "Mutex destroy failure %s", strerror( ret ) );
    }
}


QMutex::QMutex()
{
    d = new QMutexPrivate();
}

QMutex::~QMutex()
{
    delete d;
}

void QMutex::lock()
{
    int ret = pthread_mutex_lock( &( d->mymutex ) );
    if( ret ) {
	qFatal( "Mutex lock failure %s\n", strerror( ret ) );
    }
}

void QMutex::unlock()
{
    int ret = pthread_mutex_unlock( &( d->mymutex ) );
    if( ret ) {
        qFatal( "Mutex unlock failure %s\n", strerror( ret ) );
    }
}

bool QMutex::locked()
{
    return true;
}


class QThreadQtEvent {

public:

    QObject * o;
    QEvent * e;

};

class QThreadEventsPrivate : public QObject {

    Q_OBJECT

public:

    QThreadEventsPrivate();

    QList<QThreadQtEvent> myevents;
    QMutex myeventmutex;

public slots:

    void sendEvents();

private:

};

#include "qthread_unix.moc"

QThreadEventsPrivate::QThreadEventsPrivate()
{
    myevents.setAutoDelete( TRUE );
    connect( qApp, SIGNAL( guiThreadAwake() ), this, SLOT( sendEvents() ) );
}

void QThreadEventsPrivate::sendEvents()
{
    myeventmutex.lock();
    QThreadQtEvent * qte;
    for( qte = myevents.first(); qte != 0; qte = myevents.next() ) {
	qApp->postEvent( qte->o, qte->e );
    }
    myevents.clear();
    qApp->sendPostedEvents();
    myeventmutex.unlock();
}

static QThreadEventsPrivate * qthreadeventsprivate = 0;

extern "C" {
    static void * start_thread(QThread * t) {
	t->run();
	return 0;
    }
}

int QThread::currentThread()
{
    // A pthread_t is an int
    return pthread_self();
}

void QThread::postEvent( QObject * o, QEvent * e )
{
    if( !qthreadeventsprivate ) {
	qthreadeventsprivate = new QThreadEventsPrivate();
    }
    qthreadeventsprivate->myeventmutex.lock();
    QThreadQtEvent * qte = new QThreadQtEvent;
    qte->o = o;
    qte->e = e;
    qthreadeventsprivate->myevents.append( qte );
    qthreadeventsprivate->myeventmutex.unlock();
    qApp->wakeUpGuiThread();
}

void QThread::wait(const QThread &)
{
}

void QThread::yield()
{
}

void * QThread::threadData()
{
    return 0;
}

void QThread::setThreadData(void *)
{
}

unsigned int QThread::threadId()
{
    return 0;
}

QThread::QThread()
{
}

QThread::~QThread()
{
}

void QThread::start()
{
  // Error checking would be good
  pthread_t foo;
  pthread_create(&foo,0,start_thread,(void *)this);
  pthread_detach(foo);
}

void QThread::run()
{
    // Default implementation does nothing
}

QThreadEvent::QThreadEvent()
{
}

QThreadEvent::~QThreadEvent()
{
}

void QThreadEvent::wait()
{
}

void QThreadEvent::wait(const QTime & t)
{
}

void QThreadEvent::wakeOne()
{
}

void QThreadEvent::wakeAll()
{
}
