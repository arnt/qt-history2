/****************************************************************************
** $Id$
**
** Implementation of QWaitCondition class for Win32
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

#include "qwaitcondition.h"
#include "qnamespace.h"
#include "qmutex.h"
#include "qt_windows.h"

#define Q_MUTEX_T void*
#include <private/qmutex_p.h>
#include <private/qcriticalsection_p.h>

/*
  QWaitConditionPrivate
*/

class QWaitConditionPrivate
{
public:
    QWaitConditionPrivate();
    ~QWaitConditionPrivate();

    int wait( unsigned long time = ULONG_MAX , bool countWaiter = TRUE);
    Qt::HANDLE handle;
    Qt::HANDLE single;
    QCriticalSection s;
    int waitersCount;
};

QWaitConditionPrivate::QWaitConditionPrivate()
: waitersCount(0)
{
#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	handle = CreateEvent( NULL, TRUE, FALSE, NULL );
	single = CreateEvent( NULL, FALSE, FALSE, NULL );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	handle = CreateEventA( NULL, TRUE, FALSE, NULL );
	single = CreateEventA( NULL, FALSE, FALSE, NULL );
    }
#endif

#ifdef QT_CHECK_RANGE
    if ( !handle || !single )
    qSystemWarning( "Condition init failure" );
#endif
}

QWaitConditionPrivate::~QWaitConditionPrivate()
{
    if ( !CloseHandle( handle ) || !CloseHandle( single ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Condition destroy failure" );
#endif
    }
}

int QWaitConditionPrivate::wait( unsigned long time , bool countWaiter )
{
    s.enter();
    if ( countWaiter )
	waitersCount++;
    Qt::HANDLE hnds[2] = { handle, single };
    s.leave();
    int ret = WaitForMultipleObjects( 2, hnds, FALSE, time );
    s.enter();
    waitersCount--;
    s.leave();
    return ret;
}

/*
  QWaitCondition implementation
*/

QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate();
}

QWaitCondition::~QWaitCondition()
{
    delete d;
}

bool QWaitCondition::wait( unsigned long time )
{
    switch ( d->wait(time) ) {
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED:
    case WAIT_ABANDONED+1:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
    qSystemWarning( "Condition wait failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

bool QWaitCondition::wait( QMutex *mutex, unsigned long time)
{
    if ( !mutex )
       return FALSE;

    if ( mutex->d->type() == Q_MUTEX_RECURSIVE ) {
#ifdef QT_CHECK_RANGE
	qWarning("QWaitCondition::wait: Cannot wait on recursive mutexes.");
#endif
	return FALSE;
    }

    d->s.enter();
    d->waitersCount++;
    d->s.leave();
    mutex->unlock();
    int result = d->wait(time, FALSE);
    mutex->lock();
    d->s.enter();
    bool lastWaiter = ( (result == WAIT_OBJECT_0 )  && d->waitersCount == 0 ); // last waiter on waitAll?
    d->s.leave();
    if ( lastWaiter ) {
	if ( !ResetEvent ( d->handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Condition could not be reset" );
#endif
	}
    }
    switch ( result ) {
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED:
    case WAIT_ABANDONED+1:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
    qSystemWarning( "Condition wait failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

void QWaitCondition::wakeOne()
{
    d->s.enter();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
	if ( !SetEvent( d->single ) ) {
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Condition could not be set" );
#endif
	}
    }
    d->s.leave();
}

void QWaitCondition::wakeAll()
{
    d->s.enter();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
	if ( !PulseEvent( d->handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Condition could not be set" );
#endif
	}
    }
    d->s.leave();
}

#endif
