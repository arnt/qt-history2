/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <unistd.h>

#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "qmutex.h"

#ifndef QT_NO_THREAD
#include "qmutex_p.h"




/*!
    \class QMutex qmutex.h
    \threadsafe
    \brief The QMutex class provides access serialization between threads.

    \ingroup thread
    \ingroup environment

    The purpose of a QMutex is to protect an object, data structure or
    section of code so that only one thread can access it at a time
    (This is similar to the Java \c synchronized keyword). For
    example, say there is a method which prints a message to the user
    on two lines:

    \code
    int number = 6;

    void method1()
    {
        number *= 5;
	number /= 4;
    }

    void method1()
    {
        number *= 3;
	number /= 2;
    }
    \endcode

    If these two methods are called in succession, the following happens:

    \code
    // method1()
    number *= 5;	// number is now 30
    number /= 4;	// number is now 7

    // method2()
    number *= 3;	// nubmer is now 21
    number /= 2;	// number is now 10
    \endcode

    If these two methods are called simultaneously from two threads then the
    following sequence could result:

    \code
    // Thread 1 calls method1()
    number *= 5;	// number is now 30

    // Thread 2 calls method2().
    //
    // Most likely Thread 1 has been put to sleep by the operating
    // system to allow Thread 2 to run.
    number *= 3;	// number is now 90
    number /= 2;	// number is now 45

    // Thread 1 finishes executing.
    number /= 4;	// number is now 11, instead of 10
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
*/

/*!
    Constructs a new mutex. The mutex is created in an unlocked state.
    A recursive mutex is created if \a recursive is true; a normal
    mutex is created if \a recursive is false (the default). With a
    recursive mutex, a thread can lock the same mutex multiple times
    and it will not be unlocked until a corresponding number of
    unlock() calls have been made.
*/
QMutex::QMutex(bool recursive)
    : d(new QMutexPrivate)
{
    d->recursive = recursive;
    d->owner = 0;
    d->count = 0;

    int code;
    code = pthread_mutex_init(&d->mutex, NULL);
    if (code != 0)
	qWarning("QMutex: cannot create mutex: %s", strerror(code));
    if (d->recursive) {
        code = pthread_mutex_init(&d->mutex2, NULL);
	if (code != 0)
	    qWarning("QMutex: cannot create support mutex: %s", strerror(code));
    }
}

/*!
    Destroys the mutex.

    \warning If you destroy a mutex that still holds a lock the
    resultant behavior is undefined.
*/
QMutex::~QMutex()
{
    int code;
    if (d->recursive) {
        code = pthread_mutex_destroy(&d->mutex2);
	if (code != 0)
	    qWarning("QMutex: cannot destroy support mutex: %s", strerror(code));
    }
    code = pthread_mutex_destroy(&d->mutex);
    if (code != 0)
        qWarning("QMutex: cannot destroy mutex: %s", strerror(code));

    delete d;
}

/*!
    Attempt to lock the mutex. If another thread has locked the mutex
    then this call will \e block until that thread has unlocked it.

    \sa unlock()
*/
void QMutex::lock()
{
    int code;
    if (! d->recursive) {
        code = pthread_mutex_lock(&d->mutex);
	if (code != 0)
	    qWarning("QMutex::lock: %s", strerror(code));
        return;
    }

    pthread_mutex_lock(&d->mutex2);

    if (d->count > 0 && d->owner == pthread_self()) {
        d->count++;
    } else {
        pthread_mutex_unlock(&d->mutex2);

        code = pthread_mutex_lock(&d->mutex);
	if (code != 0)
	    qWarning("QMutex::lock: %s", strerror(code));

        pthread_mutex_lock(&d->mutex2);
        d->count = 1;
        d->owner = pthread_self();
    }

    pthread_mutex_unlock(&d->mutex2);
}

/*!
    Attempt to lock the mutex. If the lock was obtained, this function
    returns true. If another thread has locked the mutex, this
    function returns false, instead of waiting for the mutex to become
    available, i.e. it does not block.

    If the lock was obtained, the mutex must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa lock(), unlock()
*/
bool QMutex::tryLock()
{
    int code;
    if (! d->recursive) {
	code = pthread_mutex_trylock(&d->mutex);
        if (code != 0) {
            if (code != EBUSY)
                qWarning("QMutex:tryLock: %s", strerror(code));
	    return false;
	}
        return true;
    }

    bool ret = true;

    pthread_mutex_lock(&d->mutex2);

    if (d->count > 0 && d->owner == pthread_self()) {
        d->count++;
    } else {
	code = pthread_mutex_trylock(&d->mutex);

        if (code != 0) {
            if (code != EBUSY)
                qWarning("QMutex:tryLock: %s", strerror(code));
            ret = false;
        } else {
            d->count = 1;
            d->owner = pthread_self();
        }
    }

    pthread_mutex_unlock(&d->mutex2);

    return ret;
}

/*!
    Unlocks the mutex. Attempting to unlock a mutex in a different
    thread to the one that locked it results in an error. Unlocking a
    mutex that is not locked results in undefined behaviour (varies
    between different Operating Systems' thread implementations).

    \sa lock()
*/
void QMutex::unlock()
{
    int code;
    if (! d->recursive) {
	code = pthread_mutex_unlock(&d->mutex);
	if (code != 0)
	    qWarning("QMutex::unlock: %s", strerror(code));
        return;
    }

    pthread_mutex_lock(&d->mutex2);

    if (d->owner == pthread_self()) {
        // do nothing if the count is already 0... to reflect the behaviour described
        // in the docs
        if (d->count && (--d->count) < 1) {
            d->count = 0;
	    code = pthread_mutex_unlock(&d->mutex);
	    if (code != 0)
		qWarning("QMutex::unlock: %s", strerror(code));
        }
    } else {
        qWarning("QMutex::unlock: unlock from different thread than locker");
        qWarning("                was locked by %lu, unlock attempt from %lu",
                 (unsigned long) d->owner, (unsigned long) pthread_self());
    }

    pthread_mutex_unlock(&d->mutex2);
}

/*! \fn bool QMutex::isLocked()

    Returns true if the mutex is locked by another thread; otherwise
    returns false.

    \warning Due to differing implementations of recursive mutexes on
    various platforms, calling this function from the same thread that
    previously locked the mutex will return undefined results.

    \sa lock(), unlock()
*/

/*!
    \class QMutexLocker qmutex.h
    \brief The QMutexLocker class simplifies locking and unlocking QMutexes.

    \threadsafe

    \ingroup thread
    \ingroup environment

    The purpose of QMutexLocker is to simplify QMutex locking and
    unlocking. Locking and unlocking a QMutex in complex functions and
    statements or in exception handling code is error prone and
    difficult to debug. QMutexLocker should be used in such situations
    to ensure that the state of the mutex is well defined and always
    locked and unlocked properly.

    QMutexLocker should be created within a function where a QMutex
    needs to be locked. The mutex is locked when QMutexLocker is
    created, and unlocked when QMutexLocker is destroyed.

    For example, this complex function locks a QMutex upon entering
    the function and unlocks the mutex at all the exit points:

    \code
    int complexFunction(int flag)
    {
	mutex.lock();

	int return_value = 0;

	switch (flag) {
	case 0:
	case 1:
	    {
		mutex.unlock();
		return moreComplexFunction(flag);
	    }

	case 2:
	    {
		int status = anotherFunction();
		if (status < 0) {
		    mutex.unlock();
		    return -2;
		}
		return_value = status + flag;
		break;
	    }

	default:
	    {
		if (flag > 10) {
		    mutex.unlock();
		    return -1;
		}
		break;
	    }
	}

	mutex.unlock();
	return return_value;
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

	int return_value = 0;

	switch (flag) {
	case 0:
	case 1:
	    {
		return moreComplexFunction(flag);
	    }

	case 2:
	    {
		int status = anotherFunction();
		if (status < 0)
		    return -2;
		return_value = status + flag;
		break;
	    }

	default:
	    {
		if (flag > 10)
		    return -1;
		break;
	    }
	}

	return return_value;
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
	    ...
	    ...

	    while (! signalled)
		waitcondition.wait(locker.mutex());

	    ...
	    ...
	    ...
	}
    };
    \endcode

    \sa QMutex, QWaitCondition
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

    Destroys the QMutexLocker and unlocks the mutex which was locked
    in the constructor.

    \sa QMutexLocker::QMutexLocker(), QMutex::unlock()
*/

/*!
    \fn QMutex *QMutexLocker::mutex() const

    Returns a pointer to the mutex which was locked in the
    constructor.

    \sa QMutexLocker::QMutexLocker()
*/

/*!
    \fn void QMutexLocker::unlock()

    Unlocks this mutex locker.

    \sa relock()
*/

/*!
    \fn void QMutexLocker::relock()

    Relocks an  unlocked mutex locker.

    \sa unlock()
*/

#endif // QT_NO_THREAD
