/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread.cpp#10 $
**
** Implementation of QThread class
**
** Created : 961223
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qthread.h"

#if defined(QTHREAD_WIN32)
#include <windows.h>
#include <process.h>
#elif defined(QTHREAD_POSIX)
#error "something in the tea does not compile."
#endif


/*!
  \class QThread qthread.h
  \brief QThread represents an OS thread.

  \ingroup kernel

  Threads are concurrently executing code.

  All threads in an application (process) share all global variables and file
  handles. It therefore might be necessary to keep threads from accessing
  the same global data at the same time; see QMutex for that.

  \sa QMutex
*/

/*!
  \fn QThread::QThread( QThreadID id )

  Constructs a new QThread object from a running thread having the
  identifier \e id. If \e id is -1, the thread is invalid.
*/


/*!
  \fn QThread::QThread( QThreadFunction func, void *args=0, int stackSize=-1 )

  Constructs a new thread object and starts a thread. The function \e func
  is called with the arguments \e args. The stack size of this thread is
  set to \e stackSize bytes, or it is set automatically if the default
  argument -1 is used.
*/

/*!
  \fn QThread::QThread( const QThread &thread )
  Constructs a thread object referring to the same running thread as
  \e thread.
*/


/*!
  \fn QThread::~QThread()
  Destroys the thread object. Note that the thread is not terminated.
  You must call terminate() to kill a thread.
*/

QThread::~QThread()
{
}


/*!
  \fn QThread &QThread::operator=( const QThread &thread )
  Assigns \e thread to this thread and returns a reference to this
  thread.
*/

/*!
  \fn bool QThread::isValid() const
  Returns TRUE if the thread ID is valid, otherwise FALSE.
  \sa id()
*/


/*!
  \fn QThreadID QThread::id() const
  Returns the thread identifier.
*/


/*!
  Returns the thread's priority.
*/

int QThread::priority() const
{
#if defined(QTHREAD_WIN32)
    return GetThreadPriority( id() );
#elif defined(QTHREAD_POSIX)
#else
    return 0;
#endif
}

/*!
  Sets the thread's priority to \e priority.
*/

void QThread::setPriority( int priority )
{
#if defined(QTHREAD_WIN32)
    SetThreadPriority( id(), priority );
#elif defined(QTHREAD_POSIX)
#else
#endif
}


/*!
  \fn void QThread::suspend()
  Suspends the thread execution.
  \sa resume()
*/

/*!
  \fn void QThread::resume()
  Resumes the execution of a suspended thread.
  \sa suspend()
*/

/*!
  \fn void QThread::terminate()
  Terminates this thread and sets the thread identifier to -1.

  Other QThread objects that refer to this thread keep the original
  identifier.
*/


/*!
  Returns a QThread object that contain the identifier of the current
  thread.
  \sa currentThreadId()
*/

QThread QThread::currentThread()
{
    return QThread( currentThreadId() );
}

/*!
  Returns the identifier of the current thread.
  \sa currentThread()
*/

QThreadID QThread::currentThreadId()
{
#if defined(QTHREAD_WIN32)
    return GetCurrentThread();
#elif defined(QTHREAD_POSIX)
#else
    return (QThreadID)-1;
#endif
}


/*!
  Starts a thread and returns the thread identifier.
*/

QThreadID QThread::start( QThreadFunction func, void *args,
			  int stackSize )
{
#if defined(QTHREAD_WIN32)
    if ( stackSize < 0 )
	stackSize = 0;
    return (QThreadID)_beginthread( func, stackSize, args );
#elif defined(QTHREAD_POSIX)
#else
    return (QThreadID)-1;
#endif
}


/*!
  Exits the current thread.
*/

void QThread::exit( int retcode )
{
#if defined(QTHREAD_WIN32)
    CloseHandle( GetCurrentThread() );
    _endthreadex( retcode );
#elif defined(QTHREAD_POSIX)
#else
#endif
}


/*!
  \overload void QThread::suspend( QThreadID id )
  This static function uses a thread identifier instead of a QThread object.
*/

void QThread::suspend( QThreadID id )
{
#if defined(QTHREAD_WIN32)
    SuspendThread( id );
#endif
}

/*!
  \overload void QThread::resume( QThreadID id )
  This static function uses a thread identifier instead of a QThread object.
*/

void QThread::resume( QThreadID id )
{
#if defined(QTHREAD_WIN32)
    ResumeThread( id );
#endif
}

/*!
  \overload void QThread::terminate( QThreadID id )
  This static function uses a thread identifier instead of a QThread object.
*/

void QThread::terminate( QThreadID id )
{
#if defined(QTHREAD_WIN32)
    TerminateThread( id, 0 );
#elif defined(QTHREAD_POSIX)
#else
#endif
}


/*!  Returns TRUE if Qt is able to deliver threading on this
  combination of operating and window system, and FALSE if not.
*/

bool QThread::available()
{
    return TRUE;    
}
