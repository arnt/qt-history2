/****************************************************************************
**
** Implementation of timers, socket notifiers, and event handling
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qplatformdefs.h"
#include "qguieventloop.h"
#include "qapplication.h"
#include "qevent.h"
#include "qt_mac.h"
#include <qhash.h>
#include "qsocketnotifier.h"
#include "private/qwidget_p.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#define QMAC_EVENT_NOWAIT kEventDurationNoWait

#include "qguieventloop_p.h"
#define d d_func()
#define q q_func()

//Externals
void qt_event_request_timer(MacTimerInfo *); //qapplication_mac.cpp
MacTimerInfo *qt_event_get_timer(EventRef); //qapplication_mac.cpp
void qt_event_request_select(QGuiEventLoop *); //qapplication_mac.cpp
void qt_event_request_sockact(QGuiEventLoop *); //qapplication_mac.cpp
void qt_event_request_updates(); //qapplication_mac.cpp
void qt_event_request_wakeup(); //qapplication_mac.cpp
bool qt_mac_send_event(QEventLoop::ProcessEventsFlags, EventRef, WindowPtr =0); //qapplication_mac.cpp
extern bool qt_is_gui_used; //qapplication.cpp

static EventLoopTimerUPP timerUPP = 0;       //UPP
static EventLoopTimerUPP mac_select_timerUPP = 0;

/*****************************************************************************
  Timers stuff
 *****************************************************************************/

/* timer call back */
QMAC_PASCAL static void qt_activate_mac_timer(EventLoopTimerRef, void *data)
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

int QGuiEventLoop::registerTimer(int interval, QObject *obj)
{
    if (!d->macTimerList)
	d->macTimerList = new MacTimerList;

    static int serial_id = 777;
    MacTimerInfo t;
    t.obj = obj;
    t.mac_timer = 0;
    t.interval = interval;
    t.pending = true;
    t.id = serial_id++;
    if (interval) {
	if (!timerUPP)
	    timerUPP = NewEventLoopTimerUPP(qt_activate_mac_timer);
	EventTimerInterval mint = (((EventTimerInterval)interval) / 1000);
	d->macTimerList->append(t); //carbon timers go at the end..
	if (InstallEventLoopTimer(GetMainEventLoop(), mint, mint,
				 timerUPP, &d->macTimerList->last(), &d->macTimerList->last().mac_timer)) {
	    qFatal("This cannot really happen, can it!?!");
	    return 0; //exceptional error
	}
	d->macTimerList->last().pending = false;
    } else {
	d->zero_timer_count++;
	d->macTimerList->prepend(t); //zero timers come first
	d->macTimerList->first().pending = false;
    }
    return t.id;
}

QMAC_PASCAL static Boolean find_timer_event(EventRef event, void *data)
{
    return (qt_event_get_timer(event) == ((MacTimerInfo *)data));
}

bool QGuiEventLoop::unregisterTimer(int id)
{
    if(!d->macTimerList || id <= 0)
	return false;				// not init'd or invalid timer
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

bool QGuiEventLoop::unregisterTimers(QObject *obj)
{
    if(!d->macTimerList)				// not initialized
	return false;
    MacTimerList removes;
    for (MacTimerList::Iterator it = d->macTimerList->begin(); it != d->macTimerList->end(); ++it) {
	const MacTimerInfo &t = *it;
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
	    removes += (*it);
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


/*****************************************************************************
  QGuiEventLoop Implementation
 *****************************************************************************/

void QGuiEventLoop::init()
{
    d->macSockets = 0;
    d->macTimerList = 0;
    d->select_timer = 0;
    d->zero_timer_count = 0;
}

void QGuiEventLoop::cleanup()
{
    //timer cleanup
    d->zero_timer_count = 0;
    if(d->macTimerList) {
	for (MacTimerList::Iterator it = d->macTimerList->begin(); it != d->macTimerList->end(); ++it) {
	    const MacTimerInfo &t = *it;
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
QMAC_PASCAL void qt_mac_select_timer_callbk(EventLoopTimerRef, void *me)
{
    QGuiEventLoop *eloop = (QGuiEventLoop*)me;
    if(QMacBlockingFunction::blocking()) { //just send it immediately
	timeval tm;
	memset(&tm, '\0', sizeof(tm));
	eloop->d->eventloopSelect(QEventLoop::AllEvents, &tm);
    } else {
	qt_event_request_select(eloop);
    }
}
void qt_mac_internal_select_callbk(int, int, QGuiEventLoop *eloop)
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
    const CFDataRef data = (const CFDataRef)CFReadStreamCopyProperty(stream, kCFStreamPropertySocketNativeHandle);
    CFDataGetBytes(data, CFRangeMake(0, sizeof(in_sock)), (UInt8 *)&in_sock);
    CFRelease(data);
    qt_mac_internal_select_callbk(in_sock, QSocketNotifier::Read, (QGuiEventLoop*)me);
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
    const CFDataRef data = (const CFDataRef)CFWriteStreamCopyProperty(stream, kCFStreamPropertySocketNativeHandle);
    CFDataGetBytes(data, CFRangeMake(0, sizeof(in_sock)), (UInt8 *)&in_sock);
    CFRelease(data);
    qt_mac_internal_select_callbk(in_sock, QSocketNotifier::Write, (QGuiEventLoop*)me);
}

void QGuiEventLoop::registerSocketNotifier(QSocketNotifier *notifier)
{
    QEventLoop::registerSocketNotifier(notifier);

    MacSocketInfo *mac_notifier = 0;
    if(notifier->type() == QSocketNotifier::Read) {
	mac_notifier = new MacSocketInfo;
	CFStreamCreatePairWithSocket(kCFAllocatorDefault, notifier->socket(), &mac_notifier->read_not, 0);
	CFStreamClientContext ctx;
	memset(&ctx, '\0', sizeof(ctx));
	ctx.info = this;
	CFReadStreamSetClient(mac_notifier->read_not, kCFStreamEventOpenCompleted, qt_mac_select_read_callbk, &ctx);
	CFReadStreamScheduleWithRunLoop(mac_notifier->read_not, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
	CFReadStreamOpen(mac_notifier->read_not);
    } else if(notifier->type() == QSocketNotifier::Write) {
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

void QGuiEventLoop::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    QEventLoop::unregisterSocketNotifier(notifier);
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

bool QGuiEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount();
    return qGlobalPostedEventsCount() || (qt_is_gui_used && GetNumEventsInQueue(GetMainEventQueue()));
}

bool QGuiEventLoop::processEvents(ProcessEventsFlags flags)
{
#if 0
    //TrackDrag says you may not use the EventManager things..
    if(qt_mac_in_drag) {
	qWarning("Qt: Cannot process events whilst dragging!");
	return false;
    }
#endif
    int	   nevents = 0;

    if(!qt_mac_safe_pdev) { //create an empty widget and this can be used for a port anytime
	QWidget *tlw = new QWidget(0, "empty_widget", Qt::WDestructiveClose);
	tlw->hide();
	qt_mac_safe_pdev = tlw;
    }

    QApplication::sendPostedEvents();
    activateTimers(); //send null timersn

    EventRef event;
    do {
	do {
	    if(ReceiveNextEvent(0, 0, QMAC_EVENT_NOWAIT, true, &event) != noErr)
		break;
	    if(qt_mac_send_event(flags, event))
		nevents++;
	    ReleaseEvent(event);
	} while(GetNumEventsInQueue(GetMainEventQueue()));
	QApplication::sendPostedEvents();
    } while(GetNumEventsInQueue(GetMainEventQueue()));
    if(d->quitnow || d->exitloop)
	return false;

    QApplication::sendPostedEvents();
    bool canWait = d->exitloop || d->quitnow ? false : (flags & WaitForMore);

    if(canWait && !d->zero_timer_count) {
	emit aboutToBlock();
#if defined( QMAC_USE_APPLICATION_EVENT_LOOP )
	RunApplicationEventLoop();
#else
	while(CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e20, true) == kCFRunLoopRunTimedOut);
#endif

	// we are awake, broadcast it
	emit awake();
    }
    return nevents > 0;
}

int QGuiEventLoop::activateTimers()
{
    if(!d->zero_timer_count)
	return 0;
    int ret = 0;
    for(MacTimerList::Iterator it = d->macTimerList->begin(); it != d->macTimerList->end(); ++it) {
	const MacTimerInfo &t = *it;
	if(!t.interval) {
	    ret++;
	    QTimerEvent e(t.id);
	    QApplication::sendEvent(t.obj, &e);
	}
    }
    return ret;
}

void QGuiEventLoop::wakeUp()
{
    qt_event_request_wakeup();
}

void QGuiEventLoop::flush()
{
//    sendPostedEvents();
    if(qApp) {
	QWidgetList tlws = QApplication::topLevelWidgets();
	for(int i = 0; i < tlws.size(); i++) {
	    QWidget *tlw = tlws.at(i);
	    if(tlw->isVisible()) 
		QMacSavedPortInfo::flush(tlw);
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

    void addRef() { ++ref; }
    bool subRef() { return (--ref); }

protected:
    void timerEvent(QTimerEvent *)
    {
	if(QApplication::eventLoop()->activateTimers())
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
