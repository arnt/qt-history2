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
#include "qthread_p.h"
#include "qthreadstorage.h"
#include "qmutex.h"

#include <qcoreapplication.h>
#include <qpointer.h>

#include <private/qcoreapplication_p.h>
#include <private/qeventdispatcher_win_p.h>

#include <windows.h>
#include <assert.h>

#ifndef Q_OS_TEMP
#ifndef _MT
#define _MT
#endif
#include <process.h>
#endif

void qt_watch_adopted_thread(const HANDLE adoptedThreadHandle, QThread *qthread);
void qt_adopted_thread_watcher_function(void *);

static DWORD qt_current_thread_data_tls_index = TLS_OUT_OF_INDEXES;
void qt_create_tls()
{
    if (qt_current_thread_data_tls_index != TLS_OUT_OF_INDEXES)
        return;
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    qt_current_thread_data_tls_index = TlsAlloc();
}


/*
    QThreadData
*/
QThreadData *QThreadData::current()
{
    qt_create_tls();
    QThreadData *threadData = reinterpret_cast<QThreadData *>(TlsGetValue(qt_current_thread_data_tls_index));
    if (!threadData) {
        threadData = new QThreadData;
        TlsSetValue(qt_current_thread_data_tls_index, threadData);
        threadData->thread = new QAdoptedThread(threadData);
        threadData->deref();
        const bool isMainThread = q_atomic_test_and_set_ptr(&QCoreApplicationPrivate::theMainThread, 0, threadData->thread);
        if (!isMainThread) {
            HANDLE realHandle;
            DuplicateHandle(GetCurrentProcess(), 
                    GetCurrentThread(), 
                    GetCurrentProcess(),
                    &realHandle, 
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS);
            qt_watch_adopted_thread(realHandle, threadData->thread);
        }    
    }
    return threadData;
}

void QAdoptedThread::init()
{
    d_func()->handle = GetCurrentThread();
    d_func()->id = GetCurrentThreadId();
}

static QVector<HANDLE> qt_adopted_thread_handles;
static QVector<QThread *> qt_adopted_qthreads;
static QMutex qt_adopted_thread_watcher_mutex;
static HANDLE qt_adopted_thread_watcher_handle = 0;
static HANDLE qt_adopted_thread_wakeup = 0;

/*! \internal
    Adds an adopted thread to the list of threads that Qt watches to make sure 
    the thread data is properly cleaned up. This function starts the watcher 
    thread if neccesary.
*/
void qt_watch_adopted_thread(const HANDLE adoptedThreadHandle, QThread *qthread)
{
    QMutexLocker lock(&qt_adopted_thread_watcher_mutex);
    qt_adopted_thread_handles.append(adoptedThreadHandle);
    qt_adopted_qthreads.append(qthread);
    
    // Start watcher thread if it is not already running.
    if (qt_adopted_thread_watcher_handle == 0) {
        if (qt_adopted_thread_wakeup == 0) {
            qt_adopted_thread_wakeup = QT_WA_INLINE(CreateEventW(0, false, false, 0),
                                                    CreateEventA(0, false, false, 0));
            qt_adopted_thread_handles.prepend(qt_adopted_thread_wakeup);
        }
        
        qt_adopted_thread_watcher_handle =
            (HANDLE)_beginthread(qt_adopted_thread_watcher_function, 0, NULL);
    } else {
        SetEvent(qt_adopted_thread_wakeup);
    }
}

/*! \internal
    This function loops and waits for native adopted threads to finish.
    When this happens it derefs the QThreadData for the adopted thread
    to make sure it gets cleaned up properly.
*/
void qt_adopted_thread_watcher_function(void *)
{
    forever { 
        qt_adopted_thread_watcher_mutex.lock();
        
        const uint handleCount = qt_adopted_thread_handles.count();
        
        if (handleCount == 1) {
            CloseHandle(qt_adopted_thread_watcher_handle);
            qt_adopted_thread_watcher_handle = 0;
            qt_adopted_thread_watcher_mutex.unlock();        
            break;
        }
        
        QVector<HANDLE> handlesCopy = qt_adopted_thread_handles;
        qt_adopted_thread_watcher_mutex.unlock();

        DWORD ret = WaitForMultipleObjects(handlesCopy.count(), handlesCopy.constData(), false, INFINITE);

        if (ret == WAIT_FAILED || !(ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + handleCount)) {
            qWarning("QThread internal error while waiting for adopted threads: %d", int(GetLastError()));
            continue;
        }
        const int handleIndex = ret - WAIT_OBJECT_0;   

        if (handleIndex == 0){
            // New handle to watch was added.
            continue;
        } else {
            QThreadData::get2(qt_adopted_qthreads.at(handleIndex -1))->deref();
            CloseHandle(qt_adopted_thread_handles.at(handleIndex));
            QMutexLocker lock(&qt_adopted_thread_watcher_mutex);
            qt_adopted_thread_handles.remove(handleIndex);
        }
    }
}

/**************************************************************************
 ** QThreadPrivate
 *************************************************************************/

void QThreadPrivate::createEventDispatcher(QThreadData *data)
{
    data->eventDispatcher = new QEventDispatcherWin32;
    data->eventDispatcher->startingUp();
}

unsigned int __stdcall QThreadPrivate::start(void *arg)
{
    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadData *data = QThreadData::get2(thr);

    qt_create_tls();
    TlsSetValue(qt_current_thread_data_tls_index, data);

    QThread::setTerminationEnabled(false);    

    data->quitNow = false;
    // ### TODO: allow the user to create a custom event dispatcher
    createEventDispatcher(data);

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

    if (lockAnyway)
        d->mutex.lock();
    d->priority = QThread::InheritPriority;
    d->running = false;
    d->finished = true;
    if (d->terminated)
        emit thr->terminated();
    d->terminated = false;
    emit thr->finished();

    if (d->data->eventDispatcher) {
        d->data->eventDispatcher->closingDown();
        QAbstractEventDispatcher *eventDispatcher = d->data->eventDispatcher;
        d->data->eventDispatcher = 0;
        delete eventDispatcher;
    }

    QThreadStorageData::finish(d->data->tls);
    d->data->tls = 0;

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
        qWarning("QThread::wait: Thread tried to wait on itself");
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
        qWarning("QThread::setPriority: Cannot set priority, thread is not running");
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
        qWarning("QThread::setPriority: Argument cannot be InheritPriority");
        return;
    }

    if (!SetThreadPriority(d->handle, prio)) {
        qErrnoWarning("QThread::setPriority: Failed to set thread priority");
    }
}
