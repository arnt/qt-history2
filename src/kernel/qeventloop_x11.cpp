/****************************************************************************
** $Id$
**
** Implementation of QEventLoop class
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qeventloop_p.h" // includes qplatformdefs.h
#include "qeventloop.h"
#include "qapplication.h"
#include "qbitarray.h"
#include "qcolor_p.h"
#include "qt_x11.h"

#include <errno.h>


// resolve the conflict between X11's FocusIn and QEvent::FocusIn
static const int XFocusOut = FocusOut;
static const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

static const int XKeyPress = KeyPress;
static const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease

// from qapplication.cpp
extern bool qt_is_gui_used;

// from qeventloop_unix.cpp
extern timeval *qt_wait_timer();
extern void cleanupTimers();


void QEventLoop::init()
{
    // initialize the common parts of the event loop
    pipe( d->thread_pipe );
    d->sn_highest = -1;

    // intitialize the X11 parts of the event loop
    d->xfd = -1;
    if ( qt_is_gui_used )
        d->xfd = XConnectionNumber( QPaintDevice::x11AppDisplay() );
}

void QEventLoop::cleanup()
{
    // cleanup the common parts of the event loop
    close( d->thread_pipe[0] );
    close( d->thread_pipe[1] );
    cleanupTimers();

    // cleanup the X11 parts of the event loop
    d->xfd = -1;
}

bool QEventLoop::x11ProcessEvent( XEvent *event )
{
    return qApp->x11ProcessEvent( event );
}

bool QEventLoop::processNextEvent( ProcessEventsFlags flags, bool canWait )
{
    // process events from the X server
    XEvent event;
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker( &d->mutex );
#endif

    // we are awake, broadcast it
    emit awake();

    // handle gui and posted events
    if (qt_is_gui_used ) {
	QApplication::sendPostedEvents();

	// Two loops so that posted events accumulate
	while ( XPending(QPaintDevice::x11AppDisplay()) ) {
	    // also flushes output buffer
	    while ( XPending(QPaintDevice::x11AppDisplay()) ) {
		if ( d->exitloop ) {          // quit between events
		    return FALSE;
		}

		XNextEvent( QPaintDevice::x11AppDisplay(), &event );	// get next event
		nevents++;

		if ( flags & ExcludeUserInput ) {
		    bool block_event = FALSE;
		    switch ( event.type ) {
		    case ButtonPress:
		    case ButtonRelease:
		    case MotionNotify:
		    case XKeyPress:
		    case XKeyRelease:
		    case EnterNotify:
		    case LeaveNotify:
			block_event = TRUE;
			break;

		    default: break;
		    }

		    if ( block_event )
			continue;
		}

		if ( x11ProcessEvent( &event ) == 1 ) {
		    return TRUE;
		}
	    }

	    QApplication::sendPostedEvents();
	}
    }

    if ( d->exitloop ) {			// break immediately
	return FALSE;
    }

    QApplication::sendPostedEvents();

    // Process timers and socket notifiers - the common UNIX stuff

    // return the maximum time we can wait for an event.
    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or X event
    if ( !canWait ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }

    int highest = 0;
    if ( ! ( flags & ExcludeSocketNotifiers ) ) {
	// return the highest fd we can wait for input on
	if ( d->sn_highest >= 0 ) {			// has socket notifier(s)
	    if ( d->sn_vec[0].list && ! d->sn_vec[0].list->isEmpty() )
		d->sn_vec[0].select_fds = d->sn_vec[0].enabled_fds;
	    else
		FD_ZERO( &d->sn_vec[0].select_fds );
	    if ( d->sn_vec[1].list && ! d->sn_vec[1].list->isEmpty() )
		d->sn_vec[1].select_fds = d->sn_vec[1].enabled_fds;
	    if ( d->sn_vec[2].list && ! d->sn_vec[2].list->isEmpty() )
		d->sn_vec[2].select_fds = d->sn_vec[2].enabled_fds;
	} else {
	    FD_ZERO( &d->sn_vec[0].select_fds );
	}

	highest = d->sn_highest;
    }

    if ( qt_is_gui_used ) {
	// select for events on the event socket - only on X11
	FD_SET( d->xfd, &d->sn_vec[0].select_fds );
	highest = QMAX( highest, d->xfd );
    }

    FD_SET( d->thread_pipe[0], &d->sn_vec[0].select_fds );
    highest = QMAX( highest, d->thread_pipe[0] );




    /*
      ### TODO - a cleaner way to do preselect handlers should be invented...
      virtual functions?

      if ( qt_preselect_handler ) {
      QVFuncList::Iterator end = qt_preselect_handler->end();
      for ( QVFuncList::Iterator it = qt_preselect_handler->begin(); it != end; ++it )
      (**it)();
      }
    */




    // unlock the GUI mutex and select.  when we return from this function, there is
    // something for us to do
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->unlock();
#endif

    int nsel;
    nsel = select( highest + 1,
		   &d->sn_vec[0].select_fds,
		   d->sn_vec[1].list ? &d->sn_vec[1].select_fds : 0,
		   d->sn_vec[2].list ? &d->sn_vec[2].select_fds : 0,
		   tm );

    // relock the GUI mutex before processing any pending events
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->lock();
#endif

    if ( nsel == -1 ) {
	if ( errno != EINTR && errno != EAGAIN )
	    perror( "select" );
	return (nevents > 0);
    }

#undef FDCAST

    // some other thread woke us up... consume the data on the thread pipe so that
    // select doesn't immediately return next time
    if ( nsel > 0 && FD_ISSET( d->thread_pipe[0], &d->sn_vec[0].select_fds ) ) {
	char c;
	::read( d->thread_pipe[0], &c, 1 );
    }




    /*
      ### TODO - a cleaner way to do postselect handlers should be invented...
      virtual functions?

      if ( qt_postselect_handler ) {
      QVFuncList::Iterator end = qt_postselect_handler->end();
      for ( QVFuncList::Iterator it = qt_postselect_handler->begin(); it != end; ++it )
      (**it)();
      }
    */




    // activate socket notifiers
    if ( ! ( flags & ExcludeSocketNotifiers ) && nsel > 0 && d->sn_highest >= 0 ) {
	// if select says data is ready on any socket, then set the socket notifier
	// to pending
	int i;
	for ( i=0; i<3; i++ ) {
	    if ( ! d->sn_vec[i].list )
		continue;

	    QPtrList<QSockNot> *list = d->sn_vec[i].list;
	    QSockNot *sn = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, &d->sn_vec[i].select_fds ) )
		    setSocketNotifierPending( sn->obj );
		sn = list->next();
	    }
	}

	nevents += activateSocketNotifiers();
    }

    // activate timers
    nevents += activateTimers();

    // color approx. optimization - only on X11
    qt_reset_color_avail();

    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventLoop::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return ( qGlobalPostedEventsCount() || XPending( QPaintDevice::x11AppDisplay() ) );
}
