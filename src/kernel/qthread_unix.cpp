/****************************************************************************
** $Id$
**
** QThread class for Unix
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 or for Qt/Embedded may use this file in accordance
** with the Qt Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#if defined(QT_THREAD_SUPPORT)

#include "qplatformdefs.h"

#include "qthread.h"
#include <private/qthreadinstance_p.h>
#include <private/qmutexpool_p.h>
#include <qthreadstorage.h>

#ifndef QT_H
#  include "qapplication.h"
#endif // QT_H

#include <errno.h>
#include <sched.h>


static QThreadInstance *main_instance = 0;


static QMutexPool *qt_thread_mutexpool = 0;


static pthread_once_t storage_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t storage_key;
static void create_storage_key()
{
    pthread_key_create( &storage_key, NULL );
}


/**************************************************************************
 ** QThreadInstance
 *************************************************************************/

QThreadInstance *QThreadInstance::current()
{
    pthread_once( &storage_key_once, create_storage_key );
    QThreadInstance *ret = (QThreadInstance *) pthread_getspecific( storage_key );
    if ( ! ret ) {
	if ( main_instance ) {
	    qWarning( "QThread: ERROR: creating QThreadInstance for unknown thread %lx\n"
		      "This instance and all per-thread data will be leaked.",
		      (long)QThread::currentThread() );
	}

	ret = new QThreadInstance;
	ret->args[1] = ret;
	ret->running = TRUE;
	ret->orphan = TRUE;
	ret->thread_id = pthread_self();

	pthread_setspecific( storage_key, ret );
    }
    return ret;
}

QThreadInstance::QThreadInstance( unsigned int stackSize )
    :  stacksize( stackSize ), thread_storage( 0 ),
       finished( FALSE ), running( FALSE ), orphan( FALSE ),
       thread_id( 0 )
{
    args[0] = args[1] = 0;

    // threads have not been initialized yet, do it now
    if ( ! qt_thread_mutexpool ) QThread::initialize();
}

void *QThreadInstance::start( void *_arg )
{
    void **arg = (void **) _arg;

    pthread_once( &storage_key_once, create_storage_key );
    pthread_setspecific( storage_key, arg[1] );
    pthread_cleanup_push( QThreadInstance::finish, arg[1] );
    pthread_testcancel();

    ( (QThread *) arg[0] )->run();

    pthread_cleanup_pop( TRUE );
    return 0;
}

void QThreadInstance::finish( void * )
{
    QThreadInstance *d = current();

    if ( ! d ) {
#ifdef QT_CHECK_STATE
	qWarning( "QThread: internal error: zero data for running thread." );
#endif // QT_CHECK_STATE
	return;
    }

    QMutexLocker locker( d->mutex() );
    d->running = FALSE;
    d->finished = TRUE;
    d->args[0] = d->args[1] = 0;


    QThreadStoragePrivate::finish( d->thread_storage );
    d->thread_storage = 0;

    d->thread_id = 0;
    d->thread_done.wakeAll();

    if ( d->orphan )
	delete d;
}

QMutex *QThreadInstance::mutex() const
{
    return qt_thread_mutexpool ? qt_thread_mutexpool->get( (void *) this ) : 0;
}

void QThreadInstance::terminate()
{
    if ( ! thread_id ) return;
    pthread_cancel( thread_id );
}


/**************************************************************************
 ** QThread
 *************************************************************************/

/*!
    This returns the thread handle of the currently executing thread.

    \warning The handle returned by this function is used for internal
    purposes and should \e not be used in any application code. On
    Windows, the returned value is a pseudo handle for the current
    thread, and it cannot be used for numerical comparison.
*/
Qt::HANDLE QThread::currentThread()
{
    return (HANDLE) pthread_self();
}

/*! \internal
  Initializes the QThread system.
*/
void QThread::initialize()
{
    if ( ! qt_global_mutexpool )
	qt_global_mutexpool = new QMutexPool( TRUE, 73 );
    if ( ! qt_thread_mutexpool )
	qt_thread_mutexpool = new QMutexPool( FALSE, 127 );

    // create a QThreadInstance for the main() thread
    pthread_once( &storage_key_once, create_storage_key );
    main_instance = (QThreadInstance *) pthread_getspecific( storage_key );
    if ( ! main_instance ) {
	main_instance = new QThreadInstance;
	main_instance->args[1] = main_instance;
	main_instance->running = TRUE;
	main_instance->orphan = TRUE;
	main_instance->thread_id = pthread_self();

	pthread_setspecific( storage_key, main_instance );
    }
}

/*! \internal
  Cleans up the QThread system.
*/
void QThread::cleanup()
{
    delete qt_global_mutexpool;
    delete qt_thread_mutexpool;
    qt_global_mutexpool = 0;
    qt_thread_mutexpool = 0;

    // cleanup the QThreadInstance for the main() thread
    QThreadInstance::finish( main_instance );
    main_instance = 0;
    pthread_setspecific( storage_key, 0 );
}

/*!
    Ends the execution of the calling thread and wakes up any threads
    waiting for its termination.
*/
void QThread::exit()
{
    pthread_exit( 0 );
}

/*! \obsolete
    Use QApplication::postEvent() instead.
*/
void QThread::postEvent( QObject * receiver, QEvent * event )
{
    QApplication::postEvent( receiver, event );
}

/*  \internal
    helper function to do thread sleeps, since usleep()/nanosleep()
    aren't reliable enough (in terms of behavior and availability)
*/
static void thread_sleep( struct timespec *ti )
{
    pthread_mutex_t mtx;
    pthread_cond_t cnd;

    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cnd, 0);

    pthread_mutex_lock( &mtx );
    (void) pthread_cond_timedwait( &cnd, &mtx, ti );
    pthread_mutex_unlock( &mtx );

    pthread_cond_destroy( &cnd );
    pthread_mutex_destroy( &mtx );
}

/*!
    System independent sleep. This causes the current thread to sleep
    for \a secs seconds.
*/
void QThread::sleep( unsigned long secs )
{
    struct timeval tv;
    gettimeofday( &tv, 0 );
    struct timespec ti;
    ti.tv_sec = tv.tv_sec + secs;
    ti.tv_nsec = ( tv.tv_usec * 1000 );
    thread_sleep( &ti );
}

/*!
    System independent sleep. This causes the current thread to sleep
    for \a msecs milliseconds
*/
void QThread::msleep( unsigned long msecs )
{
    struct timeval tv;
    gettimeofday( &tv, 0 );
    struct timespec ti;

    ti.tv_nsec = ( tv.tv_usec + ( msecs % 1000 ) * 1000 ) * 1000;
    ti.tv_sec = tv.tv_sec + ( msecs / 1000 ) + ( ti.tv_nsec / 1000000000 );
    ti.tv_nsec %= 1000000000;
    thread_sleep( &ti );
}

/*!
    System independent sleep. This causes the current thread to sleep
    for \a usecs microseconds
*/
void QThread::usleep( unsigned long usecs )
{
    struct timeval tv;
    gettimeofday( &tv, 0 );
    struct timespec ti;

    ti.tv_nsec = ( tv.tv_usec + ( usecs % 1000000 ) ) * 1000;
    ti.tv_sec = tv.tv_sec + ( usecs / 1000000 ) + ( ti.tv_nsec / 1000000000 );
    ti.tv_nsec %= 1000000000;
    thread_sleep( &ti );
}

/*!
    Begins execution of the thread by calling run(), which should be
    reimplemented in a QThread subclass to contain your code.  The
    operating system will schedule the thread according to the \a
    priority argument.

    If you try to start a thread that is already running, this
    function will wait until the the thread has finished and then
    restart the thread.

    \sa Priority
*/
void QThread::start(Priority priority)
{
    QMutexLocker locker( d->mutex() );

    if ( d->running )
	d->thread_done.wait( locker.mutex() );
    d->running = TRUE;
    d->finished = FALSE;

    int ret;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    switch (priority) {
    case InheritPriority:
	{
	    pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
	    break;
	}

    default:
	{
	    int sched_policy;
	    if (pthread_attr_getschedpolicy(&attr, &sched_policy) != 0) {
		qDebug("couldn't get the default schedule policy?");
		sched_policy = SCHED_RR;
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
	    }

	    int prio_min = sched_get_priority_min(sched_policy);
	    int prio_max = sched_get_priority_max(sched_policy);
	    int prio;
	    switch (priority) {
	    case IdlePriority:
		prio = prio_min;
		break;

	    case HighestPriority:
		prio = prio_max;
		break;

	    default:
		// crudely scale our priority enum values to the prio_min/prio_max
		prio = (((prio_max - prio_min) / TimeCriticalPriority) *
			priority) + prio_min;
		prio = QMAX(prio_min, QMIN(prio_max, prio));
		break;
	    }

	    sched_param sp;
	    sp.sched_priority = prio;

	    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	    pthread_attr_setschedparam(&attr, &sp);
	    break;
	}
    }

    if ( d->stacksize > 0 ) {
	ret = pthread_attr_setstacksize( &attr, d->stacksize );
	if ( ret ) {
#ifdef QT_CHECK_STATE
	    qWarning( "QThread::start: thread stack size error: %s", strerror( ret ) ) ;
#endif // QT_CHECK_STATE

	    // we failed to set the stacksize, and as the documentation states,
	    // the thread will fail to run...
	    d->running = FALSE;
	    d->finished = FALSE;
	    return;
	}
    }
    d->args[0] = this;
    d->args[1] = d;
    ret = pthread_create( &d->thread_id, &attr, QThreadInstance::start, d->args );
    pthread_attr_destroy( &attr );

    if ( ret ) {
#ifdef QT_CHECK_STATE
	qWarning( "QThread::start: thread creation error: %s", strerror( ret ) );
#endif // QT_CHECK_STATE

	d->running = FALSE;
	d->finished = FALSE;
	d->args[0] = d->args[1] = 0;
    }
}

void QThread::start()
{
    start(InheritPriority);
}

/*!
    A thread calling this function will block until either of these
    conditions is met:

    \list
    \i The thread associated with this QThread object has finished
       execution (i.e. when it returns from \l{run()}). This function
       will return TRUE if the thread has finished. It also returns
       TRUE if the thread has not been started yet.
    \i \a time milliseconds has elapsed. If \a time is ULONG_MAX (the
    	default), then the wait will never timeout (the thread must
	return from \l{run()}). This function will return FALSE if the
	wait timed out.
    \endlist

    This provides similar functionality to the POSIX \c pthread_join() function.
*/
bool QThread::wait( unsigned long time )
{
    QMutexLocker locker( d->mutex() );

    if ( d->thread_id == pthread_self() ) {
#ifdef QT_CHECK_STATE
	qWarning( "QThread::wait: thread tried to wait on itself" );
#endif // QT_CHECK_STATE

	return FALSE;
    }

    if ( d->finished || ! d->running )
	return TRUE;
    return d->thread_done.wait( locker.mutex(), time );
}


#endif // QT_THREAD_SUPPORT
