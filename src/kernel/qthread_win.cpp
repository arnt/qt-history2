/****************************************************************************
** $Id$
**
** QThread class for windows
**
** Created : 20000913
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#if defined(QT_THREAD_SUPPORT)

#include "qt_windows.h"

#include "qthread.h"
#include <private/qthreadinstance_p.h>
#include <private/qmutexpool_p.h>
#include "qthreadstorage.h"

#include <private/qcriticalsection_p.h>

#ifndef Q_OS_TEMP
#ifndef _MT
#define _MT
#endif
#include <process.h>
#endif


static QThreadInstance main_instance = {
    0, { 0, 0 }, 0, 0, 1, 0, 0, 0
};


static QMutexPool *qt_thread_mutexpool = 0;


static DWORD qt_tls_index = 0;
static void create_tls()
{
    if ( qt_tls_index ) return;

    static QCriticalSection cs;
    cs.enter();
    if ( ! qt_tls_index )
	qt_tls_index = TlsAlloc();
    cs.leave();
}


/**************************************************************************
 ** QThreadInstance
 *************************************************************************/

QThreadInstance *QThreadInstance::current()
{
    create_tls();
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
    thread_id = 0;

    // threads have not been initialized yet, do it now
    if ( ! qt_thread_mutexpool ) QThread::initialize();
}

void QThreadInstance::deinit()
{
    }

unsigned int __stdcall QThreadInstance::start( void *_arg )
{
    void **arg = (void **) _arg;

    create_tls();
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

    d->thread_id = 0;

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
    thread_id = 0;

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
    if ( ! qt_global_mutexpool )
	qt_global_mutexpool = new QMutexPool( TRUE, 73 );
    if ( ! qt_thread_mutexpool )
	qt_thread_mutexpool = new QMutexPool( FALSE, 127 );
}

void QThread::cleanup()
{
    delete qt_global_mutexpool;
    delete qt_thread_mutexpool;
    qt_global_mutexpool = 0;
    qt_thread_mutexpool = 0;

    QThreadInstance::finish(&main_instance);
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
					    d->args, CREATE_SUSPENDED, &(d->thread_id));

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

void QThread::start()
{
    start(InheritPriority);
}

bool QThread::wait( unsigned long time )
{
    QMutexLocker locker( d->mutex() );

    if ( d->thread_id == GetCurrentThreadId() ) {
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

    d->thread_id = 0;

    CloseHandle( d->handle );

    if ( d->orphan ) {
        d->deinit();
	delete d;
    }

    _endthreadex(0);
}


#endif // QT_THREAD_SUPPORT
