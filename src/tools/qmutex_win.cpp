/****************************************************************************
** $Id$
**
** Implementation of QMutex class for Win32
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#if defined(QT_THREAD_SUPPORT)

#include "qt_windows.h"
#include "qmutex.h"
#include "qnamespace.h"

#define Q_MUTEX_T void*
#include <private/qmutex_p.h>
#include <private/qcriticalsection_p.h>

QMutexPrivate::~QMutexPrivate()
{
    if ( !CloseHandle( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex destroy failure" );
#endif
    }
}

/*
  QRecursiveMutexPrivate - implements a recursive mutex
*/

class QRecursiveMutexPrivate : public QMutexPrivate
{
public:
    QRecursiveMutexPrivate();

    void lock();
    void unlock();
    bool locked();
    bool trylock();
#ifdef QT_CHECK_RANGE
    virtual int type() const { return Q_MUTEX_RECURSIVE; }
#endif
};

QRecursiveMutexPrivate::QRecursiveMutexPrivate()
{
#ifdef Q_OS_TEMP
	handle = CreateMutex( NULL, FALSE, NULL );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	handle = CreateMutex( NULL, FALSE, NULL );
    else
#endif
	handle = CreateMutexA( NULL, FALSE, NULL );
#endif
#ifdef QT_CHECK_RANGE
    if ( !handle )
	qSystemWarning( "Mutex init failure" );
#endif

}

void QRecursiveMutexPrivate::lock()
{
    switch ( WaitForSingleObject( handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex lock failure" );
#endif
	break;
    case WAIT_ABANDONED:
#ifdef QT_CHECK_RANGE
	qWarning( "Thread terminated while locking mutex!" );
#endif
	// Fall through
    default:
	break;
    }
}

void QRecursiveMutexPrivate::unlock()
{
    if ( !ReleaseMutex( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex unlock failure" );
#endif
    }
}

bool QRecursiveMutexPrivate::locked()
{
    switch ( WaitForSingleObject( handle, 0) ) {
    case WAIT_TIMEOUT:
	return TRUE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    case WAIT_OBJECT_0:
	return FALSE;
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

bool QRecursiveMutexPrivate::trylock()
{
    switch ( WaitForSingleObject( handle, 0) ) {
    case WAIT_FAILED:
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    case WAIT_OBJECT_0:
	break;
    default:
	break;
    }
    return TRUE;
}

/*
  QNonRecursiveMutexPrivate - implements a non-recursive mutex
*/

class QNonRecursiveMutexPrivate : public QRecursiveMutexPrivate
{
public:
    QNonRecursiveMutexPrivate();

    void lock();
    void unlock();
    bool trylock();
#ifdef QT_CHECK_RANGE
    int type() const { return Q_MUTEX_NORMAL; };
#endif

    unsigned int threadID;
    QCriticalSection protect;
};

QNonRecursiveMutexPrivate::QNonRecursiveMutexPrivate()
    : QRecursiveMutexPrivate()
{
    threadID = 0;
}

void QNonRecursiveMutexPrivate::lock()
{
    protect.enter();

    if ( threadID == GetCurrentThreadId() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Non-recursive mutex already locked by this thread" );
#endif
    } else {
	protect.leave();
	switch ( WaitForSingleObject( handle, INFINITE ) ) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Mutex lock failure" );
#endif
	    break;
	case WAIT_ABANDONED:
#ifdef QT_CHECK_RANGE
	    qWarning( "Thread terminated while locking mutex!" );
#endif
	    // Fall through
	default:
	    protect.enter();
	    threadID = GetCurrentThreadId();
	    protect.leave();
	    break;
	}
    }
}

void QNonRecursiveMutexPrivate::unlock()
{
    protect.enter();
    QRecursiveMutexPrivate::unlock();
    threadID = 0;
    protect.leave();
}

bool QNonRecursiveMutexPrivate::trylock()
{
    protect.enter();

    if (threadID == GetCurrentThreadId()) {
	// locked by this thread already, return FALSE
	return FALSE;
    }

    protect.leave();

    switch (WaitForSingleObject(handle, 0)) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
	return FALSE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    default:
	protect.enter();
	threadID = GetCurrentThreadId();
	protect.leave();
	break;
    }

    return TRUE;
}

/*
  QMutex implementation
*/

QMutex::QMutex(bool recursive)
{
    if ( recursive )
	d = new QRecursiveMutexPrivate();
    else
	d = new QNonRecursiveMutexPrivate();
}

QMutex::~QMutex()
{
    delete d;
}

void QMutex::lock()
{
    d->lock();
}

void QMutex::unlock()
{
    d->unlock();
}

bool QMutex::locked()
{
    return d->locked();
}

bool QMutex::tryLock()
{
    return d->trylock();
}

#endif
