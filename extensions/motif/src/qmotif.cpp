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

// #define GRAB_DEBUG
#ifdef GRAB_DEBUG
#  define GDEBUG qDebug
#else
#  define GDEBUG if(false)qDebug
#endif

// #define EVENT_DEBUG
#ifdef EVENT_DEBUG
#  define EDEBUG qDebug
#else
#  define EDEBUG if(false)qDebug
#endif

#include <qapplication.h>
#include <qevent.h>
#include <qhash.h>
#include <qsocketnotifier.h>
#include <qwidget.h>
#include <qvector.h>

#include <private/qabstracteventdispatcher_p.h>
#include <private/qthread_p.h>

#include "qmotif.h"

#include <qx11info_x11.h>

#include <stdlib.h>

// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease

typedef QHash<XtIntervalId, QObject *> TimerHash;
typedef QHash<int, QSocketNotifier *> SocketNotifierHash;

Boolean qmotif_event_dispatcher(XEvent *event);

class QMotifPrivate : public QAbstractEventDispatcherPrivate
{
public:
    QMotifPrivate();
    ~QMotifPrivate();

    void hookMeUp();
    void unhook();

    Display *display;
    XtAppContext appContext, ownContext;

    typedef QVector<XtEventDispatchProc> DispatcherArray;
    QHash<Display*,DispatcherArray> dispatchers;
    QWidgetMapper mapper;

    TimerHash timerHash;
    SocketNotifierHash socketNotifierHash;

    bool interrupt;

    // arguments for Xt display initialization
    const char* applicationClass;
    XrmOptionDescRec* options;
    int numOptions;
};

static QMotifPrivate *static_d = 0;
static XEvent* last_xevent = 0;

QMotifPrivate::QMotifPrivate()
    : appContext(NULL), ownContext(NULL), interrupt(false),
      applicationClass(0), options(0), numOptions(0)
{
    if (static_d)
	qWarning("QMotif: should only have one QMotif instance!");
    static_d = this;
}

QMotifPrivate::~QMotifPrivate()
{
    if (static_d == this)
        static_d = 0;
}

void QMotifPrivate::hookMeUp()
{
    // worker to plug Qt into Xt (event dispatchers)
    // and Xt into Qt (QMotifEventLoop)

    // ### TODO extensions?
    DispatcherArray &qt_dispatchers = dispatchers[QX11Info::display()];
    DispatcherArray &qm_dispatchers = dispatchers[display];

    qt_dispatchers.resize(LASTEvent);
    qt_dispatchers.fill(0);

    qm_dispatchers.resize(LASTEvent);
    qm_dispatchers.fill(0);

    int et;
    for (et = 2; et < LASTEvent; et++) {
	qt_dispatchers[et] =
	    XtSetEventDispatcher(QX11Info::display(), et, ::qmotif_event_dispatcher);
	qm_dispatchers[et] =
	    XtSetEventDispatcher(display, et, ::qmotif_event_dispatcher);
    }
}

void QMotifPrivate::unhook()
{
    // unhook Qt from Xt (event dispatchers)
    // unhook Xt from Qt? (QMotifEventLoop)

    // ### TODO extensions?
    DispatcherArray &qt_dispatchers = dispatchers[QX11Info::display()];
    DispatcherArray &qm_dispatchers = dispatchers[display];

    for (int et = 2; et < LASTEvent; ++et) {
	(void) XtSetEventDispatcher(QX11Info::display(), et, qt_dispatchers[et]);
	(void) XtSetEventDispatcher(display, et, qm_dispatchers[et]);
    }

    /*
      We cannot destroy the app context here because it closes the X
      display, something QApplication does as well a bit later.
      if (ownContext)
          XtDestroyApplicationContext(ownContext);
     */
    appContext = ownContext = 0;
}

extern bool qt_try_modal(QWidget *, XEvent *); // defined in qapplication_x11.cpp

static bool xt_grab = false;
static Window xt_grab_focus_window = None;
static Display *xt_grab_display = 0;

Boolean qmotif_event_dispatcher(XEvent *event)
{
    QApplication::sendPostedEvents();

    if (xt_grab && event->xany.display == xt_grab_display) {
	if (event->type == XFocusIn && event->xfocus.mode == NotifyWhileGrabbed) {
	    GDEBUG("Xt: grab moved to window 0x%lx (detail %d)",
		   event->xany.window, event->xfocus.detail);
	    xt_grab_focus_window = event->xany.window;
	} else {
	    if (event->type == XFocusOut && event->xfocus.mode == NotifyUngrab) {
		GDEBUG("Xt: grab ended for 0x%lx (detail %d)",
		       event->xany.window, event->xfocus.detail);
		xt_grab = false;
		xt_grab_focus_window = None;
                xt_grab_display = 0;
	    } else if (event->type == DestroyNotify
		       && event->xany.window == xt_grab_focus_window) {
		GDEBUG("Xt: grab window destroyed (0x%lx)", xt_grab_focus_window);
		xt_grab = false;
		xt_grab_focus_window = None;
                xt_grab_display = 0;
	    }
	}
    }

    QWidgetMapper *mapper = &static_d->mapper;
    QWidgetMapper::Iterator it = mapper->find(event->xany.window);
    QWidget *widget = it == mapper->end() ? 0 : *it;
    if (!widget && QWidget::find(event->xany.window) == 0) {
	if (!xt_grab
            && (event->type == XFocusIn
                && (event->xfocus.mode == NotifyGrab
                    || event->xfocus.mode == NotifyWhileGrabbed))
            && (event->xfocus.detail != NotifyPointer
                && event->xfocus.detail != NotifyPointerRoot)) {
            GDEBUG("Xt: grab started for 0x%lx (detail %d)",
                   event->xany.window, event->xfocus.detail);
	    xt_grab = true;
	    xt_grab_focus_window = event->xany.window;
            xt_grab_display = event->xany.display;
	}

	// event is not for Qt, try Xt
	Widget w = XtWindowToWidget(QMotif::display(), event->xany.window);

	while (w && (it = mapper->find(XtWindow(w))) == mapper->end()) {
	    if (XtIsShell(w)) {
		break;
	    }
	    w = XtParent(w);
	}
	widget = it != mapper->end() ? *it : 0;

 	if (widget && (event->type == XKeyPress ||
                       event->type == XKeyRelease))  {
	    // remap key events to keep accelerators working
 	    event->xany.window = widget->winId();
 	}
    }

    bool delivered = false;
    if (event->xany.display == QX11Info::display()) {
	/*
	  If the mouse has been grabbed for a window that we don't know
	  about, we shouldn't deliver any pointer events, since this will
	  intercept the event that ends the mouse grab that Xt/Motif
	  started.
	*/
	bool do_deliver = true;
	if (xt_grab && (event->type == ButtonPress   ||
                        event->type == ButtonRelease ||
                        event->type == MotionNotify  ||
                        event->type == EnterNotify   ||
                        event->type == LeaveNotify))
	    do_deliver = false;

	last_xevent = event;
	delivered = do_deliver && (qApp->x11ProcessEvent(event) != -1);
	last_xevent = 0;
	if (widget) {
	    switch (event->type) {
	    case EnterNotify:
	    case LeaveNotify:
		event->xcrossing.focus = False;
		delivered = false;
		break;
	    case XKeyPress:
	    case XKeyRelease:
		delivered = true;
		break;
	    case XFocusIn:
	    case XFocusOut:
		delivered = false;
		break;
	    default:
		delivered = false;
		break;
	    }
	}
    }

    if (delivered) return True;

    // discard user input events when we have an active popup widget
    if (QApplication::activePopupWidget()) {
	switch (event->type) {
	case ButtonPress:			// disallow mouse/key events
	case ButtonRelease:
	case MotionNotify:
	case XKeyPress:
	case XKeyRelease:
	case EnterNotify:
	case LeaveNotify:
	case ClientMessage:
	    EDEBUG("Qt: active popup discarded event, type %d", event->type);
	    return True;

	default:
	    break;
	}
    }

    if (! xt_grab && QApplication::activeModalWidget()) {
	if (widget) {
	    // send event through Qt modality handling...
	    if (!qt_try_modal(widget, event)) {
		EDEBUG("Qt: active modal widget discarded event, type %d", event->type);
		return True;
	    }
	} else {
	    // we could have a pure Xt shell as a child of the active modal widget
	    Widget xw = XtWindowToWidget(QMotif::display(), event->xany.window);
	    while (xw && (it = mapper->find(XtWindow(xw))) == mapper->end())
		xw = XtParent(xw);

	    QWidget *qw = it != mapper->end() ? *it : 0;
	    while (qw && qw != QApplication::activeModalWidget())
		qw = qw->parentWidget();

	    if (!qw) {
		// event is destined for an Xt widget, but since Qt has an
		// active modal widget, we stop here...
		switch (event->type) {
		case ButtonPress:			// disallow mouse/key events
		case ButtonRelease:
		case MotionNotify:
		case XKeyPress:
		case XKeyRelease:
		case EnterNotify:
		case LeaveNotify:
		case ClientMessage:
		    EDEBUG("Qt: active modal widget discarded event, type %d", event->type);
		    return True;
		default:
		    break;
		}
	    }
	}
    }

    // make click-to-focus work with QMotifWidget children
    if (!xt_grab && event->type == ButtonPress) {
	QWidget *qw = 0;
	Widget xw = XtWindowToWidget(QMotif::display(), event->xany.window);
	while (xw && (it = mapper->find(XtWindow(xw))) != mapper->end()) {
	    qw = *it;
	    xw = XtParent(xw);
	}

	if (qw && !qw->hasFocus() && (qw->focusPolicy() & Qt::ClickFocus))
	    qw->setFocus();
    }

    Q_ASSERT(static_d->dispatchers.find(event->xany.display) != static_d->dispatchers.end());
    return static_d->dispatchers[event->xany.display][event->type](event);
}



/*!
    \class QMotif
    \brief The QMotif class provides the basis of the Motif Extension.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \extension Motif

    QMotif only provides a few public functions, but it is at the
    heart of the integration. QMotif is responsible for initializing
    the Xt toolkit and the Xt application context. It does not open a
    connection to the X server, that is done by QApplication.

    The only member function in QMotif that depends on an X server
    connection is QMotif::initialize(). QMotif must be created before
    QApplication.

    Example usage of QMotif and QApplication:
    \code
    static char *resources[] = {
	...
    };

    int main(int argc, char **argv)
    {
	QMotif integrator("AppClass");
	XtAppSetFallbackResources(integrator.applicationContext(),
				   resources);
	QApplication app(argc, argv);

	...

	return app.exec();
    }
    \endcode
*/

/*!
    Creates QMotif, which allows Qt and Xt/Motif integration.

    If \a context is 0, QMotif creates a default application context
    itself. The context is accessible through applicationContext().

    All arguments passed to this function (\a applicationClass, \a
    options and \a numOptions) are used to call XtDisplayInitialize()
    after QApplication has been constructed.
*/
QMotif::QMotif(const char *applicationClass, XtAppContext context,
		XrmOptionDescRec *options , int numOptions)
    : QAbstractEventDispatcher(*new QMotifPrivate, 0)
{
    Q_D(QMotif);
    XtToolkitInitialize();
    if (context)
	d->appContext = context;
    else
	d->ownContext = d->appContext = XtCreateApplicationContext();
    d->applicationClass = applicationClass;
    d->options = options;
    d->numOptions = numOptions;
}


/*!
    Destroys QMotif.
*/
QMotif::~QMotif()
{ }

/*!
    Returns the application context.
*/
XtAppContext QMotif::applicationContext() const
{
    Q_D(const QMotif);
    return d->appContext;
}

/*!
    Returns the X11 display connection used by the Qt Motif Extension.
*/
Display *QMotif::display()
{
    return static_d->display;
}

/*!\internal
 */
XEvent* QMotif::lastEvent()
{
    return last_xevent;
}

/*!\internal
 */
void QMotif::registerWidget(QWidget* w)
{
    if (!static_d)
	return;
    static_d->mapper.insert(w->winId(), w);
}

/*!\internal
 */
void QMotif::unregisterWidget(QWidget* w)
{
    if (!static_d)
	return;
    static_d->mapper.remove(w->winId());
}

/*! \internal
  Redeliver the given XEvent to Xt.

  Rationale: An XEvent handled by Qt does not go through the Xt event
  handlers, and the internal state of Xt/Motif widgets will not be
  updated. This function should only be used if an event delivered by
  Qt to a QWidget needs to be sent to an Xt/Motif widget.
*/
bool QMotif::redeliverEvent(XEvent *event)
{
    // redeliver the event to Xt, NOT through Qt
    return (static_d->dispatchers[event->xany.display][event->type](event));
};

/*! \reimp
 */
bool QMotif::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QMotif);

    emit awake();

    // Qt uses posted events to do lots of delayed operations, like
    // repaints... these need to be delivered before we go to sleep
    QApplication::sendPostedEvents();

    QThreadData *data = QThreadData::get(thread());
    const bool canWait = (!data->postEventList.isEmpty()
                          && !d->interrupt
                          && (flags & QEventLoop::WaitForMoreEvents));

    XtInputMask allowedMask = XtIMAll;
    if (flags & QEventLoop::ExcludeSocketNotifiers)
        allowedMask &= ~XtIMAlternateInput;
    if (flags & 0x08) // 0x08 == ExcludeTimers for X11 only
        allowedMask &= ~XtIMTimer;

    // get the pending event mask from Xt and process the next event
    XtInputMask pendingMask = XtAppPending(d->appContext);
    pendingMask &= allowedMask;
    if (pendingMask & XtIMTimer) {
	pendingMask &= ~XtIMTimer;
	// zero timers will starve the Xt X event dispatcher... so
	// process something *instead* of a timer first...
	if (pendingMask != 0)
	    XtAppProcessEvent(d->appContext, pendingMask);
	// and process a timer afterwards
	pendingMask = XtIMTimer;
    }

    if (canWait) {
        emit aboutToBlock();
	XtAppProcessEvent(d->appContext, allowedMask);
    } else if (pendingMask != 0) {
	XtAppProcessEvent(d->appContext, pendingMask);
    }

    return true;
}

/*! \reimp
 */
bool QMotif::hasPendingEvents()
{
    Q_D(QMotif);
    QThreadData *data = QThreadData::get(thread());
    return (!data->postEventList.isEmpty() || XtAppPending(d->appContext) != 0);
}

/*! \internal
 */
void qmotif_socket_notifier_handler(XtPointer, int *, XtInputId *id)
{
    QSocketNotifier *notifier = static_d->socketNotifierHash.value(*id);
    if (!notifier)
        return; // shouldn't happen
    QEvent event(QEvent::SockAct);
    QApplication::sendEvent(notifier, &event);
}

/*! \reimp
 */
void QMotif::registerSocketNotifier(QSocketNotifier *notifier)
{
    Q_D(QMotif);

    XtInputMask mask;
    switch (notifier->type()) {
    case QSocketNotifier::Read:
	mask = XtInputReadMask;
	break;
    case QSocketNotifier::Write:
	mask = XtInputWriteMask;
	break;
    case QSocketNotifier::Exception:
	mask = XtInputExceptMask;
	break;
    default:
	qWarning("QMotifEventLoop: socket notifier has invalid type");
	return;
    }

    XtInputId id = XtAppAddInput(d->appContext, notifier->socket(), (XtPointer) mask,
				 qmotif_socket_notifier_handler, this);
    d->socketNotifierHash.insert(id, notifier);
}

/*! \reimp
 */
void QMotif::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    Q_D(QMotif);
    SocketNotifierHash::Iterator it = d->socketNotifierHash.begin();
    while (*it != notifier) ++it;
    if (*it != notifier) { // this shouldn't happen
	qWarning("QMotifEventLoop: failed to unregister socket notifier");
	return;
    }

    XtRemoveInput(it.key());
    d->socketNotifierHash.erase(it);
}

/*! \internal
 */
void qmotif_timer_handler(XtPointer pointer, XtIntervalId *)
{
    QObject *object = reinterpret_cast<QObject *>(pointer);
    if (!object)
        return; // shouldn't happen
    QEvent event(QEvent::Timer);
    QApplication::sendEvent(object, &event);
}

/*! \reimp
 */
int QMotif::registerTimer(int interval, QObject *object)
{
    Q_D(QMotif);
    XtIntervalId id = XtAppAddTimeOut(d->appContext, interval, qmotif_timer_handler, object);
    d->timerHash.insert(id, object);
    return id;
}

/*! \reimp
 */
bool QMotif::unregisterTimer(int timerId)
{
    Q_D(QMotif);
    XtRemoveTimeOut(timerId);
    d->timerHash.remove(timerId);
    return true;
}

/*! \reimp
 */
bool QMotif::unregisterTimers(QObject *object)
{
    Q_D(QMotif);
    TimerHash::iterator it = d->timerHash.begin();
    const TimerHash::iterator end = d->timerHash.end();
    for (; it != end; ++it) {
        if (it.value() == object) {
            XtRemoveTimeOut(it.key());
            it = d->timerHash.erase(it);
        }
    }
    return true;
}

/*! \internal
 */
static void qmotif_wakeup_handler(XtPointer, XtIntervalId *id)
{ XtRemoveTimeOut(*id); }

/*! \reimp
 */
void QMotif::wakeUp()
{
    Q_D(QMotif);
    if (d->appContext) {
        // start a zero timer to for the Xt to wake up
        XtAppAddTimeOut(d->appContext, 0, qmotif_wakeup_handler, 0);
    }
}

/*! \reimp
 */
void QMotif::interrupt()
{
    Q_D(QMotif);
    d->interrupt = true;
    wakeUp();
}

/*! \reimp
 */
void QMotif::flush()
{
    Q_D(QMotif);
    XFlush(d->display);
    XFlush(QX11Info::display());
}

/*! \reimp
 */
void QMotif::startingUp()
{
    Q_D(QMotif);
    /*
      QApplication could be using a Display from an outside source, so
      we should only initialize the display if the current application
      context does not contain the QApplication display
    */
    bool display_found = false;
    Display **displays;
    Cardinal x, count;
    XtGetDisplays(d->appContext, &displays, &count);
    for (x = 0; x < count && ! display_found; ++x) {
	if (displays[x] == QX11Info::display())
	    display_found = true;
    }
    if (displays)
	XtFree((char *) displays);

    int argc;
    argc = qApp->argc();
    char **argv = new char*[argc];
    QByteArray appName = qApp->objectName().toLatin1();

    if (! display_found) {
    	argc = qApp->argc();
        for (int i = 0; i < argc; ++i)
            argv[i] = qApp->argv()[i];

	XtDisplayInitialize(d->appContext,
                            QX11Info::display(),
                            appName.constData(),
                            d->applicationClass,
                            d->options,
                            d->numOptions,
                            &argc,
                            argv);
    }

    // open a second connection to the X server... QMotifWidget and
    // QMotifDialog will use this connection to create their wrapper
    // shells, which will allow for Motif<->Qt clipboard operations
    d->display = XOpenDisplay(DisplayString(QX11Info::display()));
    if (!d->display) {
	qWarning("%s: (QMotif) cannot create second connection to X server '%s'",
		 qApp->argv()[0], DisplayString(QX11Info::display()));
	::exit(1);
    }

    argc = qApp->argc();
    for (int i = 0; i < argc; ++i)
        argv[i] = qApp->argv()[i];

    XtDisplayInitialize(d->appContext,
			d->display,
			appName.constData(),
			d->applicationClass,
			d->options,
			d->numOptions,
			&argc,
                        argv);
    XSync(d->display, False);

    delete [] argv;

    // setup event dispatchers
    d->hookMeUp();
}

/*! \reimp
 */
void QMotif::closingDown()
{
    Q_D(QMotif);
    d->unhook();
    XtCloseDisplay(d->display);
    d->display = 0;
}
