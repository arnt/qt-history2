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
#include "qmutexpool_p.h"
#include "qthreadstorage.h"

#include <qcoreapplication.h>
#include <qpointer.h>

#include <windows.h>

#ifndef Q_OS_TEMP
#ifndef _MT
#define _MT
#endif
#include <process.h>
#endif

#define d d_func()

static DWORD qt_tls_index = TLS_OUT_OF_INDEXES;
void qt_create_tls()
{
    if (qt_tls_index != TLS_OUT_OF_INDEXES) return;

    static QMutex mutex;
    mutex.lock();
    qt_tls_index = TlsAlloc();
    mutex.unlock();
}


/**************************************************************************
 ** QThreadPrivate
 *************************************************************************/

// QThreadPrivate *QThreadPrivate::current()
// {
//     QThreadPrivate *ret = 0;
//     if (qt_tls_index != TLS_OUT_OF_INDEXES)
//         ret = (QThreadPrivate *)TlsGetValue(qt_tls_index);
//     if (!ret)
//         return &main_instance;
//     return ret;
// }

unsigned int __stdcall QThreadPrivate::start(void *arg)
{
    TlsSetValue(qt_tls_index, arg);

    QThread *thr = reinterpret_cast<QThread *>(arg);
    emit thr->started();
    thr->run();
    finish(arg);
    return 0;
}

void QThreadPrivate::finish(void *arg, bool lockAnyway)
{
    QThread *thr = reinterpret_cast<QThread *>(arg);

    if (lockAnyway)
        thr->d->mutex()->lock();
    thr->d->running = false;
    thr->d->finished = true;
    if (thr->d->terminated)
        emit thr->terminated();
    thr->d->terminated = false;
    emit thr->finished();

    QThreadStorageData::finish(thr->d->tls);
    thr->d->tls = 0;

    if (!thr->d->waiters) {
        CloseHandle(thr->d->handle);
        thr->d->handle = 0;
    }

    thr->d->id = 0;

    if (lockAnyway)
        thr->d->mutex()->unlock();
}


/**************************************************************************
 ** QThread
 *************************************************************************/

Qt::HANDLE QThread::currentThread()
{
    return (Qt::HANDLE)GetCurrentThreadId();
}

QThread *QThread::currentQThread()
{
    return reinterpret_cast<QThread*>(TlsGetValue(qt_tls_index));
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
    QMutexLocker locker(d->mutex());

    if (d->running && !d->finished) {
        qWarning("Thread is already running");
        wait();
    }

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
        qSystemWarning("Failed to create thread");
        d->running = false;
        d->finished = true;
        return;
    }

    int prio;
    switch (priority) {
    case IdlePriority:
        prio = THREAD_PRIORITY_IDLE;
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
        prio = THREAD_PRIORITY_TIME_CRITICAL;
        break;

    case InheritPriority:
    default:
        prio = GetThreadPriority(GetCurrentThread());
        break;
    }

    if (! SetThreadPriority(d->handle, prio)) {
        qSystemWarning("Failed to set thread priority");
    }

    if (ResumeThread(d->handle) == 0xffffffff) {
        qSystemWarning("Failed to resume newly created thread");
    }
}


void QThread::terminate()
{
    QMutexLocker locker(d->mutex());
    if (!d->running)
        return;
    TerminateThread(d->handle, 0);
    d->terminated = true;
    QThreadPrivate::finish(this, false);
}

bool QThread::wait(unsigned long time)
{
    QMutexLocker locker(d->mutex());

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
    case WAIT_ABANDONED:
    case WAIT_FAILED:
        qSystemWarning("Thread wait failure");
    case WAIT_TIMEOUT:
    default:
        break;
    }

    locker.mutex()->lock();
    --d->waiters;

    if (d->finished && !d->waiters) {
        CloseHandle(d->handle);
        d->handle = 0;
    }

    return ret;
}

void QThread::exit()
{
    QThread *thread = QThread::currentQThread();

    if (!thread) {
        qWarning("QThread::exit() called without a QThread instance.");
        _endthreadex(0);
        return;
    }

    QThreadPrivate::finish(thread);
    _endthreadex(0);
}
