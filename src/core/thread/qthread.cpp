/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qthread.h"
#include "qmutex.h"

#include <qeventloop.h>
#include <qhash.h>

#include "qthread_p.h"

#define d d_func()
#define q q_func()




/*
  QThreadPrivate
*/

QThreadPrivate::QThreadPrivate()
    : QObjectPrivate(), running(false), finished(false), terminated(false),
      stackSize(0), eventloop(0), tls(0)
{
#ifdef Q_OS_UNIX
    thread_id = 0;
#endif
}

/*
    Sets the eventloop for the specified thread.
*/
static QEventLoop *globalEventLoop = 0;
void QThreadPrivate::setEventLoop(QThread *thread, QEventLoop *eventLoop)
{
    if (!thread) {
        if (eventLoop)
            Q_ASSERT_X(!globalEventLoop, "QEventLoop",
                       "Cannot have more than one event loop per application");
        globalEventLoop = eventLoop;
        return;
    }
    if (eventLoop)
        Q_ASSERT_X(!thread->d->eventloop, "QEventLoop",
                   "Cannot have more than one event loop per thread");
    thread->d->eventloop = eventLoop;
}

/*
    Returns the eventloop for the specified thread.
*/
QEventLoop *QThreadPrivate::eventLoop(QThread *thread)
{
    if (thread)
        return thread->d->eventloop;
    return globalEventLoop;
}

/*
    Returns the eventloop for the specified thread.
*/
Q_GLOBAL_STATIC(QPostEventList, globalPostEventList)
QPostEventList *QThreadPrivate::postEventList(QThread *thread)
{
    if (thread)
        return &thread->d->postedEvents;
    static QStaticSpinLock spinlock = 0;
    QSpinLockLocker locker(spinlock);
    return globalPostEventList();
}

/*
   Returns the tls for the specified thread.
*/
void **&QThreadPrivate::threadLocalStorage(QThread *thread)
{
    if (!thread) {
        static void **globalTLS = 0;
        return globalTLS;
    }
    return thread->d->tls;
}




/*!
    \class QThread qthread.h
    \threadsafe
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
        for(int count = 0; count < 20; count++) {
            sleep(1);
            qDebug("Ping!");
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
    \fn void QThread::started()

    This signal is emitted when the thread starts executing.
*/

/*!
    \fn void QThread::finished()

    This signal is emitted when the thread has finished executing.
*/

/*!
    \fn void QThread::terminated()

    This signal is emitted when the thread is terminated.
*/

/*!
    \enum QThread::Priority

    This enum type indicates how the operating system should schedule
    newly created threads.

    \value IdlePriority scheduled only when no other threads are
           running.

    \value LowestPriority scheduled less often than LowPriority.
    \value LowPriority scheduled less often than NormalPriority.

    \value NormalPriority the default priority of the operating
           system.

    \value HighPriority scheduled more often than NormalPriority.
    \value HighestPriority scheduled more often then HighPriority.

    \value TimeCriticalPriority scheduled as often as possible.

    \value InheritPriority use the same priority as the creating
           thread.  This is the default.
*/

/*!
    Constructs a new thread. The thread does not begin executing until
    start() is called.
*/
QThread::QThread()
    : QObject(*(new QThreadPrivate), 0)
{ }

/*!
    QThread destructor.

    Note that deleting a QThread object will not stop the execution of
    the thread it represents. Deleting a running QThread (i.e.
    finished() returns false) will probably result in a program crash.
    You can wait() on a thread to make sure that it has finished.
*/
QThread::~QThread()
{
    QMutexLocker locker(d->mutex());
    if (d->running && !d->finished)
        qWarning("QThread object destroyed while thread is still running.");
}

/*!
    Returns true is the thread is finished; otherwise returns false.
*/
bool QThread::isFinished() const
{
    QMutexLocker locker(d->mutex());
    return d->finished;
}

/*!
    Returns true if the thread is running; otherwise returns false.
*/
bool QThread::isRunning() const
{
    QMutexLocker locker(d->mutex());
    return d->running;
}

/*!
    Set the maximum stack size for the thread to \a stackSize. If \a
    stackSize is greater than zero, the maximum stack size is set to
    \a stackSize bytes, otherwise the maximum stack size is
    automatically determined by the operating system.

    \warning Most operating systems place minimum and maximum limits
    on thread stack sizes. The thread will fail to start if the stack
    size is outside these limits.
*/
void QThread::setStackSize(uint stackSize)
{
    QMutexLocker locker(d->mutex());
    Q_ASSERT_X(!d->running, "QThread::setStackSize",
               "cannot change stack size while the thread is running");
    d->stackSize = stackSize;
}

/*!
    Returns the maximum stack size for the thread (if set with
    setStackSize()); otherwise returns zero.
*/
uint QThread::stackSize() const
{
    QMutexLocker locker(d->mutex());
    return d->stackSize;
}

/*!
    \fn void QThread::run()

    This method is pure virtual, and must be implemented in derived
    classes in order to do useful work. Returning from this method
    will end the execution of the thread.

    \sa wait()
*/
