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

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT
#include "qapplication_p.h"

#include "qguieventloop_p.h"
#include "qt_x11_p.h"
#include "qgc_x11.h"
#define QPaintDevice QX11GC // ### fix

#define d d_func()
#define q q_func()

#include <errno.h>

// resolve conflict between QEvent::KeyPress/KeyRelease and X11's
// KeyPress/KeyRelease #defines
enum {
    XKeyPress = KeyPress,
    XKeyRelease = KeyRelease
};
#undef KeyPress
#undef KeyRelease

bool QGuiEventLoop::processEvents( ProcessEventsFlags flags )
{
    int nevents = 0;

    QApplication::sendPostedEvents();

    // Two loops so that posted events accumulate
    while ( XPending( QPaintDevice::x11AppDisplay() ) ) {
	// also flushes output buffer
	while ( XPending( QPaintDevice::x11AppDisplay() ) ) {
	    if ( d->shortcut ) {
		return FALSE;
	    }

	    // process events from the X server
	    XEvent event;
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

		    if (event.xclient.message_type == ATOM(WM_PROTOCOLS) ||
			(Atom) event.xclient.data.l[0] == ATOM(WM_TAKE_FOCUS))
			break;
		    if (event.xclient.message_type == ATOM(_QT_SCROLL_DONE))
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

void QGuiEventLoop::flush()
{
    XFlush( QPaintDevice::x11AppDisplay() );
}
