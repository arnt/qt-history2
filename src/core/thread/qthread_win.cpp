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

#include <qcoreapplication.h>
#include <qpointer.h>

#include <windows.h>


#ifndef Q_OS_TEMP
#ifndef _MT
#define _MT
#endif
#include <process.h>
#endif


static QThreadInstance main_instance = {
    0, { 0, &main_instance }, 0, 0, 1, 0, 0, 0, 0
};


extern QMutexPool *static_qt_global_mutexpool; // in qmutexpool.cpp
static QMutexPool *qt_thread_mutexpool = 0;


static DWORD qt_tls_index = TLS_OUT_OF_INDEXES;
static void create_tls()
{
    if (qt_tls_index != TLS_OUT_OF_INDEXES) return;

    static QMutex mutex;
    mutex.lock();
    qt_tls_index = TlsAlloc();
    mutex.unlock();
}


/**************************************************************************
 ** QThreadInstance
 *************************************************************************/

QThreadInstance *QThreadInstance::current()
{
    QThreadInstance *ret = 0;
    if (qt_tls_index != TLS_OUT_OF_INDEXES)
	ret = (QThreadInstance *)TlsGetValue(qt_tls_index);
    if (!ret)
	return &main_instance;
    return ret;
}

void QThreadInstance::init(unsigned int stackSize)
{
    stacksize = stackSize;
    args[0] = args[1] = args[2] = 0;
    thread_storage = 0;
    finished = FALSE;
    running = FALSE;
    orphan = FALSE;

    handle = 0;
    id = 0;
    waiters = 0;

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

    QPointer<QThread> thr = reinterpret_cast<QThread *>(arg[0]);
    arg[2] = reinterpret_cast<Qt::HANDLE>(thr->thread());
    thr->QObject::setThread(QThread::currentThread());
    emit thr->started();
    thr->run();
    if (thr) {
	emit thr->finished();
	QCoreApplication::sendPostedEvents();
	thr->QObject::setThread(reinterpret_cast<Qt::HANDLE>(arg[2]));
	arg[0] = arg[1] = arg[2] = 0;
    }

    finish( (QThreadInstance *) arg[1] );

    return 0;
}

void QThreadInstance::finish( QThreadInstance *d )
{
    if ( ! d ) {
	qWarning( "QThread: internal error: zero data for running thread." );
	return;
    }

    QMutexLocker locker( d->mutex() );
    d->running = FALSE;
    d->finished = TRUE;
    d->args[0] = d->args[1] = d->args[2] = 0;

    QThreadStorageData::finish( d->thread_storage );
    d->thread_storage = 0;
    d->id = 0;

    if (!d->waiters) {
	CloseHandle(d->handle);
	d->handle = 0;
    }

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

    if (args[0] && args[1] && args[2]) {
	QThread *thr = reinterpret_cast<QThread *>(args[0]);
	Qt::HANDLE old = reinterpret_cast<Qt::HANDLE>(args[2]);
	emit thr->terminated();
	thr->QObject::setThread(old);
    }

    running = FALSE;
    finished = TRUE;
    args[0] = args[1] = args[2] = 0;
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

    static_qt_global_mutexpool = new QMutexPool(true);
    qt_thread_mutexpool = new QMutexPool(false);

    create_tls();
}

void QThread::cleanup()
{
    delete static_qt_global_mutexpool;
    delete qt_thread_mutexpool;
    static_qt_global_mutexpool = 0;
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
	qWarning( "Thread is already running" );
	wait();
    }

    d->running = TRUE;
    d->finished = FALSE;
    d->args[0] = this;
    d->args[1] = d;
    d->args[2] = 0;

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
	qSystemWarning("Failed to create thread");

	d->running = FALSE;
	d->finished = TRUE;
	d->args[0] = d->args[1] = d->args[2] = 0;
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
	qSystemWarning( "Failed to set thread priority" );
    }

    if (ResumeThread(d->handle) == 0xffffffff) {
	qSystemWarning( "Failed to resume newly created thread" );
    }
}


bool QThread::wait( unsigned long time )
{
    QMutexLocker locker(d->mutex());

    if ( d->id == GetCurrentThreadId() ) {
	qWarning( "Thread tried to wait on itself" );
	return FALSE;
    }
    if ( d->finished || !d->running )
	return TRUE;

    ++d->waiters;
    locker.mutex()->unlock();

    bool ret = FALSE;
    switch ( WaitForSingleObject( d->handle, time ) ) {
    case WAIT_OBJECT_0:
	ret = TRUE;
	break;
    case WAIT_ABANDONED:
    case WAIT_FAILED:
	qSystemWarning( "Thread wait failure" );
    case WAIT_TIMEOUT:
    default:
	break;
    }

    locker.mutex()->lock();
    --d->waiters;

    if (d->finished && !d->waiters) {
	CloseHandle(d->handle);
	d->handle = 0;
    }

    return ret;
}

void QThread::exit()
{
    QThreadInstance *d = QThreadInstance::current();

    if ( ! d ) {
	qWarning( "QThread::exit() called without a QThread instance." );
	_endthreadex(0);
	return;
    }

    QThreadInstance::finish(d);
    _endthreadex(0);
}
