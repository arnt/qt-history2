/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_win.cpp#1 $
**
** QThread class for windows
**
** Created : 931107
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qthread.h"

#if defined(QT_THREAD_SUPPORT)

#include "qwindowdefs.h"
#include "qt_windows.h"
#include "qobject.h"
#include "qapplication.h"
#include "qintdict.h"
#include <process.h>

void qSystemWarning( const QString& message )
{
    int error = GetLastError();
    TCHAR* string;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			  NULL,
			  error,
			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			  (LPTSTR)&string,
			  0,
			  NULL );

    qWarning( message + QString("\tError code %1 - %2").arg( error ).arg( qt_winQString(string) ) );
    LocalFree( (HLOCAL)string );
}

#ifdef CHECK_RANGE
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
        {InitializeCriticalSection(&section);}
    ~QCriticalSection()
        {DeleteCriticalSection(&section);}
    void enter()
        {EnterCriticalSection(&section);}
    void leave()
        {LeaveCriticalSection(&section);}
private:
    CRITICAL_SECTION section;
};

/*
  QMutexPrivate - implements a normal mutex
*/

class QMutexPrivate
{
public:
    HANDLE handle;

    QMutexPrivate();
    virtual ~QMutexPrivate();
    virtual void lock();
    virtual void unlock();
    virtual bool locked();
#ifdef CHECK_RANGE
    virtual int type() { return QMUTEX_TYPE_NORMAL; }
#endif
};

QMutexPrivate::QMutexPrivate()
{
    handle = CreateMutex( NULL, FALSE, NULL );
#ifdef CHECK_RANGE
    if ( !handle )
	qSystemWarning( "Mutex init failure" );
#endif

}

QMutexPrivate::~QMutexPrivate()
{
    if ( !CloseHandle( handle ) ) {
#ifdef CHECK_RANGE
	qSystemWarning( "Mutex destroy failure" );
#endif
    }
}

void QMutexPrivate::lock()
{
    switch ( WaitForSingleObject( handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef CHECK_RANGE
	qSystemWarning( "Couldn't lock mutex" );
#endif
	break;
    case WAIT_ABANDONED:
#ifdef CHECK_RANGE
	qWarning( "Thread terminated while locking mutex!" );
#endif
	// Fall through
    default:
	break;
    }
}

void QMutexPrivate::unlock()
{
    if ( !ReleaseMutex( handle ) ) {
#ifdef CHECK_RANGE
	qSystemWarning( "Mutex unlock failure" );
#endif
    }
}

bool QMutexPrivate::locked()
{
    switch ( WaitForSingleObject( handle, 0) ) {
    case WAIT_TIMEOUT:
	return TRUE;
    case WAIT_ABANDONED_0:
#ifdef CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    case WAIT_OBJECT_0:
	return FALSE;
    case WAIT_FAILED:
#ifdef CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

/*
  QMutexPrivate - implements a recursive mutex
 */

class QRecursiveMutexPrivate : public QMutexPrivate
{
public:
    QRecursiveMutexPrivate();
    virtual void lock();
#ifdef CHECK_RANGE
    virtual int type() { return QMUTEX_TYPE_RECURSIVE; };
#endif
};

QRecursiveMutexPrivate::QRecursiveMutexPrivate()
    : QMutexPrivate()
{
}

void QRecursiveMutexPrivate::lock()
{
    switch ( WaitForSingleObject( handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef CHECK_RANGE
	qSystemWarning( "Couldn't lock mutex" );
#endif
	break;
    case WAIT_ABANDONED:
#ifdef CHECK_RANGE
	qWarning( "Thread terminated while locking mutex!" );
#endif
	// Fall through
    default:
	break;
    }
}

/*
  QMutex implementation
*/

QMutex::QMutex(bool recursive)
{
    if ( recursive )
        d = new QRecursiveMutexPrivate();
    else
       d = new QMutexPrivate();
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
    QMutex eventMutex;
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
    eventMutex.lock();
    QThreadQtEvent * qte;
    for( qte = events.first(); qte != 0; qte = events.next() )
        qApp->postEvent( qte->object, qte->event );
    events.clear();
    eventMutex.unlock();
}

void QThreadEventsPrivate::add(QThreadQtEvent * e)
{
    events.append( e );
}

/*
  QConditionPrivate
*/

class QConditionPrivate
{
public:
    QConditionPrivate();
    ~QConditionPrivate();

    int wait( unsigned long time = ULONG_MAX , bool countWaiter = TRUE);
    HANDLE handle;
    HANDLE single;
    QCriticalSection s;
    int waitersCount;
};

QConditionPrivate::QConditionPrivate()
: waitersCount(0)
{
    handle = CreateEvent( NULL, TRUE, FALSE, NULL );
    single = CreateEvent( NULL, FALSE, FALSE, NULL );
#ifdef CHECK_RANGE
    if ( !handle || !single )
    qSystemWarning( "Condition init failure" );
#endif
}

QConditionPrivate::~QConditionPrivate()
{
    if ( !CloseHandle( handle ) || !CloseHandle( single ) ) {
#ifdef CHECK_RANGE
        qSystemWarning( "Condition destroy failure" );
#endif
    }
}

int QConditionPrivate::wait( unsigned long time , bool countWaiter )
{
    s.enter();
    if ( countWaiter )
        waitersCount++;
    HANDLE hnds[2] = { handle, single };
    s.leave();
    int ret = WaitForMultipleObjects( 2, hnds, FALSE, time );
    s.enter();
    waitersCount--;
    s.leave();
    return ret;
}

/*
  QCondition implementation
*/

QCondition::QCondition()
{
    d = new QConditionPrivate();
}

QCondition::~QCondition()
{
    delete d;
}

bool QCondition::wait( unsigned long time )
{
    switch ( d->wait(time) ) {
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED:
    case WAIT_ABANDONED+1:
    case WAIT_FAILED:
#ifdef CHECK_RANGE
    qSystemWarning( "Condition wait failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

bool QCondition::wait( QMutex *mutex, unsigned long time)
{
    if ( !mutex )
       return FALSE;
    d->s.enter();
#ifdef CHECK_RANGE
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
    #ifdef CHECK_RANGE
        qSystemWarning( "Condition could not be reset" );
    #endif
        }
    }
    return TRUE;
}

void QCondition::wakeOne()
{
    d->s.enter();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
        if ( !SetEvent( d->single ) ) {
    #ifdef CHECK_RANGE
        qSystemWarning( "Condition could not be set" );
    #endif
        }
    }
    d->s.leave();
}

void QCondition::wakeAll()
{
    d->s.enter();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
        if ( !SetEvent( d->handle ) ) {
    #ifdef CHECK_RANGE
        qSystemWarning( "Condition could not be set" );
    #endif
        }
    }
    d->s.leave();
}

/*
  QThreadPrivate
*/

class QThreadPrivate
{
public:
    static void internalRun( QThread* );

    QThreadPrivate();
    ~QThreadPrivate();

    void init( QThread* );

    HANDLE handle;
    unsigned int id;
    bool finished  : 1;
    bool running   : 1;
    bool runonce   : 1;
};

#if !defined(_CC_BOR_)
extern "C"
#endif
static unsigned int __stdcall start_thread(void* that )
{
    QThreadPrivate::internalRun( (QThread*)that );

    return 0;
}

void QThreadPrivate::internalRun( QThread* that )
{
    that->d->finished = FALSE;
    that->d->running = TRUE;
    that->run();
    that->d->finished = TRUE;
    that->d->running = FALSE;
}

QThreadPrivate::QThreadPrivate()
{
    runonce = FALSE;
}

QThreadPrivate::~QThreadPrivate()
{
    if ( !finished ) {
	qWarning("QThread object destroyed while thread is still running. Terminating thread...");
	TerminateThread( handle, 0 );
	finished = TRUE;
    } else if ( !CloseHandle( handle ) ) {
#ifdef CHECK_RANGE
	    qSystemWarning( "Thread destroy failure");
#endif
    }
}

void QThreadPrivate::init( QThread* that )
{
    handle = (HANDLE)_beginthreadex( NULL, NULL, start_thread,
	that,
	CREATE_SUSPENDED,
	&(id) );

#ifdef CHECK_RANGE
    if ( !handle )
	qSystemWarning( "Couldn't create thread" );
#endif
}

/*
  QThread static functions
*/

HANDLE QThread::currentThread()
{
    return GetCurrentThread();
}

void QThread::postEvent( QObject *o,QEvent *e )
{
    if( !qthreadEventsPrivate )
        qthreadEventsPrivate = new QThreadEventsPrivate();
    qthreadEventsPrivate->eventMutex.lock();
    qthreadEventsPrivate->add( new QThreadQtEvent(o,e)  );
    qthreadEventsPrivate->eventMutex.unlock();
    qApp->wakeUpGuiThread();
}

void QThread::exit()
{
    _endthreadex(0);
}

void QThread::sleep( unsigned long s )
{
    ::Sleep( s * 1000 );
}

void QThread::msleep( unsigned long ms )
{
    ::Sleep( ms );
}

void QThread::usleep( unsigned long mys )
{
    ::Sleep( ( mys / 1000 ) + 1 );
}

/*
  QThread implementation
*/

QThread::QThread()
{
    d = new QThreadPrivate;
    d->init( this );
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
    if ( d->runonce )
	d->init( this );

    d->runonce = TRUE;

    if ( ResumeThread( d->handle ) == -1 ) {
#ifdef CHECK_RANGE
	qSystemWarning( "Couldn't start thread" );
#endif
    }
}

bool QThread::wait( unsigned long time )
{
    if ( d->handle == QThread::currentThread() ) {
#ifdef CHECK_RANGE
	qWarning( "Thread tried to wait on itself" );
#endif
	return FALSE;
    }
    if ( d->finished )
	return TRUE;
    switch ( WaitForSingleObject( d->handle, time ) ) {
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED_0:
    case WAIT_FAILED:
#ifdef CHECK_RANGE
	qSystemWarning( "Thread wait failure" );
#endif
	return FALSE;
    default:
	break;
    }
    return TRUE;
}

bool QThread::running() const
{
    return d->running;
}

bool QThread::finished() const
{
    return d->finished;
}

/*
  QSemaphore implementation
*/

class QSemaphorePrivate
{
public:
    HANDLE handle;
    int count;
    int maxCount;
    QMutex countMutex;
    QCondition dontBlock;
};

QSemaphore::QSemaphore( int maxcount )
{
    d = new QSemaphorePrivate;
    d->maxCount = maxcount;
    d->handle = CreateSemaphore( NULL, maxcount, maxcount, NULL );
#ifdef CHECK_RANGE
    if ( !d->handle )
	qSystemWarning( "Semaphore init failure" );
#endif
    d->count = maxcount;
}

QSemaphore::~QSemaphore()
{
    if ( !CloseHandle( d->handle ) ) {
#ifdef CHECK_RANGE
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
    if ( d->count == d->maxCount )
	return d->count;

    d->countMutex.lock();
    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, 1, NULL ) ) {
#ifdef CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
    } else {
	c = ++d->count;
	d->dontBlock.wakeAll();
    }
    d->countMutex.unlock();

    return c;
}

int QSemaphore::operator -=(int s)
{
    d->countMutex.lock();
    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, s, NULL ) ) {
#ifdef CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
    } else {
	d->dontBlock.wakeAll();
	d->count += s;
	if ( d->count > d->maxCount )
	    d->count = d->maxCount;
	c = d->count;
    }
    d->countMutex.unlock();

    return c;
}

int QSemaphore::operator++(int)
{
    switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef CHECK_RANGE
	qSystemWarning( "Semaphore wait failure" );
#endif
	return d->count;
    default:
	break;
    }
    d->countMutex.lock();
    int c = --d->count;
    d->countMutex.unlock();

    return c;
}

int QSemaphore::operator +=(int s)
{
    while ( d->count < s )
	d->dontBlock.wait();

    d->countMutex.lock();
    for ( int i = 0; i < s; i++ ) {
	switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
#ifdef CHECK_RANGE
	    qSystemWarning( "Semaphore wait failure" );
#endif
	    return d->count;
	default:
	    break;
	}
	d->count--;
    }
    int c = d->count;
    d->countMutex.unlock();

    return c;
}

#include "qthread_win.moc"

#endif