#include "qmotifeventloop.h"
#include "qmotif.h"
#include "qmotif_p.h"

#include <qapplication.h>
#include <qintdict.h>

// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease


class QMotifEventLoopPrivate
{
public:
    QMotifEventLoopPrivate()
	: motif( 0 ), activate_timers( FALSE ), timerid( -1 )
    {
    }

    QIntDict<QSocketNotifier> socknotDict;
    QMotif *motif;
    bool activate_timers;
    int timerid;
};

/*!
  \class QMotifEventLoop
  \brief The QMotifEventLoop class integrates the Xt/Motif and Qt event loops.

  \extension QMotif

  QMotifEventLoop provides a reimplementation of QEventLoop to operate
  completely through Xt event delivery functions.  This provides
  seamless integration with Xt/Motif, since all events, timers and
  external input sources will be handled by one common event loop.
*/

/*!
    Creates a QMotifEventLoop.

    The \a motif integrator object is used to keep all events working.
    The \a parent and \a name arguements are passed on to the
    QEventLoop constructor.
*/
QMotifEventLoop::QMotifEventLoop( QMotif *motif,
				  QObject *parent, const char *name )
    : QEventLoop( parent, name )
{
    d = new QMotifEventLoopPrivate;
    d->motif = motif;
    d->motif->d->eventloop = this;
}

/*!
    Destroys the QMotifEventLoop.
 */
QMotifEventLoop::~QMotifEventLoop()
{
    delete d;
}

/*! \internal
 */
void qmotif_socknot_handler( XtPointer pointer, int *, XtInputId *id )
{
    QMotifEventLoop *eventloop = (QMotifEventLoop *) pointer;
    QSocketNotifier *socknot = eventloop->d->socknotDict.find( *id );
    if ( ! socknot ) // this shouldn't happen
	return;
    eventloop->setSocketNotifierPending( socknot );
}

/*! \reimp
 */
void QMotifEventLoop::registerSocketNotifier( QSocketNotifier *notifier )
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

    XtInputId id = XtAppAddInput( d->motif->applicationContext(),
				  notifier->socket(), (XtPointer) mask,
				  qmotif_socknot_handler, this );
    d->socknotDict.insert( id, notifier );

    QEventLoop::registerSocketNotifier( notifier );
}

/*! \reimp
 */
void QMotifEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )
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
void qmotif_timeout_handler( XtPointer pointer, XtIntervalId * )
{
    QMotifEventLoop *eventloop = (QMotifEventLoop *) pointer;
    eventloop->d->activate_timers = TRUE;
    eventloop->d->timerid = -1;
}

/*! \reimp
 */
bool QMotifEventLoop::processNextEvent( ProcessEventsFlags flags, bool canWait )
{
    // Qt uses posted events to do lots of delayed operations, like repaints... these
    // need to be delivered before we go to sleep
    QApplication::sendPostedEvents();

    // make sure we fire off Qt's timers
    int ttw = timeToWait();
    if ( d->timerid != -1 ) {
	XtRemoveTimeOut( d->timerid );
    }
    d->timerid = -1;
    if ( ttw != -1 ) {
	d->timerid =
	    XtAppAddTimeOut( d->motif->applicationContext(), ttw,
			     qmotif_timeout_handler, this );
    }

    // get the pending event mask from Xt and process the next event
    XtInputMask pendingmask = XtAppPending( d->motif->applicationContext() );
    XtInputMask mask = pendingmask;
    if ( pendingmask & XtIMTimer ) {
	mask &= ~XtIMTimer;
	// zero timers will starve the Xt X event dispatcher... so process
	// something *instead* of a timer first...
	if ( mask != 0 )
	    XtAppProcessEvent( d->motif->applicationContext(), mask );
	// and process a timer afterwards
	mask = pendingmask & XtIMTimer;
    }

    if ( canWait )
	XtAppProcessEvent( d->motif->applicationContext(), XtIMAll );
    else
	XtAppProcessEvent( d->motif->applicationContext(), mask );

    int nevents = 0;
    if ( ! ( flags & ExcludeSocketNotifiers ) )
	nevents += activateSocketNotifiers();

    if ( d->activate_timers ) {
	nevents += activateTimers();
    }
    d->activate_timers = FALSE;

    return ( canWait || ( pendingmask != 0 ) || nevents > 0 );
}
