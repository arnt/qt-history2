#include "qmotifeventloop.h"
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


class QMotifEventLoopPrivate
{
public:
    QMotifEventLoopPrivate()
	: motif( 0 ), timerid( -1 )
    {
    }

    QMotif *motif;
    int timerid;
};

QMotifEventLoop::QMotifEventLoop( QMotif *motif,
				  QObject *parent, const char *name )
    : QEventLoop( parent, name )
{
    d = new QMotifEventLoopPrivate;
    d->motif = motif;
    d->motif->d->eventloop = this;
}

QMotifEventLoop::~QMotifEventLoop()
{
    delete d;
}

void qmotif_timeout_handler( XtPointer pointer, XtIntervalId *id )
{
    QMotifEventLoop *eventloop = (QMotifEventLoop *) pointer;
    if ( id && *id == (XtIntervalId) eventloop->d->timerid ) {
	eventloop->activateTimers();
	eventloop->d->timerid = -1;
    }
}

bool QMotifEventLoop::processNextEvent( int eventType, bool canWait )
{
    // Qt uses posted events to do lots of delayed operations, like repaints... these
    // need to be delivered before we go to sleep
    QApplication::sendPostedEvents();

    // make sure we fire off Qt's timers
    int ttw = timeToWait();
    if ( d->timerid != -1 )
	XtRemoveTimeOut( d->timerid );
    d->timerid = -1;
    if ( ttw != -1 )
	d->timerid =
	    XtAppAddTimeOut( d->motif->applicationContext(), ttw,
			     qmotif_timeout_handler, this );

    // get the pending event mask from Xt and process the next event
    XtInputMask pendingmask = XtAppPending( d->motif->applicationContext() );
    if ( canWait ) {
	XtAppProcessEvent( d->motif->applicationContext(), XtIMAll );
    } else if ( pendingmask )
	XtAppProcessEvent( d->motif->applicationContext(), pendingmask );

    return ( canWait || ( pendingmask != 0 ) );
}
