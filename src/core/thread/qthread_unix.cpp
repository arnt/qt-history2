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

#include <unistd.h>

#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <sys/time.h>

#include "qthread.h"
#include "qthreadstorage.h"

#include "qmutex_p.h"
#include "qmutexpool_p.h"
#include "qthreadinstance_p.h"

#include <qcoreapplication.h>
#include <qpointer.h>

static QThreadInstance main_instance = {
    0, { 0, &main_instance }, 0, 0, 1, 0, PTHREAD_COND_INITIALIZER, 0
};

extern QMutexPool *static_qt_global_mutexpool;
static QMutexPool *qt_thread_mutexpool = 0;

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

typedef void*(*QtThreadCallback)(void*);

static pthread_once_t storage_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t storage_key;
static void create_storage_key()
{
    pthread_key_create(&storage_key, NULL);
}

#if defined(Q_C_CALLBACKS)
}
#endif


/**************************************************************************
 ** QThreadInstance
 *************************************************************************/

QThreadInstance *QThreadInstance::current()
{
    pthread_once(&storage_key_once, create_storage_key);
    QThreadInstance *ret =
        (QThreadInstance *) pthread_getspecific(storage_key);
    if (! ret) return &main_instance;
    return ret;
}

void QThreadInstance::init(unsigned int stackSize)
{
    stacksize = stackSize;
    args[0] = args[1] = args[2] = 0;
    thread_storage = 0;

    finished = false;
    running =false;
    orphan = false;

    pthread_cond_init(&thread_done, NULL);
    thread_id = 0;

    // threads have not been initialized yet, do it now
    if (! qt_thread_mutexpool) QThread::initialize();
}

void QThreadInstance::deinit()
{
    pthread_cond_destroy(&thread_done);
}

void *QThreadInstance::start(void *_arg)
{
    void **arg = (void **) _arg;

    pthread_once(&storage_key_once, create_storage_key);
    pthread_setspecific(storage_key, arg[1]);
    pthread_cleanup_push(QThreadInstance::finish, arg[1]);
    pthread_testcancel();

    QPointer<QThread> thr = reinterpret_cast<QThread *>(arg[0]);
    arg[2] = reinterpret_cast<void *>(thr->thread());
    thr->QObject::setThread(QThread::currentThread());
    emit thr->started();
    thr->run();
    if (thr) {
	emit thr->finished();
	QCoreApplication::sendPostedEvents();
	thr->QObject::setThread(reinterpret_cast<Qt::HANDLE>(arg[2]));
	arg[0] = arg[1] = arg[2] = 0;
    }

    pthread_cleanup_pop(true);
    return 0;
}

void QThreadInstance::finish(void *)
{
    QThreadInstance *d = current();

    if (! d) {
	qWarning("QThread: internal error: zero data for running thread.");
	return;
    }

    QMutexLocker locker(d->mutex());

    if (d->args[0] && d->args[1] && d->args[2]) {
	// terminated!
	QThread *thr = reinterpret_cast<QThread *>(d->args[0]);
	Qt::HANDLE old = reinterpret_cast<Qt::HANDLE>(d->args[2]);
	emit thr->terminated();
	thr->QObject::setThread(old);
    }

    d->running = false;
    d->finished = true;
    d->args[0] = d->args[1] = d->args[2] = 0;

    QThreadStorageData::finish(d->thread_storage);
    d->thread_storage = 0;

    d->thread_id = 0;
    pthread_cond_broadcast(&d->thread_done);

    if (d->orphan) {
        d->deinit();
	delete d;
    }
}

QMutex *QThreadInstance::mutex() const
{
    return qt_thread_mutexpool ? qt_thread_mutexpool->get((void *) this) : 0;
}

void QThreadInstance::terminate()
{
    if (! thread_id) return;
    pthread_cancel(thread_id);
}


/**************************************************************************
 ** QThread
 *************************************************************************/

/*!
    This returns the thread handle of the currently executing thread.

    \warning The handle returned by this function is used for internal
    purposes and should \e not be used in any application code. On
    Windows, the returned value is a pseudo handle for the current
    thread, and it cannot be used for numerical comparison.
*/
Qt::HANDLE QThread::currentThread()
{
    return (Qt::HANDLE)pthread_self();
}

/*! \internal
  Initializes the QThread system.
*/
void QThread::initialize()
{
    extern QMutexPool *static_qt_global_mutexpool;
    if (static_qt_global_mutexpool)
	return;

    static_qt_global_mutexpool = new QMutexPool(true);
    qt_thread_mutexpool = new QMutexPool(false);
}

/*! \internal
  Cleans up the QThread system.
*/
void QThread::cleanup()
{
    delete static_qt_global_mutexpool;
    delete qt_thread_mutexpool;
    static_qt_global_mutexpool = 0;
    qt_thread_mutexpool = 0;

    // cleanup the QThreadInstance for the main() thread
    QThreadInstance::finish(&main_instance);
}

/*!
    Ends the execution of the calling thread and wakes up any threads
    waiting for its termination.
*/
void QThread::exit()
{
    pthread_exit(0);
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
    function will wait until the the thread has finished and then
    restart the thread.

    \sa Priority
*/
void QThread::start(Priority priority)
{
    QMutexLocker locker(d->mutex());

    if (d->running)
        pthread_cond_wait(&d->thread_done, &locker.mutex()->d->mutex);
    d->running = true;
    d->finished = false;

    int ret;
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
		qWarning("QThread: could not determine the default schedule policy");
		sched_policy = SCHED_RR;
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
	    }

	    int prio_min = sched_get_priority_min(sched_policy);
	    int prio_max = sched_get_priority_max(sched_policy);
            if (prio_min == -1 || prio_max == -1) {
                // failed to get the scheduling parameters, don't
                // bother setting the priority
                qWarning("QThread: could not determine priority range");
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
		prio = (((prio_max - prio_min) / TimeCriticalPriority) *
			priority) + prio_min;
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

    if (d->stacksize > 0) {
#if defined(_POSIX_THREAD_ATTR_STACKSIZE) && (_POSIX_THREAD_ATTR_STACKSIZE-0 > 0)
	ret = pthread_attr_setstacksize(&attr, d->stacksize);
#else
	ret = ENOSYS; // stack size not supported, automatically fail
#endif // _POSIX_THREAD_ATTR_STACKSIZE

	if (ret) {
	    qWarning("QThread::start: thread stack size error: %s", strerror(ret)) ;

	    // we failed to set the stacksize, and as the documentation states,
	    // the thread will fail to run...
	    d->running = false;
	    d->finished = false;
	    return;
	}
    }

    d->args[0] = this;
    d->args[1] = d;
    d->args[2] = 0;
    ret = pthread_create(&d->thread_id, &attr, (QtThreadCallback)QThreadInstance::start, d->args);
    pthread_attr_destroy(&attr);

    if (ret) {
	qWarning("QThread::start: thread creation error: %s", strerror(ret));

	d->running = false;
	d->finished = false;
	d->args[0] = d->args[1] = d->args[2] = 0;
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
    QMutexLocker locker(d->mutex());

    if (d->thread_id == pthread_self()) {
	qWarning("QThread::wait: thread tried to wait on itself");
	return false;
    }

    if (d->finished || ! d->running)
	return true;

    int ret;
    if (time != ULONG_MAX) {
	struct timeval tv;
	gettimeofday(&tv, 0);

	timespec ti;
	ti.tv_nsec = ( tv.tv_usec + ( time % 1000 ) * 1000 ) * 1000;
	ti.tv_sec = tv.tv_sec + (time / 1000) + ( ti.tv_nsec / 1000000000 );
        ti.tv_nsec %= 1000000000;

	ret = pthread_cond_timedwait(&d->thread_done, &locker.mutex()->d->mutex, &ti);
    } else {
	ret = pthread_cond_wait(&d->thread_done, &locker.mutex()->d->mutex);
    }

    if (ret != 0 && ret != ETIMEDOUT)
	qWarning("QWaitCondition::wait() failure: %s",strerror(ret));

    return (ret == 0);
}
