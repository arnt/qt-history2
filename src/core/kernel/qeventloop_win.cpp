/****************************************************************************
**
** Implementation of Win32 startup routines and event handling.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qeventloop_p.h"
#include "qeventloop.h"
#include "qcoreapplication.h"
#include "qhash.h"
#include "qsocketnotifier.h"
#include "qt_windows.h"
#define d d_func()
#define q q_func()

#include "qmutex.h"

extern uint qGlobalPostedEventsCount();
extern HINSTANCE qWinAppInst();

static DWORD qt_gui_thread = 0;
// Simpler timers are needed when Qt does not have the event loop,
// such as for plugins.
#ifndef Q_OS_TEMP
extern Q_CORE_EXPORT bool        qt_win_use_simple_timers = true;
#else
extern Q_CORE_EXPORT bool        qt_win_use_simple_timers = false;
#endif
void CALLBACK   qt_simple_timer_func(HWND, UINT, UINT, DWORD);

extern Q_CORE_EXPORT int         numZeroTimers        = 0;                // number of full-speed timers

Q_CORE_EXPORT bool winPeekMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                     UINT wMsgFilterMax, UINT wRemoveMsg)
{
    QT_WA({
        return PeekMessage(msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    } , {
        return PeekMessageA(msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    });
}

Q_CORE_EXPORT bool winPostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    QT_WA({
        return PostMessage(hWnd, msg, wParam, lParam);
    } , {
        return PostMessageA(hWnd, msg, wParam, lParam);
    });
}

Q_CORE_EXPORT bool winGetMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                     UINT wMsgFilterMax)
{
    QT_WA({
        return GetMessage(msg, hWnd, wMsgFilterMin, wMsgFilterMax);
    } , {
        return GetMessageA(msg, hWnd, wMsgFilterMin, wMsgFilterMax);
    });
}


// Activate a timer, used by both event-loop based and simple timers.

Q_CORE_EXPORT bool qt_dispatch_timer(uint timerId, MSG *msg)
{
    long res = 0;
    QEventLoop *eventLoop = QEventLoop::instance();
    if (!msg || !QCoreApplication::instance() || !eventLoop->winEventFilter(msg, &res))
        return eventLoop->d->activateTimer(timerId);
    return true;
}

//
// Internal data structure for timers
//

void CALLBACK qt_simple_timer_func(HWND, UINT, UINT idEvent, DWORD)
{
    qt_dispatch_timer(idEvent, 0);
}

//
// Timer activation (called from the event loop when WM_TIMER arrives)
//

bool QEventLoopPrivate::activateTimer(uint id)    // activate timer
{
    if (timerVec.isEmpty())                                // should never happen
        return false;
    register TimerInfo *t = d->timerDict.value(id);
    if (!t)                                       // no such timer id
        return false;
    QTimerEvent e(t->ind + 1);
    QCoreApplication::sendEvent(t->obj, &e);      // send event
    return true;                                  // timer event was processed
}

void QEventLoopPrivate::activateZeroTimers()                // activate full-speed timers
{
    if (timerVec.isEmpty())
        return;
    uint i=0;
    register TimerInfo *t = 0;
    int n = numZeroTimers;
    while (n--) {
        for (;;) {
            t = d->timerVec.at(i++);
            if (t && t->zero)
                break;
            else if (i == d->timerVec.size())                // should not happen
                return;
        }
        QTimerEvent e(t->ind + 1);
        QCoreApplication::sendEvent(t->obj, &e);
    }
}


//
// Main timer functions for starting and killing timers
//


int QEventLoop::registerTimer(int interval, QObject *obj)
{
    register TimerInfo *t = new TimerInfo;
    t->ind  = d->timerVec.isEmpty() ? 1 : d->timerVec.last()->ind + 1;
    t->obj  = obj;

    if (qt_win_use_simple_timers) {
        t->zero = false;
        t->id = SetTimer(0, 0, (uint)interval,
                         (TIMERPROC)qt_simple_timer_func);
    } else {
        t->zero = interval == 0;
        if (t->zero) {                        // add zero timer
            t->id = (uint)50000 + t->ind;     // unique, high id ##########
            numZeroTimers++;
        } else {
            t->id = SetTimer(0, 0, (uint)interval, 0);
        }
    }
    if (t->id == 0) {
        qSystemWarning("registerTimer: Failed to create a timer");
        delete t;                               // could not set timer
        return 0;
    }
    d->timerVec.append(t);                        // store in timer vector
    d->timerDict.insert(t->id, t);                // store in dict
    return t->ind;                              // return index in vector
}

bool QEventLoop::unregisterTimer(int ind)
{
    if (d->timerVec.isEmpty() || ind <= 0)
        return false;

    TimerInfo *t = 0;
    for (int i=0; i<d->timerVec.size(); ++i) {
        if (d->timerVec.at(i)->ind == ind) {
            t = d->timerVec.at(i);
            break;
        }
    }

    if (!t)
        return false;
    if (t->zero)
        numZeroTimers--;
    else
        KillTimer(0, t->id);
    d->timerDict.remove(t->id);
    d->timerVec.removeAll(t);
    delete t;
    return true;
}

bool QEventLoop::unregisterTimers(QObject *obj)
{
    if (d->timerVec.isEmpty())
        return false;
    register TimerInfo *t;
    for (int i=0; i<d->timerVec.size(); i++) {
        t = d->timerVec.at(i);
        if (t && t->obj == obj) {                // object found
            if (t->zero)
                numZeroTimers--;
            else
                KillTimer(0, t->id);
            d->timerDict.remove(t->id);
            d->timerVec.removeAt(i);
            delete t;
        }
    }
    return true;
}

/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

bool qt_dispatch_socketnotifier(MSG *msg)
{
    int type = -1;
#ifndef Q_OS_TEMP
    switch (WSAGETSELECTEVENT(msg->lParam)) {
    case FD_READ:
    case FD_CLOSE:
    case FD_ACCEPT:
        type = 0;
        break;
    case FD_WRITE:
    case FD_CONNECT:
        type = 1;
        break;
    case FD_OOB:
        type = 2;
        break;
    }
#endif
    if (type >= 0) {

#ifdef GWLP_USERDATA
        QEventLoop *eventLoop = (QEventLoop*)GetWindowLongPtrA(msg->hwnd, GWLP_USERDATA);
#else
        QEventLoop *eventLoop = (QEventLoop*)GetWindowLongA(msg->hwnd, GWL_USERDATA);
#endif
        if (eventLoop) {
            QSNDict *sn_vec[3] = { &eventLoop->d->sn_read, &eventLoop->d->sn_write, &eventLoop->d->sn_except };
            QSNDict *dict = sn_vec[type];

            QSockNot *sn = dict ? dict->value(msg->wParam) : 0;
            if (sn) {
                if (eventLoop)
                    eventLoop->setSocketNotifierPending(sn->obj);
            }
        }
    }

    return true;
}

static LRESULT CALLBACK qt_sn_win_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_USER) {        // socket notifier message
        MSG msg;
        msg.hwnd = hwnd;
        msg.message = message;
        msg.wParam = wp;
        msg.lParam = lp;
        qt_dispatch_socketnotifier(&msg);
        return 0;
    } else {
        return  DefWindowProc(hwnd, message, wp, lp);
    }
}

static HWND qt_create_sn_window(QEventLoop *eventLoop)
{
    HINSTANCE hi = qWinAppInst();
    WNDCLASSA wc;
    wc.style = 0;
    wc.lpfnWndProc = qt_sn_win_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "QtSocketNotifier_Internal_Widget";
    RegisterClassA(&wc);

    HWND wnd = CreateWindowA(wc.lpszClassName,  // classname
                             wc.lpszClassName,  // window name
                             0,                 // style
                             0, 0, 0, 0,        // geometry
                             0,                 // parent
                             0,                 // menu handle
                             hi,                // application
                             0);                // windows creation data.

#ifdef GWLP_USERDATA
    SetWindowLongPtrA(wnd, GWLP_USERDATA, (LONG_PTR)eventLoop);
#else
    SetWindowLongA(wnd, GWL_USERDATA, (LONG)eventLoop);
#endif

    if (!wnd) {
        qWarning("Failed to create socket notifier receiver window: %d\n", GetLastError());
    }
    return wnd;
}

/*****************************************************************************
  QEventLoop Implementation
 *****************************************************************************/

void QEventLoop::init()
{
    qt_gui_thread = GetCurrentThreadId();
    d->sn_win = 0;
}

void QEventLoop::cleanup()
{
    if(!d->timerVec.isEmpty()) { //cleanup timers
        register TimerInfo *t;
        for (int i=0; i<d->timerVec.size(); i++) {                // kill all pending timers
            t = d->timerVec.at(i);
            if (t && !t->zero)
                KillTimer(0, t->id);
            delete t;
        }

        if (qt_win_use_simple_timers) {
            // Dangerous to leave WM_TIMER events in the queue if they have our
            // timerproc (eg. Qt-based DLL plugins may be unloaded)
            MSG msg;
            while (winPeekMessage(&msg, (HWND)-1, WM_TIMER, WM_TIMER, PM_REMOVE))
                continue;
        }
    }

    if (d->sn_win)
        DestroyWindow(d->sn_win);
    d->sn_win = 0;

    QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
    for (int i=0; i<3; i++) {
        QSNDict *snDict= sn_vec[i];
        for(QSNDict::Iterator it = snDict->begin(); it != snDict->end(); ++it)
            delete (*it);
    }    
}

void QEventLoop::registerSocketNotifier(QSocketNotifier *notifier)
{
    if (notifier == 0 || notifier->socket() < 0 || notifier->type() < 0 || notifier->type() > 2) {
        qWarning("SocketNotifier: Internal error");
        return;
    }

    int socket = notifier->socket();
    int type = notifier->type();

    QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
    QSNDict *dict = sn_vec[type];

    if (QCoreApplication::closingDown()) // ### d->exitloop?
        return; // after sn_cleanup, don't reinitialize.

    if (!d->sn_win)
        d->sn_win = qt_create_sn_window(this);

    if (dict->contains(socket)) {
        const char *t[] = { "read", "write", "exception" };
        qWarning("QSocketNotifier: Multiple socket notifiers for "
                    "same socket %d and type %s", socket, t[type]);
    }

    QSockNot *sn = new QSockNot;
    sn->obj = notifier;
    sn->fd  = socket;
    dict->insert(sn->fd, sn);

#ifndef Q_OS_TEMP
    int sn_event = 0;
    if (d->sn_read.contains(socket))
        sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if (d->sn_write.contains(socket))
        sn_event |= FD_WRITE | FD_CONNECT;
    if (d->sn_except.contains(socket))
        sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect(socket, d->sn_win, sn_event ? WM_USER : 0, sn_event);
#else
/*
    fd_set        rd,wt,ex;
    FD_ZERO(&rd);
    FD_ZERO(&wt);
    FD_ZERO(&ex);
    if (sn_read && sn_read->find(sockfd))
        FD_SET(sockfd, &rd);
    if (sn_write && sn_write->find(sockfd))
        FD_SET(sockfd, &wt);
    if (sn_except && sn_except->find(sockfd))
        FD_SET(sockfd, &ex);
    select(1, &rd, &wt, &ex, NULL);
*/
#endif
}

void QEventLoop::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    if (notifier == 0 || notifier->socket() < 0 || notifier->type() < 0 || notifier->type() > 2) {
        qWarning("SocketNotifier: Internal error");
        return;
    }

    int socket = notifier->socket();
    int type = notifier->type();

    QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
    QSNDict *dict = sn_vec[type];

    QSockNot *sn = dict->value(socket);
    if (!sn)
        return;

    d->sn_pending_list.removeAll(sn);                // remove from activation list

    dict->remove(socket);
    delete sn;

#ifndef Q_OS_TEMP // ### This probably needs fixing
    int sn_event = 0;
    if (d->sn_read.contains(socket))
        sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if (d->sn_write.contains(socket))
        sn_event |= FD_WRITE | FD_CONNECT;
    if (d->sn_except.contains(socket))
        sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect(socket, d->sn_win, sn_event ? WM_USER : 0, sn_event);
#else
    fd_set        rd,wt,ex;
    FD_ZERO(&rd);
    FD_ZERO(&wt);
    FD_ZERO(&ex);
    if (sn_read && sn_read->find(sockfd))
        FD_SET(sockfd, &rd);
    if (sn_write && sn_write->find(sockfd))
        FD_SET(sockfd, &wt);
    if (sn_except && sn_except->find(sockfd))
        FD_SET(sockfd, &ex);
    select(1, &rd, &wt, &ex, NULL);
#endif
}

void QEventLoop::setSocketNotifierPending(QSocketNotifier *notifier)
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if (sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
        qWarning("QSocketNotifier: Internal error");
        return;
    }

    QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
    QSNDict *dict = sn_vec[type];
    QSockNot *sn   = dict->value(sockfd);
    if (!sn)
        return;

    if (d->sn_pending_list.indexOf(sn) >= 0)
        return;
    d->sn_pending_list.append(sn);
}

bool QEventLoop::hasPendingEvents() const
{
    MSG msg;
    return qGlobalPostedEventsCount() || winPeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
}

/*!
    \internal

    Process the Windows event \a message.
*/
void QEventLoop::winProcessEvent(void *message)
{
    if (d->process_event_handler && d->process_event_handler(message))
        return;

    MSG *msg = (MSG*)message;

    if (msg->message == WM_TIMER) {
        if (qt_dispatch_timer(msg->wParam, msg))
            return;
    }

    QT_WA({
        DispatchMessage(msg);
    } , {
        DispatchMessageA(msg);
    });
}

/*!
    \internal

    Returns true if there is an event filter for the given \a message
    and this \a message is handled by that filter; otherwise returns
    false. The result of the event filtering is put in \a result if \a
    result is not 0.
*/
bool QEventLoop::winEventFilter(void *message, long *result)
{
    if (d->event_filter && d->event_filter(message, result))
        return true;

#ifdef QT_COMPAT
    if (QCoreApplication::instance()->winEventFilter((MSG*)message))
        return true;
#endif

    return false;
}

bool QEventLoop::processEvents(ProcessEventsFlags flags)
{
    MSG         msg;

    emit awake();

    QCoreApplication::sendPostedEvents();

    bool shortcut = d->exitloop || d->quitnow;
    bool canWait = d->exitloop || d->quitnow ? false : (flags & WaitForMore);

    if (flags & ExcludeUserInput) {
        // purge all userinput messages from eventloop
        while (winPeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
            ;
        while (winPeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
            ;
        while (winPeekMessage(&msg, 0, WM_MOUSEWHEEL, WM_MOUSEWHEEL, PM_REMOVE))
            ;
        // ### tablet?

        // now that we have eaten all userinput we shouldn't wait for the next one...
        canWait = false;
    }

    // activate all full-speed timers until there is a message (timers have low priority)
    bool message = false;
    if (numZeroTimers) {
        while (numZeroTimers && !(message=winPeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))) {
            d->activateZeroTimers();
        }
    }

    message = winPeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    if (!message && !canWait) // still no message, and shouldn't block
        return false;

    // process all messages, unless userinput is blocked, then we process only one
    if (flags & ExcludeUserInput) {
        winProcessEvent(&msg);
        shortcut = d->exitloop || d->quitnow;
    } else {
        while (message && !shortcut) {
            winProcessEvent(&msg);
            message = winPeekMessage(&msg, 0, 0, 0, PM_REMOVE);
            shortcut = d->exitloop || d->quitnow;
        }
    }

    // don't wait if there are pending socket notifiers
    canWait = d->sn_pending_list.isEmpty() && canWait;

    shortcut = d->exitloop || d->quitnow;

    // wait for next message if allowed to block
    if (canWait && !shortcut) {
        emit aboutToBlock();
        if (!winGetMessage(&msg, 0, 0, 0)) {
            exit(0);
            return false;
        }
        winProcessEvent(&msg);
    }

    if (!(flags & ExcludeSocketNotifiers))
        activateSocketNotifiers();

    QCoreApplication::sendPostedEvents();

    return true;
}

void QEventLoop::wakeUp()
{
    if (GetCurrentThreadId() != qt_gui_thread)
        QT_WA({
            PostThreadMessageW(qt_gui_thread, WM_NULL, 0, 0);
        } , {
            PostThreadMessageA(qt_gui_thread, WM_NULL, 0, 0);
        });
}

int QEventLoop::timeToWait() const
{
    return -1;
}

int QEventLoop::activateTimers()
{
    return 0;
}

int QEventLoop::activateSocketNotifiers()
{
    if (d->sn_pending_list.isEmpty())
        return 0; // nothing to do

    int n_act = 0;
    QEvent event(QEvent::SockAct);
    while (!d->sn_pending_list.isEmpty()) {
        QSockNot *sn = d->sn_pending_list.takeFirst();
        QCoreApplication::sendEvent(sn->obj, &event);
        n_act++;
    }

    return n_act;
}
