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
#include "qwscommand_qws.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "qwindowsystem_qws.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#include <errno.h>

#include "qguieventloop_p.h"
#define d d_func()
#define q q_func()

// from qapplication_qws.cpp
extern QWSDisplay* qt_fbdpy; // QWS `display'

bool QGuiEventLoop::processEvents(ProcessEventsFlags flags)
{
    // process events from the QWS server
    int           nevents = 0;

    // handle gui and posted events
    QApplication::sendPostedEvents();

    while (qt_fbdpy->eventPending()) {        // also flushes output buffer
        if (d->shortcut) {
            return false;
        }

        QWSEvent *event = qt_fbdpy->getEvent();        // get next event
        nevents++;

        bool ret = qApp->qwsProcessEvent(event) == 1;
        delete event;
        if (ret) {
            return true;
        }
    }

    if (d->shortcut) {
        return false;
    }

    extern QList<QWSCommand*> *qt_get_server_queue();
    if (!qt_get_server_queue()->isEmpty()) {
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

void QGuiEventLoop::init()
{

}

void QGuiEventLoop::cleanup()
{

}

void QGuiEventLoop::flush()
{
    if(qApp)
        qApp->sendPostedEvents();
    (void)qt_fbdpy->eventPending(); // flush
}


