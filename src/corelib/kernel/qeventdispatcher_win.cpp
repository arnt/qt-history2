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

#include "qeventdispatcher_win_p.h"

#include "qcoreapplication.h"
#include "qhash.h"
#include "qlibrary.h"
#include "qpair.h"
#include "qsocketnotifier.h"
#include "qwineventnotifier_p.h"

#include "qabstracteventdispatcher_p.h"
#include <private/qthread_p.h>

class QEventDispatcherWin32Private;

struct QSockNot {
    QSocketNotifier *obj;
    int fd;
};
typedef QHash<int, QSockNot *> QSNDict;

struct TimerInfo {                              // internal timer info
    int     ind;                                // - Qt timer identifier - 1
    int interval;
    QObject *obj;                               // - object to receive events
    int    type;                                // GDI timer, fast multimedia timer or zero timer
    QEventDispatcherWin32Private *dispatcher;
    bool pendingEvent;                          // needed to stop adding timer events from the fast timer if there is already a event pending
    int fastInd;                                // id of fast timer

    enum TimerType
    {
        Normal,
        Fast,
        Off
    };
};
typedef QList<TimerInfo*>  TimerVec;            // vector of TimerInfo structs
typedef QHash<int,TimerInfo*> TimerDict;        // fast dict of timers

#if !defined(DWORD_PTR) && !defined(Q_WS_WIN64)
#define DWORD_PTR DWORD
#endif

typedef MMRESULT(WINAPI *ptimeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
typedef MMRESULT(WINAPI *ptimeKillEvent)(UINT);

static ptimeSetEvent qtimeSetEvent = 0;
static ptimeKillEvent qtimeKillEvent = 0;

static void resolveTimerAPI()
{
    if (!qtimeSetEvent) {
        qtimeSetEvent = (ptimeSetEvent)QLibrary::resolve(QLatin1String("winmm"), "timeSetEvent");
        qtimeKillEvent = (ptimeKillEvent)QLibrary::resolve(QLatin1String("winmm"), "timeKillEvent");
    }
}


class QEventDispatcherWin32Private : public QAbstractEventDispatcherPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherWin32)
public:
    QEventDispatcherWin32Private();
    ~QEventDispatcherWin32Private();

    DWORD threadId;

    bool interrupt;


    // internal window handle used for socketnotifiers/timers/etc
    HWND internalHwnd;

    // timers
    TimerVec timerVec;
    TimerDict timerDict;

    // socket notifiers
    QSNDict sn_read;
    QSNDict sn_write;
    QSNDict sn_except;

    // event notifier
    QWinEventNotifier wakeUpNotifier;

    QList<QWinEventNotifier *> winEventNotifierList;
    void activateEventNotifier(QWinEventNotifier * wen);

    QList<MSG> queuedUserInputEvents;

    CRITICAL_SECTION fastTimerCriticalSection;
};

QEventDispatcherWin32Private::QEventDispatcherWin32Private()
    : threadId(GetCurrentThreadId()), interrupt(false), internalHwnd(0)
{
    resolveTimerAPI();
    InitializeCriticalSection(&fastTimerCriticalSection);

    wakeUpNotifier.setHandle(QT_WA_INLINE(CreateEventW(0, false, false, L"Wakeup event"),
                                          CreateEventA(0, false, false, "Wakeup event")));
}

QEventDispatcherWin32Private::~QEventDispatcherWin32Private()
{
    wakeUpNotifier.setEnabled(false);
    CloseHandle(wakeUpNotifier.handle());

    DeleteCriticalSection(&fastTimerCriticalSection);
}

void QEventDispatcherWin32Private::activateEventNotifier(QWinEventNotifier * wen)
{
    QEvent event(QEvent::WinEventAct);
    QCoreApplication::sendEvent(wen, &event);
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

// This function is called by a workerthread
void WINAPI CALLBACK qt_fast_timer_proc(uint timerId, uint /*reserved*/, DWORD_PTR user, DWORD_PTR /*reserved*/, DWORD_PTR /*reserved*/)
{
    if (!timerId) // sanity check
        return;

    QCoreApplication *app = QCoreApplication::instance();
    Q_ASSERT_X(app, "qt_fast_timer_proc", "Timer fired, but no QCoreApplication");
    if (!app) {
        qtimeKillEvent(timerId);
        return;
    }

    TimerInfo *t = (TimerInfo*)user;
    Q_ASSERT(t);
    EnterCriticalSection(&t->dispatcher->fastTimerCriticalSection);
    if (t->type == TimerInfo::Off) { // timer stopped
        qtimeKillEvent(timerId);
        LeaveCriticalSection(&t->dispatcher->fastTimerCriticalSection);
        delete t;
        return;
    }
    if (!t->pendingEvent) {
        t->pendingEvent = true;
        PostMessage(t->dispatcher->internalHwnd, WM_TIMER, WPARAM(t->ind), 0);
    }
    LeaveCriticalSection(&t->dispatcher->fastTimerCriticalSection);
}

LRESULT CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_NCCREATE) {
            return true;
    } else if (message == WM_USER) {

        // socket notifier message
        MSG msg;
        msg.hwnd = hwnd;
        msg.message = message;
        msg.wParam = wp;
        msg.lParam = lp;

        QCoreApplication *app = QCoreApplication::instance();
        long result;
        if (app && app->filterEvent(&msg, &result))
            return result;

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
        if (type >= 0) {

    #ifdef GWLP_USERDATA
            QEventDispatcherWin32 *eventDispatcher =
                (QEventDispatcherWin32 *) GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    #else
            QEventDispatcherWin32 *eventDispatcher =
                (QEventDispatcherWin32 *) GetWindowLongA(hwnd, GWL_USERDATA);
    #endif
            if (eventDispatcher) {
                QEventDispatcherWin32Private *d = eventDispatcher->d_func();
                QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
                QSNDict *dict = sn_vec[type];

                QSockNot *sn = dict ? dict->value(wp) : 0;
                if (sn) {
                    QEvent event(QEvent::SockAct);
                    QCoreApplication::sendEvent(sn->obj, &event);
                }
            }
        }
        return 0;

    } else if (message == WM_TIMER) {

        MSG msg;
        msg.hwnd = hwnd;
        msg.message = message;
        msg.wParam = wp;
        msg.lParam = lp;

        QCoreApplication *app = QCoreApplication::instance();
        Q_ASSERT_X(app, "qt_interal_proc", "Timer fired, but no QCoreApplication");
        if (!app) {
            KillTimer(hwnd, wp);
            return 0;
        }

        long result;
        if (app->filterEvent(&msg, &result))
            return result;

        QEventDispatcherWin32 *eventDispatcher =
            qobject_cast<QEventDispatcherWin32 *>(QAbstractEventDispatcher::instance());
        Q_ASSERT(eventDispatcher != 0);
        QEventDispatcherWin32Private *d = eventDispatcher->d_func();

        TimerInfo *t = d->timerDict.value(wp);
        if (t) {
            QTimerEvent *e = new QTimerEvent(t->ind);
            QCoreApplication::postEvent(t->obj, e);
            TimerInfo *tn = d->timerDict.value(wp);
            if (tn && t == tn) // check it was not deleted or that it is a new TimerInfo
                tn->pendingEvent = false;
        }
        return 0;
    }

    return  DefWindowProc(hwnd, message, wp, lp);

}

static HWND qt_create_internal_window(QEventDispatcherWin32 *eventDispatcher)
{
    extern HINSTANCE qWinAppInst();
    HINSTANCE hi = qWinAppInst();
    WNDCLASSA wc;
    wc.style = 0;
    wc.lpfnWndProc = qt_internal_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "QEventDispatcherWin32_Internal_Widget";
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
        qWarning("Failed to create QEventDispatcherWin32 internal window: %d\n", (int)GetLastError());
    }
    return wnd;
}

QEventDispatcherWin32::QEventDispatcherWin32(QObject *parent)
    : QAbstractEventDispatcher(*new QEventDispatcherWin32Private, parent)
{
    Q_D(QEventDispatcherWin32);
    d->internalHwnd = qt_create_internal_window(this);
}

QEventDispatcherWin32::~QEventDispatcherWin32()
{
    Q_D(QEventDispatcherWin32);

    // clean up any socketnotifiers
    while (!d->sn_read.isEmpty())
        unregisterSocketNotifier((*(d->sn_read.begin()))->obj);
    while (!d->sn_write.isEmpty())
        unregisterSocketNotifier((*(d->sn_write.begin()))->obj);
    while (!d->sn_except.isEmpty())
        unregisterSocketNotifier((*(d->sn_except.begin()))->obj);

    // clean up any timers
    while (!d->timerDict.isEmpty())
        unregisterTimer((*(d->timerDict.begin()))->ind);

    DestroyWindow(d->internalHwnd);
}

bool QEventDispatcherWin32::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherWin32);

    emit awake();

    bool canWait;
    bool retVal = false;
    do {
        QCoreApplication::sendPostedEvents();

        DWORD waitRet = 0;
        QThreadData *data = QThreadData::get(thread());
        HANDLE pHandles[MAXIMUM_WAIT_OBJECTS - 1];
        while (!d->interrupt) {
            DWORD nCount = d->winEventNotifierList.count();
            Q_ASSERT(nCount < MAXIMUM_WAIT_OBJECTS - 1);

            MSG msg;
            bool haveMessage;

            if (!(flags & QEventLoop::ExcludeUserInputEvents) && !d->queuedUserInputEvents.isEmpty()) {
                // process queued user input events
                haveMessage = true;
                msg = d->queuedUserInputEvents.takeFirst();
            } else {
                haveMessage = winPeekMessage(&msg, 0, 0, 0, PM_REMOVE);
                if (haveMessage && (flags & QEventLoop::ExcludeUserInputEvents)
                    && ((msg.message >= WM_KEYFIRST
                         && msg.message <= WM_KEYLAST)
                        || (msg.message >= WM_MOUSEFIRST
                            && msg.message <= WM_MOUSELAST)
                        || msg.message == WM_MOUSEWHEEL)) {
                    // queue user input events for later processing
                    haveMessage = false;
                    d->queuedUserInputEvents.append(msg);
                }
            }
            if (!haveMessage) {
                // no message - check for signalled objects
                for (int i=0; i<(int)nCount; i++)
                    pHandles[i] = d->winEventNotifierList.at(i)->handle();
                waitRet = MsgWaitForMultipleObjectsEx(nCount, pHandles, 0, QS_ALLINPUT, MWMO_ALERTABLE);
                if ((haveMessage = (waitRet == WAIT_OBJECT_0 + nCount))) {
                    // a new message has arrived, process it
                    continue;
                }
            }
            if (haveMessage) {
                if (msg.message == WM_QUIT) {
                    if (QCoreApplication::instance())
                        QCoreApplication::instance()->quit();
                    return false;
                }

                if (!filterEvent(&msg)) {
                    TranslateMessage(&msg);
                    QT_WA({
                        DispatchMessage(&msg);
                    } , {
                        DispatchMessageA(&msg);
                    });
                }
            } else if (waitRet >= WAIT_OBJECT_0 && waitRet < WAIT_OBJECT_0 + nCount) {
                d->activateEventNotifier(d->winEventNotifierList.at(waitRet - WAIT_OBJECT_0));
            } else {
                // nothing todo so break
                break;
            }
            retVal = true;
        }

        // still nothing - wait for message or signalled objects
        canWait = (!retVal
                   && data->postEventList.size() == 0
                   && !d->interrupt
                   && (flags & QEventLoop::WaitForMoreEvents));
        if (canWait) {
            DWORD nCount = d->winEventNotifierList.count();
            Q_ASSERT(nCount < MAXIMUM_WAIT_OBJECTS - 1);
            for (int i=0; i<(int)nCount; i++)
                pHandles[i] = d->winEventNotifierList.at(i)->handle();

            emit aboutToBlock();
            waitRet = MsgWaitForMultipleObjectsEx(nCount, pHandles, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
            if (waitRet >= WAIT_OBJECT_0 && waitRet < WAIT_OBJECT_0 + nCount) {
                d->activateEventNotifier(d->winEventNotifierList.at(waitRet - WAIT_OBJECT_0));
                retVal = true;
            }
        }
    } while (canWait);

    if (d->interrupt)
        d->interrupt = false;
    return retVal;
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
    WSAAsyncSelect(socket, d->internalHwnd, sn_event ? WM_USER : 0, sn_event);
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
    WSAAsyncSelect(socket, d->internalHwnd, sn_event ? WM_USER : 0, sn_event);
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

void QEventDispatcherWin32::registerTimer(int timerId, int interval, QObject *object)
{
    Q_D(QEventDispatcherWin32);

    register ::TimerInfo *t = new ::TimerInfo;
    t->ind  = timerId;
    t->interval = interval;
    t->obj  = object;
    t->dispatcher = 0;
    t->type = ::TimerInfo::Normal;
    t->pendingEvent = false;
    t->fastInd = 0;

    int ok = 0;

    if (interval > 10 || !interval || !qtimeSetEvent) {
        if (!interval) // optimization for single-shot-zero-timer
            PostMessage(d->internalHwnd, WM_TIMER, WPARAM(t->ind), 0);
        ok = SetTimer(d->internalHwnd, t->ind, (uint) interval, 0);

    } else {
        t->dispatcher = d;
        t->type = ::TimerInfo::Fast;
        t->fastInd = qtimeSetEvent(interval, 1, qt_fast_timer_proc, (DWORD_PTR)t, TIME_CALLBACK_FUNCTION|TIME_PERIODIC);
        ok = t->fastInd;
        if (ok == 0) { // fall back to normal timer if no more multimedia timers avaiable
            t->dispatcher = 0;
            t->type = ::TimerInfo::Normal;
            ok = SetTimer(d->internalHwnd, t->ind, (uint) interval, 0);
        }
    }

    if (ok == 0) {
        qErrnoWarning("QEventDispatcherWin32::registerTimer: Failed to create a timer");
        delete t;                               // could not set timer
        return;
    }

    d->timerVec.append(t);                      // store in timer vector
    d->timerDict.insert(t->ind, t);          // store timers in dict
}

bool QEventDispatcherWin32::unregisterTimer(int timerId)
{
    Q_D(QEventDispatcherWin32);
    if (d->timerVec.isEmpty() || timerId <= 0)
        return false;

    ::TimerInfo *t = d->timerDict.value(timerId);
    if (!t)
        return false;

    d->timerDict.remove(t->ind);
    d->timerVec.removeAll(t);

    switch (t->type) {
    case ::TimerInfo::Fast:
        EnterCriticalSection(&d->fastTimerCriticalSection);
        t->type = ::TimerInfo::Off; // kill timer (and delete t) from callback
        LeaveCriticalSection(&d->fastTimerCriticalSection);
        break;
    case ::TimerInfo::Normal:
        KillTimer(d->internalHwnd, t->ind);
        delete t;
        break;
    }
    return true;
}

bool QEventDispatcherWin32::unregisterTimers(QObject *object)
{
    Q_D(QEventDispatcherWin32);
    if (d->timerVec.isEmpty())
        return false;
    register ::TimerInfo *t;
    for (int i=0; i<d->timerVec.size(); i++) {
        t = d->timerVec.at(i);
        if (t && t->obj == object) {                // object found
            d->timerDict.remove(t->ind);
            d->timerVec.removeAt(i);
            switch (t->type) {
            case ::TimerInfo::Fast:
                EnterCriticalSection(&d->fastTimerCriticalSection);
                t->type = ::TimerInfo::Off; // kill timer (and delete t) from callback
                LeaveCriticalSection(&d->fastTimerCriticalSection);
                break;
            case ::TimerInfo::Normal:
                KillTimer(d->internalHwnd, t->ind);
                delete t;
                break;
            }

            --i;
        }
    }
    return true;
}

QList<QEventDispatcherWin32::TimerInfo>
QEventDispatcherWin32::registeredTimers(QObject *object) const
{
    Q_D(const QEventDispatcherWin32);
    QList<TimerInfo> list;
    for (int i = 0; i < d->timerVec.size(); ++i) {
        const ::TimerInfo *t = d->timerVec.at(i);
        if (t && t->obj == object)
            list << TimerInfo(t->ind, t->interval);
    }
    return list;
}

bool QEventDispatcherWin32::registerEventNotifier(QWinEventNotifier *notifier)
{
    Q_D(QEventDispatcherWin32);

    if (d->winEventNotifierList.contains(notifier))
        return true;

    if (d->winEventNotifierList.count() >= MAXIMUM_WAIT_OBJECTS - 2) {
        qWarning("QWinEventNotifier: Can not have more then %d enabled at one time", MAXIMUM_WAIT_OBJECTS - 2);
        return false;
    }
    d->winEventNotifierList.append(notifier);
    return true;
}

void QEventDispatcherWin32::unregisterEventNotifier(QWinEventNotifier *notifier)
{
    Q_D(QEventDispatcherWin32);

    int i = d->winEventNotifierList.indexOf(notifier);
    if (i != -1)
        d->winEventNotifierList.takeAt(i);
}

void QEventDispatcherWin32::activateEventNotifiers()
{
    Q_D(QEventDispatcherWin32);
    //### this could break if events are removed/added in the activation
    for (int i=0; i<d->winEventNotifierList.count(); i++) {
        if (WaitForSingleObjectEx(d->winEventNotifierList.at(i)->handle(), 0, true) == WAIT_OBJECT_0)
            d->activateEventNotifier(d->winEventNotifierList.at(i));
    }
}

void QEventDispatcherWin32::wakeUp()
{
    Q_D(QEventDispatcherWin32);
    SetEvent(d->wakeUpNotifier.handle());
}

void QEventDispatcherWin32::interrupt()
{
    Q_D(QEventDispatcherWin32);
    d->interrupt = true;
    wakeUp();
}

void QEventDispatcherWin32::flush()
{ }


void QEventDispatcherWin32::startingUp()
{
    Q_D(QEventDispatcherWin32);

    if (d->wakeUpNotifier.handle()) d->wakeUpNotifier.setEnabled(true);
}
