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
#include "qthread_p.h"
#include "qthreadstorage.h"

#include <private/qeventdispatcher_win_p.h>
#include <qcoreapplication.h>
#include <qpointer.h>

#include <windows.h>
#include <assert.h>

#ifndef Q_OS_TEMP
#ifndef _MT
#define _MT
#endif
#include <process.h>
#endif

static DWORD qt_current_thread_tls_index = TLS_OUT_OF_INDEXES;
void qt_create_tls()
{
    if (qt_current_thread_tls_index != TLS_OUT_OF_INDEXES)
        return;
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    qt_current_thread_tls_index = TlsAlloc();
}




/**************************************************************************
 ** QThreadPrivate
 *************************************************************************/

void QThreadPrivate::setCurrentThread(QThread *thread)
{
    qt_create_tls();
    TlsSetValue(qt_current_thread_tls_index, thread);
}

unsigned int __stdcall QThreadPrivate::start(void *arg)
{
    qt_create_tls();
    TlsSetValue(qt_current_thread_tls_index, arg);
    QThread::setTerminationEnabled(false);

    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadPrivate::setCurrentThread(thr);

    QThreadData *data = &thr->d_func()->data;
    data->quitNow = false;
    // ### TODO: allow the user to create a custom event dispatcher
    data->eventDispatcher = new QEventDispatcherWin32;
    data->eventDispatcher->startingUp();

    emit thr->started();
    QThread::setTerminationEnabled(true);
    thr->run();

    finish(arg);
    return 0;
}

void QThreadPrivate::finish(void *arg, bool lockAnyway)
{
    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadPrivate *d = thr->d_func();
    QThreadData *data = &d->data;

    if (lockAnyway)
        d->mutex.lock();
    d->priority = QThread::InheritPriority;
    d->running = false;
    d->finished = true;
    if (d->terminated)
        emit thr->terminated();
    d->terminated = false;
    emit thr->finished();

    if (data->eventDispatcher) {
        data->eventDispatcher->closingDown();
        QAbstractEventDispatcher *eventDispatcher = data->eventDispatcher;
        data->eventDispatcher = 0;
        delete eventDispatcher;
    }

    QThreadStorageData::finish(data->tls);
    data->tls = 0;

    if (!d->waiters) {
        CloseHandle(d->handle);
        d->handle = 0;
    }

    d->id = 0;

    if (lockAnyway)
        d->mutex.unlock();
}


/**************************************************************************
 ** QThread
 *************************************************************************/

Qt::HANDLE QThread::currentThreadId()
{
    return (Qt::HANDLE)GetCurrentThreadId();
}

QThread *QThread::currentThread()
{
    return reinterpret_cast<QThread *>(TlsGetValue(qt_current_thread_tls_index));
}

void QThread::sleep(unsigned long secs)
{
    ::Sleep(secs * 1000);
}

void QThread::msleep(unsigned long msecs)
{
    ::Sleep(msecs);
}

void QThread::usleep(unsigned long usecs)
{
    ::Sleep((usecs / 1000) + 1);
}


void QThread::start(Priority priority)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (d->running)
        return;

    d->running = true;
    d->finished = false;
    d->terminated = false;

    /*
      NOTE: we create the thread in the suspended state, set the
      priority and then resume the thread.

      since threads are created with normal priority by default, we
      could get into a case where a thread (with priority less than
      NormalPriority) tries to create a new thread (also with priority
      less than NormalPriority), but the newly created thread preempts
      its 'parent' and runs at normal priority.
    */
    d->handle = (Qt::HANDLE) _beginthreadex(NULL, d->stackSize, QThreadPrivate::start,
                                            this, CREATE_SUSPENDED, &(d->id));

    if (!d->handle) {
        qErrnoWarning(errno, "QThread::start: Failed to create thread");
        d->running = false;
        d->finished = true;
        return;
    }

    // Since Win 9x will have problems if the priority is idle or time critical
    // we have to use the closest one instead
    int prio;
    d->priority = priority;
    switch (d->priority) {
    case IdlePriority:
	if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
	    prio = THREAD_PRIORITY_LOWEST;
	} else {
	    prio = THREAD_PRIORITY_IDLE;
	}
        break;

    case LowestPriority:
        prio = THREAD_PRIORITY_LOWEST;
        break;

    case LowPriority:
        prio = THREAD_PRIORITY_BELOW_NORMAL;
        break;

    case NormalPriority:
        prio = THREAD_PRIORITY_NORMAL;
        break;

    case HighPriority:
        prio = THREAD_PRIORITY_ABOVE_NORMAL;
        break;

    case HighestPriority:
        prio = THREAD_PRIORITY_HIGHEST;
        break;

    case TimeCriticalPriority:
	if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
	    prio = THREAD_PRIORITY_HIGHEST;
	} else {
	    prio = THREAD_PRIORITY_TIME_CRITICAL;
	}
        break;

    case InheritPriority:
    default:
        prio = GetThreadPriority(GetCurrentThread());
        break;
    }

    if (!SetThreadPriority(d->handle, prio)) {
        qErrnoWarning("QThread::start: Failed to set thread priority");
    }

    if (ResumeThread(d->handle) == (DWORD) -1) {
        qErrnoWarning("QThread::start: Failed to resume new thread");
    }
}

void QThread::terminate()
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);
    if (!d->running)
        return;
    if (!d->terminationEnabled) {
        d->terminatePending = true;
        return;
    }
    TerminateThread(d->handle, 0);
    d->terminated = true;
    QThreadPrivate::finish(this, false);
}

bool QThread::wait(unsigned long time)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (d->id == GetCurrentThreadId()) {
        qWarning("Thread tried to wait on itself");
        return false;
    }
    if (d->finished || !d->running)
        return true;

    ++d->waiters;
    locker.mutex()->unlock();

    bool ret = false;
    switch (WaitForSingleObject(d->handle, time)) {
    case WAIT_OBJECT_0:
        ret = true;
        break;
    case WAIT_FAILED:
        qErrnoWarning("QThread::wait: Thread wait failure");
        break;
    case WAIT_ABANDONED:
    case WAIT_TIMEOUT:
    default:
        break;
    }

    locker.mutex()->lock();
    --d->waiters;

    if (ret && !d->finished) {
        // thread was terminated by someone else
        d->terminated = true;
        QThreadPrivate::finish(this, false);
    }

    if (d->finished && !d->waiters) {
        CloseHandle(d->handle);
        d->handle = 0;
    }

    return ret;
}

void QThread::setTerminationEnabled(bool enabled)
{
    QThread *thr = currentThread();
    Q_ASSERT_X(thr != 0, "QThread::setTerminationEnabled()",
               "Current thread was not started with QThread.");
    QThreadPrivate *d = thr->d_func();
    QMutexLocker locker(&d->mutex);
    d->terminationEnabled = enabled;
    if (enabled && d->terminatePending) {
        d->terminated = true;
        QThreadPrivate::finish(thr, false);
        locker.unlock(); // don't leave the mutex locked!
        _endthreadex(0);
    }
}

void QThread::setPriority(Priority priority)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);
    if (!d->running) {
        qWarning("QThread::setPriority(): cannot set priority, thread is not running");
        return;
    }

    // copied from start() with a few modifications:

    // Since Win 9x will have problems if the priority is idle or time critical
    // we have to use the closest one instead
    int prio;
    d->priority = priority;
    switch (d->priority) {
    case IdlePriority:
	if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
	    prio = THREAD_PRIORITY_LOWEST;
	} else {
	    prio = THREAD_PRIORITY_IDLE;
	}
        break;

    case LowestPriority:
        prio = THREAD_PRIORITY_LOWEST;
        break;

    case LowPriority:
        prio = THREAD_PRIORITY_BELOW_NORMAL;
        break;

    case NormalPriority:
        prio = THREAD_PRIORITY_NORMAL;
        break;

    case HighPriority:
        prio = THREAD_PRIORITY_ABOVE_NORMAL;
        break;

    case HighestPriority:
        prio = THREAD_PRIORITY_HIGHEST;
        break;

    case TimeCriticalPriority:
	if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
	    prio = THREAD_PRIORITY_HIGHEST;
	} else {
	    prio = THREAD_PRIORITY_TIME_CRITICAL;
	}
        break;

    case InheritPriority:
    default:
        qWarning("QThread::setPriority(): argument cannot be InheritPriority");
        return;
    }

    if (!SetThreadPriority(d->handle, prio)) {
        qErrnoWarning("QThread::start: Failed to set thread priority");
    }
}
