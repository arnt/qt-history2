/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_win.cpp#1 $
**
** QThread class for windows
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifdef QT_THREAD_SUPPORT

#include "qthread.h"
#include "qt_windows.h"
#include "qapplication.h"
#include "qobject.h"

class QMutexPrivate {
};

class QThreadPrivate {

public:

};

class QThreadEventPrivate {

public:

};

// Stub implementation

QMutex::QMutex()
{
}

QMutex::~QMutex()
{
}

void QMutex::lock()
{
}

void QMutex::unlock()
{
}

bool QMutex::locked()
{
    return true;
}

MUTEX_HANDLE QMutex::handle()
{
    return 0;
}

class Q_EXPORT QThreadQtEvent {

public:

    QObject * o;
    QEvent * e;

};

class Q_EXPORT QThreadEventsPrivate : public QObject {

    Q_OBJECT

public:

    QThreadEventsPrivate();

    QList<QThreadQtEvent> myevents;
    QMutex myeventmutex;

    void add(QThreadQtEvent *);

public slots:

    void sendEvents();

private:

};

#include "qthread_win.moc"

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

void QThreadEventsPrivate::add(QThreadQtEvent * e)
{
   myevents.append( e );
}

static QThreadEventsPrivate * qthreadeventsprivate = 0;


extern "C" static unsigned long start_thread(QThread * t)
{
  t->run();
  return 0;
}

THREAD_HANDLE QThread::currentThread()
{
  return 0;
}

void QThread::postEvent(QObject * o,QEvent * e)
{
    if( !qthreadeventsprivate ) {
        qthreadeventsprivate = new QThreadEventsPrivate();
    }
    qthreadeventsprivate->myeventmutex.lock();
    QThreadEvent * qte;
    qte = new QThreadQtEvent();
    qte->o = o;
    qte->e = e;
    qthreadeventsprivate->add( qte );
    qthreadeventsprivate->myeventmutex.unlock();
    qApp->wakeUpGuiThread();

}

void QThread::wait(QThread &)
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

THREAD_HANDLE QThread::handle()
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
  //_beginthread(start_thread,0,(void *)this);
  unsigned long threadid;
  if( CreateThread(0,0, (LPTHREAD_START_ROUTINE) start_thread, (void *) this,
		   0,&threadid) == 0 ) {
    qFatal("Eek! Couldn't make thread!");
  }
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
}

QThreadEvent::~QThreadEvent()
{
}

void QThreadEvent::wait()
{
}

void QThreadEvent::wait(QTime &)
{
}

void QThreadEvent::wakeOne()
{
}

void QThreadEvent::wakeAll()
{
}

THREADEVENT_HANDLE QThreadEvent::handle()
{
    return 0;
}

#endif

