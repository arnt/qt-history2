/****************************************************************************
** $Id$
**
** QThread class for windows
**
** Created : 20000913
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qthread.h"
#include "qt_windows.h"
#include "qobject.h"
#include "qapplication.h"
#include "qintdict.h"

#include <private/qcriticalsection_p.h>

#ifndef Q_OS_TEMP
#ifndef _MT
#define _MT
#endif
#include <process.h>
#endif

/*
  QThreadQtEvent
*/

class QThreadQtEvent
{
public:
    QThreadQtEvent(QObject *o, QEvent *e)
	: object(o), event(e)
    {}
    QObject *object;
    QEvent *event;
};

/*
  QThreadEventsPrivate
*/

class QThreadPostEventPrivate : public QObject
{
    Q_OBJECT
public:
    QThreadPostEventPrivate();
    QPtrList<QThreadQtEvent> events;
    QCriticalSection protect;
    void add(QThreadQtEvent *);
public slots:
    void sendEvents();
private:
};

static QThreadPostEventPrivate * qthreadposteventprivate = 0;

QThreadPostEventPrivate::QThreadPostEventPrivate()
{
    events.setAutoDelete( TRUE );
    connect( qApp, SIGNAL( guiThreadAwake() ), this, SLOT( sendEvents() ) );
}

void QThreadPostEventPrivate::sendEvents()
{
    protect.enter();
    QThreadQtEvent * qte;
    for( qte = events.first(); qte != 0; qte = events.next() )
	qApp->postEvent( qte->object, qte->event );
    events.clear();
    protect.leave();
}

void QThreadPostEventPrivate::add(QThreadQtEvent * e)
{
    events.append( e );
}

/*
  QThreadPrivate
*/

class QThreadPrivate
{
public:
    static void internalRun( QThread* );

    QThreadPrivate();
    ~QThreadPrivate();

    Qt::HANDLE handle;
    unsigned int id;
    bool finished  : 1;
    bool running   : 1;
    bool deleted   : 1;
};

static QIntDict<QThreadPrivate> *threadDict = 0;
static QCriticalSection *dictSection_ = 0;
static QCriticalSection *dictSection()
{
    if ( !dictSection_ )
	dictSection_ = new QCriticalSection();
    return dictSection_;
}

#if defined(Q_C_CALLBACKS)
extern "C"
#endif
static unsigned int __stdcall start_thread(void* that )
{
    QThreadPrivate::internalRun( (QThread*)that );

    return 0;
}

QThreadPrivate::QThreadPrivate()
{
    handle = 0;
    id = 0;
    running = FALSE;
    finished = FALSE;
    deleted = FALSE;
}

QThreadPrivate::~QThreadPrivate()
{
    if ( running && !finished ) {
	qWarning("QThread object destroyed while thread is still running. Terminating thread...");
	TerminateThread( handle, 0 );
	finished = TRUE;
    } else if ( handle && !CloseHandle( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning("Thread destroy failure");
#endif
    }
}

void QThreadPrivate::internalRun( QThread* that )
{
    QThreadPrivate *d = that->d;
    dictSection()->enter();
    if ( !threadDict )
	threadDict = new QIntDict<QThreadPrivate>();
    threadDict->insert( d->id, d );
    dictSection()->leave();

    that->run();
    d->running = FALSE;

    dictSection()->enter();
    if ( threadDict ) {
	threadDict->remove( d->id );
    }
    d->id = 0;
    dictSection()->leave();

    if ( d->deleted )
	delete d;
    else
	d->finished = TRUE;
}

/*
  QThread implementation
*/

/*
  QThread static functions
*/

Qt::HANDLE QThread::currentThread()
{
    return (Qt::HANDLE)GetCurrentThreadId();
}

void QThread::initialize()
{
    if( !qthreadposteventprivate )
	qthreadposteventprivate = new QThreadPostEventPrivate();
}

void QThread::cleanup()
{
    delete qthreadposteventprivate;
    qthreadposteventprivate = 0;
}

void QThread::postEvent( QObject *o,QEvent *e )
{
    qthreadposteventprivate->protect.enter();
    qthreadposteventprivate->add( new QThreadQtEvent(o,e)  );
    qthreadposteventprivate->protect.leave();
    qApp->wakeUpGuiThread();
}

void QThread::exit()
{
    DWORD id = GetCurrentThreadId();
    dictSection()->enter();
    QThreadPrivate *that = threadDict->take( id );
    if ( !threadDict->count() ) {
	delete threadDict;
	threadDict = 0;
    }
    dictSection()->leave();

    if ( that ) {
	that->running = FALSE;
	that->finished = TRUE;
	CloseHandle( that->handle );
	that->handle = 0;
	that->id = 0;
    }

    _endthreadex(0);
}

void QThread::sleep( unsigned long secs )
{
    ::Sleep( secs * 1000 );
}

void QThread::msleep( unsigned long msecs )
{
    ::Sleep( msecs );
}

void QThread::usleep( unsigned long usecs )
{
    ::Sleep( ( usecs / 1000 ) + 1 );
}

QThread::QThread()
{
    d = new QThreadPrivate;
}

QThread::~QThread()
{
    if ( threadDict ) {
	dictSection()->enter();
	threadDict->remove( d->id );
	dictSection()->leave();
	if ( !threadDict->count() ) {
	    delete threadDict;
	    threadDict = 0;
	}
    }
    if ( dictSection_ && !threadDict ) {
	delete dictSection_;
	dictSection_ = 0;
    }

    if( d->running && !d->finished ) {
#if defined(QT_CHECK_RANGE)
	qWarning("QThread object destroyed while thread is still running.");
#endif
	d->deleted = TRUE;
    } else {
	delete d;
    }
}

void QThread::start()
{
    if ( d->running && !d->finished ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Thread is already running" );
#endif
	wait();
    }

    d->running = TRUE;
    d->finished = FALSE;
    d->handle = (Qt::HANDLE)_beginthreadex( NULL, NULL, start_thread,
	this, 0, &(d->id) );

#ifdef QT_CHECK_RANGE
    if ( !d->handle )
	qSystemWarning( "Couldn't create thread" );
#endif
}

bool QThread::wait( unsigned long time )
{
    if ( d->id == GetCurrentThreadId() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "Thread tried to wait on itself" );
#endif
	return FALSE;
    }
    if ( d->finished || !d->running )
	return TRUE;
    switch ( WaitForSingleObject( d->handle, time ) ) {
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED_0:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Thread wait failure" );
#endif
	return FALSE;
    default:
	break;
    }
    return TRUE;
}

bool QThread::finished() const
{
    return d->finished;
}

bool QThread::running() const
{
    return d->running;
}

#include "qthread_win.moc"

#endif
