#if defined(QT_THREAD_SUPPORT)

#include "qnamespace.h"
#include "qsemaphore.h"
#include "qwaitcondition.h"
#include "qt_windows.h"

#include <private/qcriticalsection_p.h>

/*
  QSemaphore implementation
*/

class QSemaphorePrivate
{
public:
    Qt::HANDLE handle;
    int count;
    int maxCount;
    QCriticalSection protect;
    QWaitCondition dontBlock;
};

QSemaphore::QSemaphore( int maxcount )
{
    d = new QSemaphorePrivate;
    d->maxCount = maxcount;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	d->handle = CreateSemaphore( NULL, maxcount, maxcount, NULL );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	d->handle = CreateSemaphoreA( NULL, maxcount, maxcount, NULL );
    }
#endif

#ifdef QT_CHECK_RANGE
    if ( !d->handle )
	qSystemWarning( "Semaphore init failure" );
#endif
    d->count = maxcount;
}

QSemaphore::~QSemaphore()
{
    if ( !CloseHandle( d->handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore close failure" );
#endif
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
    d->protect.enter();
    if ( d->count == d->maxCount ) {
	d->protect.leave();
	return d->count;
    }

    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, 1, NULL ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
    } else {
	c = ++d->count;
	d->dontBlock.wakeAll();
    }
    d->protect.leave();

    return c;
}

int QSemaphore::operator -=(int s)
{
    d->protect.enter();
    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, s, NULL ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
    } else {
	d->dontBlock.wakeAll();
	d->count += s;
	if ( d->count > d->maxCount )
	    d->count = d->maxCount;
	c = d->count;
    }
    d->protect.leave();

    return c;
}

int QSemaphore::operator++(int)
{
    switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore wait failure" );
#endif
	return d->count;
    default:
	break;
    }
    d->protect.enter();
    int c = --d->count;
    d->protect.leave();

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
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Semaphore wait failure" );
#endif
	    return d->count;
	default:
	    break;
	}
	d->count--;
    }
    int c = d->count;
    d->protect.leave();

    return c;
}

#endif
