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

#include "qguieventloop_p.h"
#include "qcoreapplication.h"
#include <private/qeventloop_p.h>
#include "qmutex.h"
#include "qwidget.h"
#include "qinputcontext_p.h"

#define d d_func()
#define q q_func()

void QGuiEventLoop::init()
{

}

void QGuiEventLoop::cleanup()
{

}

extern Q_CORE_EXPORT bool qt_dispatch_timer(uint timerId, MSG *msg);

/*****************************************************************************
  Safe configuration (move,resize,setGeometry) mechanism to avoid
  recursion when processing messages.
 *****************************************************************************/

struct QWinConfigRequest {
    WId         id;                                        // widget to be configured
    int         req;                                        // 0=move, 1=resize, 2=setGeo
    int         x, y, w, h;                                // request parameters
};

static QList<QWinConfigRequest*> *configRequests = 0;

void qWinRequestConfig(WId id, int req, int x, int y, int w, int h)
{
    if (!configRequests)                        // create queue
        configRequests = new QList<QWinConfigRequest*>;
    QWinConfigRequest *r = new QWinConfigRequest;
    r->id = id;                                        // create new request
    r->req = req;
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    configRequests->append(r);                // store request in queue
}

Q_GUI_EXPORT void qWinProcessConfigRequests()                // perform requests in queue
{
    if (!configRequests)
        return;
    QWinConfigRequest *r;
    for (;;) {
        if (configRequests->isEmpty())
            break;
        r = configRequests->takeLast();
        QWidget *w = QWidget::find(r->id);
        if (w) {                                // widget exists
            if (w->testWState(Qt::WState_ConfigPending))
                return;                                // biting our tail
            if (r->req == 0)
                w->move(r->x, r->y);
            else if (r->req == 1)
                w->resize(r->w, r->h);
            else
                w->setGeometry(r->x, r->y, r->w, r->h);
        }
        delete r;
    }
    delete configRequests;
    configRequests = 0;
}

bool QGuiEventLoop::processEvents(ProcessEventsFlags flags)
{
    if (!QEventLoop::processEvents(flags))
        return false;

    if (configRequests)                        // any pending configs?
        qWinProcessConfigRequests();
    QCoreApplication::sendPostedEvents();

    return true;
}

void QGuiEventLoop::winProcessEvent(void *message)
{
    if (d->process_event_handler && d->process_event_handler(message))
        return;

    MSG *msg = (MSG*)message;

    bool handled = false;
    if (msg->message == WM_TIMER) {
        if (qt_dispatch_timer(msg->wParam, msg))
            return;
    } else if (msg->message && (!msg->hwnd || !QWidget::find(msg->hwnd))) {
        // broadcast, or message for a non-Qt widget
	long res = 0;
	handled = winEventFilter(msg, &res);
    }

    if (!handled) {
	QInputContext::TranslateMessage(msg); // translate to WM_CHAR

        QT_WA({
            DispatchMessage(msg);
        } , {
            DispatchMessageA(msg);
        });
    }
}

void QGuiEventLoop::flush()
{

}
