/****************************************************************************
**
** Implementation of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Qt extension for Xt/Motif support.
** EDITIONS: ENTERPRISE
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

#include <qgc_x11.h>

#include "qmotif.h"

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

typedef QHash<ulong,QSocketNotifier*> SockNotMapper;

Boolean qmotif_event_dispatcher( XEvent *event );
static void qmotif_keep_alive();
void qmotif_timeout_handler( XtPointer, XtIntervalId * );

class QMotifPrivate
{
public:
    QMotifPrivate();

    void hookMeUp();
    void unhook();

    Display *display;
    XtAppContext appContext, ownContext;

    typedef QVector<XtEventDispatchProc> DispatcherArray;
    QHash<Display*,DispatcherArray> dispatchers;
    QWidgetMapper mapper;

    SockNotMapper sock_not_mapper;
    int pending_socknots;
    bool activate_timers;
    XtIntervalId timerid;

    // arguments for Xt display initialization
    const char* applicationClass;
    XrmOptionDescRec* options;
    int numOptions;
};
static QMotifPrivate *static_d = 0;
static XEvent* last_xevent = 0;


/*! \internal
  Redeliver the given XEvent to Xt.

  Rationale: An XEvent handled by Qt does not go through the Xt event
  handlers, and the internal state of Xt/Motif widgets will not be
  updated. This function should only be used if an event delivered by
  Qt to a QWidget needs to be sent to an Xt/Motif widget.
*/
bool QMotif::redeliverEvent( XEvent *event )
{
    // redeliver the event to Xt, NOT through Qt
    return (static_d->dispatchers[event->xany.display][event->type](event));
};


/*!\internal
 */
XEvent* QMotif::lastEvent()
{
    return last_xevent;
}


QMotifPrivate::QMotifPrivate()
    : appContext(NULL), ownContext(NULL),
      pending_socknots(0), activate_timers(FALSE), timerid(~0u)
{
}

void QMotifPrivate::hookMeUp()
{
    // worker to plug Qt into Xt (event dispatchers)
    // and Xt into Qt (QMotifEventLoop)

    // ### TODO extensions?
    DispatcherArray &qt_dispatchers = dispatchers[QX11GC::x11AppDisplay()];
    DispatcherArray &qm_dispatchers = dispatchers[display];

    qt_dispatchers.resize( LASTEvent );
    qt_dispatchers.fill( 0 );

    qm_dispatchers.resize( LASTEvent );
    qm_dispatchers.fill( 0 );

    int et;
    for ( et = 2; et < LASTEvent; et++ ) {
	qt_dispatchers[et] =
	    XtSetEventDispatcher(QX11GC::x11AppDisplay(), et, ::qmotif_event_dispatcher);
	qm_dispatchers[et] =
	    XtSetEventDispatcher(display, et, ::qmotif_event_dispatcher);
    }
}

void QMotifPrivate::unhook()
{
    // unhook Qt from Xt (event dispatchers)
    // unhook Xt from Qt? (QMotifEventLoop)

    // ### TODO extensions?
    DispatcherArray &qt_dispatchers = dispatchers[QX11GC::x11AppDisplay()];
    DispatcherArray &qm_dispatchers = dispatchers[display];

    for (int et = 2; et < LASTEvent; ++et) {
	(void) XtSetEventDispatcher(QX11GC::x11AppDisplay(), et, qt_dispatchers[et]);
	(void) XtSetEventDispatcher(display, et, qm_dispatchers[et]);
    }

    /*
      We cannot destroy the app context here because it closes the X
      display, something QApplication does as well a bit later.
      if ( ownContext )
          XtDestroyApplicationContext( ownContext );
     */
    appContext = ownContext = 0;
}

extern bool qt_try_modal( QWidget *, XEvent * ); // defined in qapplication_x11.cpp

static bool xt_grab = FALSE;
static Window xt_grab_focus_window = None;

Boolean qmotif_event_dispatcher( XEvent *event )
{
    qmotif_keep_alive();

    QApplication::sendPostedEvents();

    if (xt_grab) {
	if (event->type == XFocusIn && event->xfocus.mode == NotifyWhileGrabbed) {
	    GDEBUG("Xt: grab moved to window 0x%lx", event->xany.window);
	    xt_grab_focus_window = event->xany.window;
	} else {
	    if (event->type == XFocusOut && event->xfocus.mode == NotifyUngrab) {
		GDEBUG("Xt: grab ended for 0x%lx", event->xany.window);
		xt_grab = FALSE;
		xt_grab_focus_window = None;
	    } else if (event->type == DestroyNotify
		       && event->xany.window == xt_grab_focus_window) {
		GDEBUG("Xt: grab window destroyed (0x%lx)", xt_grab_focus_window);
		xt_grab = FALSE;
		xt_grab_focus_window = None;
	    }
	}
    }

    QWidgetMapper *mapper = &static_d->mapper;
    QWidgetMapper::Iterator it = mapper->find(event->xany.window);
    QWidget *widget = it == mapper->end() ? 0 : *it;
    if ( !widget && QWidget::find( event->xany.window) == 0 ) {
	if (!xt_grab && (event->type == XFocusIn && event->xfocus.mode == NotifyGrab)) {
	    GDEBUG("Xt: grab started for 0x%lx", event->xany.window);
	    xt_grab = TRUE;
	    xt_grab_focus_window = event->xany.window;
	}

	// event is not for Qt, try Xt
	Widget w = XtWindowToWidget(QMotif::x11Display(), event->xany.window);

	while (w && (it = mapper->find(XtWindow(w))) == mapper->end()) {
	    if ( XtIsShell( w ) ) {
		break;
	    }
	    w = XtParent( w );
	}
	widget = it != mapper->end() ? *it : 0;

 	if ( widget && ( event->type == XKeyPress ||
			 event->type == XKeyRelease ) )  {
	    // remap key events to keep accelerators working
 	    event->xany.window = widget->winId();
 	}
    }

    bool delivered = FALSE;
    if (widget || event->xany.display == QX11GC::x11AppDisplay()) {
	/*
	  If the mouse has been grabbed for a window that we don't know
	  about, we shouldn't deliver any pointer events, since this will
	  intercept the event that ends the mouse grab that Xt/Motif
	  started.
	*/
	bool do_deliver = TRUE;
	if ( xt_grab && ( event->type == ButtonPress   ||
			  event->type == ButtonRelease ||
			  event->type == MotionNotify  ||
			  event->type == EnterNotify   ||
			  event->type == LeaveNotify ) )
	    do_deliver = FALSE;

	last_xevent = event;
	delivered = do_deliver && ( qApp->x11ProcessEvent( event ) != -1 );
	last_xevent = 0;
	if ( widget ) {
	    switch ( event->type ) {
	    case EnterNotify:
	    case LeaveNotify:
		event->xcrossing.focus = False;
		delivered = FALSE;
		break;
	    case XKeyPress:
	    case XKeyRelease:
		delivered = TRUE;
		break;
	    case XFocusIn:
	    case XFocusOut:
		delivered = FALSE;
		break;
	    default:
		delivered = FALSE;
		break;
	    }
	}
    }

    if (delivered) return True;

    // discard user input events when we have an active popup widget
    if ( QApplication::activePopupWidget() ) {
	switch ( event->type ) {
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

    if ( ! xt_grab && QApplication::activeModalWidget() ) {
	if ( widget ) {
	    // send event through Qt modality handling...
	    if ( !qt_try_modal( widget, event ) ) {
		EDEBUG("Qt: active modal widget discarded event, type %d", event->type);
		return True;
	    }
	} else {
	    // we could have a pure Xt shell as a child of the active modal widget
	    Widget xw = XtWindowToWidget(QMotif::x11Display(), event->xany.window);
	    while (xw && (it = mapper->find(XtWindow(xw))) == mapper->end())
		xw = XtParent(xw);

	    QWidget *qw = it != mapper->end() ? *it : 0;
	    while (qw && qw != QApplication::activeModalWidget())
		qw = qw->parentWidget();

	    if (!qw) {
		// event is destined for an Xt widget, but since Qt has an
		// active modal widget, we stop here...
		switch ( event->type ) {
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
	Widget xw = XtWindowToWidget(QMotif::x11Display(), event->xany.window);
	while (xw && (it = mapper->find(XtWindow(xw))) != mapper->end()) {
	    qw = *it;
	    xw = XtParent(xw);
	}

	if (qw && !qw->hasFocus() && (qw->focusPolicy() & QWidget::ClickFocus))
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
	QMotif integrator( "AppClass" );
	XtAppSetFallbackResources( integrator.applicationContext(),
				   resources );
	QApplication app( argc, argv );

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
QMotif::QMotif( const char *applicationClass, XtAppContext context,
		XrmOptionDescRec *options , int numOptions)
    : QGuiEventLoop()
{
#if defined(QT_CHECK_STATE)
    if ( static_d )
	qWarning( "QMotif: should only have one QMotif instance!" );
#endif

    d = static_d = new QMotifPrivate;
    XtToolkitInitialize();
    if ( context )
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
{
    delete d;
    static_d = 0;
}

/*!
    Returns the X11 display connection used by the Qt Motif Extension.
*/
Display *QMotif::x11Display()
{
    return static_d->display;
}

/*!
    Returns the application context.
*/
XtAppContext QMotif::applicationContext() const
{
    return d->appContext;
}

/*! \reimp
 */
void QMotif::appStartingUp()
{
    /*
      QApplication could be using a Display from an outside source, so
      we should only initialize the display if the current application
      context does not contain the QApplication display
    */
    bool display_found = FALSE;
    Display **displays;
    Cardinal x, count;
    XtGetDisplays( d->appContext, &displays, &count );
    for ( x = 0; x < count && ! display_found; ++x ) {
	if ( displays[x] == QX11GC::x11AppDisplay() )
	    display_found = TRUE;
    }
    if ( displays )
	XtFree( (char *) displays );

    int argc;
    if ( ! display_found ) {
	argc = qApp->argc();
	qDebug("XtDisplayInitialize: %p %p %s",
	       d->appContext, QX11GC::x11AppDisplay(), qApp->objectName());
	XtDisplayInitialize( d->appContext,
			     QX11GC::x11AppDisplay(),
			     qApp->objectName(),
			     d->applicationClass,
			     d->options,
			     d->numOptions,
			     &argc,
			     qApp->argv() );
    }

    // open a second connection to the X server... QMotifWidget and
    // QMotifDialog will use this connection to create their wrapper
    // shells, which will allow for Motif<->Qt clipboard operations
    d->display = XOpenDisplay(DisplayString(QX11GC::x11AppDisplay()));
    if (!d->display) {
	qWarning("%s: (QMotif) cannot create second connection to X server '%s'",
		 qApp->argv()[0], DisplayString(QX11GC::x11AppDisplay()));
	::exit( 1 );
    }

    argc = qApp->argc();
    XtDisplayInitialize(d->appContext,
			d->display,
			qApp->objectName(),
			d->applicationClass,
			d->options,
			d->numOptions,
			&argc,
			qApp->argv());
    XSync(d->display, False);

    // setup event dispatchers
    d->hookMeUp();

    // start a zero-timer to get the timer keep-alive working
    d->timerid = XtAppAddTimeOut( d->appContext, 0, qmotif_timeout_handler, 0 );
}

/*! \reimp
 */
void QMotif::appClosingDown()
{
    if ( d->timerid != ~0u )
	XtRemoveTimeOut( d->timerid );
    d->timerid = ~0u;

    d->unhook();

    XtCloseDisplay(d->display);
    d->display = 0;
}

/*!\internal
 */
void QMotif::registerWidget( QWidget* w )
{
    if ( !static_d )
	return;
    static_d->mapper.insert( w->winId(), w );
}


/*!\internal
 */
void QMotif::unregisterWidget( QWidget* w )
{
    if ( !static_d )
	return;
    static_d->mapper.erase( w->winId() );
}


/*! \internal
 */
void qmotif_socknot_handler(XtPointer pointer, int *, XtInputId *id)
{
    SockNotMapper::Iterator it = static_d->sock_not_mapper.find(*id);
    if (it == static_d->sock_not_mapper.end()) // this shouldn't happen
	return;

    QMotif *eventloop = (QMotif *) pointer;
    eventloop->setSocketNotifierPending(*it);
    if (++static_d->pending_socknots > static_d->sock_not_mapper.size()) {
	/*
	  We have too many pending socket notifiers.  Since Xt prefers
	  socket notifiers over X events, we should go ahead and
	  activate all our pending socket notifiers so that the event
	  loop doesn't freeze up because of this.
	*/
	eventloop->activateSocketNotifiers();
	static_d->pending_socknots = 0;
    }
}

/*! \reimp
 */
void QMotif::registerSocketNotifier( QSocketNotifier *notifier )
{
    XtInputMask mask;
    switch ( notifier->type() ) {
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
	qWarning( "QMotifEventLoop: socket notifier has invalid type" );
	return;
    }

    XtInputId id = XtAppAddInput(d->appContext, notifier->socket(), (XtPointer) mask,
				 qmotif_socknot_handler, this);
    d->sock_not_mapper.insert(id, notifier);

    QGuiEventLoop::registerSocketNotifier(notifier);
}

/*! \reimp
 */
void QMotif::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    SockNotMapper::Iterator it = d->sock_not_mapper.begin();
    while (*it != notifier) ++it;
    if (*it != notifier) { // this shouldn't happen
	qWarning( "QMotifEventLoop: failed to unregister socket notifier" );
	return;
    }

    XtRemoveInput( it.key() );
    d->sock_not_mapper.erase( it.key() );

    QGuiEventLoop::unregisterSocketNotifier( notifier );
}

/*! \internal

    helper function to keep timer delivery working
 */
static void qmotif_keep_alive() {
    // make sure we fire off Qt's timers
    int ttw = QApplication::eventLoop()->timeToWait();
    if ( static_d->timerid != ~0u )
	XtRemoveTimeOut( static_d->timerid );
    static_d->timerid = ~0u;
    if ( ttw != -1 ) {
	static_d->timerid =
	    XtAppAddTimeOut( static_d->appContext, ttw, qmotif_timeout_handler, 0 );
    }
}

/*! \internal
 */
void qmotif_timeout_handler( XtPointer, XtIntervalId * )
{
    static_d->timerid = ~0u;

    if ( ! QApplication::eventLoop()->loopLevel() ) {
	/*
	  when the Qt eventloop is not running, make sure that Qt
	  timers still work with an Xt keep-alive timer
	*/
	QApplication::eventLoop()->activateTimers();
	QApplication::sendPostedEvents();
	static_d->activate_timers = FALSE;

	qmotif_keep_alive();
    } else {
	static_d->activate_timers = TRUE;
    }
}

/*! \reimp
 */
bool QMotif::processEvents( ProcessEventsFlags flags )
{
    // Qt uses posted events to do lots of delayed operations, like
    // repaints... these need to be delivered before we go to sleep
    QApplication::sendPostedEvents();

    bool canWait = ( flags & WaitForMore );

    qmotif_keep_alive();

    // get the pending event mask from Xt and process the next event
    XtInputMask pendingmask = XtAppPending( d->appContext );
    XtInputMask mask = pendingmask;
    if ( pendingmask & XtIMTimer ) {
	mask &= ~XtIMTimer;
	// zero timers will starve the Xt X event dispatcher... so
	// process something *instead* of a timer first...
	if ( mask != 0 )
	    XtAppProcessEvent( d->appContext, mask );
	// and process a timer afterwards
	mask = pendingmask & XtIMTimer;
    }

    if ( canWait )
	XtAppProcessEvent( d->appContext, XtIMAll );
    else
	XtAppProcessEvent( d->appContext, mask );

    int nevents = 0;
    if ( ! ( flags & ExcludeSocketNotifiers ) ) {
	nevents += activateSocketNotifiers();
	d->pending_socknots = 0;
    }

    if ( ! ( flags & 0x08 ) && d->activate_timers ) {
	// 0x08 == ExcludeTimers for X11 only
	nevents += activateTimers();
	d->activate_timers = FALSE;
    }

    return ( canWait || ( pendingmask != 0 ) || nevents > 0 );
}
