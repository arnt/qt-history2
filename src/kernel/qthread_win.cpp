/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_win.cpp#1 $
**
** QThread class for windows
**
** Created : 20000913
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qthread.h"
#include "qt_windows.h"

#if defined(QT_THREAD_SUPPORT)

#include "qwindowdefs.h"
#include "qobject.h"
#include "qapplication.h"
#include "qintdict.h"
#include <process.h>

#ifdef QT_CHECK_RANGE
#define QMUTEX_TYPE_NORMAL 0
#define QMUTEX_TYPE_RECURSIVE 1
#endif


/*
  QCriticalSection
*/

class QCriticalSection
{
public:
    QCriticalSection()
    {
	InitializeCriticalSection(&section);
    }
    ~QCriticalSection()
    {
	DeleteCriticalSection(&section);
    }
    void enter()
    {
	EnterCriticalSection(&section);
    }
    void leave()
    {
	LeaveCriticalSection(&section);
    }

private:
    CRITICAL_SECTION section;
};

/*
  Private - implements a recursive mutex
*/

class QMutex::Private
{
public:
    Qt::HANDLE handle;

    Private();
    virtual ~Private();
    virtual void lock();
    virtual void unlock();
    virtual bool locked();
    virtual bool trylock();
#ifdef QT_CHECK_RANGE
    virtual int type() const { return QMUTEX_TYPE_RECURSIVE; }
#endif
};

QMutex::Private::Private()
{
    if ( qWinVersion() & Qt::WV_NT_based )
	handle = CreateMutex( NULL, FALSE, NULL );
    else
	handle = CreateMutexA( NULL, FALSE, NULL );
#ifdef QT_CHECK_RANGE
    if ( !handle )
	qSystemWarning( "Mutex init failure" );
#endif

}

QMutex::Private::~Private()
{
    if ( !CloseHandle( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex destroy failure" );
#endif
    }
}

void QMutex::Private::lock()
{
    switch ( WaitForSingleObject( handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex lock failure" );
#endif
	break;
    case WAIT_ABANDONED:
#ifdef QT_CHECK_RANGE
	qWarning( "Thread terminated while locking mutex!" );
#endif
	// Fall through
    default:
	break;
    }
}

void QMutex::Private::unlock()
{
    if ( !ReleaseMutex( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex unlock failure" );
#endif
    }
}

bool QMutex::Private::locked()
{
    switch ( WaitForSingleObject( handle, 0) ) {
    case WAIT_TIMEOUT:
	return TRUE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    case WAIT_OBJECT_0:
	return FALSE;
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

bool QMutex::Private::trylock()
{
    switch ( WaitForSingleObject( handle, 0) ) {
    case WAIT_FAILED:
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    case WAIT_OBJECT_0:
	break;
    default:
	break;
    }
    return TRUE;
}

/*
  QNonRecursiveMutexPrivate - implements a non-recursive mutex
*/

class QNonRecursiveMutexPrivate : public QMutex::Private
{
public:
    QNonRecursiveMutexPrivate();
    void lock();
    void unlock();
    bool trylock();
#ifdef QT_CHECK_RANGE
    virtual int type() const { return QMUTEX_TYPE_NORMAL; };
#endif

    unsigned int threadID;
    QCriticalSection protect;
};

QNonRecursiveMutexPrivate::QNonRecursiveMutexPrivate()
    : Private()
{
    threadID = 0;
}

void QNonRecursiveMutexPrivate::lock()
{
    protect.enter();

    if ( threadID == GetCurrentThreadId() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Non-recursive mutex already locked by this thread" );
#endif
    } else {
	protect.leave();
	switch ( WaitForSingleObject( handle, INFINITE ) ) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
    #ifdef QT_CHECK_RANGE
	    qSystemWarning( "Mutex lock failure" );
    #endif
	    break;
	case WAIT_ABANDONED:
    #ifdef QT_CHECK_RANGE
	    qWarning( "Thread terminated while locking mutex!" );
    #endif
	    // Fall through
	default:
	    protect.enter();
	    threadID = GetCurrentThreadId();
	    protect.leave();
	    break;
	}
    }
}

void QNonRecursiveMutexPrivate::unlock()
{
    protect.enter();
    Private::unlock();
    threadID = 0;
    protect.leave();
}

bool QNonRecursiveMutexPrivate::trylock()
{
    protect.enter();

    if (threadID == GetCurrentThreadId()) {
	// locked by this thread already, return FALSE
	return FALSE;
    }

    protect.leave();

    switch (WaitForSingleObject(handle, 0)) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
	return FALSE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    default:
	protect.enter();
	threadID = GetCurrentThreadId();
	protect.leave();
	break;
    }

    return TRUE;
}

/*
  QMutex implementation
*/

QMutex::QMutex(bool recursive)
{
    if ( recursive )
	d = new Private();
    else
	d = new QNonRecursiveMutexPrivate();
}

QMutex::~QMutex()
{
    delete d;
}

void QMutex::lock()
{
    d->lock();
}

void QMutex::unlock()
{
    d->unlock();
}

bool QMutex::locked()
{
    return d->locked();
}

bool QMutex::tryLock()
{
    return d->trylock();
}

/*
  QThreadQtEvent
*/

class QThreadQtEvent
{
public:
    QThreadQtEvent(QObject *o, QEvent *e)
	: object(o), event(e)
    {}
    QObject *object;
    QEvent *event;
};

/*
  QThreadEventsPrivate
*/

class QThreadEventsPrivate : public QObject
{
    Q_OBJECT
public:
    QThreadEventsPrivate();
    QList<QThreadQtEvent> events;
    QCriticalSection protect;
    void add(QThreadQtEvent *);
public slots:
    void sendEvents();
private:
};

static QThreadEventsPrivate * qthreadEventsPrivate = 0;

QThreadEventsPrivate::QThreadEventsPrivate()
{
    events.setAutoDelete( TRUE );
    connect( qApp, SIGNAL( guiThreadAwake() ), this, SLOT( sendEvents() ) );
}

void QThreadEventsPrivate::sendEvents()
{
    protect.enter();
    QThreadQtEvent * qte;
    for( qte = events.first(); qte != 0; qte = events.next() )
	qApp->postEvent( qte->object, qte->event );
    events.clear();
    protect.leave();
}

void QThreadEventsPrivate::add(QThreadQtEvent * e)
{
    events.append( e );
}

/*
  QWaitConditionPrivate
*/

class QWaitConditionPrivate
{
public:
    QWaitConditionPrivate();
    ~QWaitConditionPrivate();

    int wait( unsigned long time = ULONG_MAX , bool countWaiter = TRUE);
    Qt::HANDLE handle;
    Qt::HANDLE single;
    QCriticalSection s;
    int waitersCount;
};

QWaitConditionPrivate::QWaitConditionPrivate()
: waitersCount(0)
{
    if ( qWinVersion() & Qt::WV_NT_based ) {
	handle = CreateEvent( NULL, TRUE, FALSE, NULL );
	single = CreateEvent( NULL, FALSE, FALSE, NULL );
    } else {
	handle = CreateEventA( NULL, TRUE, FALSE, NULL );
	single = CreateEventA( NULL, FALSE, FALSE, NULL );
    }

#ifdef QT_CHECK_RANGE
    if ( !handle || !single )
    qSystemWarning( "Condition init failure" );
#endif
}

QWaitConditionPrivate::~QWaitConditionPrivate()
{
    if ( !CloseHandle( handle ) || !CloseHandle( single ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Condition destroy failure" );
#endif
    }
}

int QWaitConditionPrivate::wait( unsigned long time , bool countWaiter )
{
    s.enter();
    if ( countWaiter )
	waitersCount++;
    Qt::HANDLE hnds[2] = { handle, single };
    s.leave();
    int ret = WaitForMultipleObjects( 2, hnds, FALSE, time );
    s.enter();
    waitersCount--;
    s.leave();
    return ret;
}

/*
  QWaitCondition implementation
*/

QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate();
}

QWaitCondition::~QWaitCondition()
{
    delete d;
}

bool QWaitCondition::wait( unsigned long time )
{
    switch ( d->wait(time) ) {
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED:
    case WAIT_ABANDONED+1:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
    qSystemWarning( "Condition wait failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

bool QWaitCondition::wait( QMutex *mutex, unsigned long time)
{
    if ( !mutex )
       return FALSE;

    d->s.enter();
#ifdef QT_CHECK_RANGE
    if ( mutex->d->type() == QMUTEX_TYPE_RECURSIVE )
	qWarning("Unlocking recursive mutex before wait");
#endif
    d->waitersCount++;
    d->s.leave();
    mutex->unlock();
    int result = d->wait(time, FALSE);
    mutex->lock();
    d->s.enter();
    bool lastWaiter = ( (result == WAIT_OBJECT_0 )  && d->waitersCount == 0 ); // last waiter on waitAll?
    d->s.leave();
    if ( lastWaiter ) {
	if ( !ResetEvent ( d->handle ) ) {
    #ifdef QT_CHECK_RANGE
	qSystemWarning( "Condition could not be reset" );
    #endif
	}
    }
    return TRUE;
}

void QWaitCondition::wakeOne()
{
    d->s.enter();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
	if ( !SetEvent( d->single ) ) {
    #ifdef QT_CHECK_RANGE
	    qSystemWarning( "Condition could not be set" );
    #endif
	}
    }
    d->s.leave();
}

void QWaitCondition::wakeAll()
{
    d->s.enter();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
	if ( !PulseEvent( d->handle ) ) {
    #ifdef QT_CHECK_RANGE
	qSystemWarning( "Condition could not be set" );
    #endif
	}
    }
    d->s.leave();
}

/*
  Private
*/

class QThreadPrivate
{
public:
    static void internalRun( QThread* );

    Private();
    ~Private();

    Qt::HANDLE handle;
    unsigned int id;
    bool finished  : 1;
    bool running   : 1;
};

#if defined(Q_C_CALLBACKS)
extern "C"
#endif
static unsigned int __stdcall start_thread(void* that )
{
    QThreadPrivate::internalRun( (QThread*)that );

    return 0;
}

void QThreadPrivate::internalRun( QThread* that )
{
    that->run();
    that->d->finished = TRUE;
    that->d->running = FALSE;
}

QThreadPrivate::QThreadPrivate()
{
    handle = 0;
    running = FALSE;
    finished = FALSE;
}

QThreadPrivate::~QThreadPrivate()
{
    if ( running && !finished ) {
	qWarning("QThread object destroyed while thread is still running. Terminating thread...");
	TerminateThread( handle, 0 );
	finished = TRUE;
    } else if ( handle && !CloseHandle( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning("Thread destroy failure");
#endif
    }
}

/*
  QThread static functions
*/

Qt::HANDLE QThread::currentThread()
{
    return GetCurrentThread();
}

void QThread::postEvent( QObject *o,QEvent *e )
{
    if( !qthreadEventsPrivate )
	qthreadEventsPrivate = new QThreadEventsPrivate();
    qthreadEventsPrivate->protect.enter();
    qthreadEventsPrivate->add( new QThreadQtEvent(o,e)  );
    qthreadEventsPrivate->protect.leave();
    qApp->wakeUpGuiThread();
}

void QThread::exit()
{
    _endthreadex(0);
}

void QThread::sleep( unsigned long secs )
{
    ::Sleep( secs * 1000 );
}

void QThread::msleep( unsigned long msecs )
{
    ::Sleep( msecs );
}

void QThread::usleep( unsigned long usecs )
{
    ::Sleep( ( usecs / 1000 ) + 1 );
}

/*
  QThread implementation
*/

QThread::QThread()
{
    d = new QThreadPrivate;
}

QThread::~QThread()
{
    if( d->running && !d->finished ) {
	qWarning("QThread object destroyed while thread is still running.");
    } else {
	delete d;
    }
}

void QThread::start()
{
    if ( d->running && !d->finished ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Thread is already running" );
#endif
	wait();
    }

    d->running = TRUE;
    d->finished = FALSE;
    d->handle = (Qt::HANDLE)_beginthreadex( NULL, NULL, start_thread,
	this, 0, &(d->id) );

#ifdef QT_CHECK_RANGE
    if ( !d->handle )
	qSystemWarning( "Couldn't create thread" );
#endif
}

bool QThread::wait( unsigned long time )
{
    if ( d->id == GetCurrentThreadId() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Thread tried to wait on itself" );
#endif
	return FALSE;
    }
    if ( d->finished || !d->running )
	return TRUE;
    switch ( WaitForSingleObject( d->handle, time ) ) {
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED_0:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Thread wait failure" );
#endif
	return FALSE;
    default:
	break;
    }
    return TRUE;
}

bool QThread::finished() const
{
    return d->finished;
}

bool QThread::running() const
{
    return d->running;
}

/*
  QSemaphore implementation
*/

class QSemaphore::Private
{
public:
    Qt::HANDLE handle;
    int count;
    int maxCount;
    QCriticalSection protect;
    QWaitCondition dontBlock;
};

QSemaphore::QSemaphore( int maxcount )
{
    d = new Private;
    d->maxCount = maxcount;
    if ( qWinVersion() & Qt::WV_NT_based ) {
	d->handle = CreateSemaphore( NULL, maxcount, maxcount, NULL );
    } else {
	d->handle = CreateSemaphoreA( NULL, maxcount, maxcount, NULL );
    }

#ifdef QT_CHECK_RANGE
    if ( !d->handle )
	qSystemWarning( "Semaphore init failure" );
#endif
    d->count = maxcount;
}

QSemaphore::~QSemaphore()
{
    if ( !CloseHandle( d->handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore close failure" );
#endif
    }
    delete d;
}

int QSemaphore::available() const
{
    return d->count;
}

int QSemaphore::total() const
{
    return d->maxCount;
}

int QSemaphore::operator--(int)
{
    d->protect.enter();
    if ( d->count == d->maxCount ) {
	d->protect.leave();
	return d->count;
    }

    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, 1, NULL ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
    } else {
	c = ++d->count;
	d->dontBlock.wakeAll();
    }
    d->protect.leave();

    return c;
}

int QSemaphore::operator -=(int s)
{
    d->protect.enter();
    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, s, NULL ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
    } else {
	d->dontBlock.wakeAll();
	d->count += s;
	if ( d->count > d->maxCount )
	    d->count = d->maxCount;
	c = d->count;
    }
    d->protect.leave();

    return c;
}

int QSemaphore::operator++(int)
{
    switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore wait failure" );
#endif
	return d->count;
    default:
	break;
    }
    d->protect.enter();
    int c = --d->count;
    d->protect.leave();

    return c;
}

int QSemaphore::operator +=(int s)
{
    while ( d->count < s )
	d->dontBlock.wait();

    d->protect.enter();
    for ( int i = 0; i < s; i++ ) {
	switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Semaphore wait failure" );
#endif
	    return d->count;
	default:
	    break;
	}
	d->count--;
    }
    int c = d->count;
    d->protect.leave();

    return c;
}

#include "qthread_win.moc"

#endif
