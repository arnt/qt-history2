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
#include "qptrlist.h"
#include "qintdict.h"
#include "qthread_p.h"


/**************************************************************************
 ** QMutex
 *************************************************************************/

/*!
  \class QMutex qthread.h
  \brief The QMutex class provides access serialization between threads.

  \ingroup thread
  \ingroup environment

  The purpose of a QMutex is to protect an object, data structure
  or section of code so that only one thread can access it at a time
  (In Java terms, this is similar to the synchronized keyword).
  For example, say there is a method which prints a message to the
  user on two lines:

  \code
  void someMethod()
  {
     qDebug("Hello");
     qDebug("World");
  }
  \endcode

  If this method is called simultaneously from two threads then
  the following sequence could result:

  \code
  Hello
  Hello
  World
  World
  \endcode

  If we add a mutex:

  \code
  QMutex mutex;

  void someMethod()
  {
     mutex.lock();
     qDebug("Hello");
     qDebug("World");
     mutex.unlock();
  }
  \endcode

  In Java terms this would be:

  \code
  void someMethod()
  {
     synchronized {
       qDebug("Hello");
       qDebug("World");
     }
  }
  \endcode

  Then only one thread can execute someMethod at a time and the order
  of messages is always correct. This is a trivial example, of course,
  but applies to any other case where things need to happen in a particular
  sequence.

    When you call lock() in a thread, other threads that try to call
    lock() in the same place will block until the thread that got the
    lock calls unlock(). A non-blocking alternative to lock() is
    tryLock().

*/


/*!
  Constructs a new mutex. The mutex is created in an unlocked state. A
  recursive mutex is created if \a recursive is TRUE; a normal mutex is
  created if \a recursive is FALSE (the default). With a recursive
  mutex, a thread can lock the same mutex multiple times and it will
  not be unlocked until a corresponding number of unlock() calls have
  been made.
*/
QMutex::QMutex(bool recursive)
{
    if ( recursive )
	d = new QRMutexPrivate();
    else
	d = new QMutexPrivate();
}


/*!
  Destroys the mutex.
*/
QMutex::~QMutex()
{
    delete d;
}


/*!
  Attempt to lock the mutex. If another thread has locked the mutex
  then this call will \e block until that thread has unlocked it.

  \sa unlock(), locked()
*/
void QMutex::lock()
{
    d->lock();
}


/*!
  Unlocks the mutex. Attempting to unlock a mutex in a different thread
  to the one that locked it results in an error.  Unlocking a mutex that
  is not locked results in undefined behaviour (varies between
  different Operating Systems' thread implementations).

  \sa lock(), locked()
*/
void QMutex::unlock()
{
    d->unlock();
}


/*!
  Returns TRUE if the mutex is locked by another thread; otherwise
  returns FALSE.

  \warning Due to differing implementations of recursive mutexes on various
  platforms, calling this function from the same thread that previously locked
  the mutex will return undefined results.

  \sa lock(), unlock()
*/
bool QMutex::locked()
{
    return d->locked();
}


/*!
  Attempt to lock the mutex.  If the lock was obtained, this function
  returns TRUE.  If another thread has locked the mutex, this function
  returns FALSE, instead of waiting for the mutex to become available,
  i.e. it does not block.

  The mutex must be unlocked with unlock() before another thread can
  successfully lock it.

  \sa lock(), unlock(), locked()
*/
bool QMutex::tryLock()
{
    return d->trylock();
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
    {}
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
    for( qte = events.first(); qte != 0; qte = events.next() ) {
	qApp->postEvent( qte->receiver, qte->event );
    }

    events.clear();

    // ## let event compression take full effect
    // qApp->sendPostedEvents();

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
  however, QThreads begin executing in run(), which you inherit
  to provide your code. For example:

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
  should not be used in any application code.
  On Windows, the returned value is a pseudo handle for the current thread,
  and it cannot be used for numerical comparison.
*/
Qt::HANDLE QThread::currentThread()
{
#if !defined(Q_OS_SOLARIS)
    return (HANDLE) pthread_self();
#else
    return (HANDLE) thr_self();
#endif
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

  Just like \l{QApplication::postEvent()}, the \a event must be
  allocated on the heap, as it is deleted when the event has been
  posted.
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


/*!
  System independent sleep.  This causes the current thread to sleep for
  \a secs seconds.
*/
void QThread::sleep( unsigned long secs )
{
    struct timespec ts;
    ts.tv_nsec = 0;
    ts.tv_sec  = secs;
    (void) ::nanosleep(&ts, NULL);
}


/*!
  System independent sleep.  This causes the current thread to sleep for
  \a msecs milliseconds
*/
void QThread::msleep( unsigned long msecs )
{
    QThread::usleep( msecs * 1000 );
}


/*!
  System independent sleep.  This causes the current thread to sleep for
  \a usecs microseconds
*/
void QThread::usleep( unsigned long usecs )
{
    struct timespec ts;
    ts.tv_nsec = (usecs % 1000000) * 1000;
    ts.tv_sec  = usecs / 1000000;
    (void) ::nanosleep(&ts, NULL);
}


/*!
  Constructs a new thread.  The thread does not actually begin executing
  until start() is called.
*/
QThread::QThread()
{
    d = new QThreadPrivate;
}


/*!
  QThread destructor.  Note that deleting a QThread object will not stop
  the execution of the thread it represents.  Deleting a running QThread
  will probably result in a program crash.
*/
QThread::~QThread()
{
    if( d->running && !d->finished ) {
	qWarning("QThread object destroyed while thread is still running.");
    } else {
	delete d;
    }
}


/*!
  Ends execution of the calling thread and wakes up any threads waiting
  for its termination.
*/
void QThread::exit()
{
    dictMutex->lock();

    QThread *there = thrDict->find((int)QThread::currentThread());
    if (there) {
	there->d->running = FALSE;
	there->d->finished = TRUE;

	there->d->thread_done.wakeAll();
    }

    dictMutex->unlock();

#if !defined(Q_OS_SOLARIS)
    pthread_exit(0);
#else
    thr_exit(0);
#endif
}


/*!
  This begins actual execution of the thread by calling run(),
  which should be reimplemented in a QThread subclass to contain your code.
  If you try to start a thread that is already running, this call will
  wait until the thread has finished, and then restart the thread.
*/
void QThread::start()
{
    if (d->running) {
#ifdef QT_CHECK_RANGE
	qWarning("Attempt to start a thread already running");
#endif

	wait();
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
    if (d->finished || ! d->running)
	return TRUE;

    return d->thread_done.wait(time);
}


/*!
  Returns TRUE is the thread is finished; otherwise returns FALSE.
*/
bool QThread::finished() const
{
    return d->finished;
}


/*!
  Returns TRUE if the thread is running; otherwise returns FALSE.
*/
bool QThread::running() const
{
    return d->running;
}


/*! \fn void QThread::run()

  This method is pure virtual, and it must be implemented in derived classes
  in order to do useful work. Returning from this method will end execution
  of the thread.

  \sa wait()
*/


/**************************************************************************
 ** QWaitCondition
 *************************************************************************/

/*!
  \class QWaitCondition qthread.h
  \brief The QWaitCondition class allows waiting/waking for conditions between threads.

  \ingroup thread
  \ingroup environment

  QWaitConditions allow a thread to tell other threads that some sort of
  condition has been met; one or many threads can block waiting for a
  QWaitCondition to set a condition with wakeOne() or wakeAll().  Use
  wakeOne() to wake one randomly selected event or wakeAll() to wake them
  all. For example, say we have three tasks that should be performed every
  time the user presses a key; each task could be split into a thread, each
  of which would have a run() body like this:

  \code
  QWaitCondition key_pressed;

  for (;;) {
     key_pressed.wait(); // This is a QWaitCondition global variable
     // Key was pressed, do something interesting
     do_something();
  }
  \endcode

  A fourth thread would read key presses and wake the other three threads
  up every time it receives one, like this:

  \code
  QWaitCondition key_pressed;

  for (;;) {
     getchar();
     // Causes any thread in key_pressed.wait() to return from
     // that method and continue processing
     key_pressed.wakeAll();
  }
  \endcode

  Note that the order the three threads are woken up in is undefined,
  and that if some or all of the threads are still in do_something()
  when the key is pressed, they won't be woken up (since they're not
  waiting on the condition variable) and so the task will not be performed
  for that key press.  This can be avoided by, for example, doing something
  like this:

  \code
  QMutex mymutex;
  QWaitCondition key_pressed;
  int mycount=0;

  // Worker thread code
  for (;;) {
     key_pressed.wait(); // This is a QWaitCondition global variable
     mymutex.lock();
     mycount++;
     mymutex.unlock();
     do_something();
     mymutex.lock();
     mycount--;
     mymutex.unlock();
  }

  // Key reading thread code
  for (;;) {
     getchar();
     mymutex.lock();
     // Sleep until there are no busy worker threads
     while( count > 0 ) {
       mymutex.unlock();
       sleep( 1 );
       mymutex.lock();
     }
     mymutex.unlock();
     key_pressed.wakeAll();
  }
  \endcode

  The mutexes are necessary because the results of two threads
  attempting to change the value of the same variable simultaneously
  are unpredictable.

*/


/*!
  Constructs a new event signalling object.
*/
QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate;
}


/*!
  Deletes the event signalling object.
*/
QWaitCondition::~QWaitCondition()
{
    delete d;
}


/*!
  Wait on the thread event object. The thread calling this will block
  until either of these conditions is met:
  \list
  \i Another thread signals it using wakeOne() or wakeAll(). This
       function will return TRUE in this case.
  \i \a time milliseconds has elapsed.  If \a time is ULONG_MAX (the default),
       then the wait will never timeout (the event must
       be signalled).  This function will return FALSE if the
       wait timed out.
  \endlist

  \sa wakeOne(), wakeAll()
*/
bool QWaitCondition::wait(unsigned long time)
{
    return d->wait(time);
}


/*!
    \overload
  Release the locked \a mutex and wait on the thread event object. The
  \a mutex must be initially locked by the calling thread.  If \a mutex
  is not in a locked state, this function returns immediately.  The
  \a mutex will be unlocked, and the calling thread will block until
  either of these conditions is met:
  \list
  \i Another thread signals it using wakeOne() or wakeAll(). This
       function will return TRUE in this case.
  \i \a time milliseconds has elapsed.  If \a time is ULONG_MAX (the default),
       then the wait will never timeout (the event must be
       signalled).  This function will return FALSE if the
       wait timed out.
  \endlist

  The mutex will be returned to the same locked state.  This function is
  provided to allow the atomic transition from the locked state to the
  wait state.

  \sa wakeOne(), wakeAll()
*/
bool QWaitCondition::wait(QMutex *mutex, unsigned long time)
{
    return d->wait(mutex, time);
}


/*!
  This wakes one thread waiting on the QWaitCondition.  The thread that
  is woken up depends on the operating system's scheduling policies, and
  cannot be controlled or predicted.

  \sa wakeAll()
*/
void QWaitCondition::wakeOne()
{
    d->wakeOne();
}


/*!
  This wakes all threads waiting on the QWaitCondition.  The order in
  which the threads are woken up depends on the operating system's
  scheduling policies, and cannot be controlled or predicted.

  \sa wakeOne()
*/
void QWaitCondition::wakeAll()
{
    d->wakeAll();
}


/**************************************************************************
 ** QSemaphore
 *************************************************************************/
/*!
  \class QSemaphore qthread.h
  \brief The QSemaphore class provides a robust integer semaphore.

  \ingroup thread
  \ingroup environment

  QSemaphore can be used to serialize thread execution, similar to a
  QMutex.  A semaphore differs from a mutex, in that a semaphore can be
  accessed by more than one thread at a time.

  An example would be an application that stores data in a large tree
  structure.  The application creates 10 threads (commonly called a
  thread pool) to do searches on the tree.  When the application searches
  the tree for some piece of data, it uses one thread per base node to
  do the searching.  A semaphore could be used to make sure that 2 threads
  don't try to search the same branch of the tree.

  A real world example of a semaphore would be dining at a restuarant.
  A semaphore is initialized to have a maximum count equal to the number
  of chairs in the restuarant.  As people arrive, they want a seat.  As
  seats are filled, the semaphore is accessed, once per person.  As people
  leave, the access is released, allowing more people to enter. If a
  party of 10 people want to be seated, but there are only 9 seats, those
  10 people will wait, but a party of 4 people would be seated (taking
  the available seats to 5, making the party of 10 people wait longer).
*/


class QSemaphorePrivate {
public:
    QSemaphorePrivate(int);

    QMutex mutex;
    QWaitCondition cond;

    int value, max;
};


QSemaphorePrivate::QSemaphorePrivate(int m)
    : mutex(FALSE), value(0), max(m)
{
}


/*!
  Creates a new semaphore.  The semaphore can be concurrently accessed at
  most \a maxcount times.
*/
QSemaphore::QSemaphore(int maxcount)
{
    d = new QSemaphorePrivate(maxcount);
}


/*!
  Destroys the semaphore.
*/
QSemaphore::~QSemaphore()
{
    delete d;
}


/*!
  Postfix ++ operator.

  Try to get access to the semaphore.  If \l available() is >= \l total(),
  this call will block until it can get access.
*/
int QSemaphore::operator++(int)
{
    int ret;

    d->mutex.lock();

    while (d->value >= d->max)
	d->cond.wait(&(d->mutex));

    ++(d->value);
    if (d->value > d->max) d->value = d->max;
    ret = d->value;

    d->mutex.unlock();

    return ret;
}


/*!
  Postfix -- operator.

  Release access of the semaphore.  This wakes all threads waiting for
  access to the semaphore.
 */
int QSemaphore::operator--(int)
{
    int ret;

    d->mutex.lock();

    --(d->value);
    if (d->value < 0) d->value = 0;
    ret = d->value;

    d->cond.wakeAll();
    d->mutex.unlock();

    return ret;
}


/*!
  Try to get access to the semaphore.  If \l available() is >= \l total(),
  the calling thread blocks until it can get access.   The caller will
  only get access from the semaphore if it can get all \a n accesses
  at once.
*/
int QSemaphore::operator+=(int n)
{
    int ret;

    d->mutex.lock();

    while (d->value + n > d->max)
	d->cond.wait(&(d->mutex));

    d->value += n;

#ifdef QT_CHECK_RANGE
    if (d->value > d->max) {
	qWarning("QSemaphore::operator+=: attempt to allocate more resources than available");
	d->value = d->max;
    }
#endif

    ret = d->value;

    d->mutex.unlock();

    return ret;
}


/*!
  Release \a n accesses to the semaphore.
 */
int QSemaphore::operator-=(int n)
{
    int ret;

    d->mutex.lock();

    d->value -= n;

#ifdef QT_CHECK_RANGE
    if (d->value < 0) {
	qWarning("QSemaphore::operator-=: attempt to deallocate more resources than taken");
	d->value = 0;
    }
#endif

    ret = d->value;

    d->cond.wakeOne();
    d->mutex.unlock();

    return ret;
}


/*!
  This function returns the number of accesses currently available to
  the semaphore.
 */
int QSemaphore::available() const {
    int ret;

    d->mutex.lock();
    ret = d->max - d->value;
    d->mutex.unlock();

    return ret;
}


/*!
  This function returns the total number of accesses to the semaphore.
 */
int QSemaphore::total() const {
    int ret;

    d->mutex.lock();
    ret = d->max;
    d->mutex.unlock();

    return ret;
}


/*!
  Try to get access to the semaphore.  If \l available() is >= \l total(),
  the calling thread blocks until it can get access.   The caller will
  only get access from the semaphore if it can get all \a n accesses
  at once.
*/
bool QSemaphore::tryAccess(int n)
{
    if (! d->mutex.tryLock())
	return FALSE;

    if (d->value + n > d->max) {
	d->mutex.unlock();
	return FALSE;
    }

    d->value += n;

#ifdef QT_CHECK_RANGE
    if (d->value > d->max) {
	qWarning("QSemaphore::operator+=: attempt to allocate more resources than available");
	d->value = d->max;
    }
#endif

    d->mutex.unlock();

    return TRUE;
}


#include "qthread_unix.moc"


#endif // QT_THREAD_SUPPORT
