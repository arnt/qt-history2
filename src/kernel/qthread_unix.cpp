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
#include "qapplication.h"
#include "qmutex.h"
#include "qwaitcondition.h"
#include "qptrlist.h"

#ifndef QT_H
#if defined( QWS ) || defined( Q_WS_MACX )
#include "qptrdict.h"
#else
#include "qintdict.h"
#endif
#endif // QT_H

#include <errno.h>

static QMutex *dictMutex = 0;
#if defined( QWS ) || defined( Q_WS_MACX )
static QPtrDict<QThread> *thrDict = 0;
#else
static QIntDict<QThread> *thrDict = 0;
#endif


extern "C" { static void *start_thread(void *t); }


class QThreadPrivate {
public:
    pthread_t thread_id;
    QMutex mutex;
    QWaitCondition thread_done;      // Used for QThread::wait()
    bool finished, running;

    QThreadPrivate()
	: thread_id(0), finished(FALSE), running(FALSE)
    {
	if (! dictMutex)
	    dictMutex = new QMutex;
	if (! thrDict) {
#if defined( QWS ) || defined( Q_WS_MACX )
	    thrDict = new QPtrDict<QThread>;
#else
	    thrDict = new QIntDict<QThread>;
#endif
	}
    }

    ~QThreadPrivate()
    {
	dictMutex->lock();
	if (thread_id)
	    thrDict->remove((Qt::HANDLE) thread_id);
	dictMutex->unlock();

	thread_id = 0;
    }

    void init(QThread *that)
    {
	that->d->running = TRUE;
	that->d->finished = FALSE;

	int ret;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&thread_id, &attr, start_thread,
			     (void *) that);
	pthread_attr_destroy(&attr);

#ifdef QT_CHECK_STATE
	if (ret)
	    qWarning("QThread::start: thread creation error: %s", strerror(ret));
#endif
    }

    static void internalRun(QThread *that)
    {
	dictMutex->lock();
	thrDict->insert(QThread::currentThread(), that);
	dictMutex->unlock();

	that->run();

	dictMutex->lock();

	QThread *there = thrDict->find(QThread::currentThread());
	if (there) {
            QMutexLocker locker( &there->d->mutex );

	    there->d->running = FALSE;
	    there->d->finished = TRUE;

	    there->d->thread_done.wakeAll();
	}

	thrDict->remove(QThread::currentThread());
	dictMutex->unlock();
    }
};

extern "C" {
    static void *start_thread(void *t)
    {
	QThreadPrivate::internalRun( (QThread *) t );
	return 0;
    }
}

/**************************************************************************
 ** QThreadQtEvent
 *************************************************************************/

// this is for the magic QThread::postEvent()
class QThreadQtEvent
{
public:
    QThreadQtEvent(QObject *r, QEvent *e)
	: receiver(r), event(e)
    {
    }
    QObject *receiver;
    QEvent *event;
};


class QThreadPostEventPrivate : public QObject
{
    Q_OBJECT
public:
    QThreadPostEventPrivate();

    QPtrList<QThreadQtEvent> events;
    QMutex eventmutex;

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
    eventmutex.lock();

    QThreadQtEvent *qte;
    for( qte = events.first(); qte != 0; qte = events.next() )
	qApp->postEvent( qte->receiver, qte->event );
    events.clear();

    eventmutex.unlock();
}


static QThreadPostEventPrivate * qthreadposteventprivate = 0;


/**************************************************************************
 ** QThread
 *************************************************************************/

/*!
  \class QThread qthread.h
  \brief The QThread class provides platform-independent threads.

  \ingroup thread
  \ingroup environment

  A QThread represents a separate thread of control within the program;
  it shares data with all the other threads within the process but
  executes independently in the way that a separate program does on
  a multitasking operating system. Instead of starting in main(),
  QThreads begin executing in run(). You inherit run()
  to include your code. For example:

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
  reaches the end of MyThread::run(), just as an application does when
  it leaves main().

  \sa \link threads.html Thread Support in Qt\endlink.
*/


/*!
  This returns the thread handle of the currently executing thread.  The
  handle returned by this function is used for internal reasons and
  should \e not be used in any application code.
  On Windows, the returned value is a pseudo handle for the current thread,
  and it cannot be used for numerical comparison.
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
    if( !qthreadposteventprivate )
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
  event thread to an object. The \a event is put into a queue, then the
  event thread is woken which then sends the event to the \a receiver object.
  It is important to note that the event handler for the event, when called,
  will be called from the event thread and not from the thread calling
  QThread::postEvent().

  Since QThread::postEvent() posts events into the event queue of QApplication,
  you must create a QApplication object before calling QThread::postEvent().

  The event must be allocated on the heap since the post event queue
  will take ownership of the event and delete it once it has been posted.

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

    qthreadposteventprivate->eventmutex.lock();
    qthreadposteventprivate->events.append( new QThreadQtEvent(receiver, event) );
    qApp->wakeUpGuiThread();
    qthreadposteventprivate->eventmutex.unlock();
}


// helper function to do thread sleeps, since usleep()/nanosleep() aren't reliable
// enough (in terms of behavior and availability)
static void thread_sleep(struct timespec *ti)
{
    pthread_mutex_t mtx;
    pthread_cond_t cnd;

    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cnd, 0);

    pthread_mutex_lock(&mtx);
    (void) pthread_cond_timedwait(&cnd, &mtx, ti);
    pthread_mutex_unlock(&mtx);

    pthread_cond_destroy(&cnd);
    pthread_mutex_destroy(&mtx);
}


/*!
  System independent sleep.  This causes the current thread to sleep for
  \a secs seconds.
*/
void QThread::sleep( unsigned long secs )
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;
    ti.tv_sec = tv.tv_sec + secs;
    ti.tv_nsec = (tv.tv_usec * 1000);
    thread_sleep(&ti);
}


/*!
  System independent sleep.  This causes the current thread to sleep for
  \a msecs milliseconds
*/
void QThread::msleep( unsigned long msecs )
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;
    ti.tv_nsec = (tv.tv_usec * 1000) + (msecs % 1000) * 1000000;
    ti.tv_sec = tv.tv_sec + (msecs / 1000) + (ti.tv_nsec / 1000000000);
    ti.tv_nsec %= 1000000000;
    thread_sleep(&ti);
}


/*!
  System independent sleep.  This causes the current thread to sleep for
  \a usecs microseconds
*/
void QThread::usleep( unsigned long usecs )
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;
    ti.tv_nsec = (tv.tv_usec * 1000) + (usecs % 1000000) * 1000;
    ti.tv_sec = tv.tv_sec + (usecs / 1000000) + (ti.tv_nsec / 1000000000);
    ti.tv_nsec %= 1000000000;
    thread_sleep(&ti);
}


/*!
  Constructs a new thread.  The thread does not actually begin executing
  until start() is called.
*/
QThread::QThread()
{
    d = new QThreadPrivate;
    Q_CHECK_PTR( d );
}


/*!
  QThread destructor.  Note that deleting a QThread object will not stop
  the execution of the thread it represents.  Deleting a running QThread
  (ie. finished() returns FALSE) will probably result in a program crash.
  You can wait() on the thread to make sure that the thread has finished.
*/
QThread::~QThread()
{
#ifdef QT_CHECK_STATE
    {
        QMutexLocker locker( &d->mutex );
        if( d->running && !d->finished ) {
            qWarning("QThread object destroyed while thread is still running.");
            return;
        }
    }
#endif
    delete d;
}


/*!
  Ends the execution of the calling thread and wakes up any threads waiting
  for its termination.
*/
void QThread::exit()
{
    dictMutex->lock();

    QThread *there = thrDict->find(QThread::currentThread());
    if (there) {
        QMutexLocker( &there->d->mutex );

	there->d->running = FALSE;
	there->d->finished = TRUE;

	there->d->thread_done.wakeAll();
    }

    dictMutex->unlock();

    pthread_exit(0);
}


/*!
  This begins actual execution of the thread by calling run(),
  which should be reimplemented in a QThread subclass to contain your code.
  If you try to start a thread that is already running, this call will
  wait until the thread has finished, and then restart the thread.
*/
void QThread::start()
{
    QMutexLocker locker( &d->mutex );

    if (d->running) {
#ifdef QT_CHECK_STATE
	qWarning("Attempt to start a thread already running");
#endif

        d->mutex.unlock();
        wait();
        d->mutex.lock();
    }

    d->init(this);
}


/*!
  This provides similar functionality to POSIX pthread_join.  A thread
  calling this will block until either of these conditions is met:
  \list
  \i The thread associated with this QThread object has finished
       execution (i.e. when it returns from \l{run()}).  This
       function will return TRUE if the thread has finished.
       It also returns TRUE if the thread has not been started yet.
  \i \a time milliseconds has elapsed.  If \a time is ULONG_MAX (the
	default), then the wait will never timeout (the thread must return
	from \l{run()}). This function will return FALSE if the wait timed
	out.
  \endlist
*/
bool QThread::wait(unsigned long time)
{
    QMutexLocker locker( &d->mutex );
    if (d->finished || ! d->running)
	return TRUE;
    return d->thread_done.wait( &d->mutex, time);
}


/*!
  Returns TRUE is the thread is finished; otherwise returns FALSE.
*/
bool QThread::finished() const
{
    QMutexLocker locker( &d->mutex );
    return d->finished;
}


/*!
  Returns TRUE if the thread is running; otherwise returns FALSE.
*/
bool QThread::running() const
{
    QMutexLocker locker( &d->mutex );
    return d->running;
}


/*! \fn void QThread::run()

  This method is pure virtual, and it must be implemented in derived classes
  in order to do useful work. Returning from this method will end execution
  of the thread.

  \sa wait()
*/

#include "qthread_unix.moc"

#endif // QT_THREAD_SUPPORT

