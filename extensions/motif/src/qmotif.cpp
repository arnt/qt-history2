#include "qmotif.h"
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

class QMotifPrivate
{
public:
    QMotifPrivate();

    void hookMeUp();
    void unhook();

    XtAppContext appContext, ownContext;
    QMemArray<XtEventDispatchProc> dispatchers;
    QWidgetIntDict mapper;

    QIntDict<QSocketNotifier> socknotDict;
    bool activate_timers;
    int timerid;

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
    if ( static_d->dispatchers[ event->type ]( event ) )
	return TRUE;
    return FALSE;
};


/*!\internal
 */
XEvent* QMotif::lastEvent()
{
    return last_xevent;
}


QMotifPrivate::QMotifPrivate()
    : appContext(NULL), ownContext(NULL),
      activate_timers(FALSE), timerid(-1)
{
}

void QMotifPrivate::hookMeUp()
{
    // worker to plug Qt into Xt (event dispatchers)
    // and Xt into Qt (QMotifEventLoop)

    // ### TODO extensions?
    dispatchers.resize( LASTEvent );
    dispatchers.fill( 0 );
    int et;
    for ( et = 2; et < LASTEvent; et++ )
	dispatchers[ et ] =
	    XtSetEventDispatcher( QPaintDevice::x11AppDisplay(),
				  et, ::qmotif_event_dispatcher );
}

void QMotifPrivate::unhook()
{
    // unhook Qt from Xt (event dispatchers)
    // unhook Xt from Qt? (QMotifEventLoop)

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
    QApplication::sendPostedEvents();

    QWidgetIntDict *mapper = &static_d->mapper;
    QWidget* qMotif = mapper->find( event->xany.window );
    if ( !qMotif && QWidget::find( event->xany.window) == 0 ) {
	// event is not for Qt, try Xt
	Display* dpy = QPaintDevice::x11AppDisplay();
	Widget w = XtWindowToWidget( dpy, event->xany.window );
	while ( w && ! ( qMotif = mapper->find( XtWindow( w ) ) ) ) {
	    if ( XtIsShell( w ) ) {
		break;
	    }
	    w = XtParent( w );
	}

	if ( qMotif &&
	     ( event->type == XKeyPress || event->type == XKeyRelease ) )  {
	    // remap key events
	    event->xany.window = qMotif->winId();
	}
    }

    last_xevent = event;
    bool delivered = ( qApp->x11ProcessEvent( event ) != -1 );
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

    if ( delivered )
	return True;


    if ( QApplication::activePopupWidget() )
	// we get all events through the popup grabs.  discard the event
	return True;

    if ( qMotif && QApplication::activeModalWidget() ) {
	if ( !qt_try_modal(qMotif, event) )
	    return True;

    }

    if ( static_d->dispatchers[ event->type ]( event ) )
	// Xt handled the event.
	return True;

    return False;
}



/*!
    \class QMotif
    \brief The QMotif class provides the basis of the Motif Extension.

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



QMotif::QMotif( const char *applicationClass, XtAppContext context, XrmOptionDescRec *options , int numOptions)
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
  //   d->unhook();
    delete d;
}

/*!
    Returns the application context.
*/
XtAppContext QMotif::applicationContext() const
{
    return d->appContext;
}


void QMotif::appStartingUp()
{
    int argc = qApp->argc();
    XtDisplayInitialize( d->appContext,
			 QPaintDevice::x11AppDisplay(),
			 qApp->name(),
			 d->applicationClass,
			 d->options,
			 d->numOptions,
			 &argc,
			 qApp->argv() );
    d->hookMeUp();
}

void QMotif::appClosingDown()
{
    d->unhook();
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
    static_d->mapper.remove( w->winId() );
}


/*! \internal
 */
void qmotif_socknot_handler( XtPointer pointer, int *, XtInputId *id )
{
    QMotif *eventloop = (QMotif *) pointer;
    QSocketNotifier *socknot = static_d->socknotDict.find( *id );
    if ( ! socknot ) // this shouldn't happen
	return;
    eventloop->setSocketNotifierPending( socknot );
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

    XtInputId id = XtAppAddInput( d->appContext,
				  notifier->socket(), (XtPointer) mask,
				  qmotif_socknot_handler, this );
    d->socknotDict.insert( id, notifier );

    QEventLoop::registerSocketNotifier( notifier );
}

/*! \reimp
 */
void QMotif::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    QIntDictIterator<QSocketNotifier> it( d->socknotDict );
    while ( it.current() && notifier != it.current() )
	++it;
    if ( ! it.current() ) {
	// this shouldn't happen
	qWarning( "QMotifEventLoop: failed to unregister socket notifier" );
	return;
    }

    XtRemoveInput( it.currentKey() );
    d->socknotDict.remove( it.currentKey() );

    QEventLoop::unregisterSocketNotifier( notifier );
}

/*! \internal
 */
void qmotif_timeout_handler( XtPointer, XtIntervalId * )
{
    static_d->activate_timers = TRUE;
    static_d->timerid = -1;
}

/*! \reimp
 */
bool QMotif::processEvents( ProcessEventsFlags flags )
{
    // Qt uses posted events to do lots of delayed operations, like repaints... these
    // need to be delivered before we go to sleep
    QApplication::sendPostedEvents();

    bool canWait = ( flags & WaitForMore );

    // make sure we fire off Qt's timers
    int ttw = timeToWait();
    if ( d->timerid != -1 ) {
	XtRemoveTimeOut( d->timerid );
    }
    d->timerid = -1;
    if ( ttw != -1 ) {
	d->timerid =
	    XtAppAddTimeOut( d->appContext, ttw,
			     qmotif_timeout_handler, 0 );
    }

    // get the pending event mask from Xt and process the next event
    XtInputMask pendingmask = XtAppPending( d->appContext );
    XtInputMask mask = pendingmask;
    if ( pendingmask & XtIMTimer ) {
	mask &= ~XtIMTimer;
	// zero timers will starve the Xt X event dispatcher... so process
	// something *instead* of a timer first...
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
    if ( ! ( flags & ExcludeSocketNotifiers ) )
	nevents += activateSocketNotifiers();

    if ( d->activate_timers ) {
	nevents += activateTimers();
    }
    d->activate_timers = FALSE;

    return ( canWait || ( pendingmask != 0 ) || nevents > 0 );
}
