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
#include "qreadwritelock.h"
#include "qatomic.h"
#include "qreadwritelock_p.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>
/*
    Duplicated code from qmutex_unix.cpp
*/
static void report_error(int code, const char *where, const char *what)
{
    if (code != 0)
        qWarning("%s: %s failure: %s", where, what, strerror(code));
}

/*! \class QReadWriteLock

    \brief The QReadWriteLock class provides read-write locking.

    \preliminary

    It is useful for synchronizing multithreaded access to resources
    that support multiple readers, but only one writer.

    The lock is optimized for situations where there are many concurrent
    reads, and writing occurs infrequently. Writers are prioritized in front
    of readers, in the sense that readers attemting to optain the lock will
    yield to writers attemting to optain the lock.
*/

/*!
    \enum QReadWriteLock::AccessMode

    This enum describes the different access types that a thread can specify
    when requesting the mutex.

    \value ReadAccess The thread wants to perform read-operations.

    \value WriteAccess The thread wants to perform write-operations.
*/

/*!
    Constructs a QReadWriteLock object allowing \a maxReaders concurrent
    readers. The default is INT_MAX.

    \sa lock()
*/
QReadWriteLock::QReadWriteLock(const int maxReaders)
:d(new QReadWriteLockPrivate())
{
    d->maxReaders=maxReaders;
    d->waitingReaders=0;
    report_error(pthread_mutex_init(&d->mutex, NULL), "QReadWriteLock", "mutex init");
    report_error(pthread_cond_init(&d->readerWait, NULL), "QReadWriteLock", "cv init");
    report_error(pthread_cond_init(&d->writerWait, NULL), "QReadWriteLock", "cv init");
}

/*!
    Destroys the QReadWriteMutex object.

    \warning If you destroy a read/write mutex that has accesses in use
    the resultant behavior is undefined.
*/
QReadWriteLock::~QReadWriteLock()
{
    report_error(pthread_cond_destroy(&d->writerWait), "QReadWriteLock", "cv destroy");
    report_error(pthread_cond_destroy(&d->readerWait), "QReadWriteLock", "cv destroy");
    report_error(pthread_mutex_destroy(&d->mutex), "QReadWriteLock", "mutex destroy");
    delete d;
}

/*!
    Attempts to lock for reading. This function will block the current
    thread if another thread has locked for writing.

    \sa unlock() lockForWrite() tryLockForRead()
*/
void QReadWriteLock::lockForRead()
{
    for (;;) {
        int localAccessCount(d->accessCount);
        if(d->waitingWriters == 0 && localAccessCount != -1 && localAccessCount <= d->maxReaders) {
            if (d->accessCount.testAndSet(localAccessCount, localAccessCount + 1))
                break;
        } else {
            report_error(pthread_mutex_lock(&d->mutex), "QReadWriteLock::lock()", "mutex lock");
            ++d->waitingReaders;
            if (d->waitingWriters == 0 && d->accessCount != -1 && d->accessCount <= d->maxReaders) {
                report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::lock()", "mutex unlock");
                continue;
            }
            report_error(pthread_cond_wait(&d->readerWait, &d->mutex), "QReadWriteLock::lock()", "cv wait");
            --d->waitingReaders;
            report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::lock()", "mutex unlock");
            continue;
        }
    }
}

/*!
    Attempt to lock for reading. If the lock was obtained, this
    function returns true, otherwise it returns false instead of
    waiting for the lock to become available, i.e. it does not block.

    The lock attempt will fail if another thread has locked for
    writing.

    If the lock was obtained, the lock must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa unlock() lockForRead()
*/
bool QReadWriteLock::tryLockForRead()
{
    bool result;
    for(;;){
        int localAccessCount(d->accessCount);
        if(d->waitingWriters == 0 && localAccessCount != -1 && localAccessCount <= d->maxReaders) {
            if (d->accessCount.testAndSet(localAccessCount, localAccessCount + 1)) {
                result=true;
                break;
            }
        } else {
            result=false;
            break;
        }
    }
    return result;
}

 /*!
    Attempts to lock for writing. This function will block the current
    thread if any thread has locked for reading or writing.

    \sa unlock() lockForRead() tryLockForWrite()
 */
void QReadWriteLock::lockForWrite()
{
    ++d->waitingWriters;
    for(;;) {
        int localAccessCount(d->accessCount);
        if(localAccessCount == 0){
            if (d->accessCount.testAndSet(0, -1))
                break;
        } else {
            report_error(pthread_mutex_lock(&d->mutex), "QReadWriteLock::lock()", "mutex lock");
            if (d->accessCount == 0) {
                report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::lock()", "mutex unlock");
                continue;
            }
            report_error(pthread_cond_wait(&d->writerWait, &d->mutex), "QReadWriteLock::lock()", "cv wait");
            report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::lock()", "mutex unlock");
            continue;
        }
    }
    --d->waitingWriters;
}

/*!
    Attempt to lock for write. If the lock was obtained, this function
    returns true, otherwise it returns false instead of waiting for
    the lock to become available, i.e. it does not block.

    The lock attempt will fail if any thread has locked for reading or
    writing.

    If the lock was obtained, the lock must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa unlock() lockForWrite()
*/
bool QReadWriteLock::tryLockForWrite()
{
    bool result;
    ++d->waitingWriters;
    for(;;){
        int localAccessCount(d->accessCount);
        if(localAccessCount == 0){
            if (d->accessCount.testAndSet(0, -1)) {
                result=true;
                break;
            }
        } else {
            result=false;
            break;
        }
    }
    --d->waitingWriters;
    return result;
}

/*!
    Unlocks the lock.

    Attempting to unlock a lock that i not locked is an error and will result
    in program termination.

    \sa lock() trylock();
*/
void QReadWriteLock::unlock()
{
    Q_ASSERT_X(d->accessCount != 0, "QReadWriteLock::unlock()", "Cannot unlock an unlocked lock");

    bool unlocked = d->accessCount.testAndSet(-1, 0);
    if (!unlocked) {
        unlocked = !--d->accessCount;
        if (!unlocked)
            return; // still locked, can't wake anyone up
    }

    if (d->waitingWriters != 0) {
        report_error(pthread_mutex_lock(&d->mutex), "QReadWriteLock::unlock()", "mutex lock");
        pthread_cond_signal(&d->writerWait);
        report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::unlock()", "mutex unlock");
    } else if (d->waitingReaders != 0) {
        report_error(pthread_mutex_lock(&d->mutex), "QReadWriteLock::unlock()", "mutex lock");
        pthread_cond_broadcast(&d->readerWait);
        report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::unlock()", "mutex unlock");
    }
}
