/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_unix.cpp#4 $
**
** QThread class for Unix
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

#define _GNU_SOURCE

#include "qthread.h"
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <qlist.h>
#include <unistd.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qobject.h>
#include <sys/time.h>

class QMutexPrivate {

public:

    pthread_mutex_t mymutex;
    pthread_mutex_t mutex2;
    QMutexPrivate();
    ~QMutexPrivate();

    int count;
    THREAD_HANDLE thread;

};

class QThreadPrivate {

public:

    pthread_t mythread;
    QThreadEvent thread_done;      // Used for QThread::wait()
    bool finished;

};

class QThreadEventPrivate {

public:

    pthread_cond_t mycond;
    pthread_mutex_t mutex;

    QThreadEventPrivate();
    ~QThreadEventPrivate();

};

QMutexPrivate::QMutexPrivate()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK);
    int ret = pthread_mutex_init( &mymutex, &attr );

    if( ret ) {
	qFatal( "Mutex init failure %s", strerror( ret ) );
    }
    count=0;

}

QMutexPrivate::~QMutexPrivate()
{
    int ret = pthread_mutex_destroy( &mymutex );
    if( ret ) {
	qWarning( "Mutex destroy failure %s", strerror( ret ) );
    }
}

QThreadEventPrivate::QThreadEventPrivate()
{
    int ret=pthread_cond_init(&mycond,0);
    if( ret ) {
	qFatal( "Thread event init failure %s", strerror( ret ) );
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK);
    ret = pthread_mutex_init(&mutex,&attr);

    if (ret) qFatal("Thread event init failure: %s", strerror(ret));
}

QThreadEventPrivate::~QThreadEventPrivate()
{
    int ret=pthread_cond_destroy(&mycond);
    if( ret ) {
	qFatal( "Thread event init failure %s", strerror( ret ) );
    }

    ret = pthread_mutex_destroy(&mutex);
    if (ret) qFatal("Thread event destroy failure: %s", strerror(ret));
}

/*!
  \class QMutex qthread.h
  \brief The QMutex class provides access serialisation between threads.

  \ingroup environment

  The purpose of a QMutex is to protect an object, data structure
  or section of code so that only one thread can access it at a time
  (In Java terms, this is similar to the synchronized keyword).
  For example, say there is a method which prints a message to the
  user on two lines:

  void someMethod()
  {
     qDebug("Hello");
     qDebug("World");
  }

  If this method is called simultaneously from two threads then
  the following sequence could result:

  Hello
  Hello
  World
  World

  If we add a mutex:

  QMutex mutex;

  void someMethod()
  {
     mutex.lock();
     qDebug("Hello");
     qDebug("World");
     mutex.unlock();
  }

  (In Java terms this would be:

  void someMethod()
  {
     synchronized {
       qDebug("Hello");
       qDebug("World");
     }
  }

  )

  Then only one thread can execute someMethod at a time and the order
  of messages is always correct. This is a trivial example, of course,
  but applies to any other case where things need to happen in a particular
  sequence.

*/

QMutex::QMutex()
{
    d = new QMutexPrivate();
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init( &(d->mutex2),&attr);
}

/*!
  Constructs a new mutex. The mutex is created in an unlocked state.
*/

QMutex::~QMutex()
{
    delete d;
}

/*!
  Destroys the mutex
*/

void QMutex::lock()
{
    pthread_mutex_lock(&(d->mutex2));
    if(d->count>0 && d->thread==QThread::currentThread()) {
	d->count++;
    } else {
	pthread_mutex_unlock(&(d->mutex2));
	int ret = pthread_mutex_lock( &( d->mymutex ) );
	pthread_mutex_lock(&(d->mutex2));

	if( ret ) {
	    qFatal( "Mutex lock failure %s\n", strerror( ret ) );
	}

	d->count++;
	d->thread=QThread::currentThread();
    }
    pthread_mutex_unlock(&(d->mutex2));
}

/*!
  Attempt to lock the mutex. If another thread has locked the mutex
  then this call will block until that thread has unlocked it.
  Mutex locks and unlocks are recursive; that is, a thread can lock
  the same mutex multiple times and it will not be unlocked until
  a corresponding number of unlock() calls have been made.
*/

void QMutex::unlock()
{
    pthread_mutex_lock(&(d->mutex2));
    if(d->thread!=QThread::currentThread()) {
	fprintf(stderr,"Attempt to unlock from different thread than locker\n");
	fprintf(stderr,"Was locked by %ld, unlock attempt from %ld\n",
		d->thread,QThread::currentThread());
	pthread_mutex_unlock(&(d->mutex2));
	return;
    }

    d->count--;

    if (d->count<1) {
	int ret = pthread_mutex_unlock( &( d->mymutex ) );

	if( ret ) {
	    fprintf( stderr, "Mutex unlock failure %s\n", strerror( ret ) );
	    abort();
	}
	d->count=0;
    }
    pthread_mutex_unlock(&(d->mutex2));
}

/*!
  Unlocks the mutex. If the number of unlocks correspond to the number
  of preceding locks (i.e. if the mutex is locked twice and then unlocked
  twice) another thread will be able to lock the mutex. Unlocking a
  mutex that is already unlocked has no effect. Note that attempting
  to unlock a mutex in a different thread to the one that locked it is
  an error.
*/

bool QMutex::locked()
{
    int ret = pthread_mutex_trylock( &(d->mymutex) );
    if(ret==EBUSY) {
	return true;
    } else if(ret) {
	qFatal( "Mutex try lock failure %s\n", strerror( ret ) );
    }
    return false;
}

/*!
  Returns true if the mutex is locked.
*/

MUTEX_HANDLE QMutex::handle()
{
    // Eeevil?
    return (unsigned long) &( d->mymutex );
}

/*!
  Returns the (platform-specific) identifier of the mutex. Using this
  function is not unportable as such, but any use of the handle it returns
  is likely to be.
*/

class QThreadQtEvent {

public:

    QObject * o;
    QEvent * e;

};

class QThreadEventsPrivate : public QObject {

    Q_OBJECT

public:

    QThreadEventsPrivate();

    QList<QThreadQtEvent> myevents;
    QMutex myeventmutex;

public slots:

void sendEvents();

private:

};

#include "qthread_unix.moc"

static QThreadData * threadended;

QThreadEventsPrivate::QThreadEventsPrivate()
{
    myevents.setAutoDelete( TRUE );
    connect( qApp, SIGNAL( guiThreadAwake() ), this, SLOT( sendEvents() ) );
    threadended=new QThreadData;
}

void QThreadEventsPrivate::sendEvents()
{
    myeventmutex.lock();
    QThreadQtEvent * qte;
    for( qte = myevents.first(); qte != 0; qte = myevents.next() ) {
	qApp->postEvent( qte->o, qte->e );
    }
    myevents.clear();
    qApp->sendPostedEvents();
    myeventmutex.unlock();
}

static QThreadEventsPrivate * qthreadeventsprivate = 0;

extern "C" {
    static void * start_thread(void * t)
    {
	((QThread *)t)->runWrapper();
	return 0;
    }

    void destruct_dummy(void *)
    {
    }
}

/*!
  \class QThread qthread.h
  \brief The QThread class provides platform-independent threads

  \ingroup environment

  A QThread represents a separate thread of control within the program;
  it shares all data with other threads within the process but
  executes independently in the way that a separate program does on
  a multitasking operating system. Instead of starting in main(),
  however, QThreads begin executing in QThread::run, which you inherit
  to provide your code. For instance:

  class MyThread : public QThread {

  public:

    virtual void run();

  };

  void MyThread::run()
  {
    for(int count=0;count<20;count++) {
      sleep(1);
      qDebug("Ping!");
    }
  }

  int main()
  {
      QThread a;
      QThread b;
      a.start();
      b.start();
      sleep(30);
  }

  This will start two threads, each of which writes Ping! 20 times
  to the screen and exits. The sleep() at the end of main() is necessary
  because exiting main() ends the program, unceremoniously killing all
  other threads. Each MyThread stops executing when it reaches the
  end of MyThread::run, just as an application does when it leaves
  main().

*/

THREAD_HANDLE QThread::currentThread()
{
    // A pthread_t is an int
    return (THREAD_HANDLE)pthread_self();
}

/*!
  This returns the thread ID of the currently executing thread.
*/

void QThread::postEvent( QObject * o, QEvent * e )
{
    if( !qthreadeventsprivate ) {
	qthreadeventsprivate = new QThreadEventsPrivate();
    }
    qthreadeventsprivate->myeventmutex.lock();
    QThreadQtEvent * qte = new QThreadQtEvent;
    qte->o = o;
    qte->e = e;
    qthreadeventsprivate->myevents.append( qte );
    qthreadeventsprivate->myeventmutex.unlock();
    qApp->wakeUpGuiThread();
}

/*!
  Provides a way of posting an event from a thread which is not the
  event thread to an object. The event is put into a queue, then the
  event thread is woken which then sends the event to the object.
  It is important to note that the event handler for the event, when called,
  will be called from the event thread and not from the thread calling
  QThread::postEvent.
*/

THREAD_HANDLE QThread::handle()
{
    return (THREAD_HANDLE) d->mythread;
}

/*!
  Returns the platform-specific handle of the thread represented by
  this QThread. This value is not valid until QThread::start() has been
  called.
*/

void QThread::exit()
{
    QThreadEvent * done=(QThreadEvent *)threadended->data();
    done->wakeAll();
    pthread_exit(0);
}

/*!
  Ends execution of the currently-executing thread and wakes up any
  threads waiting for its termination.
*/

QThread::QThread()
{
    // Hmm. Not sure how to provide proper cleanup function here
    d=new QThreadPrivate;
    d->finished=false;
    threadended->setData( (void *) &(d->thread_done) );
}

/*!
  Constructs a new thread. The thread does not actually begin executing
  (and QThread::handle() does not return a valid thread id) until
  QThread::start is called.
*/

QThread::~QThread()
{
    delete d;
}

/*!
  QThread destructor. Note that deleting a QThread object will not stop
  the execution of the thread it represents.
*/

void QThread::wait()
{
    if(d->finished)
	return;
    d->thread_done.wait();
}

/*!
  This allows similar functionality to Posix Threads' pthread_join.
  A thread calling this will block until the thread associated with
  the QThread object has finished executing (that is, when it
  returns from QThread::run(). Multiple threads can wait for one thread to
  finish, and all will be woken.
*/

void QThread::start()
{
    // Error checking would be good
    pthread_t foo;
    pthread_create(&foo,0,start_thread,(void *)this);
    pthread_detach(foo);
}

/*!
  This begins actual execution of the thread. A new thread is created
  and begins executing in QThread::run(), which should be reimplemented
  in a QThread subclass to contain your code.
*/

void QThread::run()
{
    // Default implementation does nothing
}

/*!
  The default implementation of this method does nothing; it simply returns
  immediately, ending the thread's execution. It should be subclassed
  in order to do useful work. Returning from this method will end execution
  of the thread.
*/

void QThread::runWrapper()
{
    run();
    // Tell any threads waiting for us to wake up
    d->finished=true;
    d->thread_done.wakeAll();
}

/*!
  \class QThreadEvent qthread.h
  \brief The QThreadEvent class provides signalling of the occurrence of
         events between threads

  \ingroup environment

  QThreadEvents allow a thread to tell other threads that some sort
  of event has happened; one or many threads can block waiting for
  a QThreadEvent to signal an event, and a thread can call wakeOne
  to wake one randomly-selected event or wakeAll to wake them all.
  For example, say we have three tasks that should be performed every
  time the user presses a key; each task could be split into a thread,
  each of which would have a run() body like so:

  while(1) {
     key_pressed_event.wait();    // This is a QThreadEvent global variable
     // Key was pressed, do something interesting
     do_something();
  }

  A fourth thread would read key presses and wake the other three threads
  up every time it receives one, like so:

  while(1) {
     getchar();
     // Causes any thread in key_pressed_event.wait() to return from
     // that method and continue processing
     key_pressed_event.wakeAll();
  }

  Note that the order the three threads are woken up in is undefined,
  and that if some or all of the threads are still in do_something()
  when the key is pressed, they won't be woken up (since they're not
  waiting on the condition variable) and so the task will not be performed
  for that key press.  This can be avoided by, for example, doing something
  like this:

  QMutex mymutex;
  int mycount=0;

  // Worker thread code
  while(1) {
     key_pressed_event.wait();    // This is a QThreadEvent global variable
     mymutex.lock();
     mycount++;
     mymutex.unlock();
     do_something();
     mymutex.lock();
     mycount--;
     mymutex.unlock();
  }

  // Key reading thread code
  while(1) {
     getchar();
     mymutex.lock();
     // Sleep until there are no busy worker threads
     while(count>0) {
       sleep(1);
     }
     mymutex.unlock();
     key_pressed_event.wakeAll();
  }

  The mutexes are necessary because the results if two threads
  attempt to change the value of the same variable simultaneously
  are unpredictable.

*/

QThreadEvent::QThreadEvent()
{
    d=new QThreadEventPrivate;
}

/*!
  Constructs a new thread event signalling object.
*/

QThreadEvent::~QThreadEvent()
{
    delete d;
}
/*!
  Deletes the thread signalling object.
*/

void QThreadEvent::wait()
{
    int ret=pthread_mutex_lock(& (d->mutex) );
    if(ret) {
	qWarning("Threadevent wait lock error:%s",strerror(ret));
    }
    ret=pthread_cond_wait (&( d->mycond ), &( d->mutex ));
    if(ret) {
	qWarning("Threadevent wait error:%s",strerror(ret));
    }
    ret=pthread_mutex_unlock(& (d->mutex) );
    if(ret) {
	qWarning("Threadevent wait unlock error:%s",strerror(ret));
    }
}

/*!
  Wait on the thread event object. The thread calling this will block
  until another thread signals it using QThread::wakeOne or
  QThread::wakeAll.
*/

void QThreadEvent::wait(const QTime & t)
{
    // This is probably grotty
    timeval now;
    timespec ti;
    gettimeofday(&now,0);
    ti.tv_sec=now.tv_sec+t.second();
    ti.tv_nsec=((now.tv_usec/1000)+t.msec())*1000000;
    pthread_mutex_lock(& (d->mutex) );
    int ret=pthread_cond_timedwait (&( d->mycond ), &( d->mutex ),
				    &ti);
    pthread_mutex_unlock(& (d->mutex) );
    if(ret) {
	qWarning("Threadevent timed wait error:%s",strerror(ret));
    }
}

/*!
  This is similar to QThreadEvent::wait, but will only wait until the time
  specified by t, at which point it will return whether or not the
  event was signalled.
*/

void QThreadEvent::wakeOne()
{
    int ret=pthread_mutex_lock(& (d->mutex) );
    if(ret) {
	qFatal("Threadevent wakeOne lock error: %s\n",strerror(ret));
    }
    ret=pthread_cond_signal(& (d->mycond) );
    if(ret) {
	qFatal("Threadevent wakeOne error: %s\n",strerror(ret));
    }
    ret=pthread_mutex_unlock(& (d->mutex) );
    if(ret) {
	qFatal("Threadevent wakeOne unlock error: %s\n",strerror(ret));
    }
}

/*!
  This awakes one (randomly chosen) thread waiting on the QThreadEvent.
*/

void QThreadEvent::wakeAll()
{
    int ret=pthread_mutex_lock(& (d->mutex) );
    if(ret) {
	qFatal("Threadevent wakeAll lock error: %s\n",strerror(ret));
    }
    ret=pthread_cond_broadcast(& (d->mycond) );
    if(ret) {
	qFatal("Threadevent wakeAll error: %s\n",strerror(ret));
    }
    ret=pthread_mutex_unlock(& (d->mutex) );
    if(ret) {
	qFatal("Threadevent wakeAll unlock error: %s\n",strerror(ret));
    }
}

/*!
  This wakes all threads waiting on the QThreadEvent.
*/

THREADEVENT_HANDLE QThreadEvent::handle()
{
    return (unsigned long) &( d->mycond );
}

/*!
  Returns the platform-specific handle of the QThreadEvent. On a
  Posix Threads system this is a pthread_cond_t *.
*/


class QThreadDataPrivate {

public:

    pthread_key_t mykey;

};

/*!
  \class QThreadData qthread.h
  \brief The QThreadData class provides access to per-thread data

  QThreadData objects can be used to store data which is intended to
  be private to each thread - for instance per-thread global variables.
  Any thread can use setData() to store a pointer to some data and will
  get that same pointer back when it calls data(), regardless of what
  values other threads pass when calling setData on the same object.

*/

QThreadData::QThreadData()
{
    d=new QThreadDataPrivate;
    int ret=pthread_key_create( &(d->mykey), destruct_dummy);
    if(ret) {
	qFatal("Thread key create error: %s",strerror(ret));
    }
}

/*!
  Constructs a new thread data object.
*/

QThreadData::~QThreadData()
{
    delete d;
}

/*!
  Destroys the thread data object
*/

void QThreadData::setData(void * v)
{
    ret=pthread_setspecific( d->mykey, v);
    if(ret) {
	qWarning("Error setting thread data: %s",strerror(ret));
    }
    return (unsigned int)mykey;
}

/*!
  Sets a pointer to some thread data. If the same thread now calls
  data() it will get this value back, regardless of what calls other
  threads make to this object.
*/

void * QThreadData::data()
{
    return pthread_getspecific( d->mkey );
}

/*!
  Returns the pointer most recently set with setData().
*/

#endif
