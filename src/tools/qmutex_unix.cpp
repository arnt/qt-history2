/****************************************************************************
** $Id: $
**
** QMutex class for Unix
**
** Created : 20010725
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#if defined(QT_THREAD_SUPPORT)

#include "qplatformdefs.h"

#ifdef    Q_OS_SOLARIS
// Solaris
typedef mutex_t Q_MUTEX_T;
typedef int     Q_MUTEXATTR_T;

// helpers
#define Q_THREAD_SELF()         thr_self()
#define Q_MUTEX_INIT(a, b)      mutex_init((a), NULL, NULL)
#define Q_MUTEX_LOCK(a)         mutex_lock((a))
#define Q_MUTEX_UNLOCK(a)       mutex_unlock((a))
#define Q_MUTEX_TRYLOCK(a)      mutex_trylock((a))
#define Q_MUTEX_DESTROY(a)      mutex_destroy((a))
#define Q_MUTEX_SET_TYPE(a, b)
#define Q_MUTEXATTR_INIT(a)
#define Q_MUTEXATTR_DESTROY(a)

// mutex types
#undef  Q_NORMAL_MUTEX_TYPE
#undef  Q_RECURSIVE_MUTEX_TYPE

#else // !Q_OS_SOLARIS
// Pthreads
typedef pthread_mutex_t     Q_MUTEX_T;
typedef pthread_mutexattr_t Q_MUTEXATTR_T;

// helpers
#define Q_THREAD_SELF()         pthread_self()
#define Q_MUTEX_INIT(a, b)      pthread_mutex_init((a), (b))
#define Q_MUTEX_LOCK(a)         pthread_mutex_lock((a))
#define Q_MUTEX_UNLOCK(a)       pthread_mutex_unlock((a))
#define Q_MUTEX_TRYLOCK(a)      pthread_mutex_trylock((a))
#define Q_MUTEX_DESTROY(a)      pthread_mutex_destroy((a))
#define Q_MUTEXATTR_INIT(a)     pthread_mutexattr_init((a))
#define Q_MUTEXATTR_DESTROY(a)  pthread_mutexattr_destroy((a))

// mutex types
#  if ((defined(PTHREAD_MUTEX_RECURSIVE) && defined(PTHREAD_MUTEX_DEFAULT)) || \
        defined(Q_OS_FREEBSD)) && !defined(Q_OS_UNIXWARE7)
    // POSIX 1003.1c-1995 - We love this OS
#    define Q_MUTEX_SET_TYPE(a, b) pthread_mutexattr_settype((a), (b))
#    if defined(QT_CHECK_RANGE)
#      define Q_NORMAL_MUTEX_TYPE PTHREAD_MUTEX_ERRORCHECK
#    else
#      define Q_NORMAL_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT
#    endif
#    define Q_RECURSIVE_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE
#  elif defined(MUTEX_NONRECURSIVE_NP)
// POSIX 1003.4a pthreads draft extensions
#    define Q_MUTEX_SET_TYPE(a, b) pthread_mutexattr_setkind_np((a), (b));
#    define Q_NORMAL_MUTEX_TYPE MUTEX_NONRECURSIVE_NP
#    define Q_RECURSIVE_MUTEX_TYPE MUTEX_RECURSIVE_NP
#  else
// Unknown mutex types - skip them
#    define Q_MUTEX_SET_TYPE(a, b)
#    undef  Q_NORMAL_MUTEX_TYPE
#    undef  Q_RECURSIVE_MUTEX_TYPE
#  endif
#endif // Q_OS_SOLARIS

#include "qmutex.h"
#include "qmutex_p.h"

#include <errno.h>
#include <string.h>


// Private class declarations

class QRealMutexPrivate : public QMutexPrivate {
public:
    QRealMutexPrivate(bool = FALSE);

    void lock();
    void unlock();
    bool locked();
    bool trylock();
    int type() const;

    bool recursive;
};

#ifndef    Q_RECURSIVE_MUTEX_TYPE
class QRecursiveMutexPrivate : public QMutexPrivate
{
public:
    QRecursiveMutexPrivate();
    ~QRecursiveMutexPrivate();

    void lock();
    void unlock();
    bool locked();
    bool trylock();
    int type() const;

    int count;
    unsigned long owner;
    Q_MUTEX_T handle2;
};
#endif // !Q_RECURSIVE_MUTEX_TYPE


// Private class implementation

// base destructor
QMutexPrivate::~QMutexPrivate()
{
    int ret = Q_MUTEX_DESTROY(&handle);

#ifdef QT_CHECK_RANGE
    if ( ret )
	qWarning( "Mutex destroy failure: %s", strerror( ret ) );
#endif
}

// real mutex class
QRealMutexPrivate::QRealMutexPrivate(bool recurs)
    : recursive(recurs)
{
    Q_MUTEXATTR_T attr;
    Q_MUTEXATTR_INIT(&attr);
    Q_MUTEX_SET_TYPE(&attr, recursive ? Q_RECURSIVE_MUTEX_TYPE : Q_NORMAL_MUTEX_TYPE);
    Q_UNUSED(recursive);
    int ret = Q_MUTEX_INIT(&handle, &attr);
    Q_MUTEXATTR_DESTROY(&attr);

#ifdef QT_CHECK_RANGE
    if( ret )
	qWarning( "Mutex init failure: %s", strerror( ret ) );
#endif // QT_CHECK_RANGE
}

void QRealMutexPrivate::lock()
{
    int ret = Q_MUTEX_LOCK(&handle);

#ifdef QT_CHECK_RANGE
    if (ret)
	qWarning("Mutex lock failure: %s", strerror(ret));
#endif
}

void QRealMutexPrivate::unlock()
{
    int ret = Q_MUTEX_UNLOCK(&handle);

#ifdef QT_CHECK_RANGE
    if (ret)
	qWarning("Mutex unlock failure: %s", strerror(ret));
#endif
}

bool QRealMutexPrivate::locked()
{
    int ret = Q_MUTEX_TRYLOCK(&handle);

    if (ret == EBUSY) {
	return TRUE;
    } else if (ret) {
#ifdef QT_CHECK_RANGE
	qWarning("Mutex locktest failure: %s", strerror(ret));
#endif
    } else
	Q_MUTEX_UNLOCK(&handle);

    return FALSE;
}

bool QRealMutexPrivate::trylock()
{
    int ret = Q_MUTEX_TRYLOCK(&handle);

    if (ret == EBUSY) {
	return FALSE;
    } else if (ret) {
#ifdef QT_CHECK_RANGE
	qWarning("Mutex trylock failure: %s", strerror(ret));
#endif
	return FALSE;
    }

    return TRUE;
}

int QRealMutexPrivate::type() const
{
    return recursive ? Q_MUTEX_RECURSIVE : Q_MUTEX_NORMAL;
}


#ifndef    Q_RECURSIVE_MUTEX_TYPE
QRecursiveMutexPrivate::QRecursiveMutexPrivate()
{
    Q_MUTEXATTR_T attr;
    Q_MUTEXATTR_INIT(&attr);
    Q_MUTEX_SET_TYPE(&attr, Q_NORMAL_MUTEX_TYPE);
    int ret = Q_MUTEX_INIT(&handle, &attr);
    Q_MUTEXATTR_DESTROY(&attr);

#  ifdef QT_CHECK_RANGE
    if (ret)
	qWarning( "Mutex init failure: %s", strerror(ret) );
#  endif

    Q_MUTEXATTR_INIT(&attr);
    ret = Q_MUTEX_INIT( &handle2, &attr );
    Q_MUTEXATTR_DESTROY(&attr);

#  ifdef QT_CHECK_RANGE
    if (ret)
	qWarning( "Mutex init failure: %s", strerror(ret) );
#  endif

    count = 0;
}

QRecursiveMutexPrivate::~QRecursiveMutexPrivate()
{
    int ret = Q_MUTEX_DESTROY(&handle2);

#  ifdef QT_CHECK_RANGE
    if (ret)
	qWarning( "Mutex destroy failure: %s", strerror(ret) );
#  endif
}

void QRecursiveMutexPrivate::lock()
{
    Q_MUTEX_LOCK(&handle2);

    if (count > 0 && owner == (unsigned long) Q_THREAD_SELF()) {
	count++;
    } else {
	Q_MUTEX_UNLOCK(&handle2);
	Q_MUTEX_LOCK(&handle);
	Q_MUTEX_LOCK(&handle2);
	count = 1;
	owner = (unsigned long) Q_THREAD_SELF();
    }

    Q_MUTEX_UNLOCK(&handle2);
}

void QRecursiveMutexPrivate::unlock()
{
    Q_MUTEX_LOCK(&handle2);

    if (owner == (unsigned long) Q_THREAD_SELF()) {
	// do nothing if the count is already 0... to reflect the behaviour described
	// in the docs
	if (count && (--count) < 1) {
	    count = 0;
	    Q_MUTEX_UNLOCK(&handle);
	}
    } else {
#ifdef QT_CHECK_RANGE
	qWarning("QMutex::unlock: unlock from different thread than locker");
	qWarning("                was locked by %d, unlock attempt from %d",
		 (int)owner, (int)Q_THREAD_SELF());
#endif
    }

    Q_MUTEX_UNLOCK(&handle2);
}

bool QRecursiveMutexPrivate::locked()
{
    Q_MUTEX_LOCK(&handle2);

    bool ret;
    int code = Q_MUTEX_TRYLOCK(&handle);

    if (code == EBUSY) {
	ret = TRUE;
    } else {
#ifdef QT_CHECK_RANGE
	if (code)
	    qWarning("Mutex trylock failure: %s", strerror(code));
#endif

	Q_MUTEX_UNLOCK(&handle);
	ret = FALSE;
    }

    Q_MUTEX_UNLOCK(&handle2);

    return ret;
}

bool QRecursiveMutexPrivate::trylock()
{
    Q_MUTEX_LOCK(&handle2);

    bool ret;
    int code = Q_MUTEX_TRYLOCK(&handle);

    if (code == EBUSY) {
	ret = FALSE;
    } else if (code) {
#ifdef QT_CHECK_RANGE
	qWarning("Mutex trylock failure: %s", strerror(code));
#endif
	ret = FALSE;
    } else {
	ret = TRUE;
	count++;
    }

    Q_MUTEX_UNLOCK(&handle2);

    return ret;
}

int QRecursiveMutexPrivate::type() const
{
    return Q_MUTEX_RECURSIVE;
}

#endif // !Q_RECURSIVE_MUTEX_TYPE


/*!
  \class QMutex qmutex.h
  \brief The QMutex class provides access serialization between threads.

  \ingroup thread
  \ingroup environment

  The purpose of a QMutex is to protect an object, data structure
  or section of code so that only one thread can access it at a time
  (In Java terms, this is similar to the synchronized keyword).
  For example, say there is a method which prints a message to the
  user on two lines:

  \code
  void someMethod()
  {
     qDebug("Hello");
     qDebug("World");
  }
  \endcode

  If this method is called simultaneously from two threads then
  the following sequence could result:

  \code
  Hello
  Hello
  World
  World
  \endcode

  If we add a mutex:

  \code
  QMutex mutex;

  void someMethod()
  {
     mutex.lock();
     qDebug("Hello");
     qDebug("World");
     mutex.unlock();
  }
  \endcode

  In Java terms this would be:

  \code
  void someMethod()
  {
     synchronized {
       qDebug("Hello");
       qDebug("World");
     }
  }
  \endcode

  Then only one thread can execute someMethod at a time and the order
  of messages is always correct. This is a trivial example, of course,
  but applies to any other case where things need to happen in a particular
  sequence.

    When you call lock() in a thread, other threads that try to call
    lock() in the same place will block until the thread that got the
    lock calls unlock(). A non-blocking alternative to lock() is
    tryLock().
*/

/*!
  Constructs a new mutex. The mutex is created in an unlocked state. A
  recursive mutex is created if \a recursive is TRUE; a normal mutex is
  created if \a recursive is FALSE (the default). With a recursive
  mutex, a thread can lock the same mutex multiple times and it will
  not be unlocked until a corresponding number of unlock() calls have
  been made.
*/
QMutex::QMutex(bool recursive)
{
#ifndef    Q_RECURSIVE_MUTEX_TYPE
    if ( recursive )
	d = new QRecursiveMutexPrivate();
    else
#endif // !Q_RECURSIVE_MUTEX_TYPE
	d = new QRealMutexPrivate(recursive);
}

/*!
  Destroys the mutex.
*/
QMutex::~QMutex()
{
    delete d;
}

/*!
  Attempt to lock the mutex. If another thread has locked the mutex
  then this call will \e block until that thread has unlocked it.

  \sa unlock(), locked()
*/
void QMutex::lock()
{
    d->lock();
}

/*!
  Unlocks the mutex. Attempting to unlock a mutex in a different thread
  to the one that locked it results in an error.  Unlocking a mutex that
  is not locked results in undefined behaviour (varies between
  different Operating Systems' thread implementations).

  \sa lock(), locked()
*/
void QMutex::unlock()
{
    d->unlock();
}

/*!
  Returns TRUE if the mutex is locked by another thread; otherwise
  returns FALSE.

  \warning Due to differing implementations of recursive mutexes on various
  platforms, calling this function from the same thread that previously locked
  the mutex will return undefined results.

  \sa lock(), unlock()
*/
bool QMutex::locked()
{
    return d->locked();
}

/*!
  Attempt to lock the mutex.  If the lock was obtained, this function
  returns TRUE.  If another thread has locked the mutex, this function
  returns FALSE, instead of waiting for the mutex to become available,
  i.e. it does not block.

  The mutex must be unlocked with unlock() before another thread can
  successfully lock it.

  \sa lock(), unlock(), locked()
*/
bool QMutex::tryLock()
{
    return d->trylock();
}

/*!
  \class QMutexLocker qmutex.h
  \brief The QMutexLocker class simplifies locking and unlocking of QMutex.

  \ingroup thread
  \ingroup environment

  The purpose of QMutexLocker is to simplify QMutex locking and unlocking.
  Locking and unlocking a QMutex in complex functions and statements or in
  exception handling code is error prone and difficult to debug.  QMutexLocker
  should be used in such situations to ensure that the state of the mutex is
  well defined and always locked and unlocked properly.

  QMutexLocker should be created within a function where a QMutex needs to be
  locked.  The mutex is locked when QMutexLocker is created, and unlocked when
  QMutexLocker is destroyed.

  For example, this complex function locks a QMutex upon entering the function
  and unlocks the mutex at all the exit points:

  \code
  int complexFunction( int flag )
  {
      mutex.lock();

      int return_value = 0;

      switch ( flag ) {
      case 0:
      case 1:
          {
              mutex.unlock();
	      return moreComplexFunction( flag );
          }

      case 2:
          {
	      int status = anotherFunction();
	      if ( status < 0 ) {
	          mutex.unlock();
		  return -2;
	      }
	      return_value = status + flag;
              break;
	  }

      default:
          {
              if ( flag > 10 ) {
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

  This example function will get more complicated as it is developed, which
  increases the opportunity for errors to occur.

  Using QMutexLocker greatly simplifies the code, and makes it more readable:

  \code
  int complexFunction( int flag )
  {
      QMutexLocker locker( &mutex );

      int return_value = 0;

      switch ( flag ) {
      case 0:
      case 1:
          {
	      return moreComplexFunction( flag );
          }

      case 2:
          {
	      int status = anotherFunction();
	      if ( status < 0 )
		  return -2;
	      return_value = status + flag;
              break;
	  }

      default:
          {
              if ( flag > 10 )
		  return -1;
	      break;
	  }
      }

      return return_value;
  }
  \endcode

  Now, the mutex will always be unlocked when the QMutexLocker object is
  destroyed (when the function returns).

  The same principle applies to code that throws and catches exceptions.  An
  exception that is not caught in the function that has locked the mutex has
  no way of unlocking the mutex before the exception is passed up the stack
  to the calling function.

  QMutexLocker also provides a mutex() member function that returns the
  mutex on which the QMutexLocker is operating.  This is useful for
  code that needs access to the mutex, such as QWaitCondition::wait().  For
  example:

  \code
  class SignalWaiter
  {
  private:
      QMutexLocker locker;

  public:
      SignalWaiter( QMutex *mutex )
        : locker( mutex )
      {
      }

      void waitForSignal()
      {
          ...
	  ...
	  ...

	  while ( ! signalled )
              waitcondition.wait( locker.mutex() );

	  ...
	  ...
	  ...
      }
  };
  \endcode

  \sa QMutex, QWaitCondition
*/

/*! \fn QMutexLocker::QMutexLocker( QMutex *mutex )

  Constructs a QMutexLocker and locks \a mutex.  The mutex will be unlocked
  when the QMutexLocker is destroyed.

  \sa QMutex::lock()
*/

/*! \fn QMutexLocker::~QMutexLocker()

  Destroys the QMutexLocker and unlocks the mutex which was locked in the
  constructor.

  \sa QMutexLocker::QMutexLocker(), QMutex::unlock()
*/

/*! \fn QMutex *QMutexLocker::mutex() const

  Returns a pointer to the mutex which was locked in the constructor.

  \sa QMutexLocker::QMutexLocker()
*/

#endif // QT_THREAD_SUPPORT
