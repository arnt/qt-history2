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
#include <private/qinputcontext_p.h>
#define d d_func()
#define q q_func()

#include "qmutex.h"

static LRESULT CALLBACK win_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static HWND qt_create_sn_window();

extern uint qGlobalPostedEventsCount();

static DWORD qt_gui_thread = 0;
// Simpler timers are needed when Qt does not have the event loop,
// such as for plugins.
#ifndef Q_OS_TEMP
extern Q_CORE_EXPORT bool        qt_win_use_simple_timers = true;
#else
extern Q_CORE_EXPORT bool        qt_win_use_simple_timers = false;
#endif
void CALLBACK   qt_simple_timer_func(HWND, UINT, UINT, DWORD);

static TimerVec  *timerVec = 0;
static TimerDict *timerDict = 0;

Q_CORE_EXPORT bool qt_dispatch_timer(uint, MSG *);
Q_CORE_EXPORT bool activateTimer(uint);
Q_CORE_EXPORT void activateZeroTimers();

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


//
// Internal data structure for timers
//

void CALLBACK qt_simple_timer_func(HWND, UINT, UINT idEvent, DWORD)
{
    qt_dispatch_timer(idEvent, 0);
}


// Activate a timer, used by both event-loop based and simple timers.

bool qt_dispatch_timer(uint timerId, MSG *msg)
{
    long res = 0;
    if (!msg || !QCoreApplication::instance() || !QEventLoop::instance()->winEventFilter(msg, &res))
        return activateTimer(timerId);
    return true;
}


//
// Timer activation (called from the event loop when WM_TIMER arrives)
//

bool activateTimer(uint id)                // activate timer
{
    if (!timerVec)                                // should never happen
        return false;
    register TimerInfo *t = timerDict->value(id);
    if (!t)                                        // no such timer id
        return false;
    QTimerEvent e(t->ind + 1);
    QCoreApplication::sendEvent(t->obj, &e);        // send event
    return true;                                // timer event was processed
}

void activateZeroTimers()                // activate full-speed timers
{
    if (!timerVec)
        return;
    uint i=0;
    register TimerInfo *t = 0;
    int n = numZeroTimers;
    while (n--) {
        for (;;) {
            t = timerVec->at(i++);
            if (t && t->zero)
                break;
            else if (i == timerVec->size())                // should not happen
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
    register TimerInfo *t;
    if (!timerVec) {                                // initialize timer data
        timerVec = new TimerVec;
        timerDict = new TimerDict;
    }
    int ind = timerVec->size();
    t = new TimerInfo;                                // create timer entry
    t->ind  = ind;
    t->obj  = obj;

    if (qt_win_use_simple_timers) {
        t->zero = false;
        t->id = SetTimer(0, 0, (uint)interval,
                          (TIMERPROC)qt_simple_timer_func);
    } else {
        t->zero = interval == 0;
        if (t->zero) {                        // add zero timer
            t->id = (uint)50000 + ind;                // unique, high id ##########
            numZeroTimers++;
        } else {
            t->id = SetTimer(0, 0, (uint)interval, 0);
        }
    }
    if (t->id == 0) {
        qSystemWarning("registerTimer: Failed to create a timer");
        delete t;                                // could not set timer
        return 0;
    }
    timerVec->append(t);                        // store in timer vector
    timerDict->insert(t->id, t);                // store in dict
    return ind + 1;                                // return index in vector
}

bool QEventLoop::unregisterTimer(int ind)
{
    if (!timerVec || ind <= 0)
        return false;

    TimerInfo *t = 0;
    for (int i=0; i<timerVec->size(); ++i) {
        if (timerVec->at(i)->ind == ind-1) {
            t = timerVec->at(i);
            break;
        }
    }

    if (!t)
        return false;
    if (t->zero)
        numZeroTimers--;
    else
        KillTimer(0, t->id);
    timerDict->remove(t->id);
    timerVec->removeAll(t);
    delete t;
    return true;
}

bool QEventLoop::unregisterTimers(QObject *obj)
{
    if (!timerVec)
        return false;
    register TimerInfo *t;
    for (int i=0; i<timerVec->size(); i++) {
        t = timerVec->at(i);
        if (t && t->obj == obj) {                // object found
            if (t->zero)
                numZeroTimers--;
            else
                KillTimer(0, t->id);
            timerDict->remove(t->id);
            timerVec->removeAt(i);
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

typedef QHash<int, QSockNot*> QSNDict;

static QSNDict *sn_read          = 0;
static QSNDict *sn_write  = 0;
static QSNDict *sn_except = 0;

static QSNDict**sn_vec[3] = { &sn_read, &sn_write, &sn_except };

uint        qt_sn_msg          = 0;                        // socket notifier message
static HWND sn_win          = 0;                        // win msg via this window


static void sn_cleanup()
{
    DestroyWindow(sn_win);
    sn_win = 0;
    for (int i=0; i<3; i++) {
        for(QSNDict::Iterator it = (*sn_vec[i])->begin(); it != (*sn_vec[i])->end(); ++it)
            delete (*it);
        delete *sn_vec[i];
        *sn_vec[i] = 0;
    }
}

static void sn_init()
{
    if (sn_win)
        return;
    qAddPostRoutine(sn_cleanup);
#ifdef Q_OS_TEMP
    qt_sn_msg = RegisterWindowMessage(L"QtSNEvent");
#else
    qt_sn_msg = RegisterWindowMessageA("QtSNEvent");
#endif
    sn_win = qt_create_sn_window();
    for (int i=0; i<3; i++)
        *sn_vec[i] = new QSNDict;
}

void qt_sn_activate_fd(int sockfd, int type)
{
    QSNDict *dict = *sn_vec[type];
    QSockNot *sn = dict ? dict->value(sockfd) : 0;
    if (sn)
        QCoreApplication::eventLoop()->setSocketNotifierPending(sn->obj);
}

/*****************************************************************************
  QEventLoop Implementation
 *****************************************************************************/

void QEventLoop::init()
{
    qt_gui_thread = GetCurrentThreadId();
}

void QEventLoop::cleanup()
{
    if(timerVec) { //cleanup timers
        register TimerInfo *t;
        for (int i=0; i<timerVec->size(); i++) {                // kill all pending timers
            t = timerVec->at(i);
            if (t && !t->zero)
                KillTimer(0, t->id);
            delete t;
        }
        delete timerDict;
        timerDict = 0;
        delete timerVec;
        timerVec = 0;

        if (qt_win_use_simple_timers) {
            // Dangerous to leave WM_TIMER events in the queue if they have our
            // timerproc (eg. Qt-based DLL plugins may be unloaded)
            MSG msg;
            while (winPeekMessage(&msg, (HWND)-1, WM_TIMER, WM_TIMER, PM_REMOVE))
                continue;
        }
    }
    // Socket notifier cleanup handled by qAddPostRoutine in sn_init();
}

void QEventLoop::registerSocketNotifier(QSocketNotifier *notifier)
{
    int sockfd;
    int type;
    if (notifier == 0 || (sockfd = notifier->socket()) < 0
        || (type = notifier->type()) < 0 || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    }

    QSNDict *dict = *sn_vec[type];

    if (!dict && QCoreApplication::closingDown())
        return; // after sn_cleanup, don't reinitialize.

    QSockNot *sn;
    if (sn_win == 0) {
        sn_init();
        dict = *sn_vec[type];
    }

    sn = new QSockNot;
    sn->obj = notifier;
    sn->fd  = sockfd;
    if (dict->find(sockfd)) {
        static const char *t[] = { "read", "write", "exception" };
        qWarning("QSocketNotifier: Multiple socket notifiers for "
                    "same socket %d and type %s", sockfd, t[type]);
    }
    dict->insert(sockfd, sn);

#ifndef Q_OS_TEMP
    int sn_event = 0;
    if (sn_read && sn_read->find(sockfd))
        sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if (sn_write && sn_write->find(sockfd))
        sn_event |= FD_WRITE | FD_CONNECT;
    if (sn_except && sn_except->find(sockfd))
        sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect(sockfd, sn_win, sn_event ? qt_sn_msg : 0, sn_event);
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
    int sockfd = notifier->socket();
    int type = notifier->type();
    if (sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
        qWarning("QSocketNotifier: Internal error");
        return;
    }

    QSNDict *dict = *sn_vec[type];
    if (!dict)
        return;

    QSockNot *sn = dict->value(sockfd);
    if (!sn)
        return;

    d->sn_pending_list.removeAll(sn);                // remove from activation list

    dict->remove(sockfd);
    delete sn;

#ifndef Q_OS_TEMP // ### This probably needs fixing
    int sn_event = 0;
    if (sn_read && sn_read->find(sockfd))
        sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if (sn_write && sn_write->find(sockfd))
        sn_event |= FD_WRITE | FD_CONNECT;
    if (sn_except && sn_except->find(sockfd))
        sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect(sockfd, sn_win, sn_event ? qt_sn_msg : 0, sn_event);
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

    QSNDict *dict = *sn_vec[type];
    QSockNot *sn   = dict ? (*dict)[sockfd] : 0;
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
            activateZeroTimers();
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

static LRESULT CALLBACK win_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == qt_sn_msg) {        // socket notifier message
        int type = -1;
#ifndef Q_OS_TEMP
        switch (WSAGETSELECTEVENT(lp)) {
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
        if (type >= 0)
            qt_sn_activate_fd(wp, type);
        return 0;
    } else {
        return  DefWindowProc(hwnd, msg, wp, lp);
    }
}

static HWND qt_create_sn_window()
{
    HINSTANCE hi = qWinAppInst();
    WNDCLASSA wc;
    wc.style = 0;
    wc.lpfnWndProc = win_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "QtSocketNotifier_Internal_Widget";
    if (!RegisterClassA(&wc)) {
        qWarning("Failed to register class: %d\n", GetLastError());
        return 0;
    }
    HWND wnd = CreateWindowA(wc.lpszClassName,         // classname
                             wc.lpszClassName,        // window name
                             0,                        // style
                             0, 0, 0, 0,         // geometry
                             0,                        // parent
                             0,                       // menu handle
                             hi,                // application
                             0);                // windows creation data.
    if (!wnd) {
        qWarning("Failed to create socket notifier receiver window: %d\n", GetLastError());
    }
    return wnd;
}
