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

class QMutexPrivate
{
public:
    MUTEX_HANDLE that;

    QMutexPrivate();
    ~QMutexPrivate();

    int count;
    THREAD_HANDLE thread;
};

class QThreadPrivate
{
public:
    THREAD_HANDLE handle;
    unsigned long id;
};

class QThreadEventPrivate
{

public:

};

QMutexPrivate::QMutexPrivate()
{
    SECURITY_ATTRIBUTES attr;
    attr.nLength = sizeof(attr);
    attr.bInheritHandle = TRUE;
    attr.lpSecurityDescriptor = NULL;

    that = (MUTEX_HANDLE)CreateMutex( &attr, TRUE, LPCTSTR("qt_mutex") );

    if ( !that )
	qFatal( "Mutex init failure" );

    count = 0;
}

QMutexPrivate::~QMutexPrivate()
{
    if( !CloseHandle( (HANDLE)that ) ) {
	qWarning( "Mutex destroy failure" );
    }
}

// Stub implementation

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
    switch ( WaitForSingleObject( (HANDLE)d->that, INFINITE ) ) {
    case WAIT_ABANDONED:
	break;
    case WAIT_OBJECT_0:
	if ( d->count > 0 && d->thread == QThread::currentThread() ) {
	    d->count++;
	} else {
	    d->count++;
	    d->thread=QThread::currentThread();
	}
	break;
    case WAIT_TIMEOUT:
	break;
    }
}

void QMutex::unlock()
{
    if ( d->thread != QThread::currentThread() ) {
	qWarning( "Attempt to unlock from different thread than locker\n");
	qWarning( "Was locked by %ld, unlock attempt from %ld\n",
		d->thread,QThread::currentThread());
	return;
    }

    d->count--;

    if ( d->count < 1 ) {
	if ( !ReleaseMutex( (HANDLE)d->that ) )
	    qFatal( "Mutex unlock failure" );
	d->count = 0;
    }
}

bool QMutex::locked()
{
    qDebug("QMutex::locked()");
    return true;
}

MUTEX_HANDLE QMutex::handle()
{
    qDebug("QMutex::handle()");
    return 0;
}

class QThreadQtEvent {

public:

    QObject * o;
    QEvent * e;

};

class QThreadEventsPrivate : public QObject
{
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
    qDebug("Thread::QThreadEventsPrivate()");
    myevents.setAutoDelete( TRUE );
    connect( qApp, SIGNAL( guiThreadAwake() ), this, SLOT( sendEvents() ) );
}

void QThreadEventsPrivate::sendEvents()
{
    qDebug("Thread::sendEvents()");
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
    qDebug("Thread::add()");
    myevents.append( e );
}

static QThreadEventsPrivate * qthreadeventsprivate = 0;


extern "C" static unsigned long start_thread(QThread * t)
{
    qDebug("Thread::start_thread()");
    t->run();
    return 0;
}

THREAD_HANDLE QThread::currentThread()
{
    return (THREAD_HANDLE)GetCurrentThread();
}

void QThread::postEvent(QObject * o,QEvent * e)
{
    qDebug("Thread::postEvent()");
    if( !qthreadeventsprivate ) {
        qthreadeventsprivate = new QThreadEventsPrivate();
    }
    qthreadeventsprivate->myeventmutex.lock();
    QThreadQtEvent * qte;
    qte = new QThreadQtEvent();
    qte->o = o;
    qte->e = e;
    qthreadeventsprivate->add( qte );
    qthreadeventsprivate->myeventmutex.unlock();
    qApp->wakeUpGuiThread();

}

THREAD_HANDLE QThread::handle()
{
    qDebug("QThread::handle()");
    return d->handle;
}

QThread::QThread()
{
    qDebug("QThread::QThread()");
    d = new QThreadPrivate();
}

QThread::~QThread()
{
    qDebug("QThread::~QThread()");
}

void QThread::start()
{
    d->handle = (THREAD_HANDLE) CreateThread(0,0, (LPTHREAD_START_ROUTINE) start_thread,
	this,
	0,
	&(d->id) );

    if ( !d->handle )
	qFatal("Eek! Couldn't make thread!");
}

void QThread::wait()
{
    qDebug("Thread::wait()");
}

void QThread::run()
{
    qDebug("Thread::run()");
    // Default implementation does nothing
}

void QThread::runWrapper()
{
    qDebug("Thread::runWrapper()");
    run();
    // Tell any threads waiting for us to wake up
//    d->thread_done.wakeAll();
}

QThreadEvent::QThreadEvent()
{
    d = new QThreadEventPrivate();
}

QThreadEvent::~QThreadEvent()
{
    delete d;
}

void QThreadEvent::wait()
{
    qDebug("QThreadEvent::wait()");
}

void QThreadEvent::wait( const QTime & )
{
    qDebug("QThreadEvent::wait(const QTime &)");
}

void QThreadEvent::wakeOne()
{
    qDebug("QThreadEvent::wakeOne()");
//    SwitchToThread(); only supported on NT
}

void QThreadEvent::wakeAll()
{
    qDebug("QThreadEvent::wakeAll()");
}

THREADEVENT_HANDLE QThreadEvent::handle()
{
    qDebug("QThreadEvent::handle()");
    return 0;
}

class QThreadDataPrivate {

public:
    
    pthread_key_t mykey;
    
};

QThreadData::QThreadData()
{
    d=new QThreadDataPrivate;
}

QThreadData::~QThreadData()
{
    delete d;
}

void QThreadData::setData(void * v)
{
}

void * QThreadData::data()
{
}

#endif

