/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qthread.h"
#include "qthreadinstance_p.h"




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

    If \a stackSize is greater than zero, the maximum stack size is
    set to \a stackSize bytes, otherwise the maximum stack size is
    automatically determined by the operating system.

    \warning Most operating systems place minimum and maximum limits
    on thread stack sizes. The thread will fail to start if the stack
    size is outside these limits.
*/
QThread::QThread(unsigned int stackSize)
{
    d = new QThreadInstance;
    d->init(stackSize);
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
    QMutexLocker locker(d->mutex());
    if (d->running && !d->finished) {
	qWarning("QThread object destroyed while thread is still running.");
	d->orphan = true;
	return;
    }

    d->deinit();
    delete d;
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
    QMutexLocker locker(d->mutex());
    if (d->finished || !d->running)
	return;
    d->terminate();
}

/*!
    Returns true is the thread is finished; otherwise returns FALSE.
*/
bool QThread::isFinished() const
{
    QMutexLocker locker(d->mutex());
    return d->finished;
}

/*!
    Returns true if the thread is running; otherwise returns FALSE.
*/
bool QThread::isRunning() const
{
    QMutexLocker locker(d->mutex());
    return d->running;
}

/*!
    \fn void QThread::run()

    This method is pure virtual, and must be implemented in derived
    classes in order to do useful work. Returning from this method
    will end the execution of the thread.

    \sa wait()
*/
