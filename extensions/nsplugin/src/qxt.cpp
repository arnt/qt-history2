/****************************************************************************
** $Id$
**
** Implementation of Qt extension classes for Netscape Plugin Support.
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt extension for Netscape Plugin Support.
**
** Licensees holding valid Qt Enterprise Edition licenses for X11 may use
** this file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qxt.h"

#include <qapplication.h>
#include <qwidgetintdict.h>

// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease

Boolean qmotif_event_dispatcher( XEvent *event );
static void qmotif_keep_alive();
void qmotif_timeout_handler( XtPointer, XtIntervalId * );

class QXtPrivate
{
public:
    QXtPrivate();

    void hookMeUp();
    void unhook();

    XtAppContext appContext, ownContext;
    QMemArray<XtEventDispatchProc> dispatchers;
    QWidgetIntDict mapper;

    QIntDict<QSocketNotifier> socknotDict;
    uint pending_socknots;
    bool activate_timers;
    int timerid;

    // arguments for Xt display initialization
    const char* applicationClass;
    XrmOptionDescRec* options;
    int numOptions;
};
static QXtPrivate *static_d = 0;
static XEvent* last_xevent = 0;


/*! \internal
  Redeliver the given XEvent to Xt.

  Rationale: An XEvent handled by Qt does not go through the Xt event
  handlers, and the internal state of Xt/Motif widgets will not be
  updated. This function should only be used if an event delivered by
  Qt to a QWidget needs to be sent to an Xt/Motif widget.
*/
bool QXt::redeliverEvent( XEvent *event )
{
    // redeliver the event to Xt, NOT through Qt
    if ( static_d->dispatchers[ event->type ]( event ) ) {
	// qDebug( "Xt: redelivered event" );
	return TRUE;
    }
    return FALSE;
};


/*!\internal
 */
XEvent* QXt::lastEvent()
{
    return last_xevent;
}


QXtPrivate::QXtPrivate()
    : appContext(NULL), ownContext(NULL),
      pending_socknots(0), activate_timers(FALSE), timerid(-1)
{
}

void QXtPrivate::hookMeUp()
{
    // worker to plug Qt into Xt (event dispatchers)
    // and Xt into Qt (QXtEventLoop)

    // ### TODO extensions?
    dispatchers.resize( LASTEvent );
    dispatchers.fill( 0 );
    int et;
    for ( et = 2; et < LASTEvent; et++ )
	dispatchers[ et ] =
	    XtSetEventDispatcher( QPaintDevice::x11AppDisplay(),
				  et, ::qmotif_event_dispatcher );
}

void QXtPrivate::unhook()
{
    // unhook Qt from Xt (event dispatchers)
    // unhook Xt from Qt? (QXtEventLoop)

    // ### TODO extensions?
    int et;
    for ( et = 2; et < LASTEvent; et++ )
	(void) XtSetEventDispatcher( QPaintDevice::x11AppDisplay(),
				     et, dispatchers[ et ] );
    dispatchers.resize( 0 );

    /*
      We cannot destroy the app context here because it closes the X
      display, something QApplication does as well a bit later.
      if ( ownContext )
          XtDestroyApplicationContext( ownContext );
     */
    appContext = ownContext = 0;
}

extern bool qt_try_modal( QWidget *, XEvent * ); // defined in qapplication_x11.cpp
Boolean qmotif_event_dispatcher( XEvent *event )
{
    static bool grabbed = FALSE;

    QApplication::sendPostedEvents();

    QWidgetIntDict *mapper = &static_d->mapper;
    QWidget* qMotif = mapper->find( event->xany.window );
    if ( !qMotif && QWidget::find( event->xany.window) == 0 ) {
	// event is not for Qt, try Xt
	Widget w = XtWindowToWidget( QPaintDevice::x11AppDisplay(),
				     event->xany.window );
	while ( w && ! ( qMotif = mapper->find( XtWindow( w ) ) ) ) {
	    if ( XtIsShell( w ) ) {
		break;
	    }
	    w = XtParent( w );
	}

 	if ( qMotif && ( event->type == XKeyPress ||
			 event->type == XKeyRelease ) )  {
	    // remap key events to keep accelerators working
 	    event->xany.window = qMotif->winId();
 	}

	if ( w ) {
	    if ( !grabbed && ( event->type        == XFocusIn &&
			       event->xfocus.mode == NotifyGrab ) ) {
		// qDebug( "Xt: grab started" );
		grabbed = TRUE;
	    } else if ( grabbed && ( event->type        == XFocusOut &&
				     event->xfocus.mode == NotifyUngrab ) ) {
		// qDebug( "Xt: grab ended" );
		grabbed = FALSE;
	    }
	}
    }

    /*
      If the mouse has been grabbed for a window that we don't know
      about, we shouldn't deliver any pointer events, since this will
      intercept the event that ends the mouse grab that Xt/Motif
      started.
    */
    bool do_deliver = TRUE;
    if ( grabbed && ( event->type == ButtonPress   ||
		      event->type == ButtonRelease ||
		      event->type == MotionNotify  ||
		      event->type == EnterNotify   ||
		      event->type == LeaveNotify ) )
	do_deliver = FALSE;

    last_xevent = event;
    bool delivered = do_deliver && ( qApp->x11ProcessEvent( event ) != -1 );
    last_xevent = 0;
    if ( qMotif ) {
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

    qmotif_keep_alive();

    if ( delivered ) {
	// qDebug( "Qt: delivered event" );
	return True;
    }

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
	    // qDebug( "Qt: active popup - discarding event" );
	    return True;

	default:
	    break;
	}
    }

    if ( QApplication::activeModalWidget() ) {
	if ( qMotif ) {
	    // send event through Qt modality handling...
	    if ( !qt_try_modal( qMotif, event ) ) {
		// qDebug( "Qt: active modal widget discarded event" );
		return True;
	    }
	} else if ( !grabbed ) {
	    // we could have a pure Xt shell as a child of the active
	    // modal widget
	    QWidget *qw = 0;
	    Widget xw = XtWindowToWidget( QPaintDevice::x11AppDisplay(),
					  event->xany.window );
	    while ( xw && !( qw = mapper->find( XtWindow( xw ) ) ) )
		xw = XtParent( xw );

	    while ( qw && qw != QApplication::activeModalWidget() )
		qw = qw->parentWidget();

	    if ( !qw ) {
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
		    // qDebug( "Qt: active modal widget discarded unknown event" );
		    return True;
		default:
		    break;
		}
	    }
	}
    }

    if ( static_d->dispatchers[ event->type ]( event ) ) {
	// qDebug( "Xt: delivered event" );
	// Xt handled the event.
	return True;
    }

    return False;
}



/*!
    \class QXt
    \brief The QXt class provides the basis of the Motif Extension.

    \extension Motif

    QXt only provides a few public functions, but it is at the
    heart of the integration. QXt is responsible for initializing
    the Xt toolkit and the Xt application context. It does not open a
    connection to the X server, that is done by QApplication.

    The only member function in QXt that depends on an X server
    connection is QXt::initialize(). QXt must be created before
    QApplication.

    Example usage of QXt and QApplication:
    \code
    static char *resources[] = {
	...
    };

    int main(int argc, char **argv)
    {
	QXt integrator( "AppClass" );
	XtAppSetFallbackResources( integrator.applicationContext(),
				   resources );
	QApplication app( argc, argv );

	...

	return app.exec();
    }
    \endcode
*/

/*!
    Creates QXt, which allows Qt and Xt/Motif integration.

    If \a context is 0, QXt creates a default application context
    itself. The context is accessible through applicationContext().

    All arguments passed to this function (\a applicationClass, \a
    options and \a numOptions) are used to call XtDisplayInitialize()
    after QApplication has been constructed.
*/
QXt::QXt( const char *applicationClass, XtAppContext context,
		XrmOptionDescRec *options , int numOptions)
{
#if defined(QT_CHECK_STATE)
    if ( static_d )
	qWarning( "QXt: should only have one QXt instance!" );
#endif

    d = static_d = new QXtPrivate;
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
    Destroys QXt.
*/
QXt::~QXt()
{
    delete d;
    static_d = 0;
}

/*!
    Returns the application context.
*/
XtAppContext QXt::applicationContext() const
{
    return d->appContext;
}


void QXt::appStartingUp()
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
	if ( displays[x] == QPaintDevice::x11AppDisplay() )
	    display_found = TRUE;
    }
    if ( displays )
	XtFree( (char *) displays );

    if ( ! display_found ) {
	int argc = qApp->argc();
	XtDisplayInitialize( d->appContext,
			     QPaintDevice::x11AppDisplay(),
			     qApp->name(),
			     d->applicationClass,
			     d->options,
			     d->numOptions,
			     &argc,
			     qApp->argv() );
    }

    d->hookMeUp();

    // start a zero-timer to get the timer keep-alive working
    d->timerid = XtAppAddTimeOut( d->appContext, 0, qmotif_timeout_handler, 0 );
}

void QXt::appClosingDown()
{
    if ( d->timerid != -1 )
	XtRemoveTimeOut( d->timerid );
    d->timerid = -1;

    d->unhook();
}


/*!\internal
 */
void QXt::registerWidget( QWidget* w )
{
    if ( !static_d )
	return;
    static_d->mapper.insert( w->winId(), w );
}


/*!\internal
 */
void QXt::unregisterWidget( QWidget* w )
{
    if ( !static_d )
	return;
    static_d->mapper.remove( w->winId() );
}


/*! \internal
 */
void qmotif_socknot_handler( XtPointer pointer, int *, XtInputId *id )
{
    QXt *eventloop = (QXt *) pointer;
    QSocketNotifier *socknot = static_d->socknotDict.find( *id );
    if ( ! socknot ) // this shouldn't happen
	return;
    eventloop->setSocketNotifierPending( socknot );
    if ( ++static_d->pending_socknots > static_d->socknotDict.count() ) {
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
void QXt::registerSocketNotifier( QSocketNotifier *notifier )
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
	qWarning( "QXtEventLoop: socket notifier has invalid type" );
	return;
    }

    XtInputId id = XtAppAddInput( d->appContext,
				  notifier->socket(), (XtPointer) mask,
				  qmotif_socknot_handler, this );
    d->socknotDict.insert( id, notifier );

    QEventLoop::registerSocketNotifier( notifier );
}

/*! \reimp
 */
void QXt::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    QIntDictIterator<QSocketNotifier> it( d->socknotDict );
    while ( it.current() && notifier != it.current() )
	++it;
    if ( ! it.current() ) {
	// this shouldn't happen
	qWarning( "QXtEventLoop: failed to unregister socket notifier" );
	return;
    }

    XtRemoveInput( it.currentKey() );
    d->socknotDict.remove( it.currentKey() );

    QEventLoop::unregisterSocketNotifier( notifier );
}

/*! \internal

    helper function to keep timer delivery working
 */
static void qmotif_keep_alive() {
    // make sure we fire off Qt's timers
    int ttw = QApplication::eventLoop()->timeToWait();
    if ( static_d->timerid != -1 )
	XtRemoveTimeOut( static_d->timerid );
    static_d->timerid = -1;
    if ( ttw != -1 ) {
	static_d->timerid =
	    XtAppAddTimeOut( static_d->appContext, ttw, qmotif_timeout_handler, 0 );
    }
}

/*! \internal
 */
void qmotif_timeout_handler( XtPointer, XtIntervalId * )
{
    static_d->timerid = -1;

    if ( ! QApplication::eventLoop()->loopLevel() ) {
	/*
	  when the Qt eventloop is not running, make sure that Qt
	  timers still work with an Xt keep-alive timer
	*/
	QApplication::eventLoop()->activateTimers();
	static_d->activate_timers = FALSE;

	qmotif_keep_alive();
    } else {
	static_d->activate_timers = TRUE;
    }
}

/*! \reimp
 */
bool QXt::processEvents( ProcessEventsFlags flags )
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

    if ( d->activate_timers ) {
	nevents += activateTimers();
    }
    d->activate_timers = FALSE;

    return ( canWait || ( pendingmask != 0 ) || nevents > 0 );
}
