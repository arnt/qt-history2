/****************************************************************************
**
** Implementation of QEventLoop class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qguieventloop.h"
#include "qapplication.h"
#include "qpaintdevice.h"
#include "qbitarray.h"
#include "qcolor_p.h"
#include "qt_x11_p.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT
#include "qguieventloop_p.h"
#include "qapplication_p.h"
#define d d_func()
#define q q_func()

#include <errno.h>


// resolve the conflict between X11's FocusIn and QEvent::FocusIn
#undef FocusOut
#undef FocusIn

enum {
    XKeyPress = KeyPress,
    XKeyRelease = KeyRelease
};
#undef KeyPress
#undef KeyRelease

bool QGuiEventLoop::processEvents( ProcessEventsFlags flags )
{
    // process events from the X server
    XEvent event;
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker( QApplication::qt_mutex );
#endif

    QApplication::sendPostedEvents();

    // Two loops so that posted events accumulate
    while ( XPending( QPaintDevice::x11AppDisplay() ) ) {
	// also flushes output buffer
	while ( XPending( QPaintDevice::x11AppDisplay() ) ) {
	    if ( d->shortcut ) {
		return FALSE;
	    }

	    XNextEvent( QPaintDevice::x11AppDisplay(), &event );

	    if ( flags & ExcludeUserInput ) {
		switch ( event.type ) {
		case ButtonPress:
		case ButtonRelease:
		case MotionNotify:
		case XKeyPress:
		case XKeyRelease:
		case EnterNotify:
		case LeaveNotify:
		    continue;

		case ClientMessage:
		{
		    // only keep the wm_take_focus and
		    // qt_qt_scrolldone protocols, discard all
		    // other client messages
		    if ( event.xclient.format != 32 )
			continue;

		    if ( event.xclient.message_type == ATOM(Wm_Protocols) ||
			 (Atom) event.xclient.data.l[0] == ATOM(Wm_Take_Focus) )
			break;
		    if ( event.xclient.message_type == ATOM(Qt_Scroll_Done) )
			break;
		}

		default: break;
		}
	    }

	    nevents++;
	    if ( qApp->x11ProcessEvent( &event ) == 1 )
		return TRUE;
	}
    }

    // 0x08 == ExcludeTimers for X11 only
    const uint exclude_all = ExcludeSocketNotifiers | 0x08 | WaitForMore;

    if (nevents > 0 && (flags & exclude_all) == exclude_all) {
	QApplication::sendPostedEvents();
	return TRUE;
    }

    bool retval = (nevents > 0);
    retval |= QEventLoop::processEvents(flags);

    // color approx. optimization - only on X11
    qt_reset_color_avail();

    // return true if we handled events, false otherwise
    return retval;
}

bool QGuiEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return ( qGlobalPostedEventsCount() || XPending( QPaintDevice::x11AppDisplay() ) );
}

void QGuiEventLoop::appStartingUp()
{
    d->xfd = XConnectionNumber( QPaintDevice::x11AppDisplay() );
}

void QGuiEventLoop::appClosingDown()
{
    d->xfd = -1;
}

void QGuiEventLoop::init()
{

}

void QGuiEventLoop::cleanup()
{

}
