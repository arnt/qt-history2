#include "qeventdispatcher_win.h"

#include "qhash.h"
#include "qsocketnotifier.h"

#include "qabstracteventdispatcher_p.h"
#include <private/qthread_p.h>

struct QSockNot {
    QSocketNotifier *obj;
    int fd;
};
typedef QHash<int, QSockNot *> QSNDict;

struct TimerInfo {                                // internal timer info
    uint     ind;                                // - Qt timer identifier - 1
    uint     id;                                // - Windows timer identifier
    bool     zero;                                // - zero timing
    QObject *obj;                                // - object to receive events
};
typedef QList<TimerInfo*>  TimerVec;                // vector of TimerInfo structs
typedef QHash<int,TimerInfo*> TimerDict;                // fast dict of timers

class QEventDispatcherWin32Private : public QAbstractEventDispatcherPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherWin32)
public:
    QEventDispatcherWin32Private();

    DWORD threadId;

    int numZeroTimers;
    void activateZeroTimers();
    bool activateTimer(uint id);
    TimerVec timerVec;
    TimerDict timerDict;

    HWND sn_win;

    QList<QSockNot*> sn_pending_list;
    QSNDict sn_read;
    QSNDict sn_write;
    QSNDict sn_except;
    int activateSocketNotifiers();
    void setSocketNotifierPending(QSocketNotifier *notifier);
};

QEventDispatcherWin32Private::QEventDispatcherWin32Private()
    : threadId(GetCurrentThreadId()), numZeroTimers(0), sn_win(0)
{
}

bool QEventDispatcherWin32Private::activateTimer(uint id)    // activate timer
{
    if (timerVec.isEmpty())                                // should never happen
        return false;
    register TimerInfo *t = timerDict.value(id);
    if (!t)                                       // no such timer id
        return false;
    QTimerEvent e(t->ind);
    QCoreApplication::sendEvent(t->obj, &e);      // send event
    return true;                                  // timer event was processed
}

void QEventDispatcherWin32Private::activateZeroTimers()                // activate full-speed timers
{
    if (timerVec.isEmpty())
        return;
    uint i=0;
    register TimerInfo *t = 0;
    int n = numZeroTimers;
    while (n--) {
        for (;;) {
            t = timerVec.at(i++);
            if (t && t->zero)
                break;
            else if (i == timerVec.size())                // should not happen
                return;
        }
        QTimerEvent e(t->ind);
        QCoreApplication::sendEvent(t->obj, &e);
    }
}

void QEventDispatcherWin32Private::setSocketNotifierPending(QSocketNotifier *notifier)
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if (sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
        qWarning("QSocketNotifier: Internal error");
        return;
    }
    QSNDict *sn_vec[3] = { &sn_read, &sn_write, &sn_except };
    QSNDict *dict = sn_vec[type];
    QSockNot *sn   = dict->value(sockfd);
    if (!sn)
        return;
    if (sn_pending_list.indexOf(sn) >= 0)
        return;
    sn_pending_list.append(sn);
}

int QEventDispatcherWin32Private::activateSocketNotifiers()
{
    if (sn_pending_list.isEmpty())
        return 0; // nothing to do
    int n_act = 0;
    QEvent event(QEvent::SockAct);
    while (!sn_pending_list.isEmpty()) {
        QSockNot *sn = sn_pending_list.takeFirst();
        QCoreApplication::sendEvent(sn->obj, &event);
        n_act++;
    }
    return n_act;
}

Q_CORE_EXPORT bool winPeekMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                     UINT wMsgFilterMax, UINT wRemoveMsg)
{
    QT_WA({ return PeekMessage(msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg); } ,
          { return PeekMessageA(msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg); });
}

Q_CORE_EXPORT bool winPostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    QT_WA({ return PostMessage(hWnd, msg, wParam, lParam); } , 
          { return PostMessageA(hWnd, msg, wParam, lParam); });
}

Q_CORE_EXPORT bool winGetMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                     UINT wMsgFilterMax)
{
    QT_WA({ return GetMessage(msg, hWnd, wMsgFilterMin, wMsgFilterMax); } ,
          { return GetMessageA(msg, hWnd, wMsgFilterMin, wMsgFilterMax); });
}

// Simpler timers are needed when Qt does not have the event loop,
// such as for plugins.
#ifndef Q_OS_TEMP
extern Q_CORE_EXPORT bool        qt_win_use_simple_timers = true;
#else
extern Q_CORE_EXPORT bool        qt_win_use_simple_timers = false;
#endif
void CALLBACK   qt_simple_timer_func(HWND, UINT, UINT, DWORD);

Q_CORE_EXPORT bool qt_dispatch_timer(uint timerId, MSG *msg)
{
    long res = 0;
    QEventDispatcherWin32 *eventDispatcher =
        qt_cast<QEventDispatcherWin32 *>(QAbstractEventDispatcher::instance());
    if (!msg || !QCoreApplication::instance() || !eventDispatcher->winEventFilter(msg, &res))
        return eventDispatcher->d_func()->activateTimer(timerId);
    return true;
}

void CALLBACK qt_simple_timer_func(HWND, UINT, UINT idEvent, DWORD)
{
    qt_dispatch_timer(idEvent, 0);
}

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
        QEventDispatcherWin32 *eventDispatcher =
            (QEventDispatcherWin32 *) GetWindowLongPtrA(msg->hwnd, GWLP_USERDATA);
#else
        QEventDispatcherWin32 *eventDispatcher =
            (QEventDispatcherWin32 *) GetWindowLongA(msg->hwnd, GWL_USERDATA);
#endif
        if (eventDispatcher) {
            QEventDispatcherWin32Private *d = eventDispatcher->d_func();
            QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
            QSNDict *dict = sn_vec[type];

            QSockNot *sn = dict ? dict->value(msg->wParam) : 0;
            if (sn) {
                if (eventDispatcher)
                    d->setSocketNotifierPending(sn->obj);
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

static HWND qt_create_sn_window(QEventDispatcherWin32 *eventDispatcher)
{
    extern HINSTANCE qWinAppInst();
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
    SetWindowLongPtrA(wnd, GWLP_USERDATA, (LONG_PTR)eventDispatcher);
#else
    SetWindowLongA(wnd, GWL_USERDATA, (LONG)eventDispatcher);
#endif

    if (!wnd) {
        qWarning("Failed to create socket notifier receiver window: %d\n", GetLastError());
    }
    return wnd;
}

QEventDispatcherWin32::QEventDispatcherWin32(QObject *parent)
    : QAbstractEventDispatcher(*new QEventDispatcherWin32Private, parent)
{
}

QEventDispatcherWin32::~QEventDispatcherWin32()
{
}

bool QEventDispatcherWin32::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherWin32);

    MSG msg;

    emit awake();

    QCoreApplication::sendPostedEvents();

    bool shortcut = false;
#if 0
    bool shortcut = d->exitloop || d->quitnow;
#endif

    QThreadData *data = QThreadData::current();
    bool canWait = (data->postEventList.size() == 0
#if 0
                    && !d->exitloop
                    && !d->quitnow
#endif
                    && (flags & QEventLoop::WaitForMoreEvents));

    if (flags & QEventLoop::ExcludeUserInputEvents) {
        // purge all userinput messages from eventDispatcher
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
    if (d->numZeroTimers) {
        while (d->numZeroTimers && !(message=winPeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))) {
            d->activateZeroTimers();
        }
    }

    message = winPeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    if (!message && !canWait) // still no message, and shouldn't block
        return false;

    // process all messages, unless userinput is blocked, then we process only one
    if (flags & QEventLoop::ExcludeUserInputEvents) {
        winProcessEvent(&msg);
#if 0
        shortcut = d->exitloop || d->quitnow;
#endif
    } else {
        // ### Don't send two timers in a row, since it could potentially block
        // other events from being fired.
        bool lastWasTimer = false;
        while (message && !shortcut && !lastWasTimer) {
            winProcessEvent(&msg);
            if (msg.message == WM_TIMER)
                lastWasTimer = true;
            message = winPeekMessage(&msg, 0, 0, 0, PM_REMOVE);
#if 0
            shortcut = d->exitloop || d->quitnow;
#endif
        }

        if (lastWasTimer)
            winProcessEvent(&msg);
    }

    // don't wait if there are pending socket notifiers
    canWait = d->sn_pending_list.isEmpty() && canWait;

#if 0
    shortcut = d->exitloop || d->quitnow;
#endif

    // wait for next message if allowed to block
    if (canWait && !shortcut) {
        emit aboutToBlock();
        if (!winGetMessage(&msg, 0, 0, 0)) {
            exit(0);
            return false;
        }
        winProcessEvent(&msg); 
    }

    if (!(flags & QEventLoop::ExcludeSocketNotifiers))
        d->activateSocketNotifiers();

    QCoreApplication::sendPostedEvents();

    return true;
}

bool QEventDispatcherWin32::hasPendingEvents()
{
    MSG msg;
    extern uint qGlobalPostedEventsCount();
    return qGlobalPostedEventsCount() || winPeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
}

void QEventDispatcherWin32::registerSocketNotifier(QSocketNotifier *notifier)
{
    if (notifier == 0 || notifier->socket() < 0 || notifier->type() < 0 || notifier->type() > 2) {
        qWarning("SocketNotifier: Internal error");
        return;
    }

    int socket = notifier->socket();
    int type = notifier->type();

    Q_D(QEventDispatcherWin32);
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

void QEventDispatcherWin32::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    if (notifier == 0 || notifier->socket() < 0 || notifier->type() < 0 || notifier->type() > 2) {
        qWarning("SocketNotifier: Internal error");
        return;
    }

    int socket = notifier->socket();
    int type = notifier->type();

    Q_D(QEventDispatcherWin32);
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

int QEventDispatcherWin32::registerTimer(int interval, QObject *object)
{
    register TimerInfo *t = new TimerInfo;
    Q_D(QEventDispatcherWin32);
    t->ind  = d->timerVec.isEmpty() ? 1 : d->timerVec.last()->ind + 1;
    t->obj  = object;

    if (qt_win_use_simple_timers) {
        t->zero = false;
        t->id = SetTimer(0, 0, (uint)interval,
                         (TIMERPROC)qt_simple_timer_func);
    } else {
        t->zero = interval == 0;
        if (t->zero) {                        // add zero timer
            t->id = (uint)50000 + t->ind;     // unique, high id ##########
            d->numZeroTimers++;
        } else {
            t->id = SetTimer(0, 0, (uint)interval, 0);
        }
    }
    if (t->id == 0) {
        qErrnoWarning("QEventDispatcherWin32::registerTimer: Failed to create a timer");
        delete t;                               // could not set timer
        return 0;
    }
    d->timerVec.append(t);                        // store in timer vector
    d->timerDict.insert(t->id, t);                // store in dict
    return t->ind;                              // return index in vector
}

bool QEventDispatcherWin32::unregisterTimer(int timerId)
{
    Q_D(QEventDispatcherWin32);
    if (d->timerVec.isEmpty() || timerId <= 0)
        return false;

    TimerInfo *t = 0;
    for (int i=0; i<d->timerVec.size(); ++i) {
        if (d->timerVec.at(i)->ind == timerId) {
            t = d->timerVec.at(i);
            break;
        }
    }

    if (!t)
        return false;
    if (t->zero)
        d->numZeroTimers--;
    else
        KillTimer(0, t->id);
    d->timerDict.remove(t->id);
    d->timerVec.removeAll(t);
    delete t;
    return true;
}

bool QEventDispatcherWin32::unregisterTimers(QObject *object)
{
    Q_D(QEventDispatcherWin32);
    if (d->timerVec.isEmpty())
        return false;
    register TimerInfo *t;
    for (int i=0; i<d->timerVec.size(); i++) {
        t = d->timerVec.at(i);
        if (t && t->obj == object) {                // object found
            if (t->zero)
                d->numZeroTimers--;
            else
                KillTimer(0, t->id);
            d->timerDict.remove(t->id);
            d->timerVec.removeAt(i);
            delete t;
        }
    }
    return true;
}

void QEventDispatcherWin32::wakeUp()
{
    Q_D(QEventDispatcherWin32);
    QT_WA({ PostThreadMessageW(d->threadId, WM_NULL, 0, 0); } ,
          { PostThreadMessageA(d->threadId, WM_NULL, 0, 0); });
}

void QEventDispatcherWin32::flush()
{ }

/*!
    \internal

    Process the Windows event \a message.
*/
void QEventDispatcherWin32::winProcessEvent(void *message)
{
    Q_D(QEventDispatcherWin32);
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
bool QEventDispatcherWin32::winEventFilter(void *message, long *result)
{
    Q_D(QEventDispatcherWin32);
    if (d->event_filter && d->event_filter(message, result))
        return true;

#ifdef QT_COMPAT
    if (QCoreApplication::instance()->winEventFilter((MSG*)message, result))
        return true;
#endif

    return false;
}



















#if 0


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

#include "qeventdispatcher_p.h"
#include "qeventdispatcher.h"
#include "qcoreapplication.h"
#include "qhash.h"
#include "qsocketnotifier.h"
#include "qt_windows.h"
#include <private/qthread_p.h>
#define d d_func()
#define q q_func()




extern Q_CORE_EXPORT int         numZeroTimers        = 0;                // number of full-speed timers



// Activate a timer, used by both event-loop based and simple timers.


//
// Internal data structure for timers
//

//
// Timer activation (called from the event loop when WM_TIMER arrives)
//

/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/




/*****************************************************************************
  QEventDispatcherWin32 Implementation
 *****************************************************************************/

void QEventDispatcherWin32::init()
{
    d->threadId = GetCurrentThreadId();
    d->sn_win = 0;

#ifdef Q_WIN_EVENT_NOTIFIER
    d->wen_handle_list.reserve(MAXIMUM_WAIT_OBJECTS - 1);
#endif
}

void QEventDispatcherWin32::cleanup()
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


bool QEventDispatcherWin32::hasPendingEvents() const
{
}



#ifdef Q_WIN_EVENT_NOTIFIER

bool QEventDispatcherWin32::processEvents(ProcessEventsFlags flags)
{
    MSG         msg;

    emit awake();

    QCoreApplication::sendPostedEvents();

    bool shortcut = d->exitloop || d->quitnow;

    QThreadData *data = QThreadData::current();
    bool canWait = (data->postEventList.size() == 0
                    && !d->exitloop
                    && !d->quitnow
                    && (flags & WaitForMore));

    if (flags & ExcludeUserInput) {
        // purge all userinput messages from eventDispatcher
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
        // ### Don't send two timers in a row, since it could potentially block
        // other events from being fired.
        bool lastWasTimer = false;
        while (message && !shortcut && !lastWasTimer) {
            winProcessEvent(&msg);
            if (msg.message == WM_TIMER)
                lastWasTimer = true;
            message = winPeekMessage(&msg, 0, 0, 0, PM_REMOVE);
            shortcut = d->exitloop || d->quitnow;
        }

        if (lastWasTimer)
            winProcessEvent(&msg);
    }

    // don't wait if there are pending socket notifiers
    canWait = d->sn_pending_list.isEmpty() && canWait;

    shortcut = d->exitloop || d->quitnow;

    // wait for next message if allowed to block
    if (canWait && !shortcut) {
        emit aboutToBlock();
        
        //### The message sould be empty at this point. This will not return untill
        // a new message comes. This point is not reached so offten so maybe need 
        // to do a zero timeout version out side of this "canWait" block 
        
        // has enough reserved space so this is cheap
        d->wen_handle_list.resize(d->wen_list.count());
        for (int i=0; i<d->wen_list.count(); i++)
            d->wen_handle_list[i] = d->wen_list.at(i)->handle();
        
        DWORD ret = MsgWaitForMultipleObjectsEx(d->wen_handle_list.count(), (LPHANDLE)d->wen_handle_list.data(), INFINITE, QS_ALLEVENTS, MWMO_ALERTABLE);

        if (ret == WAIT_OBJECT_0 + d->wen_handle_list.count() && winPeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            //### instead of taking this message we should jump out and let the code above
            // take care of the message incase there are some ExcludeUserInput flags
            // or else remove input events from the dwWakeMask.
            if (msg.message == WM_QUIT) {
                exit(0);
                return false;
            }
            winProcessEvent(&msg);
        } else if (ret - WAIT_OBJECT_0 >= 0 && ret - WAIT_OBJECT_0 < d->wen_handle_list.count()) {
            activateWinEventNotifier(d->wen_list.at(ret - WAIT_OBJECT_0));
        }
    }

    if (!(flags & ExcludeSocketNotifiers))
        activateSocketNotifiers();

    QCoreApplication::sendPostedEvents();

    return true;
}
#endif



#ifdef Q_WIN_EVENT_NOTIFIER

void QEventDispatcherWin32::registerWinEventNotifier(QWinEventNotifier * notifier)
{
    //### check that it is not in the list already
    if (d->wen_list.contains(notifier))
        qWarning("QWinEventNotifier: All ready registered");

    if (d->wen_list.count() >= MAXIMUM_WAIT_OBJECTS - 1)
        qWarning("QWinEventNotifier: Can not have more then %d enabled at one time", MAXIMUM_WAIT_OBJECTS - 1);
    
    d->wen_list.append(notifier);
}

void QEventDispatcherWin32::unregisterWinEventNotifier(QWinEventNotifier * notifier)
{
    int i = d->wen_list.indexOf(notifier);
    if (i != -1)
        d->wen_list.takeAt(i);
}

// will go throw the entire list an activate any that are set
int QEventDispatcherWin32::activateWinEventNotifiers()
{
    int n_act = 0;
    for (int i=0; i<d->wen_list.count(); i++) {
        QEvent event(QEvent::SockAct); // if this was out of the for loop it would need to get reset each time
        if (WaitForSingleObject((HANDLE)d->wen_list.at(i)->handle(), 0) == WAIT_OBJECT_0) {
            QCoreApplication::sendEvent(d->wen_list.at(i), &event);
            ++n_act;
        }
    }
    return n_act;
}

void QEventDispatcherWin32::activateWinEventNotifier(QWinEventNotifier * wen)
{
    QEvent event(QEvent::SockAct);
    QCoreApplication::sendEvent(wen, &event);
}

#endif

#endif