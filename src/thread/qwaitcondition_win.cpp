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

#include "qwaitcondition.h"
#include "qmutex.h"

#include <windows.h>
#include <qatomic.h>

#include "qmutex_p.h"


/*
  QWaitConditionPrivate
*/

class QWaitConditionPrivate
{
public:
    QWaitConditionPrivate();
    ~QWaitConditionPrivate();

    int wait( unsigned long time = ULONG_MAX , bool countWaiter = TRUE);
    HANDLE manual;
    HANDLE autoreset;
    QMutex mutex;
    int waitersCount;
};

QWaitConditionPrivate::QWaitConditionPrivate()
: waitersCount(0)
{
    QT_WA( {
	manual = CreateEvent( NULL, TRUE, FALSE, NULL );
	autoreset = CreateEvent( NULL, FALSE, FALSE, NULL );
    } , {
	manual = CreateEventA( NULL, TRUE, FALSE, NULL );
	autoreset = CreateEventA( NULL, FALSE, FALSE, NULL );
    } );

#ifdef QT_CHECK_RANGE
    if ( !manual || !autoreset )
    qSystemWarning( "Condition init failure" );
#endif
}

QWaitConditionPrivate::~QWaitConditionPrivate()
{
    if ( !CloseHandle( manual ) || !CloseHandle( autoreset ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Condition destroy failure" );
#endif
    }
}

int QWaitConditionPrivate::wait( unsigned long time , bool countWaiter )
{
    mutex.lock();
    if ( countWaiter )
	waitersCount++;
    HANDLE hnds[2] = { manual, autoreset };
    mutex.unlock();
    int ret = WaitForMultipleObjects( 2, hnds, FALSE, time );
    mutex.lock();
    waitersCount--;
    mutex.unlock();
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
    case WAIT_OBJECT_0:
	d->mutex.lock();
	if ( !d->waitersCount ) {
	    if ( !ResetEvent ( d->manual ) ) {
#ifdef QT_CHECK_RANGE
		qSystemWarning( "Condition could not be reset" );
#endif
	    }
	}
	d->mutex.unlock();
    default:
	break;
    }
    return TRUE;
}

bool QWaitCondition::wait( QMutex *mutex, unsigned long time)
{
    if ( !mutex )
       return FALSE;

    if ( mutex->d->recursive ) {
#ifdef QT_CHECK_RANGE
	qWarning("QWaitCondition::wait: Cannot wait on recursive mutexes.");
#endif
	return FALSE;
    }

    d->mutex.lock();
    d->waitersCount++;
    d->mutex.unlock();
    mutex->unlock();
    int result = d->wait(time, FALSE);    
    mutex->lock();
    d->mutex.lock();
    bool lastWaiter = ( (result == WAIT_OBJECT_0 )  && d->waitersCount == 0 ); // last waiter on waitAll?
    d->mutex.unlock();
    if ( lastWaiter ) {
	if ( !ResetEvent ( d->manual ) ) {
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
    d->mutex.lock();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
	if ( !SetEvent( d->autoreset ) ) {
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Condition could not be set" );
#endif
	}
    }
    d->mutex.unlock();
}

void QWaitCondition::wakeAll()
{
    d->mutex.lock();
    bool haveWaiters = (d->waitersCount > 0);
    if ( haveWaiters ) {
	if ( !SetEvent( d->manual ) ) {
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Condition could not be set" );
#endif
	}
    }
    d->mutex.unlock();
}
