/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qthread.h"
#include "qthreadinstance_p.h"
#include "qmutexpool_p.h"
#include "qthreadstorage.h"

#include <windows.h>


#ifndef Q_OS_TEMP
#ifndef _MT
#define _MT
#endif
#include <process.h>
#endif


static QThreadInstance main_instance = {
    0, { 0, &main_instance }, 0, 0, 1, 0, 0, 0
};


static QMutexPool *qt_thread_mutexpool = 0;


static DWORD qt_tls_index = 0;
static void create_tls()
{
    if ( qt_tls_index ) return;

    static QMutex mutex;
    mutex.lock();
    if ( ! qt_tls_index )
	qt_tls_index = TlsAlloc();
    mutex.unlock();
}


/**************************************************************************
 ** QThreadInstance
 *************************************************************************/

QThreadInstance *QThreadInstance::current()
{
    QThreadInstance *ret = (QThreadInstance *) TlsGetValue( qt_tls_index );
    if ( ! ret ) return &main_instance;
    return ret;
}

void QThreadInstance::init(unsigned int stackSize)
{
    stacksize = stackSize;
    args[0] = args[1] = 0;
    thread_storage = 0;
    finished = FALSE;
    running = FALSE;
    orphan = FALSE;

    handle = 0;
    id = 0;

    // threads have not been initialized yet, do it now
    if ( ! qt_thread_mutexpool ) QThread::initialize();
}

void QThreadInstance::deinit()
{
    }

unsigned int __stdcall QThreadInstance::start( void *_arg )
{
    void **arg = (void **) _arg;

    TlsSetValue( qt_tls_index, arg[1] );

    ( (QThread *) arg[0] )->run();

    finish( (QThreadInstance *) arg[1] );

    return 0;
}

void QThreadInstance::finish( QThreadInstance *d )
{
    if ( ! d ) {
#ifdef QT_CHECK_STATE
	qWarning( "QThread: internal error: zero data for running thread." );
#endif // QT_CHECK_STATE
	return;
    }

    // this is copied to QThread::exit() - if you modify this code,
    // make sure you fix below as well
    QMutexLocker locker( d->mutex() );
    d->running = FALSE;
    d->finished = TRUE;
    d->args[0] = d->args[1] = 0;

    QThreadStorageData::finish( d->thread_storage );
    d->thread_storage = 0;

    d->id = 0;

    if ( d->orphan ) {
        d->deinit();
	delete d;
    }
}

QMutex *QThreadInstance::mutex() const
{
    return qt_thread_mutexpool ? qt_thread_mutexpool->get( (void *) this ) : 0;
}

void QThreadInstance::terminate()
{
    /*
      delete the thread storage *after* the thread has been
      terminated.  we could end up deleting the thread's data while it
      is accessing it (but before the thread stops), which results in
      a crash.
    */
    void **storage = thread_storage;
    thread_storage = 0;

    running = FALSE;
    finished = TRUE;
    args[0] = args[1] = 0;
    id = 0;

    if ( orphan ) {
        deinit();
	delete this;
    }

    TerminateThread( handle, 0 );

    QThreadStorageData::finish( storage );
}


/**************************************************************************
 ** QThread
 *************************************************************************/

Qt::HANDLE QThread::currentThread()
{
    return (Qt::HANDLE)GetCurrentThreadId();
}

void QThread::initialize()
{
    if ( qt_global_mutexpool )
	return;

    qt_global_mutexpool = new QMutexPool(true);
    qt_thread_mutexpool = new QMutexPool(false);

    create_tls();
}

void QThread::cleanup()
{
    delete qt_global_mutexpool;
    delete qt_thread_mutexpool;
    qt_global_mutexpool = 0;
    qt_thread_mutexpool = 0;

    QThreadInstance::finish( &main_instance );
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


void QThread::start(Priority priority)
{
    QMutexLocker locker( d->mutex() );

    if ( d->running && !d->finished ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Thread is already running" );
#endif
	wait();
    }

    d->running = TRUE;
    d->finished = FALSE;
    d->args[0] = this;
    d->args[1] = d;

    /*
      NOTE: we create the thread in the suspended state, set the
      priority and then resume the thread.

      since threads are created with normal priority by default, we
      could get into a case where a thread (with priority less than
      NormalPriority) tries to create a new thread (also with priority
      less than NormalPriority), but the newly created thread preempts
      its 'parent' and runs at normal priority.
    */
    d->handle = (Qt::HANDLE) _beginthreadex(NULL, d->stacksize, QThreadInstance::start,
					    d->args, CREATE_SUSPENDED, &(d->id));

    if ( !d->handle ) {
#ifdef QT_CHECK_STATE
	qSystemWarning("Failed to create thread");
#endif

	d->running = FALSE;
	d->finished = TRUE;
	d->args[0] = d->args[1] = 0;
	return;
    }

    int prio;
    switch (priority) {
    case IdlePriority:
	prio = THREAD_PRIORITY_IDLE;
	break;

    case LowestPriority:
	prio = THREAD_PRIORITY_LOWEST;
	break;

    case LowPriority:
	prio = THREAD_PRIORITY_BELOW_NORMAL;
	break;

    case NormalPriority:
	prio = THREAD_PRIORITY_NORMAL;
	break;

    case HighPriority:
	prio = THREAD_PRIORITY_ABOVE_NORMAL;
	break;

    case HighestPriority:
	prio = THREAD_PRIORITY_HIGHEST;
	break;

    case TimeCriticalPriority:
	prio = THREAD_PRIORITY_TIME_CRITICAL;
	break;

    case InheritPriority:
    default:
	prio = GetThreadPriority(GetCurrentThread());
	break;
    }

    if (! SetThreadPriority(d->handle, prio)) {
#ifdef QT_CHECK_STATE
	qSystemWarning( "Failed to set thread priority" );
#endif
    }

    if (ResumeThread(d->handle) == 0xffffffff) {
#ifdef QT_CHECK_STATE
	qSystemWarning( "Failed to resume newly created thread" );
#endif
    }
}


bool QThread::wait( unsigned long time )
{
    QMutexLocker locker( d->mutex() );

    if ( d->id == GetCurrentThreadId() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Thread tried to wait on itself" );
#endif
	return FALSE;
    }
    if ( d->finished || !d->running )
	return TRUE;
    locker.mutex()->unlock();
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
    locker.mutex()->lock();
    return TRUE;
}

void QThread::exit()
{
    QThreadInstance *d = QThreadInstance::current();

    if ( ! d ) {
#ifdef QT_CHECK_STATE
	qWarning( "QThread::exit() called without a QThread instance." );
#endif // QT_CHECK_STATE

	_endthreadex(0);
	return;
    }

    // this is copied from QThreadInstance::finish() - if you modify
    // this code, make sure you fix above as well
    QMutexLocker locker( d->mutex() );
    d->running = FALSE;
    d->finished = TRUE;
    d->args[0] = d->args[1] = 0;

    QThreadStorageData::finish( d->thread_storage );
    d->thread_storage = 0;

    d->id = 0;

    CloseHandle( d->handle );

    if ( d->orphan ) {
        d->deinit();
	delete d;
    }

    _endthreadex(0);
}
