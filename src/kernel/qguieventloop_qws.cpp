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
#include "qwscommand_qws.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "qwindowsystem_qws.h"
#include "qptrqueue.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#include <errno.h>

#include "qguieventloop_p.h"
#define d d_func()
#define q q_func()

// from qapplication_qws.cpp
extern QWSDisplay* qt_fbdpy; // QWS `display'

bool QGuiEventLoop::processEvents( ProcessEventsFlags flags )
{
    // process events from the QWS server
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker( QApplication::qt_mutex );
#endif

    // handle gui and posted events
    QApplication::sendPostedEvents();

    while ( qt_fbdpy->eventPending() ) {	// also flushes output buffer
	if ( d->shortcut ) {
	    return FALSE;
	}

	QWSEvent *event = qt_fbdpy->getEvent();	// get next event
	nevents++;

	bool ret = qApp->qwsProcessEvent( event ) == 1;
	delete event;
	if ( ret ) {
	    return TRUE;
	}
    }

    if ( d->shortcut ) {
	return FALSE;
    }

    extern QPtrQueue<QWSCommand> *qt_get_server_queue();
    if ( !qt_get_server_queue()->isEmpty() ) {
	QWSServer::processEventQueue();
    }

    if (QEventLoop::processEvents(flags))
	return true;
    return (nevents > 0);
}

bool QGuiEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return qGlobalPostedEventsCount() || qt_fbdpy->eventPending();
}
