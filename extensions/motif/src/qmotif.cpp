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

#if QT_VERSION < 310
// no QEventLoop class available
typedef void (*ForeignEventProc)(XEvent*);
void qt_np_add_event_proc( ForeignEventProc fep );       // defined in qnpsupport.cpp
void qt_np_remove_event_proc( ForeignEventProc fep );    // defined in qnpsupport.cpp

static bool redelivered_so_dont_deliver_further = FALSE; // yes, this is bad :)

void qmotif_event_proc( XEvent* event )
{
    redelivered_so_dont_deliver_further = TRUE;
    (void) QMotif::redeliverEvent( event );
}
#endif

QMotifPrivate::QMotifPrivate()
    : appContext(NULL), ownContext( FALSE )
{
#if QT_VERSION >= 310
    eventloop = 0;
#endif
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

#if QT_VERSION < 310
    qt_np_add_event_proc( qmotif_event_proc );
#endif
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

#if QT_VERSION < 310
    qt_np_remove_event_proc( qmotif_event_proc );
#endif
}


static QMotif *QMotif_INSTANCE = 0;

Boolean qmotif_event_dispatcher( XEvent *event )
{
#if QT_VERSION < 310
    if ( qApp->loopLevel() == 0 ) {
#endif

        QApplication::sendPostedEvents();

#if QT_VERSION >= 310
        // ### WARNING - this is an ugly nasty horrible icky hack.  Please look away
        // now
        QWidgetIntDict *widgetmapper = ( (QWidgetIntDict *) QWidget::wmapper() );

        QWidget *motiftarget = QMotif::mapper()->find( event->xany.window );
        if ( motiftarget ) {
            if ( ! widgetmapper->find( event->xany.window ) ) {
                widgetmapper->insert( event->xany.window, motiftarget );
            } else {
                motiftarget = 0;
                qWarning( "ERROR: window 0x%lx is already in QWidget::mapper!",
                          event->xany.window );
            }
        }
#endif

        bool delivered = ( qApp->x11ProcessEvent( event ) != -1 );

#if QT_VERSION >= 310
        // Please look away again.
        if ( motiftarget )
            widgetmapper->remove( event->xany.window );
#endif

        if ( delivered )
            // Qt handled the event.
            return True;

#if QT_VERSION < 310
        if ( redelivered_so_dont_deliver_further ) {
            redelivered_so_dont_deliver_further = FALSE;
            return True;
        }
#endif

        if ( QApplication::activePopupWidget() )
            // we get all events through the popup grabs.  discard the event
            return True;

        if ( QApplication::activeModalWidget() ) {
            // disable all user events in modal mode, but let all other events through
            switch ( event->type ) {
            case ButtonPress:
            case ButtonRelease:
            case MotionNotify:
            case XKeyPress:
            case XKeyRelease:
            case EnterNotify:
            case LeaveNotify:
            case XFocusIn:
            case XFocusOut:
                return True;

            default:
                break;
            }
        }

#if QT_VERSION < 310
    }
#endif

    if ( QMotif_INSTANCE->d->dispatchers[ event->type ]( event ) )
	// Xt handled the event.
	return True;

    return False;
}

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

QMotif::~QMotif()
{
    d->unhook();
    if ( d->ownContext )
	XtDestroyApplicationContext( d->appContext );
    delete d;
}

XtAppContext QMotif::applicationContext() const
{
    if ( d->appContext == NULL ) {
	d->appContext = XtCreateApplicationContext();
	d->ownContext = TRUE;
    }
    return d->appContext;
}

void QMotif::setApplicationContext( XtAppContext appContext )
{
#if defined(QT_CHECK_STATE)
    if ( d->appContext != NULL )
	qWarning( "QMotif: WARNING: only one application context should be used." );
#endif // QT_CHECK_STATE
    d->appContext = appContext;
}

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

bool QMotif::redeliverEvent( XEvent *event )
{
    // redeliver the event to Xt, NOT through Qt
    if ( QMotif_INSTANCE->d->dispatchers[ event->type ]( event ) )
	return TRUE;
    return FALSE;
};
