/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qthread.h"
#include "qthreadstorage.h"
#include "qmutex.h"
#include "qmutexpool_p.h"
#include "qreadwritelock.h"

#include <qeventloop.h>
#include <qhash.h>

#include "qthread_p.h"


/*
  QThreadData
*/

QThreadData::QThreadData()
    : id(-1), eventDispatcher(0), tls(0)
{ }

QThreadData::~QThreadData()
{
    for (int i = 0; i < postEventList.size(); ++i) {
        const QPostEvent &pe = postEventList.at(i);
        if (pe.event) {
            pe.event->posted = false;
            delete pe.event;
        }
    }
}

QThreadData *QThreadData::get(QThread *thread)
{ return thread ? &thread->d_func()->data : 0; }



/*
  QThreadPrivate
*/

QThreadPrivate::QThreadPrivate()
    : QObjectPrivate(), running(false), finished(false), terminated(false),
      stackSize(0)
{
#if defined (Q_OS_UNIX)
    thread_id = 0;
#elif defined (Q_WS_WIN)
    handle = 0;
    id = 0;
    waiters = 0;
    terminationEnabled = true;
    terminatePending = false;
#endif

    static QBasicAtomic idCounter = Q_ATOMIC_INIT(1);
    for (;;) {
        data.id = idCounter;
        if (idCounter.testAndSet(data.id, data.id + 1))
            break;
    }
}

struct QThreadIdHash {
    QReadWriteLock lock;
    QHash<int, QThread *> table;
};

Q_GLOBAL_STATIC(QThreadIdHash, threadIdHash)

/*! \internal
 */
QThread *QThreadPrivate::threadForId(int id)
{
    QThreadIdHash *idHash = threadIdHash();
    if (!idHash)
        return 0;
    QReadWriteLockLocker locker(&idHash->lock, QReadWriteLock::ReadAccess);
    return idHash->table.value(id);
}

/*!
    \class QThread qthread.h
    \threadsafe
    \brief The QThread class provides platform-independent threads.

    \ingroup thread
    \ingroup environment
    \mainclass

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
    \value HighestPriority scheduled more often than HighPriority.

    \value TimeCriticalPriority scheduled as often as possible.

    \value InheritPriority use the same priority as the creating
           thread.  This is the default.
*/

/*!
    Constructs a new thread. The thread does not begin executing until
    start() is called.
*/
QThread::QThread(QObject *parent)
    : QObject(*(new QThreadPrivate), parent)
{
    Q_D(QThread);
    QThreadIdHash *idHash = threadIdHash();
    QReadWriteLockLocker locker(&idHash->lock, QReadWriteLock::WriteAccess);
    idHash->table.insert(d->data.id, this);
}

/*!
    QThread destructor.

    Note that deleting a QThread object will not stop the execution of
    the thread it represents. Deleting a running QThread (i.e.
    finished() returns false) will probably result in a program crash.
    You can wait() on a thread to make sure that it has finished.
*/
QThread::~QThread()
{
    Q_D(QThread);
    {
        QMutexLocker locker(&d->mutex);
        if (d->running && !d->finished)
            qWarning("QThread object destroyed while thread is still running.");
    }

    QThreadIdHash *idHash = threadIdHash();
    QReadWriteLockLocker locker(&idHash->lock, QReadWriteLock::WriteAccess);
    idHash->table.remove(d->data.id);
}

/*!
    Returns true is the thread is finished; otherwise returns false.
*/
bool QThread::isFinished() const
{
    Q_D(const QThread);
    QMutexLocker locker(&d->mutex);
    return d->finished;
}

/*!
    Returns true if the thread is running; otherwise returns false.
*/
bool QThread::isRunning() const
{
    Q_D(const QThread);
    QMutexLocker locker(&d->mutex);
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
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);
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
    Q_D(const QThread);
    QMutexLocker locker(&d->mutex);
    return d->stackSize;
}

/*!
    Enters the event loop and waits until exit() is called or the main
    widget is destroyed, and returns the value that was set to exit()
    (which is 0 if exit() is called via quit()).

    It is necessary to call this function to start event handling.

    \sa quit(), exit()
*/
int QThread::exec()
{
    QEventLoop eventLoop;
    int returnCode = eventLoop.exec();
    return returnCode;
}

/*!
    Tells the thread's event loop to exit with a return code.

    After calling this function, the thread leaves the event loop and
    returns from the call to QEventLoop::exec().  The
    QEventLoop::exec() function returns \a returnCode.

    By convention, a \a returnCode of 0 means success, any non-zero value
    indicates an error.

    Note that unlike the C library function of the same name, this
    function \e does return to the caller -- it is event processing
    that stops.

    This function does nothing if the thread does not have an event
    loop.

    \sa quit() QEventLoop
*/
void QThread::exit(int returnCode)
{
    Q_D(QThread);
    for (int i = 0; i < d->data.eventLoops.size(); ++i) {
        QEventLoop *eventLoop = d->data.eventLoops.at(i);
        eventLoop->exit(returnCode);
    }
}

/*!
    Tells the thread's event loop to exit with return code 0 (success).
    Equivalent to calling QThread::exit(0).

    This function does nothing if the thread does not have an event
    loop.

    \sa exit() QEventLoop
*/
void QThread::quit()
{ exit(); }

/*!
    \fn void QThread::run()

    This method is pure virtual, and must be implemented in derived
    classes in order to do useful work. Returning from this method
    will end the execution of the thread.

    \sa wait()
*/


/*! \internal
  Initializes the QThread system.
*/
void QThread::initialize()
{
    if (qt_global_mutexpool)
        return;
    qt_global_mutexpool = new QMutexPool(true);

#if defined (Q_OS_WIN)
    extern void qt_create_tls();
    qt_create_tls();
#endif
}


/*! \internal
  Cleans up the QThread system.
*/
void QThread::cleanup()
{
    delete qt_global_mutexpool;
    qt_global_mutexpool = 0;
}

/*!
    \fn bool QThread::finished() const

    Use isFinished() instead.
*/

/*!
    \fn bool QThread::running() const

    Use isRunning() instead.
*/

