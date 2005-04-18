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
#include <private/qt_mac_p.h>
#include "qeventdispatcher_mac_p.h"
#include "qapplication.h"
#include "qevent.h"
#include <qhash.h>
#include "qsocketnotifier.h"
#include "private/qwidget_p.h"
#include "private/qthread_p.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#define QMAC_EVENT_NOWAIT kEventDurationNoWait

/*****************************************************************************
  Externals
 *****************************************************************************/
extern void qt_event_request_timer(MacTimerInfo *); //qapplication_mac.cpp
extern MacTimerInfo *qt_event_get_timer(EventRef); //qapplication_mac.cpp
extern void qt_event_request_select(QEventDispatcherMac *); //qapplication_mac.cpp
extern void qt_event_request_sockact(QEventDispatcherMac *); //qapplication_mac.cpp
extern void qt_event_request_updates(); //qapplication_mac.cpp
extern bool qt_mac_send_event(QEventLoop::ProcessEventsFlags, EventRef, WindowPtr =0); //qapplication_mac.cpp
extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern bool qt_is_gui_used; //qapplication.cpp

static EventLoopTimerUPP timerUPP = 0;
static EventLoopTimerUPP mac_select_timerUPP = 0;

/*****************************************************************************
  Timers stuff
 *****************************************************************************/

/* timer call back */
static void qt_mac_activate_timer(EventLoopTimerRef, void *data)
{
    MacTimerInfo *tmr = (MacTimerInfo *)data;
    if(QMacBlockingFunction::blocking()) { //just send it immediately
        /* someday this is going to lead to an infite loop, I just know it. I should be marking the
           pending here, and unmarking, but of course single shot timers are removed between now
           and the return (down 4 lines) */
        QTimerEvent e(tmr->id);
        QApplication::sendEvent(tmr->obj, &e);
        QApplication::flush();
        return;
    }
    if(tmr->pending)
        return;
    tmr->pending = true;
    qt_event_request_timer(tmr);
}

void QEventDispatcherMac::registerTimer(int timerId, int interval, QObject *obj)
{
    Q_D(QEventDispatcherMac);
    if (!d->macTimerList)
        d->macTimerList = new MacTimerList;

    MacTimerInfo t;
    t.id = timerId;
    t.interval = interval;
    t.obj = obj;
    t.mac_timer = 0;
    t.pending = true;
    if (interval) {
        if (!timerUPP)
            timerUPP = NewEventLoopTimerUPP(qt_mac_activate_timer);
        EventTimerInterval mint = (((EventTimerInterval)interval) / 1000);
        d->macTimerList->append(t); //carbon timers go at the end..
        if (InstallEventLoopTimer(GetMainEventLoop(), mint, mint,
                                 timerUPP, &d->macTimerList->last(), &d->macTimerList->last().mac_timer)) {
            qFatal("This cannot really happen, can it!?!");
            return; //exceptional error
        }
        d->macTimerList->last().pending = false;
    } else {
        d->zero_timer_count++;
        if(d->zero_timer_count == 1)
            wakeUp(); //if we are blocking we need to come out of that state
        d->macTimerList->prepend(t); //zero timers come first
        d->macTimerList->first().pending = false;
    }
}

static Boolean find_timer_event(EventRef event, void *data)
{
    return (qt_event_get_timer(event) == ((MacTimerInfo *)data));
}

bool QEventDispatcherMac::unregisterTimer(int id)
{
    Q_D(QEventDispatcherMac);
    if(!d->macTimerList || id <= 0)
        return false;                                // not init'd or invalid timer
    for (int i = 0; i < d->macTimerList->size(); ++i) {
        const MacTimerInfo &t = d->macTimerList->at(i);
        if (t.id == id) {
            if (t.mac_timer) {
                RemoveEventLoopTimer(t.mac_timer);
                if (t.pending) {
                    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
                    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)&t);
                    DisposeEventComparatorUPP(fnc);
                }
            } else {
                d->zero_timer_count--;
            }
            d->macTimerList->removeAt(i);
            return true;
        }
    }
    return false;
}

bool QEventDispatcherMac::unregisterTimers(QObject *obj)
{
    Q_D(QEventDispatcherMac);
    if(!d->macTimerList)                                // not initialized
        return false;
    MacTimerList removes;
    for (int i = 0; i < d->macTimerList->size(); ++i) {
        const MacTimerInfo &t = d->macTimerList->at(i);
        if (t.obj == obj) {
            if (t.mac_timer) {
                RemoveEventLoopTimer(t.mac_timer);
                if (t.pending) {
                    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
                    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)&t);
                    DisposeEventComparatorUPP(fnc);
                }
            } else {
                d->zero_timer_count--;
            }
            removes += t;
        }
    }
    for (MacTimerList::Iterator it = removes.begin(); it != removes.end(); ++it) {
        for (int i = 0; i < d->macTimerList->size(); ++i) {
            const MacTimerInfo &info = d->macTimerList->at(i);
            if (info.id == (*it).id)
                d->macTimerList->removeAt(i);
        }
    }
    return true;
}

QList<QEventDispatcherMac::TimerInfo>
QEventDispatcherMac::registeredTimers(QObject *object) const
{
    Q_D(const QEventDispatcherMac);
    QList<TimerInfo> list;
    if (!d->macTimerList)
        return list;
    for (int i = 0; i < d->macTimerList->size(); ++i) {
        const MacTimerInfo &t = d->macTimerList->at(i);
        if (t.obj == object)
            list << TimerInfo(t.id, t.interval);
    }
    return list;
}


/*****************************************************************************
  QEventDispatcherMac Implementation
 *****************************************************************************/

QEventDispatcherMacPrivate::QEventDispatcherMacPrivate()
{
    macSockets = 0;
    macTimerList = 0;
    select_timer = 0;
    zero_timer_count = 0;
}

QEventDispatcherMac::QEventDispatcherMac(QObject *parent)
    : QEventDispatcherUNIX(*new QEventDispatcherMacPrivate, parent)
{ }

QEventDispatcherMac::~QEventDispatcherMac()
{
    Q_D(QEventDispatcherMac);
    //timer cleanup
    d->zero_timer_count = 0;
    if(d->macTimerList) {
        for (int i = 0; i < d->macTimerList->size(); ++i) {
            const MacTimerInfo &t = d->macTimerList->at(i);
            if (t.mac_timer) {
                RemoveEventLoopTimer(t.mac_timer);
                if (t.pending) {
                    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
                    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)&t);
                    DisposeEventComparatorUPP(fnc);
                }
            }
        }
        delete d->macTimerList;
        d->macTimerList = 0;
    }
    if(timerUPP) {
        DisposeEventLoopTimerUPP(timerUPP);
        timerUPP = 0;
    }
    //select cleanup
    if(d->select_timer) {
        RemoveEventLoopTimer(d->select_timer);
        d->select_timer = 0;
    }
    DisposeEventLoopTimerUPP(mac_select_timerUPP);
    mac_select_timerUPP = 0;
    if(d->macSockets) {
        for(QHash<QSocketNotifier *, MacSocketInfo *>::Iterator it = d->macSockets->begin();
            it != d->macSockets->end(); ++it) {
            if(it.key()->type() == QSocketNotifier::Read) {
                CFReadStreamUnscheduleFromRunLoop(it.value()->read_not, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
                CFReadStreamClose(it.value()->read_not);
                CFRelease(it.value()->read_not);
            } else if(it.key()->type() == QSocketNotifier::Write) {
                CFWriteStreamUnscheduleFromRunLoop(it.value()->write_not, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
                CFWriteStreamClose(it.value()->write_not);
                CFRelease(it.value()->write_not);
            }
            delete it.value();
        }
        delete d->macSockets;
        d->macSockets = 0;
    }
}

/**************************************************************************
    Socket Notifiers
 *************************************************************************/
void qt_mac_select_timer_callbk(EventLoopTimerRef, void *me)
{
    QEventDispatcherMac *eloop = (QEventDispatcherMac*)me;
    if(QMacBlockingFunction::blocking()) { //just send it immediately
        timeval tm;
        memset(&tm, '\0', sizeof(tm));
        eloop->d_func()->doSelect(QEventLoop::AllEvents, &tm);
    } else {
        qt_event_request_select(eloop);
    }
}
void qt_mac_internal_select_callbk(int, int, QEventDispatcherMac *eloop)
{
     qt_mac_select_timer_callbk(0, eloop);
}
static void qt_mac_select_read_callbk(CFReadStreamRef stream, CFStreamEventType type, void *me)
{
    if(type == kCFStreamEventOpenCompleted) {
        CFStreamClientContext ctx;
        memset(&ctx, '\0', sizeof(ctx));
        ctx.info = me;
        CFReadStreamSetClient(stream, kCFStreamEventHasBytesAvailable, qt_mac_select_read_callbk, &ctx);
    }
    int in_sock;
    QCFType<CFDataRef> data = static_cast<CFDataRef>(CFReadStreamCopyProperty(stream,
                                                            kCFStreamPropertySocketNativeHandle));
    CFDataGetBytes(data, CFRangeMake(0, sizeof(in_sock)), (UInt8 *)&in_sock);
    qt_mac_internal_select_callbk(in_sock, QSocketNotifier::Read, (QEventDispatcherMac*)me);
}
static void qt_mac_select_write_callbk(CFWriteStreamRef stream, CFStreamEventType type, void *me)
{
    if(type == kCFStreamEventOpenCompleted) {
        CFStreamClientContext ctx;
        memset(&ctx, '\0', sizeof(ctx));
        ctx.info = me;
        CFWriteStreamSetClient(stream, kCFStreamEventCanAcceptBytes, qt_mac_select_write_callbk, &ctx);
    }
    int in_sock;
    QCFType<CFDataRef> data = static_cast<CFDataRef>(CFWriteStreamCopyProperty(stream,
                                                            kCFStreamPropertySocketNativeHandle));
    CFDataGetBytes(data, CFRangeMake(0, sizeof(in_sock)), (UInt8 *)&in_sock);
    qt_mac_internal_select_callbk(in_sock, QSocketNotifier::Write, (QEventDispatcherMac*)me);
}

void QEventDispatcherMac::registerSocketNotifier(QSocketNotifier *notifier)
{
    Q_D(QEventDispatcherMac);
    QEventDispatcherUNIX::registerSocketNotifier(notifier);
    MacSocketInfo *mac_notifier = 0;
    if(notifier->type() == QSocketNotifier::Read &&
       QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
        mac_notifier = new MacSocketInfo;
        CFStreamCreatePairWithSocket(kCFAllocatorDefault, notifier->socket(), &mac_notifier->read_not, 0);
        CFStreamClientContext ctx;
        memset(&ctx, '\0', sizeof(ctx));
        ctx.info = this;
        CFReadStreamSetClient(mac_notifier->read_not, kCFStreamEventOpenCompleted, qt_mac_select_read_callbk, &ctx);
        CFReadStreamScheduleWithRunLoop(mac_notifier->read_not, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFReadStreamOpen(mac_notifier->read_not);
    } else if(notifier->type() == QSocketNotifier::Write &&
              QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
        mac_notifier = new MacSocketInfo;
        CFStreamCreatePairWithSocket(kCFAllocatorDefault, notifier->socket(), 0, &mac_notifier->write_not);
        CFStreamClientContext ctx;
        memset(&ctx, '\0', sizeof(ctx));
        ctx.info = this;
        CFWriteStreamSetClient(mac_notifier->write_not, kCFStreamEventOpenCompleted, qt_mac_select_write_callbk, &ctx);
        CFWriteStreamScheduleWithRunLoop(mac_notifier->write_not, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFWriteStreamOpen(mac_notifier->write_not);
    }
    if(mac_notifier) {
        if(!d->macSockets)
            d->macSockets = new QHash<QSocketNotifier *, MacSocketInfo *>;
        d->macSockets->insert(notifier, mac_notifier);
    }
    if(!d->select_timer) {
        if(!mac_select_timerUPP)
            mac_select_timerUPP = NewEventLoopTimerUPP(qt_mac_select_timer_callbk);
        InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0.1,
                              mac_select_timerUPP, (void *)this, &d->select_timer);
    }
}

void QEventDispatcherMac::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    Q_D(QEventDispatcherMac);
    QEventDispatcherUNIX::unregisterSocketNotifier(notifier);
    if(d->macSockets) {
        if(MacSocketInfo *mac_notifier = d->macSockets->value(notifier)) {
            d->macSockets->remove(notifier);
            if(notifier->type() == QSocketNotifier::Read) {
                CFReadStreamUnscheduleFromRunLoop(mac_notifier->read_not, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
                CFReadStreamClose(mac_notifier->read_not);
                CFRelease(mac_notifier->read_not);
            } else if(notifier->type() == QSocketNotifier::Write) {
                CFWriteStreamUnscheduleFromRunLoop(mac_notifier->write_not, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
                CFWriteStreamClose(mac_notifier->write_not);
                CFRelease(mac_notifier->write_not);
            }
            delete mac_notifier;
        }
    }
    if(d->sn_highest == -1 && d->select_timer) {
        RemoveEventLoopTimer(d->select_timer);
        d->select_timer = 0;
    }
}

bool QEventDispatcherMac::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount();
    return qGlobalPostedEventsCount() || (qt_is_gui_used && GetNumEventsInQueue(GetMainEventQueue()));
}

bool QEventDispatcherMac::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherMac);
#if 0
    //TrackDrag says you may not use the EventManager things..
    if(qt_mac_in_drag) {
        qWarning("Qt: Cannot process events whilst dragging!");
        return false;
    }
#endif
    emit awake();

    if(!qt_mac_safe_pdev) { //create an empty widget and this can be used for a port anytime
        QWidget *tlw = new QWidget;
        tlw->setAttribute(Qt::WA_DeleteOnClose);
        tlw->setObjectName("empty_widget");
        tlw->hide();
        qt_mac_safe_pdev = tlw;
    }

    bool retVal = false;
    for (;;) {
        QThreadData *threadData = QThreadData::get(thread());
        if (threadData->postEventList.size() > 0)
            retVal = true;
        QApplication::sendPostedEvents();
        if (d->activateTimers() > 0) //send null timers
            retVal = true;

        do {
            EventRef event;
            if (!(flags & QEventLoop::ExcludeUserInputEvents)
                    && !d->queuedUserInputEvents.isEmpty()) {
                // process a pending user input event
                event = d->queuedUserInputEvents.takeFirst();
            } else {
                if(ReceiveNextEvent(0, 0, QMAC_EVENT_NOWAIT, true, &event)
                      != noErr)
                    break;
                // else
                if (flags & QEventLoop::ExcludeUserInputEvents) {
                     UInt32 ekind = GetEventKind(event),
                            eclass = GetEventClass(event);
                     switch(eclass) {
                         case kEventClassQt:
                             if(ekind != kEventQtRequestContext)
                                 break;
                             // fall through
                         case kEventClassMouse:
                         case kEventClassKeyboard:
                             d->queuedUserInputEvents.append(event);
                             continue;
                     }
                }
            }

            if (!filterEvent(&event) && qt_mac_send_event(flags, event))
                retVal = true;
            ReleaseEvent(event);
        } while(!d->interrupt && GetNumEventsInQueue(GetMainEventQueue()));

        QApplication::sendPostedEvents();

        bool canWait = (!retVal
                        && threadData->postEventList.size() == 0
                        && !d->interrupt
                        && (flags & QEventLoop::WaitForMoreEvents)
                        && !d->zero_timer_count);
        if (canWait) {
            emit aboutToBlock();
            while(CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e20, true) == kCFRunLoopRunTimedOut);
        } else {
            break;
        }
    }
    d->interrupt = false;
    return retVal;
}

int QEventDispatcherMacPrivate::activateTimers()
{
    if(!zero_timer_count)
        return 0;
    int ret = 0;
    for (int i = 0; i < macTimerList->size(); ++i) {
        const MacTimerInfo &t = macTimerList->at(i);
        if(!t.interval) {
            ret++;
            QTimerEvent e(t.id);
            QApplication::sendEvent(t.obj, &e);
        }
    }
    return ret;
}

void QEventDispatcherMac::wakeUp()
{
    CFRunLoopStop((CFRunLoopRef)GetCFRunLoopFromEventLoop(GetMainEventLoop()));
}

void QEventDispatcherMac::flush()
{
//    sendPostedEvents();
    if(qApp) {
        QWidgetList tlws = QApplication::topLevelWidgets();
        for(int i = 0; i < tlws.size(); i++) {
            QWidget *tlw = tlws.at(i);
            if(tlw->isVisible()) {
#if defined(QMAC_NO_COREGRAPHICS) || 1
                QDFlushPortBuffer(GetWindowPort(qt_mac_window_for(tlw)), NULL);
#else
                HIWindowFlush(qt_mac_window_for(tlw));
#endif
            }
        }
    }
}

/* This allows the eventloop to block, and will keep things going - including keeping away
   the evil spinning cursor */
class QMacBlockingFunction::Object : public QObject
{
    QAtomic ref;
public:
    Object() { startTimer(100); }

    void addRef() { ref.ref(); }
    bool subRef() { return (ref.deref()); }

protected:
    void timerEvent(QTimerEvent *)
    {
        // if(QApplication::eventLoop()->activateTimers())
            QApplication::flush();
    }
};
QMacBlockingFunction::Object *QMacBlockingFunction::block = 0;
QMacBlockingFunction::QMacBlockingFunction()
{
    if(!block)
        block = new QMacBlockingFunction::Object;
    block->addRef();
}
QMacBlockingFunction::~QMacBlockingFunction()
{
    Q_ASSERT(block);
    if(!block->subRef()) {
        delete block;
        block = 0;
    }
}
