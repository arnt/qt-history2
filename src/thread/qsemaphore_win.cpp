/****************************************************************************
**
** Implementation of QSemaphore class for Win32.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsemaphore.h"
#include "qwaitcondition.h"
#include "qmutex.h"

#include <windows.h>

/*
  QSemaphore implementation
*/

class QSemaphorePrivate
{
public:
    HANDLE handle;
    int count;
    int maxCount;
    QMutex protect;
};

QSemaphore::QSemaphore( int maxcount )
{
    d = new QSemaphorePrivate;
    d->maxCount = maxcount;
    QT_WA( {
	d->handle = CreateSemaphore( NULL, maxcount, maxcount, NULL );
    } , {
	d->handle = CreateSemaphoreA( NULL, maxcount, maxcount, NULL );
    } );

    if ( !d->handle )
	qSystemWarning( "Semaphore init failure" );
    d->count = maxcount;
}

QSemaphore::~QSemaphore()
{
    if ( !CloseHandle( d->handle ) ) {
	qSystemWarning( "Semaphore close failure" );
    }
    delete d;
}

int QSemaphore::available() const
{
    return d->count;
}

int QSemaphore::total() const
{
    return d->maxCount;
}

int QSemaphore::operator--(int)
{
    d->protect.lock();
    if ( d->count == d->maxCount ) {
	d->protect.unlock();
	return d->count;
    }

    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, 1, NULL ) ) {
	qSystemWarning( "Semaphore release failure" );
    } else {
	c = ++d->count;
	d->dontBlock.wakeAll();
    }
    d->protect.unlock();

    return c;
}

int QSemaphore::operator -=(int s)
{
    if ( !ReleaseSemaphore( d->handle, s, NULL ) ) {
	qSystemWarning( "Semaphore release failure" );
	d->protect.lock();
	int c = d->count;
	d->protect.unlock();
	return c;
    }

    d->protect.lock();
    int c = d->count;
    d->count += s;
    if ( d->count > d->maxCount )
	d->count = d->maxCount;
    c = d->count;
    d->dontBlock.wakeAll();
    d->protect.unlock();
    return c;
}

int QSemaphore::operator++(int)
{
    switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
	qSystemWarning( "Semaphore wait failure" );
	return d->count;
    default:
	break;
    }
    d->protect.lock();
    int c = --d->count;
    d->protect.unlock();

    return c;
}

int QSemaphore::operator +=(int s)
{
    while ( d->count < s )
	d->dontBlock.wait();

    d->protect.enter();
    for ( int i = 0; i < s; i++ ) {
	switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
	    qSystemWarning( "Semaphore wait failure" );
	    return d->count;
	default:
	    break;
	}
	d->protect.lock();
	d->count--;
	d->protect.unlock();
    }
    int c = d->count;
    d->protect.leave();

    return c;
}

bool QSemaphore::tryAccess( int n )
{
    d->protect.lock();
    if ( available() < n ) {
	d->protect.unlock();
	return FALSE;
    }
    d->protect.unlock();
    operator+=( n );

    return TRUE;
}
