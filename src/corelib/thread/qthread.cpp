/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
#include "qabstracteventdispatcher.h"

#include <qeventloop.h>
#include <qhash.h>

#include "qthread_p.h"

/*
  QThreadData
*/

QThreadData::QThreadData()
    : _ref(1), thread(0), quitNow(false), eventDispatcher(0), canWait(true), tls(0)
{
    // fprintf(stderr, "QThreadData %p created\n", this);
}

QThreadData::~QThreadData()
{
    Q_ASSERT(_ref == 0);

    QThread *t = thread;
    thread = 0;
    delete t;

    for (int i = 0; i < postEventList.size(); ++i) {
        const QPostEvent &pe = postEventList.at(i);
        if (pe.event) {
            --pe.receiver->d_func()->postedEvents;
#ifdef QT3_SUPPORT
            if (pe.event->type() == QEvent::ChildInserted)
                --pe.receiver->d_func()->postedChildInsertedEvents;
#endif
            pe.event->posted = false;
            delete pe.event;
        }
    }

    // fprintf(stderr, "QThreadData %p destroyed\n", this);
}

QThreadData *QThreadData::get2(QThread *thread)
{
    Q_ASSERT_X(thread != 0, "QThread", "internal error");
    return thread->d_func()->data;
}

void QThreadData::ref()
{
#ifndef QT_NO_THREAD
    (void) _ref.ref();
    Q_ASSERT(_ref != 0);
#endif
}

void QThreadData::deref()
{
#ifndef QT_NO_THREAD
    if (!_ref.deref())
        delete this;
#endif
}


#ifndef QT_NO_THREAD
/*
  QThreadPrivate
*/

QThreadPrivate::QThreadPrivate(QThreadData *d)
    : QObjectPrivate(), running(false), finished(false), terminated(false),
      stackSize(0), priority(QThread::InheritPriority), data(d)
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

    if (!data)
        data = new QThreadData;
}

QThreadPrivate::~QThreadPrivate()
{
    data->deref();
}

/*
  QAdoptedThread
*/

QAdoptedThread::QAdoptedThread(QThreadData *data)
    : QThread(*new QThreadPrivate(data))
{
    // thread should be running and not finished for the lifetime
    // of the application (even if QCoreApplication goes away)
    d_func()->running = true;
    d_func()->finished = false;
    init();

    // fprintf(stderr, "new QAdoptedThread = %p\n", this);
}

QAdoptedThread::~QAdoptedThread()
{
    QThreadPrivate::finish(this);

    // fprintf(stderr, "~QAdoptedThread = %p\n", this);
}

QThread *QAdoptedThread::createThreadForAdoption()
{
    QThread *t = new QAdoptedThread(0);
    t->moveToThread(t);
    return t;
}

/*!
    \class QThread
    \brief The QThread class provides platform-independent threads.

    \ingroup thread
    \ingroup environment
    \mainclass

    A QThread represents a separate thread of control within the
    program; it shares data with all the other threads within the
    process but executes independently in the way that a separate
    program does on a multitasking operating system. Instead of
    starting in \c main(), QThreads begin executing in run().
    To create your own threads, subclass QThread and reimplement
    run(). For example:

    \code
        class MyThread : public QThread
        {
        public:
            void run();
        };

        void MyThread::run()
        {
            QTcpSocket socket;
            // connect QTcpSocket's signals somewhere meaningful
            ...
            socket.connectToHost(hostName, portNumber);
            exec();
        }
    \endcode

    This will create a QTcpSocket in the thread and then execute the
    thread's event loop. Use the start() method to begin execution.
    Execution ends when you return from run(), just as an application
    does when it leaves main(). QThread will notifiy you via a signal
    when the thread is started(), finished(), and terminated(), or
    you can use isFinished() and isRunning() to query the state of
    the thread. Use wait() to block until the thread has finished
    execution.

    Each thread gets its own stack from the operating system. The
    operating system also determines the default size of the stack.
    You can use setStackSize() to set a custom stack size.

    Each QThread can have its own event loop. You can start the event
    loop by calling exec(); you can stop it by calling exit() or
    quit(). Having an event loop in a thread makes it possible to
    connect signals from other threads to slots in this threads,
    using a mechanism called \l{Qt::QueuedConnection}{queued
    connections}. It also makes it possible to use classes that
    require the event loop, such as QTimer and QTcpSocket, in the
    thread.

    In extreme cases, you may want to forcibly terminate() an
    executing thread. However, doing so is dangerous and discouraged.
    Please read the documentation for terminate() and
    setTerminationEnabled() for detailed information.

    The static functions currentThreadId() and currentThread() return
    identifiers for the currently executing thread. The former
    returns a platform specific ID for the thread; the latter returns
    a QThread pointer.

    QThread also provides platform independent sleep functions in
    varying resolutions. Use sleep() for full second resolution,
    msleep() for millisecond resolution, and usleep() for microsecond
    resolution.

    \sa {Thread Support in Qt}, QThreadStorage, QMutex, QSemaphore, QWaitCondition,
        {Mandelbrot Example}, {Semaphores Example}, {Wait Conditions Example}
*/

/*!
    \fn void QThread::started()

    This signal is emitted when the thread starts executing.

    \sa finished(), terminated()
*/

/*!
    \fn void QThread::finished()

    This signal is emitted when the thread has finished executing.

    \sa started(), terminated()
*/

/*!
    \fn void QThread::terminated()

    This signal is emitted when the thread is terminated.

    \sa started(), finished()
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
           thread. This is the default.
*/

/*!
    Returns a pointer to a QThread which represents the currently
    executing thread.
*/
QThread *QThread::currentThread()
{
    QThreadData *data = QThreadData::current();
    Q_ASSERT(data != 0);
    return data->thread;
}

/*!
    Constructs a new thread with the given \a parent. The thread does
    not begin executing until start() is called.

    \sa start()
*/
QThread::QThread(QObject *parent)
    : QObject(*(new QThreadPrivate), parent)
{
    Q_D(QThread);
    // fprintf(stderr, "QThreadData %p created for thread %p\n", d->data, this);
    d->data->thread = this;
}

/*! \internal
 */
QThread::QThread(QThreadPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    Q_D(QThread);
    // fprintf(stderr, "QThreadData %p taken from private data for thread %p\n", d->data, this);
    d->data->thread = this;
}

/*!
    Destroys the thread.

    Note that deleting a QThread object will not stop the execution
    of the thread it represents. Deleting a running QThread (i.e.
    isFinished() returns false) will probably result in a program
    crash. You can wait() on a thread to make sure that it has
    finished.
*/
QThread::~QThread()
{
    Q_D(QThread);
    {
        QMutexLocker locker(&d->mutex);
        if (d->running && !d->finished)
            qWarning("QThread: Destroyed while thread is still running");

        d->data->thread = 0;
    }
}

/*!
    Returns true is the thread is finished; otherwise returns false.

    \sa isRunning()
*/
bool QThread::isFinished() const
{
    Q_D(const QThread);
    QMutexLocker locker(&d->mutex);
    return d->finished;
}

/*!
    Returns true if the thread is running; otherwise returns false.

    \sa isFinished()
*/
bool QThread::isRunning() const
{
    Q_D(const QThread);
    QMutexLocker locker(&d->mutex);
    return d->running;
}

/*!
    Sets the maximum stack size for the thread to \a stackSize. If \a
    stackSize is greater than zero, the maximum stack size is set to
    \a stackSize bytes, otherwise the maximum stack size is
    automatically determined by the operating system.

    \warning Most operating systems place minimum and maximum limits
    on thread stack sizes. The thread will fail to start if the stack
    size is outside these limits.

    \sa stackSize()
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

    \sa setStackSize()
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
    Q_D(QThread);
    d->mutex.lock();
    QEventLoop eventLoop;
    d->mutex.unlock();
    int returnCode = eventLoop.exec();
    return returnCode;
}

/*!
    Tells the thread's event loop to exit with a return code.

    After calling this function, the thread leaves the event loop and
    returns from the call to QEventLoop::exec(). The
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
    QMutexLocker locker(&d->mutex);
    d->data->quitNow = true;
    for (int i = 0; i < d->data->eventLoops.size(); ++i) {
        QEventLoop *eventLoop = d->data->eventLoops.at(i);
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

    This method is pure virtual and must be implemented in derived
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

/*! \fn void QThread::setPriority(Priority priority)
    \since 4.1

    This function sets the \a priority for a running thread. If the
    thread is not running, this function does nothing and returns
    immediately.  Use start() to start a thread with a specific
    priority.

    The \a priority argument can be any value in the \c
    QThread::Priority enum except for \c InheritPriorty.

    \sa Priority priority() start()
*/

/*!
    \since 4.1

    Returns the priority for a running thread.  If the thread is not
    running, this function returns \c InheritPriority.

    \sa Priority setPriority() start()
*/
QThread::Priority QThread::priority() const
{
    Q_D(const QThread);
    QMutexLocker locker(&d->mutex);
    return d->priority;
}

#else // QT_NO_THREAD

#include <private/qcoreapplication_p.h>

QThread* QThread::instance = 0;

QThread::QThread() : QObject(*new QThreadPrivate, (QObject*)0)
{
    Q_D(QThread);
    d->data->thread = this;
    QCoreApplicationPrivate::theMainThread = this;
}

QThreadData* QThreadData::current()
{
    if (QThread::instance)
        return QThread::instance->d_func()->data;
    return 0;
}

#endif // QT_NO_THREAD
