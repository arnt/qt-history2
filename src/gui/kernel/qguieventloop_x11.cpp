/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include <private/qcolor_p.h>

#include "qapplication_p.h"

#include "qguieventloop_p.h"
#include "qt_x11_p.h"
#include "qx11info_x11.h"

#define d d_func()
#define q q_func()

#include <errno.h>

bool QGuiEventLoop::processEvents(ProcessEventsFlags flags)
{
    int nevents = 0;

    QApplication::sendPostedEvents();

    // Two loops so that posted events accumulate
    do {
        while (!d->shortcut &&
	       XEventsQueued(QX11Info::display(), QueuedAlready)) {
            // process events from the X server
            XEvent event;
            XNextEvent(QX11Info::display(), &event);

            if (flags & ExcludeUserInput) {
                switch (event.type) {
                case ButtonPress:
                case ButtonRelease:
                case MotionNotify:
                case XKeyPress:
                case XKeyRelease:
                case EnterNotify:
                case LeaveNotify:
                    continue;

                case ClientMessage: {
                    // only keep the wm_take_focus and
                    // qt_qt_scrolldone protocols, discard all
                    // other client messages
                    if (event.xclient.format != 32)
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
            if (qApp->x11ProcessEvent(&event) == 1)
                return true;
        }
    } while (!d->shortcut && XEventsQueued(QX11Info::display(), QueuedAfterFlush));

    if (d->shortcut)
	return false;

    // 0x08 == ExcludeTimers for X11 only
    const uint exclude_all = ExcludeSocketNotifiers | 0x08 | WaitForMore;
    if (nevents > 0 && (flags & exclude_all) == exclude_all) {
        QApplication::sendPostedEvents();
        return true;
    }

    bool retval = (nevents > 0);
    retval |= QEventLoop::processEvents(flags);

    // return true if we handled events, false otherwise
    return retval;
}

bool QGuiEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return (qGlobalPostedEventsCount() || XPending(QX11Info::display()));
}

void QGuiEventLoop::appStartingUp()
{
    d->xfd = XConnectionNumber(QX11Info::display());
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
    XFlush(QX11Info::display());
}
