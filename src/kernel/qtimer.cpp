/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.cpp#7 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qtimer.cpp#7 $";
#endif


/*----------------------------------------------------------------------------
  \class QTimer qtimer.h
  \brief The QTimer class provides timer signals and single-shot timers.

  This class starts an internal \link QTimerEvent timer event\endlink
  that is received by the event() handler.  Timer events are more
  low-level and they do not emit signals or provide single-shot timers,
  which QTimer does.

  The limitation of QTimer is that is can have only one active timer.
 ----------------------------------------------------------------------------*/


const int INV_TIMER = -1;			// invalid timer id


/*----------------------------------------------------------------------------
  Constructs a timer with a \e parent and a \e name.

  Notice that the destructor of the parent object will
  destroy this timer object.
 ----------------------------------------------------------------------------*/

QTimer::QTimer( QObject *parent, const char *name )
    : QObject( parent, name )
{
    initMetaObject();
    id = INV_TIMER;
}

/*----------------------------------------------------------------------------
  Destroys the timer.
 ----------------------------------------------------------------------------*/

QTimer::~QTimer()
{
    if ( id != INV_TIMER )			// stop running timer
	stop();
}


/*----------------------------------------------------------------------------
  \fn void QTimer::timeout()
  This signal is emitted when the timer is activated.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QTimer::isActive() const
  Returns TRUE if the timer is running (pending), or FALSE is the timer is
  idle.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Starts the timer with a \e msecs milliseconds timeout.

  If \e sshot is TRUE, the timer will be activated only once,
  otherwise it will continue until it is stopped.

  Any pending timer will be stopped.

  \sa stop(), changeInterval(), isActive()
 ----------------------------------------------------------------------------*/

int QTimer::start( long msec, bool sshot )	// start timer
{
    if ( id != INV_TIMER )			// stop running timer
	stop();
    single = sshot;
    return id = startTimer( msec );
}


/*----------------------------------------------------------------------------
  Changes the timeout interval to \e msec milliseconds.

  If the timer signal is pending, it will be stopped and restarted,
  otherwise it will be started.

  \sa start(), isActive()
 ----------------------------------------------------------------------------*/

void QTimer::changeInterval( long msec )	// change running timer
{
    if ( id == INV_TIMER )			// create new timer
	start( msec );
    else {
	killTimer( id );			// restart timer
	id = startTimer( msec );
    }
}

/*----------------------------------------------------------------------------
  Stops the timer.
  \sa start()
 ----------------------------------------------------------------------------*/

void QTimer::stop()
{
    if ( id != INV_TIMER ) {
	killTimer( id );
	id = INV_TIMER;
    }
}


/*----------------------------------------------------------------------------
  Handles timer events.  Emits timeout() when a timer event is received.
 ----------------------------------------------------------------------------*/

bool QTimer::event( QEvent *e )
{
    if ( e->type() != Event_Timer )		// ignore all other events
	return FALSE;
    if ( single )				// stop single shot timer
	stop();
    emit timeout();				// emit timeout signal
    return TRUE;
}
