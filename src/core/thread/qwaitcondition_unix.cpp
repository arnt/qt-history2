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

#include "qplatformdefs.h"
#include "qwaitcondition.h"
#include "qmutex.h"
#include "qmutex_p.h"
#include <errno.h>
#include <string.h>


struct QWaitConditionPrivate {
    pthread_cond_t cond;
};


/*!
    \class QWaitCondition qwaitcondition.h
    \threadsafe
    \brief The QWaitCondition class allows waiting/waking for conditions between threads.

    \ingroup thread
    \ingroup environment

    QWaitConditions allow a thread to tell other threads that some
    sort of condition has been met; one or many threads can block
    waiting for a QWaitCondition to set a condition with wakeOne() or
    wakeAll(). Use wakeOne() to wake one randomly selected event or
    wakeAll() to wake them all. For example, say we have three tasks
    that should be performed every time the user presses a key; each
    task could be split into a thread, each of which would have a
    run() body like this:

    \code
    QWaitCondition key_pressed;

    for (;;) {
        key_pressed.wait(); // This is a QWaitCondition global variable
        // Key was pressed, do something interesting
        do_something();
    }
    \endcode

    A fourth thread would read key presses and wake the other three
    threads up every time it receives one, like this:

    \code
    QWaitCondition key_pressed;

    for (;;) {
        getchar();
        // Causes any thread in key_pressed.wait() to return from
        // that method and continue processing
        key_pressed.wakeAll();
    }
    \endcode

    Note that the order the three threads are woken up in is
    undefined, and that if some or all of the threads are still in
    do_something() when the key is pressed, they won't be woken up
    (since they're not waiting on the condition variable) and so the
    task will not be performed for that key press. This can be
    avoided by, for example, doing something like this:

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
        while(mycount > 0) {
            mymutex.unlock();
            sleep(1);
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
    Constructs a new event signalling, i.e. wait condition, object.
*/
QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate;

    int ret = pthread_cond_init(&d->cond, NULL);
    if (ret)
        qWarning("QWaitCondition: constructor failure: %s", strerror(ret));
}


/*!
    Deletes the event signalling, i.e. wait condition, object.
*/
QWaitCondition::~QWaitCondition()
{
    int ret = pthread_cond_destroy(&d->cond);
    if (ret != 0) {
        qWarning("QWaitCondition: destructor failure: %s", strerror(ret));

        // seems we have threads waiting on us, lets wake them up
        pthread_cond_broadcast(&d->cond);
    }

    delete d;
}

/*!
    This wakes one thread waiting on the QWaitCondition. The thread
    that is woken up depends on the operating system's scheduling
    policies, and cannot be controlled or predicted.

    \sa wakeAll()
*/
void QWaitCondition::wakeOne()
{
    int ret = pthread_cond_signal(&d->cond);
    if (ret != 0)
        qWarning("QWaitCondition::wakeOne() failure: %s", strerror(ret));
}

/*!
    This wakes all threads waiting on the QWaitCondition. The order in
    which the threads are woken up depends on the operating system's
    scheduling policies, and cannot be controlled or predicted.

    \sa wakeOne()
*/
void QWaitCondition::wakeAll()
{
    int ret =pthread_cond_broadcast(&d->cond);
    if (ret != 0)
        qWarning("QWaitCondition::wakeAll() failure: %s", strerror(ret));
}

/*!
    Release the locked \a mutex and wait on the thread event object.
    The \a mutex must be initially locked by the calling thread. If \a
    mutex is not in a locked state, this function returns immediately.
    If \a mutex is a recursive mutex, this function returns
    immediately. The \a mutex will be unlocked, and the calling thread
    will block until either of these conditions is met:
    \list
    \i Another thread signals it using wakeOne() or wakeAll(). This
       function will return true in this case.
    \i \a time milliseconds has elapsed. If \a time is ULONG_MAX (the
       default), then the wait will never timeout (the event must be
       signalled). This function will return false if the wait timed
       out.
    \endlist

    The mutex will be returned to the same locked state. This funtion
    is provided to allow the atomic transition from the locked state
    to the wait state.

    \sa wakeOne(), wakeAll()
*/
bool QWaitCondition::wait(QMutex *mutex, unsigned long time)
{
    if (! mutex)
        return false;

    if (mutex->d->recursive) {
        qWarning("QWaitCondition: cannot wait on recursive mutexes");
        return false;
    }

    int ret;
    if (time != ULONG_MAX) {
        struct timeval tv;
        gettimeofday(&tv, 0);

        timespec ti;
        ti.tv_nsec = (tv.tv_usec + (time % 1000) * 1000) * 1000;
        ti.tv_sec = tv.tv_sec + (time / 1000) + (ti.tv_nsec / 1000000000);
        ti.tv_nsec %= 1000000000;

        ret = pthread_cond_timedwait(&d->cond, &mutex->d->mutex, &ti);
    } else {
        ret = pthread_cond_wait(&d->cond, &mutex->d->mutex);
    }

    if (ret && ret != ETIMEDOUT)
        qWarning("QWaitCondition::wait() failure: %s",strerror(ret));

    return (ret == 0);
}
