#include "qmotif.h"
#include "qmotif_p.h"

#include <qapplication.h>

// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease


QMotifPrivate::QMotifPrivate()
    : appContext(NULL)
{
    eventloop = 0;
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
}


static QMotif *QMotif_INSTANCE = 0;

static XEvent* last_xevent = 0;


extern bool qt_try_modal( QWidget *, XEvent * ); // defined in qapplication_x11.cpp
Boolean qmotif_event_dispatcher( XEvent *event )
{
    QApplication::sendPostedEvents();

    QWidgetIntDict *mapper = QMotif::mapper();
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

    if ( QMotif_INSTANCE->d->dispatchers[ event->type ]( event ) )
	// Xt handled the event.
	return True;

    return False;
}


bool QMotif::dispatchQEvent( QEvent* e, QWidget* w)
{
    switch ( e->type() ) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
	if ( last_xevent ) {
	    last_xevent->xany.window = w->winId();
	    redeliverEvent( last_xevent );
	}
	break;
    case QEvent::FocusIn:
    {
	XFocusInEvent ev = { XFocusIn, 0, TRUE, w->x11Display(), w->winId(),
			       NotifyNormal, NotifyPointer  };
	redeliverEvent( (XEvent*)&ev );
	break;
    }
    case QEvent::FocusOut:
    {
	XFocusOutEvent ev = { XFocusOut, 0, TRUE, w->x11Display(), w->winId(),
			       NotifyNormal, NotifyPointer  };
	redeliverEvent( (XEvent*)&ev );
	break;
    }
    default:
	break;
    }
    return FALSE;
}

/*!
    \class QMotif
    \brief The QMotif class is the core behind the QMotif Extension.

    \extension QMotif

    QMotif only provides a few public functions, but is the brains
    behind the integration.  QMotif is responsible for initializing
    the Xt toolkit and the Xt application context.  It does not open a
    connection to the X server, this is done by using QApplication.

    The only member function in QMotif that depends on an X server
    connection is QMotif::initialize().	 QMotif can be created before
    or after QApplication.

    Example usage of QMotif and QApplication:

    \code
    static char *resources[] = {
	...
    };

    int main(int argc, char **argv)
    {
	QMotif integrator;

	XtAppSetFallbackResources( integrator.applicationContext(),
				   resources );

	QApplication app( argc, argv );
	integrator.initialize( &argc, argv, "AppClass", NULL, 0 );

	...


	int ret = app.exec();

	XtDestroyApplication( integrator.applicationContext() );
	integrator.setApplicationContext( 0 );

	return ret;
    }
    \endcode
*/

/*!
  Creates QMotif, which allows Qt and Xt/Motif integration.

  The Xt toolkit is initialized by this constructor by calling
  XtToolkitInitialize().
*/
QMotif::QMotif()
{
#if defined(QT_CHECK_STATE)
    if ( QMotif_INSTANCE )
	qWarning( "QMotif: should only have one QMotif instance!" );
#endif

    QMotif_INSTANCE = this;
    d = new QMotifPrivate;
    XtToolkitInitialize();
}


/*!
  Destroys QMotif.

  \warning The application context is not destroyed automatically.
  This must be done before destroying the QMotif instance.
*/
QMotif::~QMotif()
{
    d->unhook();
    delete d;
}

/*!
  Returns the application context.  If no application context has been
  set, then QMotif creates one.

  The applicaiton context is \e not destroyed in the QMotif destructor
  if QMotif created the application context.  The application
  programmer must do this before destroying the QMotif instance.
*/
XtAppContext QMotif::applicationContext() const
{
    if ( d->appContext == NULL )
	d->appContext = XtCreateApplicationContext();
    return d->appContext;
}

/*!
    Sets the application context to \a appContext.
*/
void QMotif::setApplicationContext( XtAppContext appContext )
{
#if defined(QT_CHECK_STATE)
    if ( d->appContext != NULL )
	qWarning( "QMotif: WARNING: only one application context should be used." );
#endif // QT_CHECK_STATE
    d->appContext = appContext;
}

/*!
    Initialize the application context. All arguments passed to this
    function (\a argc, \a argv, \a applicationClass, \a options and \a
    numOptions) are used to call XtDisplayInitialize().
*/
void QMotif::initialize( int *argc, char **argv, char *applicationClass,
			      XrmOptionDescRec *options, int numOptions )
{
    XtDisplayInitialize( applicationContext(),
			 QPaintDevice::x11AppDisplay(),
			 qApp->name(),
			 applicationClass,
			 options,
			 numOptions,
			 argc,
			 argv );
    d->hookMeUp();
}

/*! \internal
  Redeliver the given XEvent to Xt.  This is used by QMotifDialog and
  QMotifWidget.

  Rationale: An XEvent handled by Qt does not go through the Xt event
  handlers, and the internal state of Xt/Motif widgets will not be
  updated.  This function should only be used if an event delivered by
  Qt to a QWidget needs to be sent to an Xt/Motif widget.

  You should not need to call this function.
*/
bool QMotif::redeliverEvent( XEvent *event )
{
    // redeliver the event to Xt, NOT through Qt
    if ( QMotif_INSTANCE->d->dispatchers[ event->type ]( event ) )
	return TRUE;
    return FALSE;
};

/*! \internal
  Returns the the Xt/Motif to QWidget mapper.  This mapper is used for
  delivery of modal events in QMotifDialog/QMotifWidget.
*/
QWidgetIntDict *QMotif::mapper()
{
#if defined(QT_CHECK_STATE)
    if ( ! QMotif_INSTANCE ) {
	qWarning( "QMotif::mapper: must create a QMotif first!" );
	return 0;
    }
#endif // QT_CHECK_STATE

    return &QMotif_INSTANCE->d->mapper;
}
