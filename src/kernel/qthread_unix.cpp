/****************************************************************************
** $Id$
**
** QThread class for Unix
**
** Created : 20000913
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
#undef connect
#endif

#include "qthread.h"

#include "qmutex.h"
#include "qwaitcondition.h"
#include <private/qmutexpool_p.h>

#ifndef QT_H
#  include "qapplication.h"
#  include "qptrdict.h"
#  include "qptrlist.h"
#endif // QT_H

#include <errno.h>

static QPtrDict<QThreadPrivate> *qt_thread_dict = 0;
static QMutexPool *qt_thread_mutexpool = 0;

class QGlobalThreadInitializer
{
public:
    inline QGlobalThreadInitializer()
    {
	qt_thread_dict = new QPtrDict<QThreadPrivate>;
	qt_thread_mutexpool = new QMutexPool( FALSE );
    }

    inline ~QGlobalThreadInitializer()
    {
	delete qt_thread_dict;
	delete qt_thread_mutexpool;
	qt_thread_dict = 0;
	qt_thread_mutexpool = 0;
    }
};
QGlobalThreadInitializer qt_global_thread_initializer;

extern "C" { static void *start_thread( void *arg ); }
extern "C" { static void finish_thread( void * ); }


class QThreadPrivate {
public:
    QWaitCondition thread_done;
    pthread_t thread_id;
    bool finished : 1;
    bool running  : 1;
    bool orphan   : 1;

    QThreadPrivate();

    static inline void start( QThread *thread ) { thread->run(); }
};

inline QThreadPrivate::QThreadPrivate()
    : thread_id( 0 ), finished( FALSE ), running( FALSE ), orphan( FALSE )
{
}

extern "C" {
    static void *start_thread( void *arg )
    {
	pthread_cleanup_push( finish_thread, 0 );
	pthread_testcancel();

	QThreadPrivate::start( (QThread *) arg );

	pthread_cleanup_pop( TRUE );

	return 0;
    }

    static void finish_thread( void * )
    {
	QThreadPrivate *d = 0;
	{
	    QMutexLocker locker( qt_global_mutexpool->get( qt_thread_dict ) );
	    d = qt_thread_dict->take( (void *) QThread::currentThread() );
	}

	if ( ! d ) {
	    qWarning( "QThread: internal error: data missing for running thread." );
	    return;
	}

	QMutexLocker locker( qt_thread_mutexpool->get( d ) );
	d->running = FALSE;
	d->finished = TRUE;
	d->thread_id = 0;

	d->thread_done.wakeAll();

	if ( d->orphan )
	    delete d;
    }
}

/**************************************************************************
 ** QThreadQtEvent
 *************************************************************************/

// this is for the magic QThread::postEvent()
class QThreadQtEvent
{
public:
    QThreadQtEvent( QObject *r, QEvent *e ) : receiver( r ), event( e ) { }
    QObject *receiver;
    QEvent *event;
};


class QThreadPostEventPrivate : public QObject
{
    Q_OBJECT
public:
    QThreadPostEventPrivate();

    QPtrList<QThreadQtEvent> events;

public slots:
    void sendEvents();
};


QThreadPostEventPrivate::QThreadPostEventPrivate()
{
    events.setAutoDelete( TRUE );
    connect( qApp, SIGNAL( guiThreadAwake() ), this, SLOT( sendEvents() ) );
}


// this is called from the QApplication::guiThreadAwake signal, and the
// application mutex is already locked
void QThreadPostEventPrivate::sendEvents()
{
    QMutexLocker locker( qt_global_mutexpool->get( this ) );
    QThreadQtEvent *qte;
    for ( qte = events.first(); qte != 0; qte = events.next() ) {
	if ( qte->receiver )
	    qApp->postEvent( qte->receiver, qte->event );
    }
    events.clear();
}


static QThreadPostEventPrivate * qthreadposteventprivate = 0;


void qthread_removePostedEvents( QObject *receiver )
{
    qthreadposteventprivate->eventmutex.lock();

    QThreadQtEvent *qte;
    for ( qte = qthreadposteventprivate->events.first(); qte != 0;
	  qte = qthreadposteventprivate->events.next() ) {
	if ( qte->receiver == receiver ) {
	    qte->receiver = 0;
	    delete qte->event;
	    qte->event = 0;
	}
    }

    qthreadposteventprivate->eventmutex.unlock();
}



/**************************************************************************
 ** QThread
 *************************************************************************/

/*!
    \class QThread qthread.h
    \brief The QThread class provides platform-independent threads.

    \ingroup thread
    \ingroup environment

    A QThread represents a separate thread of control within the
    program; it shares data with all the other threads within the
    process but executes independently in the way that a separate
    program does on a multitasking operating system. Instead of
    starting in main(), QThreads begin executing in run(). You inherit
    run() to include your code. For example:

    \code
    class MyThread : public QThread {

    public:

	virtual void run();

    };

    void MyThread::run()
    {
	for( int count = 0; count < 20; count++ ) {
	    sleep( 1 );
	    qDebug( "Ping!" );
	}
    }

    int main()
    {
	MyThread a;
	MyThread b;
	a.start();
	b.start();
	a.wait();
	b.wait();
    }
    \endcode

    This will start two threads, each of which writes Ping! 20 times
    to the screen and exits. The wait() calls at the end of main() are
    necessary because exiting main() ends the program, unceremoniously
    killing all other threads. Each MyThread stops executing when it
    reaches the end of MyThread::run(), just as an application does
    when it leaves main().

    \sa \link threads.html Thread Support in Qt\endlink.
*/


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
    if( ! qthreadposteventprivate && qApp )
	qthreadposteventprivate = new QThreadPostEventPrivate();
}


/*! \internal
  Cleans up the QThread system.
*/
void QThread::cleanup()
{
    delete qthreadposteventprivate;
    qthreadposteventprivate = 0;
}


/*!
    Provides a way of posting an event from a thread which is not the
    event thread to an object.

    This is achieved as follows:
    \list
    \i The \a event is put into a queue;
    \i the event thread is woken up;
    \i the event thread sends the event to the \a receiver object.
    \endlist
    It is important to note that the event handler for the event, when
    called, will be called from the event thread and not from the
    thread calling QThread::postEvent().

    Since QThread::postEvent() posts events into the event queue of
    QApplication, you must create a QApplication object before calling
    QThread::postEvent().

    The event must be allocated on the heap since the post event queue
    will take ownership of the event and delete it once it has been
    posted.

    \sa QApplication::postEvent()
*/
void QThread::postEvent( QObject * receiver, QEvent * event )
{
#if defined(QT_CHECK_STATE)
    if (! qthreadposteventprivate) {
	qWarning("QThread::postEvent: cannot post event - threads not initialized.");
	return;
    }
#endif // QT_CHECK_STATE

    QMutexLocker locker( qt_global_mutexpool->get( qthreadposteventprivate ) );
    qthreadposteventprivate->events.append( new QThreadQtEvent(receiver, event) );
    qApp->wakeUpGuiThread();
}


// helper function to do thread sleeps, since usleep()/nanosleep() aren't reliable
// enough (in terms of behavior and availability)
static void thread_sleep( struct timespec *ti )
{
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;

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
    ti.tv_nsec = ( tv.tv_usec * 1000 ) + ( msecs % 1000 ) * 1000000;
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
    ti.tv_nsec = ( tv.tv_usec * 1000 ) + ( usecs % 1000000 ) * 1000;
    ti.tv_sec = tv.tv_sec + ( usecs / 1000000 ) + ( ti.tv_nsec / 1000000000 );
    ti.tv_nsec %= 1000000000;
    thread_sleep( &ti );
}

/*!
    Constructs a new thread. The thread does not begin executing until
    start() is called.
*/
QThread::QThread()
{
    d = new QThreadPrivate;
    Q_CHECK_PTR( d );
}

/*!
    QThread destructor.

    Note that deleting a QThread object will not stop the execution of
    the thread it represents. Deleting a running QThread (i.e.
    finished() returns FALSE) will probably result in a program crash.
    You can wait() on a thread to make sure that it has finished.
*/
QThread::~QThread()
{
    QMutexLocker locker( qt_thread_mutexpool->get( d ) );

    if ( d->thread_id ) {
	QMutexLocker dict_locker( qt_global_mutexpool->get( qt_thread_dict ) );
	qt_thread_dict->remove( (void *) d->thread_id );
    }

    if ( d->running && !d->finished ) {
#ifdef QT_CHECK_STATE
	qWarning("QThread object destroyed while thread is still running.");
#endif

	d->orphan = TRUE;
	return;
    }


    delete d;
}

/*!
    Ends the execution of the calling thread and wakes up any threads
    waiting for its termination.
*/
void QThread::exit()
{
    pthread_exit( 0 );
}

/*!
    This begins the execution of the thread by calling run(), which
    should be reimplemented in a QThread subclass to contain your
    code. If you try to start a thread that is already running, this
    call will wait until the thread has finished, and then restart the
    thread.
*/
void QThread::start()
{
    QMutexLocker locker( qt_thread_mutexpool->get( d ) );

    if ( d->running ) {
#ifdef QT_CHECK_STATE
	qWarning( "Attempt to start a thread already running" );
#endif

	d->thread_done.wait( locker.mutex() );
    }

    d->running = TRUE;
    d->finished = FALSE;

    int ret;
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    pthread_attr_setinheritsched( &attr, PTHREAD_INHERIT_SCHED );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
    ret = pthread_create( &d->thread_id, &attr, start_thread, (void *) this );
    pthread_attr_destroy( &attr );

    if ( !ret ) {
	QMutexLocker dict_locker( qt_global_mutexpool->get( qt_thread_dict ) );
	qt_thread_dict->insert( (void *) d->thread_id, d );
    } else {
#ifdef QT_CHECK_STATE
	qWarning( "QThread::start: thread creation error: %s", strerror( ret ) );
#endif
    }
}

/*!
    This function terminates the execution of the thread. The thread
    may or may not be terminated immediately, depending on the
    operating systems scheduling policies. Use QThread::wait()
    after terminate() for synchronous termination.

    When the thread is terminated, all threads waiting for the
    the thread to finish will be woken up.

    \warning This function is dangerous, and its use is discouraged.
    The thread can be terminate at any point in its code path.  Threads
    can be terminated while modifying data.  There is no chance for
    the thread to cleanup after itself, unlock any held mutexes, etc.
    In short, use this function only if \e absolutely necessary.
*/
void QThread::terminate()
{
    QMutexLocker private_locker( qt_thread_mutexpool->get( d ) );
    if ( d->finished || !d->running )
	return;
    if ( ! d->thread_id )
	return;
    pthread_cancel( d->thread_id );
}

/*!
    This provides similar functionality to POSIX pthread_join. A thread
    calling this will block until either of these conditions is met:
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
*/
bool QThread::wait( unsigned long time )
{
    QMutexLocker locker( qt_thread_mutexpool->get( d ) );
    if ( d->finished || ! d->running )
	return TRUE;
    return d->thread_done.wait( locker.mutex(), time );
}

/*!
    Returns TRUE is the thread is finished; otherwise returns FALSE.
*/
bool QThread::finished() const
{
    QMutexLocker locker( qt_thread_mutexpool->get( d ) );
    return d->finished;
}

/*!
    Returns TRUE if the thread is running; otherwise returns FALSE.
*/
bool QThread::running() const
{
    QMutexLocker locker( qt_thread_mutexpool->get( d ) );
    return d->running;
}

/*!
    \fn void QThread::run()

    This method is pure virtual, and must be implemented in derived
    classes in order to do useful work. Returning from this method
    will end the execution of the thread.

    \sa wait()
*/

#include "qthread_unix.moc"

#endif // QT_THREAD_SUPPORT

