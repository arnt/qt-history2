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
#include "qmutex.h"

#ifndef QT_NO_THREAD
#include "qatomic.h"
#include "qmutex_p.h"
#include <errno.h>
#include <string.h>


static void report_error(int code, const char *where, const char *what)
{
    if (code != 0)
        qWarning("%s: %s failure: %s", where, what, strerror(code));
}


/*!
    \class QMutex
    \brief The QMutex class provides access serialization between threads.

    \threadsafe

    \ingroup thread
    \ingroup environment
    \mainclass

    The purpose of a QMutex is to protect an object, data structure or
    section of code so that only one thread can access it at a time
    (this is similar to the Java \c synchronized keyword). It is
    usually best to use a mutex with a QMutexLocker since this makes
    it easy to ensure that locking and unlocking are performed
    consistently.

    For example, say there is a method that prints a message to the
    user on two lines:

    \code
        int number = 6;

        void method1()
        {
            number *= 5;
            number /= 4;
        }

        void method2()
        {
            number *= 3;
            number /= 2;
        }
    \endcode

    If these two methods are called in succession, the following happens:

    \code
        // method1()
        number *= 5;        // number is now 30
        number /= 4;        // number is now 7

        // method2()
        number *= 3;        // nubmer is now 21
        number /= 2;        // number is now 10
    \endcode

    If these two methods are called simultaneously from two threads then the
    following sequence could result:

    \code
        // Thread 1 calls method1()
        number *= 5;        // number is now 30

        // Thread 2 calls method2().
        //
        // Most likely Thread 1 has been put to sleep by the operating
        // system to allow Thread 2 to run.
        number *= 3;        // number is now 90
        number /= 2;        // number is now 45

        // Thread 1 finishes executing.
        number /= 4;        // number is now 11, instead of 10
    \endcode

    If we add a mutex, we should get the result we want:

    \code
        QMutex mutex;
        int number = 6;

        void method1()
        {
            mutex.lock();
            number *= 5;
            number /= 4;
            mutex.unlock();
        }

        void method2()
        {
            mutex.lock();
            number *= 3;
            number /= 2;
            mutex.unlock();
        }
    \endcode

    Then only one thread can modify \c number at any given time and
    the result is correct. This is a trivial example, of course, but
    applies to any other case where things need to happen in a
    particular sequence.

    When you call lock() in a thread, other threads that try to call
    lock() in the same place will block until the thread that got the
    lock calls unlock(). A non-blocking alternative to lock() is
    tryLock().

    \sa QMutexLocker, QReadWriteLock, QSemaphore, QWaitCondition
*/

/*!
    \enum QMutex::RecursionMode

    \value Recursive  In this mode, a thread can lock the same mutex
                      multiple times and the mutex won't be unlocked
                      until a corresponding number of unlock() calls
                      have been made.

    \value NonRecursive  In this mode, a thread may only lock a mutex
                         once.

    \sa QMutex()
*/

/*!
    Constructs a new mutex. The mutex is created in an unlocked state.

    If \a mode is QMutex::Recursive, a thread can lock the same mutex
    multiple times and the mutex won't be unlocked until a
    corresponding number of unlock() calls have been made. The
    default is QMutex::NonRecursive.

    \sa lock(), unlock()
*/
QMutex::QMutex(RecursionMode mode)
    : d(new QMutexPrivate)
{
    d->recursive = (mode == Recursive);
    d->count = 0;
    report_error(pthread_mutex_init(&d->mutex, NULL), "QMutex", "mutex init");
    report_error(pthread_cond_init(&d->cond, NULL), "QMutex", "cv init");
}

/*!
    Destroys the mutex.

    \warning Destroying a locked mutex may result in undefined behavior.
*/
QMutex::~QMutex()
{
    report_error(pthread_cond_destroy(&d->cond), "QMutex", "cv destroy");
    report_error(pthread_mutex_destroy(&d->mutex), "QMutex", "mutex destroy");
    delete d;
}

/*!
    Locks the mutex. If another thread has locked the mutex then this
    call will block until that thread has unlocked it.

    \sa unlock()
*/
void QMutex::lock()
{
    void *self = (void *) pthread_self();
    void *none = 0;

    ++d->waiters;
    if (!d->owner.testAndSet(none, self)) {
        if (!d->recursive || d->owner != self) {
            if (d->owner == self) {
                qWarning("QMutex::lock(): Deadlock detected in thread %p",
                         static_cast<void *>(d->owner));
            }

            report_error(pthread_mutex_lock(&d->mutex), "QMutex::lock()", "mutex lock");
            while (!d->owner.testAndSet(none, self))
                report_error(pthread_cond_wait(&d->cond, &d->mutex), "QMutex::lock()", "cv wait");
            report_error(pthread_mutex_unlock(&d->mutex), "QMutex::lock()", "mutex unlock");
        }
    }
    --d->waiters;
    ++d->count;
}

/*!
    Attempts to lock the mutex. If the lock was obtained, this function
    returns true. If another thread has locked the mutex, this
    function returns false immediately.

    If the lock was obtained, the mutex must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa lock(), unlock()
*/
bool QMutex::tryLock()
{
    void *self = (void *) pthread_self();
    void *none = 0;

    if (!d->owner.testAndSet(none, self)) {
        if (!d->recursive || d->owner != self)
            return false;
    }
    ++d->count;
    return true;
}

/*!
    Unlocks the mutex. Attempting to unlock a mutex in a different
    thread to the one that locked it results in an error. Unlocking a
    mutex that is not locked results in undefined behavior.

    \sa lock()
*/
void QMutex::unlock()
{
    void *none = 0;

    Q_ASSERT_X(d->owner == (void *) pthread_self(), "QMutex::unlock()",
               "A mutex must be unlocked in the same thread that locked it.");

    if (!--d->count) {
        (void) d->owner.exchange(none);
        if (d->waiters != 0) {
            pthread_mutex_lock(&d->mutex);
            pthread_cond_signal(&d->cond);
            pthread_mutex_unlock(&d->mutex);
        }
    }
}

/*!
    \fn bool QMutex::locked()

    Returns true if the mutex is locked by another thread; otherwise
    returns false.

    It is generally a bad idea to use this function, because code
    that uses it generally has a race condition. Use tryLock() and
    unlock() instead.

    \oldcode
    \newcode
    \endcode

    \warning Due to differing implementations of recursive mutexes on
    various platforms, calling this function from the same thread that
    previously locked the mutex will return undefined results.
*/

/*!
    \class QMutexLocker
    \brief The QMutexLocker class is a convenience class that simplifies
    locking and unlocking mutexes.

    \threadsafe

    \ingroup thread
    \ingroup environment

    The purpose of QMutexLocker is to simplify QMutex locking and
    unlocking. Locking and unlocking a QMutex in complex functions and
    statements or in exception handling code is error-prone and
    difficult to debug. QMutexLocker can be used in such situations
    to ensure that the state of the mutex is always well-defined.

    QMutexLocker should be created within a function where a QMutex
    needs to be locked. The mutex is locked when QMutexLocker is
    created, and unlocked when QMutexLocker is destroyed.

    For example, this complex function locks a QMutex upon entering
    the function and unlocks the mutex at all the exit points:

    \code
        int complexFunction(int flag)
        {
            mutex.lock();

            int retVal = 0;

            switch (flag) {
            case 0:
            case 1:
                mutex.unlock();
                return moreComplexFunction(flag);
            case 2:
                {
                    int status = anotherFunction();
                    if (status < 0) {
                        mutex.unlock();
                        return -2;
                    }
                    retVal = status + flag;
                }
                break;
            default:
                if (flag > 10) {
                    mutex.unlock();
                    return -1;
                }
                break;
            }

            mutex.unlock();
            return retVal;
        }
    \endcode

    This example function will get more complicated as it is
    developed, which increases the likelihood that errors will occur.

    Using QMutexLocker greatly simplifies the code, and makes it more
    readable:

    \code
        int complexFunction(int flag)
        {
            QMutexLocker locker(&mutex);

            int retVal = 0;

            switch (flag) {
            case 0:
            case 1:
                return moreComplexFunction(flag);
            case 2:
                {
                    int status = anotherFunction();
                    if (status < 0)
                        return -2;
                    retVal = status + flag;
                }
                break;
            default:
                if (flag > 10)
                    return -1;
                break;
            }

            return retVal;
        }
    \endcode

    Now, the mutex will always be unlocked when the QMutexLocker
    object is destroyed (when the function returns since \c locker is
    an auto variable).

    The same principle applies to code that throws and catches
    exceptions. An exception that is not caught in the function that
    has locked the mutex has no way of unlocking the mutex before the
    exception is passed up the stack to the calling function.

    QMutexLocker also provides a mutex() member function that returns
    the mutex on which the QMutexLocker is operating. This is useful
    for code that needs access to the mutex, such as
    QWaitCondition::wait(). For example:

    \code
        class SignalWaiter
        {
        private:
            QMutexLocker locker;

        public:
            SignalWaiter(QMutex *mutex)
                : locker(mutex)
            {
            }

            void waitForSignal()
            {
                ...
                while (!signalled)
                    waitCondition.wait(locker.mutex());
                ...
            }
        };
    \endcode

    \sa QReadLocker, QWriteLocker, QMutex
*/

/*!
    \fn QMutexLocker::QMutexLocker(QMutex *mutex)

    Constructs a QMutexLocker and locks \a mutex. The mutex will be
    unlocked when the QMutexLocker is destroyed. If \a mutex is zero,
    QMutexLocker does nothing.

    \sa QMutex::lock()
*/

/*!
    \fn QMutexLocker::~QMutexLocker()

    Destroys the QMutexLocker and unlocks the mutex that was locked
    in the constructor.

    \sa QMutex::unlock()
*/

/*!
    \fn QMutex *QMutexLocker::mutex() const

    Returns a pointer to the mutex that was locked in the
    constructor.
*/

/*!
    \fn void QMutexLocker::unlock()

    Unlocks this mutex locker.

    \sa relock()
*/

/*!
    \fn void QMutexLocker::relock()

    Relocks an unlocked mutex locker.

    \sa unlock()
*/

#endif // QT_NO_THREAD
