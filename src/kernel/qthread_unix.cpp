/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_unix.cpp#4 $
**
** QThread class for Unix
**
** Created : 931107
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qthread.h"
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <qlist.h>
#include <unistd.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qobject.h>

class QMutexPrivate {

public:

    pthread_mutex_t mymutex;
    QMutexPrivate();
    ~QMutexPrivate();

};

QMutexPrivate::QMutexPrivate()
{
    int ret = pthread_mutex_init( &mymutex, 0 );
    if( ret ) {
	qFatal( "Mutex init failure %s", strerror( ret ) );
    }
}

QMutexPrivate::~QMutexPrivate()
{
    int ret = pthread_mutex_destroy( &mymutex );
    if( ret ) {
	qWarning( "Mutex destroy failure %s", strerror( ret ) );
    }
}

QMutex::QMutex()
{
    d = new QMutexPrivate();
}

QMutex::~QMutex()
{
    delete d;
}

void QMutex::lock()
{
    int ret = pthread_mutex_lock( &( d->mymutex ) );
    if( ret ) {
	qFatal( "Mutex lock failure %s\n", strerror( ret ) );
    }
}

void QMutex::unlock()
{
    int ret = pthread_mutex_unlock( &( d->mymutex ) );
    if( ret ) {
        qFatak( "Mutex unlock failure %s\n", strerror( ret ) );
    }
}

int QThread::currentThread()
{
    // A pthread_t is an int
    return pthread_self();
}

class QThreadEvent {

public:

    QObject * o;
    QEvent * e;

};

class QThreadPrivate : public QObject {

    Q_OBJECT

public:

    bool woken;

    QThreadPrivate();
    void wakeGuiThread();

    QList<QThreadEvent> myevents;
    QMutex myeventmutex;

public slots:

    void socketActivated( int );

private:

    int wakeupPipe[2];

};

#include "qthread_unix.moc"

QThreadPrivate::QThreadPrivate()
{
    if (  pipe(  wakeupPipe )  ) {
	qFatal( "Couldn't open thread pipe: %s\n", strerror( errno ) );
    }
    myevents.setAutoDelete( TRUE );
    QSocketNotifier * sn = new QSocketNotifier ( wakeupPipe[1], QSocketNotifier::Read, this );
    connect( sn, SIGNAL( activated( int ) ), this, SLOT( socketActivated( int ) ) );
    woken = false;
}

void QThreadPrivate::socketActivated( int )
{
    char c;
    read( wakeupPipe[0], &c, 1 );
    QThread::sendPostedEvents();
}

static QThreadPrivate * qthreadprivate = 0;

void QThreadPrivate::wakeGuiThread()
{
    char c = 1;
    write(  wakeupPipe[1], &c, 1  );
}

void QThread::postEvent( QObject * o, QEvent * e )
{
    if( !qthreadprivate ) {
	qthreadprivate = new QThreadPrivate();
    }
    qthreadprivate->myeventmutex.lock();
    QThreadEvent * qte = new QThreadEvent;
    qte->o = o;
    qte->e = e;
    qthreadprivate->myevents.append( qte );
    qthreadprivate->myeventmutex.unlock();
    if( !qthreadprivate->woken ) {
	qthreadprivate->wakeGuiThread();
	qthreadprivate->woken = true;
    }
}

void QThread::sendPostedEvents()
{
    if( !qthreadprivate ) {
	qthreadprivate = new QThreadPrivate();
    }
    qthreadprivate->myeventmutex.lock();
    qthreadprivate->woken = false;
    QThreadEvent * qte;
    for( qte = qthreadprivate->myevents.first(); qte != 0; qte = qthreadprivate->myevents.next() ) {
	qApp->postEvent( qte->o, qte->e );
    }
    qthreadprivate->myevents.clear();
    qApp->sendPostedEvents();
    qthreadprivate->myeventmutex.unlock();
}
