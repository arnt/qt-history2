/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.cpp#9 $
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

RCSTAG("$Id: //depot/qt/main/src/kernel/qtimer.cpp#9 $")


/*----------------------------------------------------------------------------
  \class QTimer qtimer.h

  \brief The QTimer class provides timer signals and single-shot timers.

  \ingroup time
  \ingroup event

  It uses the \link QTimerEvent timer event \endlink internally to
  provide a more versatile timer.  QTimer is very easy to use, create
  a QTimer, call start() to start it and connect its timeout() to the
  appropriate slots, then when the time is up it will emit timeout().

  Note that a QTimer object is destroyed automatically when its parent
  object is destroyed.

  \todo add QTimer::timeLeft()

  \sa QTimerEvent QObject::event() QWidget::timerEvent()
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
