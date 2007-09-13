/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwseventdispatcher_glib_p.h"

#include "qapplication.h"






#include "qplatformdefs.h"
#include "qapplication.h"
#include "private/qwscommand_qws_p.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "qwindowsystem_qws.h"


#include <glib.h>

QT_BEGIN_NAMESPACE

// from qapplication_qws.cpp
extern QWSDisplay* qt_fbdpy; // QWS `display'

//from qwindowsystem_qws.cpp
extern QList<QWSCommand*> *qt_get_server_queue();

struct GQWSEventSource
{
    GSource source;
    QEventLoop::ProcessEventsFlags flags;
    QWSEventDispatcherGlib *q;
    QWSEventDispatcherGlibPrivate *d;
};

class QWSEventDispatcherGlibPrivate : public QEventDispatcherGlibPrivate
{
    Q_DECLARE_PUBLIC(QWSEventDispatcherGlib)

public:
    QWSEventDispatcherGlibPrivate();
    GQWSEventSource *qwsEventSource;
    QList<QWSEvent*> queuedUserInputEvents;
};

static gboolean qwsEventSourcePrepare(GSource *s, gint *timeout)
{
    if (timeout)
        *timeout = -1;
    GQWSEventSource *source = reinterpret_cast<GQWSEventSource *>(s);
    return qt_fbdpy->eventPending() || !source->d->queuedUserInputEvents.isEmpty()
        || !qt_get_server_queue()->isEmpty() ;
}

static gboolean qwsEventSourceCheck(GSource *s)
{
    GQWSEventSource *source = reinterpret_cast<GQWSEventSource *>(s);
    return qt_fbdpy->eventPending() || !source->d->queuedUserInputEvents.isEmpty()
        || !qt_get_server_queue()->isEmpty() ;
}

static gboolean qwsEventSourceDispatch(GSource *s, GSourceFunc callback, gpointer user_data)
{
    GQWSEventSource *source = reinterpret_cast<GQWSEventSource *>(s);

    //??? ulong marker = XNextRequest(X11->display);
    do {
        QWSEvent *event;
        if (!(source->flags & QEventLoop::ExcludeUserInputEvents)
            && !source->d->queuedUserInputEvents.isEmpty()) {
            // process a pending user input event
            event = source->d->queuedUserInputEvents.takeFirst();
        } else if (qt_fbdpy->eventPending()) {
            event = qt_fbdpy->getEvent();

            if (source->flags & QEventLoop::ExcludeUserInputEvents) {
                // queue user input events

                if (event->type == QWSEvent::Mouse || event->type == QWSEvent::Key) {
                    source->d->queuedUserInputEvents.append(event);
                    continue;
                }
            }
        } else {
            // no event to process
            break;
        }

        // send through event filter
        if (source->q->filterEvent(event)) {
            delete event;
            continue;
        }

        bool ret = qApp->qwsProcessEvent(event) == 1;
        delete event;
        if (ret) {
            return true;
        }

    } while (qt_fbdpy->eventPending());

    if (callback)
        callback(user_data);
    return true;
}

static GSourceFuncs qwsEventSourceFuncs = {
    qwsEventSourcePrepare,
    qwsEventSourceCheck,
    qwsEventSourceDispatch,
    NULL,
    NULL,
    NULL
};

QWSEventDispatcherGlibPrivate::QWSEventDispatcherGlibPrivate()
{
    qwsEventSource = reinterpret_cast<GQWSEventSource *>(g_source_new(&qwsEventSourceFuncs,
                                                                      sizeof(GQWSEventSource)));
    g_source_set_can_recurse(&qwsEventSource->source, true);

    qwsEventSource->flags = QEventLoop::AllEvents;
    qwsEventSource->q = 0;
    qwsEventSource->d = 0;

    g_source_attach(&qwsEventSource->source, mainContext);
}

QWSEventDispatcherGlib::QWSEventDispatcherGlib(QObject *parent)
    : QEventDispatcherGlib(*new QWSEventDispatcherGlibPrivate, parent)
{
}

QWSEventDispatcherGlib::~QWSEventDispatcherGlib()
{
    Q_D(QWSEventDispatcherGlib);

    g_source_destroy(&d->qwsEventSource->source);
    d->qwsEventSource = 0;
}

bool QWSEventDispatcherGlib::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QWSEventDispatcherGlib);
    QEventLoop::ProcessEventsFlags saved_flags = d->qwsEventSource->flags;
    d->qwsEventSource->flags = flags;
    bool returnValue = QEventDispatcherGlib::processEvents(flags);
    d->qwsEventSource->flags = saved_flags;
    return returnValue;
}

void QWSEventDispatcherGlib::startingUp()
{
     Q_D(QWSEventDispatcherGlib);
     d->qwsEventSource->q = this;
     d->qwsEventSource->d = d;
}

QT_END_NAMESPACE
