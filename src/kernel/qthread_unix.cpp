/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_unix.cpp#4 $
**
** QThread class for Unix
**
** Created : 931107
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#ifdef QT_THREAD_SUPPORT

#define _GNU_SOURCE

#include "qthread.h"
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <qlist.h>
#include <unistd.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qobject.h>
#include <sys/time.h>

class QMutexPrivate {

public:

    pthread_mutex_t mymutex;
    QMutexPrivate();
    ~QMutexPrivate();

    int count;
    THREAD_HANDLE thread;

};

class QThreadPrivate {

public:

    pthread_t mythread;
    QThreadEvent thread_done;      // Used for QThread::wait()
    pthread_key_t mykey;

};

class QThreadEventPrivate {

public:

    pthread_cond_t mycond;
    QMutex m;

    QThreadEventPrivate();
    ~QThreadEventPrivate();

};

QMutexPrivate::QMutexPrivate()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK);
    int ret = pthread_mutex_init( &mymutex, &attr );

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

QThreadEventPrivate::QThreadEventPrivate()
{
    int ret=pthread_cond_init(&mycond,0);
    if( ret ) {
	qFatal( "Thread event init failure %s", strerror( ret ) );
    }
}

QThreadEventPrivate::~QThreadEventPrivate()
{
    int ret=pthread_cond_destroy(&mycond);
    if( ret ) {
	qFatal( "Thread event init failure %s", strerror( ret ) );
    }
}

QMutex::QMutex()
{
    d = new QMutexPrivate();
}

QMutex::~QMutex()
{
    //delete d;
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
    int ret = pthread_mutex_trylock( &(d->mymutex) );
    if(ret==EBUSY) {
	return true;
    } else if(ret) {
	qFatal( "Mutex try lock failure %s\n", strerror( ret ) );
    }
    return false;
}

MUTEX_HANDLE QMutex::handle()
{
    // Eeevil?
    return (unsigned long) &( d->mymutex );
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
    static void * start_thread(QThread * t)
    {
	t->runWrapper();
	return 0;
    }

    void destruct_dummy(void *)
    {
    }
}


THREAD_HANDLE QThread::currentThread()
{
    // A pthread_t is an int
    return (THREAD_HANDLE)pthread_self();
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

void QThread::yield()
{
    // Do nothing. This is a real OS.
}

// Do we want to support more than one thread data?
void * QThread::threadData()
{
  return pthread_getspecific( d->mykey );
}

void QThread::setThreadData(void * v)
{
  int ret=pthread_setspecific( d->mykey, v);
  if(ret) {
      qWarning("Error setting thread data: %s",strerror(ret));
  }
}

THREAD_HANDLE QThread::handle()
{
    return d->mythread;
}

QThread::QThread()
{
    // Hmm. Not sure how to provide proper cleanup function here
    d=new QThreadPrivate;
    int ret=pthread_key_create(& ( d->mykey ), destruct_dummy);
    if(ret) {
	qFatal("Thread key create error: %s",strerror(ret));
    }
}

QThread::~QThread()
{
    int ret=pthread_key_delete( d->mykey );
    if(ret) {
	qWarning("Thread key destroy error: %s",strerror(ret));
    }
    delete d;
}

void QThread::wait()
{
    d->thread_done.wait();
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

void QThread::runWrapper()
{
    run();
    // Tell any threads waiting for us to wake up
    d->thread_done.wakeAll();
}

QThreadEvent::QThreadEvent()
{
    d=new QThreadEventPrivate;
}

QThreadEvent::~QThreadEvent()
{
    delete d;
}

void QThreadEvent::wait()
{
    d->m.lock();
    int ret=pthread_cond_wait (&( d->mycond ),
			       ( pthread_mutex_t * )( d->m.handle() ));
    d->m.unlock();
    if(ret) {
	qWarning("Threadevent wait error:%s",strerror(ret));
    }
}

void QThreadEvent::wait(const QTime & t)
{
    // This is probably grotty
    timeval now;
    timespec ti;
    gettimeofday(&now,0);
    ti.tv_sec=now.tv_sec+t.second();
    ti.tv_nsec=((now.tv_usec/1000)+t.msec())*1000000;
    d->m.lock();
    int ret=pthread_cond_timedwait (&( d->mycond ),
				    ( pthread_mutex_t * )( d->m.handle() ),
				    &ti);
    d->m.unlock();
    if(ret) {
	qWarning("Threadevent timed wait error:%s",strerror(ret));
    }
}

void QThreadEvent::wakeOne()
{
    int ret=pthread_cond_signal(& (d->mycond) );
    if(ret) {
	qFatal("Threadevent wakeOne error: %s\n",strerror(ret));
    }
}

void QThreadEvent::wakeAll()
{
    int ret=pthread_cond_broadcast(& (d->mycond) );
    if(ret) {
	qFatal("Threadevent wakeAll error: %s\n",strerror(ret));
    }
}

THREADEVENT_HANDLE QThreadEvent::handle()
{
    return (unsigned long) &( d->mycond );
}

#endif
