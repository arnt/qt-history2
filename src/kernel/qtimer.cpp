/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.cpp#5 $
**
** Implementation of QTimer class
**
** Author  : Haavard Nord
** Created : 931111
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtimer.h"
#include "qevent.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qtimer.cpp#5 $";
#endif

/*! \class QTimer qtimer.h

  \brief The QTimer class provides millisecond timers.

  This class is not yet documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */


const int INV_TIMER = -1;			// invalid timer id


QTimer::QTimer( QObject *parent, const char *name ) : QObject( parent, name )
{
    initMetaObject();
    id = INV_TIMER;
}

QTimer::~QTimer()
{
    if ( id != INV_TIMER )			// stop running timer
	stop();
}


int QTimer::start( long msec, bool sshot )	// start timer
{
    if ( id != INV_TIMER )			// stop running timer
	stop();
    single = sshot;
    return id = startTimer( msec );
}

void QTimer::changeInterval( long msec )	// change running timer
{
    if ( id == INV_TIMER )			// create new timer
	start( msec );
    else {
	killTimer( id );			// restart timer
	id = startTimer( msec );
    }
}

void QTimer::stop()				// stop timer
{
    if ( id != INV_TIMER ) {
	killTimer( id );
	id = INV_TIMER;
    }
}


bool QTimer::event( QEvent *e )			// process events
{
    if ( e->type() != Event_Timer )		// ignore all other events
	return FALSE;
    if ( single )				// stop single shot timer
	stop();
    emit timeout();				// emit timeout signal
    return TRUE;
}
