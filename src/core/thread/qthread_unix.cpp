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

#include "qthread.h"
#include "qthreadstorage.h"
#include "qthread_p.h"

#include <sched.h>
#include <errno.h>
#include <string.h>


/*
  QThreadData
*/

static pthread_once_t current_data_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t current_data_key;
static void create_current_data_key()
{ pthread_key_create(&current_data_key, NULL); }

QThreadData *QThreadData::current()
{
    pthread_once(&current_data_key_once, create_current_data_key);
    return reinterpret_cast<QThreadData *>(pthread_getspecific(current_data_key));
}

void QThreadData::setCurrent(QThreadData *data)
{
    pthread_once(&current_data_key_once, create_current_data_key);
    pthread_setspecific(current_data_key, data);
}



/*
   QThreadPrivate
*/

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

typedef void*(*QtThreadCallback)(void*);

#if defined(Q_C_CALLBACKS)
}
#endif

static pthread_once_t current_thread_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t current_thread_key;
static void create_current_thread_key()
{ pthread_key_create(&current_thread_key, NULL); }

void *QThreadPrivate::start(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    pthread_once(&current_thread_key_once, create_current_thread_key);
    pthread_setspecific(current_thread_key, arg);
    pthread_cleanup_push(QThreadPrivate::finish, arg);

    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadData::setCurrent(&thr->d_func()->data);

    emit thr->started();
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    thr->run();

    pthread_cleanup_pop(1);
    return 0;
}

void QThreadPrivate::finish(void *arg)
{
    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadPrivate *d = thr->d_func();
    QMutexLocker locker(&d->mutex);

    d->running = false;
    d->finished = true;
    if (d->terminated)
        emit thr->terminated();
    d->terminated = false;
    emit thr->finished();

    QThreadStorageData::finish(d->data.tls);
    d->data.tls = 0;

    d->thread_id = 0;
    d->thread_done.wakeAll();
}




/**************************************************************************
 ** QThread
 *************************************************************************/

/*!
    Returns the thread handle of the currently executing thread.

    \warning The handle returned by this function is used for internal
    purposes and should \e not be used in any application code. On
    Windows, the returned value is a pseudo handle for the current
    thread, and it cannot be used for numerical comparison.
*/
Qt::HANDLE QThread::currentThread()
{
    // requires a C cast here otherwise we run into trouble on AIX
    return (Qt::HANDLE)pthread_self();
}

/*!
    Returns a pointer to the currently executing QThread. If the
    current thread was not started using the QThread API (e.g. the GUI
    thread), this function returns zero.
*/
QThread *QThread::currentQThread()
{
    pthread_once(&current_thread_key_once, create_current_thread_key);
    return reinterpret_cast<QThread *>(pthread_getspecific(current_thread_key));
}

/*  \internal
    helper function to do thread sleeps, since usleep()/nanosleep()
    aren't reliable enough (in terms of behavior and availability)
*/
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
    System independent sleep. This causes the current thread to sleep
    for \a secs seconds.
*/
void QThread::sleep(unsigned long secs)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;
    ti.tv_sec = tv.tv_sec + secs;
    ti.tv_nsec = (tv.tv_usec * 1000);
    thread_sleep(&ti);
}

/*!
    System independent sleep. This causes the current thread to sleep
    for \a msecs milliseconds
*/
void QThread::msleep(unsigned long msecs)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;

    ti.tv_nsec = (tv.tv_usec + (msecs % 1000) * 1000) * 1000;
    ti.tv_sec = tv.tv_sec + (msecs / 1000) + (ti.tv_nsec / 1000000000);
    ti.tv_nsec %= 1000000000;
    thread_sleep(&ti);
}

/*!
    System independent sleep. This causes the current thread to sleep
    for \a usecs microseconds
*/
void QThread::usleep(unsigned long usecs)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    struct timespec ti;

    ti.tv_nsec = (tv.tv_usec + (usecs % 1000000)) * 1000;
    ti.tv_sec = tv.tv_sec + (usecs / 1000000) + (ti.tv_nsec / 1000000000);
    ti.tv_nsec %= 1000000000;
    thread_sleep(&ti);
}

/*!
    Begins execution of the thread by calling run(), which should be
    reimplemented in a QThread subclass to contain your code.  The
    operating system will schedule the thread according to the \a
    priority argument.

    If you try to start a thread that is already running, this
    function will wait until the thread has finished and then
    restart the thread.

    \sa Priority
*/
void QThread::start(Priority priority)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);
    if (d->running)
        d->thread_done.wait(locker.mutex());

    d->running = true;
    d->finished = false;
    d->terminated = false;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) && (_POSIX_THREAD_PRIORITY_SCHEDULING-0 >= 0)
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
                // failed to get the scheduling policy, don't bother
                // setting the priority
                qWarning("QThread: cannot determine default scheduler policy");
                break;
            }

            int prio_min = sched_get_priority_min(sched_policy);
            int prio_max = sched_get_priority_max(sched_policy);
            if (prio_min == -1 || prio_max == -1) {
                // failed to get the scheduling parameters, don't
                // bother setting the priority
                qWarning("QThread: cannot determine scheduler priority range");
                break;
            }

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
                prio = (((prio_max - prio_min) / TimeCriticalPriority) * priority) + prio_min;
                prio = qMax(prio_min, qMin(prio_max, prio));
                break;
            }

            sched_param sp;
            sp.sched_priority = prio;

            pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
            pthread_attr_setschedparam(&attr, &sp);
            break;
        }
    }
#endif // _POSIX_THREAD_PRIORITY_SCHEDULING

    if (d->stackSize > 0) {
#if defined(_POSIX_THREAD_ATTR_STACKSIZE) && (_POSIX_THREAD_ATTR_STACKSIZE-0 > 0)
        int code = pthread_attr_setstacksize(&attr, d->stackSize);
#else
        int code = ENOSYS; // stack size not supported, automatically fail
#endif // _POSIX_THREAD_ATTR_STACKSIZE

        if (code) {
            qWarning("QThread::start: thread stack size error: %s", strerror(code)) ;

            // we failed to set the stacksize, and as the documentation states,
            // the thread will fail to run...
            d->running = false;
            d->finished = false;
            return;
        }
    }

    int code =
        pthread_create(&d->thread_id, &attr, QThreadPrivate::start, this);
    if (code == EPERM) {
        // caller does not have permission to set the scheduling
        // parameters/policy
        pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
        code =
            pthread_create(&d->thread_id, &attr, QThreadPrivate::start, this);
    }

    pthread_attr_destroy(&attr);

    if (code) {
        qWarning("QThread::start: thread creation error: %s", strerror(code));

        d->running = false;
        d->finished = false;
        d->thread_id = 0;
    }
}

/*!
    This function terminates the execution of the thread. The thread
    may or may not be terminated immediately, depending on the
    operating systems scheduling policies. Use QThread::wait()
    after terminate() for synchronous termination.

    When the thread is terminated, all threads waiting for the thread
    to finish will be woken up.

    \warning This function is dangerous, and its use is discouraged.
    The thread can be terminate at any point in its code path.  Threads
    can be terminated while modifying data.  There is no chance for
    the thread to cleanup after itself, unlock any held mutexes, etc.
    In short, use this function only if \e absolutely necessary.

    Termination can be explicitly enabled or disabled by calling
    QThread::setTerminationEnabled().  Calling this function while
    termination is disabled results in the termination being deferred,
    until termination is re-enabled.  See the documentation of
    QThread::setTerminationEnabled() for more information.

    \sa QThread::setTerminationEnabled()
*/
void QThread::terminate()
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (!d->thread_id)
        return;

    int code = pthread_cancel(d->thread_id);
    if (code) {
        qWarning("QThread::start: thread termination error: %s", strerror(code));
    } else {
        d->terminated = true;
    }
}

/*!
    A thread calling this function will block until either of these
    conditions is met:

    \list
    \i The thread associated with this QThread object has finished
       execution (i.e. when it returns from \l{run()}). This function
       will return true if the thread has finished. It also returns
       true if the thread has not been started yet.
    \i \a time milliseconds has elapsed. If \a time is ULONG_MAX (the
        default), then the wait will never timeout (the thread must
        return from \l{run()}). This function will return false if the
        wait timed out.
    \endlist

    This provides similar functionality to the POSIX \c pthread_join() function.
*/
bool QThread::wait(unsigned long time)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (d->thread_id == pthread_self()) {
        qWarning("QThread::wait: thread tried to wait on itself");
        return false;
    }

    if (d->finished || !d->running)
        return true;

    return d->thread_done.wait(locker.mutex(), time);
}

/*!
    Enables or disables termination of the current thread based on the
    \a enabled parameter.  The thread must have been started by
    QThread.

    When \a enabled is false, termination is disabled.  Future calls
    to QThread::terminate() will return immediately without effect.
    Instead, the termination is deferred until termination is enabled.

    When \a enabled is true, termination is enabled.  Future calls to
    QThread::terminate() will terminate the thread normally.  If
    termination has been deferred (i.e. QThread::terminate() was
    called with termination disabled), this function will terminate
    the calling thread \e immediately.  Note that this function will
    not return in this case.

    \sa QThread::terminate()
*/
void QThread::setTerminationEnabled(bool enabled)
{
    Q_ASSERT_X(currentQThread != 0, "QThread::setTerminationEnabled()",
               "Current thread was not started with QThread.");
    pthread_setcancelstate(enabled ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE, NULL);
    if (enabled)
        pthread_testcancel();
}
