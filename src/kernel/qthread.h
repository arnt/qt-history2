/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread.h#11 $
**
** Definition of QThread class
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

#ifndef QTHREAD_H
#define QTHREAD_H

#ifndef QT_H
#include "qglobal.h"
#include "qwindowdefs.h"
#endif // QT_H


// Microsoft Windows
#if defined(_MT) && defined(_OS_WIN32_)
#define QTHREAD_WIN32
typedef HANDLE QThreadID;

// pthreads on unix.  might need to include some more stuff?
#elif (defined(MT) || defined(REENTRANT) || defined(PTHREADS)) && defined(UNIX)
#define QTHREAD_POSIX
typedef int QThreadID;

// no thread support
#else
#define QTHREAD_NONE
typedef int QThreadID; // so the file compiles
#endif


const QThreadID invalidQThreadID = (QThreadID)-1;

typedef void (*QThreadFunction)(void*);


class Q_EXPORT QThread
{
public:
    QThread( QThreadID id = invalidQThreadID );
    QThread( QThreadFunction func, void *args=0, int stackSize=-1 );
    QThread( const QThread & );
    virtual ~QThread();

    QThread    &operator=( const QThread & );

    bool	isValid() const;

    QThreadID	id() const;

    int		priority() const;
    virtual void setPriority( int );

    virtual void suspend();
    virtual void resume();
    virtual void terminate();

  // Static thread functions
    static QThread   currentThread();
    static QThreadID currentThreadId();
    static QThreadID start( QThreadFunction func, void *args=0,
			    int stackSize=-1 );
    static void	exit( int retcode=0 );

    static void	suspend( QThreadID );
    static void	resume( QThreadID );
    static void	terminate( QThreadID );

    static bool available();

private:
    QThreadID	tid;
    void * d;
};


inline QThread::QThread( QThreadID id )
    : tid(id), d(0)
{
}

inline QThread::QThread( QThreadFunction func, void *args,
			 int stackSize )
{
    tid = QThread::start( func, args, stackSize );
    d = 0;
}

inline QThread::QThread( const QThread &t )
    : tid(t.tid), d(0)
{
}

inline QThread &QThread::operator=( const QThread &t )
{
    tid = t.tid;
    return *this;
}

inline bool QThread::isValid() const
{
    return tid >= 0;
}

inline QThreadID QThread::id() const
{
    return tid;
}

inline void QThread::suspend()
{
    suspend( id() );
}

inline void QThread::resume()
{
    resume( id() );
}

inline void QThread::terminate()
{
    terminate( id() );
}


#endif // QTHREAD_H
