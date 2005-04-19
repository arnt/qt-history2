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

    \threadsafe

    \ingroup thread
    \ingroup environment

    A read-write lock is a synchronization tool for protecting
    resources that can be accessed for reading and writing. This type
    of lock is useful if you want to allow multiple threads to have
    simultaneous read-only access, but as soon as one thread wants to
    write to the resource, all other threads must be blocked until
    the writing is complete.

    In many cases, QReadWriteLock is a direct competitor to QMutex.
    QReadWriteLock is a good choice if there are many concurrent
    reads and writing occurs infrequently.

    Example:

    \code
        QReadWriteLock lock;

        void ReaderThread::run()
        {
            ...
            lock.lockForRead();
            read_file();
            lock.unlock();
            ...
        }

        void WriterThread::run()
        {
            ...
            lock.lockForWrite();
            write_file();
            lock.unlock();
            ...
        }
    \endcode

    To ensure that writers aren't blocked forever by readers, readers
    attempting to obtain a lock will not succeed if there is a blocked
    writer waiting for access, even if the lock is currently only
    accessed by other readers. Also, if the lock is accessed by a
    writer and another writer comes in, that writer will have
    priority over any readers that might also be waiting.

    \sa QReadLocker, QWriteLocker, QMutex, QSemaphore
*/

/*!
    Constructs a QReadWriteLock object.

    \sa lockForRead(), lockForWrite()
*/
QReadWriteLock::QReadWriteLock()
:d(new QReadWriteLockPrivate())
{
    d->waitingReaders=0;
    report_error(pthread_mutex_init(&d->mutex, NULL), "QReadWriteLock", "mutex init");
    report_error(pthread_cond_init(&d->readerWait, NULL), "QReadWriteLock", "cv init");
    report_error(pthread_cond_init(&d->writerWait, NULL), "QReadWriteLock", "cv init");
}

/*!
    Destroys the QReadWriteLock object.

    \warning Destroying a read-write lock that is in use may result
    in undefined behavior.
*/
QReadWriteLock::~QReadWriteLock()
{
    report_error(pthread_cond_destroy(&d->writerWait), "QReadWriteLock", "cv destroy");
    report_error(pthread_cond_destroy(&d->readerWait), "QReadWriteLock", "cv destroy");
    report_error(pthread_mutex_destroy(&d->mutex), "QReadWriteLock", "mutex destroy");
    delete d;
}

/*!
    Locks the lock for reading. This function will block the current
    thread if another thread has locked for writing.

    \sa unlock() lockForWrite() tryLockForRead()
*/
void QReadWriteLock::lockForRead()
{
    d->waitingReaders.ref();
    for (;;) {
        int localAccessCount(d->accessCount);
        if(d->waitingWriters == 0 && localAccessCount != -1 && localAccessCount < INT_MAX) {
            if (d->accessCount.testAndSet(localAccessCount, localAccessCount + 1))
                break;
        } else {
            report_error(pthread_mutex_lock(&d->mutex), "QReadWriteLock::lock()", "mutex lock");
            if (d->waitingWriters == 0 && d->accessCount != -1 && d->accessCount < INT_MAX) {
                report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::lock()", "mutex unlock");
                continue;
            }
            report_error(pthread_cond_wait(&d->readerWait, &d->mutex), "QReadWriteLock::lock()", "cv wait");
            report_error(pthread_mutex_unlock(&d->mutex), "QReadWriteLock::lock()", "mutex unlock");
            continue;
        }
    }
    d->waitingReaders.deref();
}

/*!
    Attempts to lock for reading. If the lock was obtained, this
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
        if(d->waitingWriters == 0 && localAccessCount != -1 && localAccessCount < INT_MAX) {
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
    Locks the lock for writing. This function will block the current
    thread if any thread has locked for reading or writing.

    \sa unlock() lockForRead() tryLockForWrite()
 */
void QReadWriteLock::lockForWrite()
{
    d->waitingWriters.ref();
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
    d->waitingWriters.deref();
}

/*!
    Attempts to lock for writing. If the lock was obtained, this
    function returns true; otherwise, it returns false immediately.

    The lock attempt will fail if any thread has locked for reading or
    writing.

    If the lock was obtained, the lock must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa unlock() lockForWrite()
*/
bool QReadWriteLock::tryLockForWrite()
{
    bool result;
    d->waitingWriters.ref();
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
    d->waitingWriters.deref();
    return result;
}

/*!
    Unlocks the lock.

    Attempting to unlock a lock that is not locked is an error, and will result
    in program termination.

    \sa lockForRead() lockForWrite() tryLockForRead() tryLockForWrite()
*/
void QReadWriteLock::unlock()
{
    Q_ASSERT_X(d->accessCount != 0, "QReadWriteLock::unlock()", "Cannot unlock an unlocked lock");

    bool unlocked = d->accessCount.testAndSet(-1, 0);
    if (!unlocked) {
        unlocked = !d->accessCount.deref();
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

/*!
    \class QReadLocker
    \brief The QReadLocker class is a convenience class that
    simplifies locking and unlocking read-write locks for read access.

    \threadsafe

    \ingroup thread
    \ingroup environment

    The purpose of QReadLock (and QWriteLock) is to simplify
    QReadWriteLock locking and unlocking. Locking and unlocking
    statements or in exception handling code is error-prone and
    difficult to debug. QReadLocker can be used in such situations
    to ensure that the state of the lock is always well-defined.

    Here's an example that uses QReadLocker to lock and unlock a
    read-write lock for reading:

    \code
        QReadWriteLock lock;

        QByteArray readData()
        {
            QReadLocker locker(&lock);
            ...
            return data;
        }
    \endcode

    It is equivalent to the following code:

    \code
        QReadWriteLock lock;

        QByteArray readData()
        {
            locker.lockForRead();
            ...
            locker.unlock();
            return data;
        }
    \endcode

    The QMutexLocker documentation shows examples where the use of a
    locker object greatly simplifies programming.

    \sa QWriteLocker, QReadWriteLock
*/

/*!
    \fn QReadLocker::QReadLocker(QReadWriteLock *lock)

    Constructs a QReadLocker and locks \a lock for reading. The lock
    will be unlocked when the QReadLocker is destroyed. If \c lock is
    zero, QReadLocker does nothing.

    \sa QReadWriteLock::lockForRead()
*/

/*!
    \fn QReadLocker::~QReadLocker()

    Destroys the QReadLocker and unlocks the lock that was passed to
    the constructor.

    \sa QReadWriteLock::unlock()
*/

/*!
    \fn void QReadLocker::unlock()

    Unlocks the lock associated with this locker.

    \sa QReadWriteLock::unlock()
*/

/*!
    \fn void QReadLocker::relock()

    Relocks an unlocked lock.

    \sa unlock()
*/

/*!
    \fn QReadWriteLock *QReadLocker::readWriteLock() const

    Returns a pointer to the read-write lock that was passed
    to the constructor.
*/

/*!
    \class QWriteLocker
    \brief The QWriteLocker class is a convenience class that
    simplifies locking and unlocking read-write locks for write access.

    \threadsafe

    \ingroup thread
    \ingroup environment

    The purpose of QWriteLock (and QReadLock) is to simplify
    QReadWriteLock locking and unlocking. Locking and unlocking
    statements or in exception handling code is error-prone and
    difficult to debug. QWriteLocker can be used in such situations
    to ensure that the state of the lock is always well-defined.

    Here's an example that uses QWriteLocker to lock and unlock a
    read-write lock for writing:

    \code
        QReadWriteLock lock;

        void writeData(const QByteArray &data)
        {
            QWriteLocker locker(&lock);
            ...
        }
    \endcode

    It is equivalent to the following code:

    \code
        QReadWriteLock lock;

        void writeData(const QByteArray &data)
        {
            locker.lockForWrite();
            ...
            locker.unlock();
        }
    \endcode

    The QMutexLocker documentation shows examples where the use of a
    locker object greatly simplifies programming.

    \sa QReadLocker, QReadWriteLock
*/

/*!
    \fn QWriteLocker::QWriteLocker(QReadWriteLock *lock)

    Constructs a QWriteLocker and locks \a lock for writing. The lock
    will be unlocked when the QWriteLocker is destroyed. If \c lock is
    zero, QWriteLocker does nothing.

    \sa QReadWriteLock::lockForWrite()
*/

/*!
    \fn QWriteLocker::~QWriteLocker()

    Destroys the QWriteLocker and unlocks the lock that was passed to
    the constructor.

    \sa QReadWriteLock::unlock()
*/

/*!
    \fn void QWriteLocker::unlock()

    Unlocks the lock associated with this locker.

    \sa QReadWriteLock::unlock()
*/

/*!
    \fn void QWriteLocker::relock()

    Relocks an unlocked lock.

    \sa unlock()
*/

/*!
    \fn QReadWriteLock *QWriteLocker::readWriteLock() const

    Returns a pointer to the read-write lock that was passed
    to the constructor.
*/
