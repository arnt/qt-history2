#include "qplatformdefs.h"

#include <errno.h>

#include "qeventloop.h"
#include "qeventloop_p.h"

#include "qapplication.h"
#include "qbitarray.h"
#include "qcolor_p.h"
#include "qdatetime.h"

#include "qt_x11.h"






extern bool qt_is_gui_used; // from qapplication.cpp





/*****************************************************************************
  Timer handling; Xlib has no application timer support so we'll have to
  make our own from scratch.

  NOTE: These functions are for internal use. QObject::startTimer() and
	QObject::killTimer() are for public use.
	The QTimer class provides a high-level interface which translates
	timer events into signals.

  qStartTimer( interval, obj )
	Starts a timer which will run until it is killed with qKillTimer()
	Arguments:
	    int interval	timer interval in milliseconds
	    QObject *obj	where to send the timer event
	Returns:
	    int			timer identifier, or zero if not successful

  qKillTimer( timerId )
	Stops a timer specified by a timer identifier.
	Arguments:
	    int timerId		timer identifier
	Returns:
	    bool		TRUE if successful

  qKillTimer( obj )
	Stops all timers that are sent to the specified object.
	Arguments:
	    QObject *obj	object receiving timer events
	Returns:
	    bool		TRUE if successful
 *****************************************************************************/

//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef QPtrList<TimerInfo> TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list

static void	initTimers();
void	cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval		*qt_wait_timer();
timeval	*qt_wait_timer_max = 0;
int		qt_activate_timers();

//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline timeval &operator+=( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ( (t1.tv_usec += t2.tv_usec) >= 1000000 ) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
}

static inline timeval operator+( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000 ) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}

static inline timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}


//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//

static int allocTimerId()			// find avail timer identifier
{
    int i = timerBitVec->size()-1;
    while ( i >= 0 && (*timerBitVec)[i] )
	i--;
    if ( i < 0 ) {
	i = timerBitVec->size();
	timerBitVec->resize( 4 * i );
	for( int j=timerBitVec->size()-1; j > i; j-- )
	    timerBitVec->clearBit( j );
    }
    timerBitVec->setBit( i );
    return i+1;
}

static void insertTimer( const TimerInfo *ti )	// insert timer info into list
{
    TimerInfo *t = timerList->first();
    int index = 0;
#if defined(QT_DEBUG)
    int dangerCount = 0;
#endif
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
#if defined(QT_DEBUG)
	if ( t->obj == ti->obj )
	    dangerCount++;
#endif
	t = timerList->next();
	index++;
    }
    timerList->insert( index, ti );		// inserts sorted
#if defined(QT_DEBUG)
    if ( dangerCount > 16 )
	qDebug( "QObject: %d timers now exist for object %s::%s",
	       dangerCount, ti->obj->className(), ti->obj->name() );
#endif
}

static inline void getTime( timeval &t )	// get time of day
{
    gettimeofday( &t, 0 );
    while ( t.tv_usec >= 1000000 ) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}

static void repairTimer( const timeval &time )	// repair broken timer
{
    if ( !timerList )				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}

//
// Timer activation functions (called from the event loop)
//

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.

  The result is bounded to qt_wait_timer_max if this exists.
*/

timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if ( timerList && timerList->count() ) {	// there are waiting timers
	getTime( currentTime );
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if ( currentTime < t->timeout ) {	// time to wait
	    tm = t->timeout - currentTime;
	} else {
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	if ( qt_wait_timer_max && *qt_wait_timer_max < tm )
	    tm = *qt_wait_timer_max;
	return &tm;
    }
    if ( qt_wait_timer_max ) {
	tm = *qt_wait_timer_max;
	return &tm;
    }
    return 0;					// no timers
}

/*
  Activates the timer events that have expired. Returns the number of timers
  (not 0-timer) that were activated.
*/

int qt_activate_timers()
{
    if ( !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    int n_act = 0;
    register TimerInfo *t;
    while ( maxcount-- ) {			// avoid starvation
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if ( !t || currentTime < t->timeout )	// no timer has expired
	    break;
	timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
    }
    return n_act;
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    Q_CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    Q_CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	timerList = 0;
	delete timerBitVec;
	timerBitVec = 0;
    }
}


//
// Main timer functions for starting and killing timers
//

int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 ||
	 id > (int)timerBitVec->size() || !obj )// cannot create timer
	return 0;
    timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    Q_CHECK_PTR( t );
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    void qt_np_enable_timers();			// in qnpsupport.cpp
    qt_np_enable_timers();
    return id;
}

bool qKillTimer( int id )
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 ||
	 id > (int)timerBitVec->size() || !timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	timerBitVec->clearBit( id-1 );		// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    timerBitVec->clearBit( t->id-1 );
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}





























































/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

struct QSockNot {
    QObject *obj;
    int	     fd;
    fd_set  *queue;
};

typedef QPtrList<QSockNot> QSNList;
typedef QPtrListIterator<QSockNot> QSNListIt;

static int	sn_highest = -1;
static QSNList *sn_read	   = 0;
static QSNList *sn_write   = 0;
static QSNList *sn_except  = 0;

static fd_set	sn_readfds;			// fd set for reading
static fd_set	sn_writefds;			// fd set for writing
static fd_set	sn_exceptfds;			// fd set for exceptions
static fd_set	sn_queued_read;
static fd_set	sn_queued_write;
static fd_set	sn_queued_except;

static struct SN_Type {
    QSNList **list;
    fd_set   *fdspec;
    fd_set   *fdres;
    fd_set   *queue;
} sn_vec[3] = {
    { &sn_read,	  &sn_readfds,	 &app_readfds,   &sn_queued_read },
    { &sn_write,  &sn_writefds,	 &app_writefds,  &sn_queued_write },
    { &sn_except, &sn_exceptfds, &app_exceptfds, &sn_queued_except } };


static QSNList *sn_act_list = 0;


static void sn_cleanup()
{
    delete sn_act_list;
    sn_act_list = 0;
    for ( int i=0; i<3; i++ ) {
	delete *sn_vec[i].list;
	*sn_vec[i].list = 0;
    }
}


static void sn_init()
{
    if ( !sn_act_list ) {
	sn_act_list = new QSNList;
	Q_CHECK_PTR( sn_act_list );
	qAddPostRoutine( sn_cleanup );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNList  *list = *sn_vec[type].list;
    fd_set   *fds  =  sn_vec[type].fdspec;
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( !list ) {
	    sn_init();
	    list = new QSNList;			// create new list
	    Q_CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	Q_CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
	sn->queue = sn_vec[type].queue;
	if ( list->isEmpty() ) {
	    list->insert( 0, sn );
	} else {				// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(QT_CHECK_STATE)
	    if ( p && p->fd == sockfd ) {
		static const char *t[] = { "read", "write", "exception" };
		qWarning( "QSocketNotifier: Multiple socket notifiers for "
			 "same socket %d and type %s", sockfd, t[type] );
	    }
#endif
	    if ( p )
		list->insert( list->at(), sn );
	    else
		list->append( sn );
	}
	FD_SET( sockfd, fds );
	sn_highest = QMAX(sn_highest,sockfd);

    } else {					// disable notifier

	if ( list == 0 )
	    return FALSE;			// no such fd set
	QSockNot *sn = list->first();
	while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
	    sn = list->next();
	if ( !sn )				// not found
	    return FALSE;
	FD_CLR( sockfd, fds );			// clear fd bit
	FD_CLR( sockfd, sn->queue );
	if ( sn_act_list )
	    sn_act_list->removeRef( sn );	// remove from activation list
	list->remove();				// remove notifier found above
	if ( sn_highest == sockfd ) {		// find highest fd
	    sn_highest = -1;
	    for ( int i=0; i<3; i++ ) {
		if ( *sn_vec[i].list && (*sn_vec[i].list)->count() )
		    sn_highest = QMAX(sn_highest,  // list is fd-sorted
				      (*sn_vec[i].list)->getFirst()->fd);
	    }
	}
    }

    return TRUE;
}


//
// We choose a random activation order to be more fair under high load.
// If a constant order is used and a peer early in the list can
// saturate the IO, it might grab our attention completely.
// Also, if we're using a straight list, the callback routines may
// delete other entries from the list before those other entries are
// processed.
//

static int sn_activate()
{
    if ( !sn_act_list )
	sn_init();
    int i, n_act = 0;
    for ( i=0; i<3; i++ ) {			// for each list...
	if ( *sn_vec[i].list ) {		// any entries?
	    QSNList  *list = *sn_vec[i].list;
	    fd_set   *fds  = sn_vec[i].fdres;
	    QSockNot *sn   = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, fds ) &&	// store away for activation
		     !FD_ISSET( sn->fd, sn->queue ) ) {
		    sn_act_list->insert( (rand() & 0xff) %
					 (sn_act_list->count()+1),
					 sn );
		    FD_SET( sn->fd, sn->queue );
		}
		sn = list->next();
	    }
	}
    }
    if ( sn_act_list->count() > 0 ) {		// activate entries
	QEvent event( QEvent::SockAct );
	QSNListIt it( *sn_act_list );
	QSockNot *sn;
	while ( (sn=it.current()) ) {
	    ++it;
	    sn_act_list->removeRef( sn );
	    if ( FD_ISSET(sn->fd, sn->queue) ) {
		FD_CLR( sn->fd, sn->queue );
		QApplication::sendEvent( sn->obj, &event );
		n_act++;
	    }
	}
    }
    return n_act;
}

















void QEventLoop::init()
{
    d->xfd = -1;
    if ( qt_is_gui_used )
        d->xfd = XConnectionNumber( QPaintDevice::x11AppDisplay() );

#if defined(Q_OS_UNIX)
    pipe( d->thread_pipe );
#endif
}

void QEventLoop::cleanup()
{
#ifdef Q_OS_UNIX
    close( d->thread_pipe[0] );
    close( d->thread_pipe[1] );
#endif // Q_OS_UNIX
}

void QEventLoop::addExternal( int fd )
{
    // ### TODO!
}

void QEventLoop::removeExternal( int fd )
{
    // ### TODO!
}

void QEventLoop::wakeUp()
{
#if defined(Q_OS_UNIX)
    char c = 0;
    int nbytes;
    if ( ::ioctl( d->thread_pipe[0], FIONREAD, (char*)&nbytes ) >= 0 && nbytes == 0 ) {
	::write(  d->thread_pipe[1], &c, 1  );
    }
#endif
}

bool QEventLoop::x11ProcessEvent( XEvent *event )
{
    return qApp->x11ProcessEvent( event );
}


bool QEventLoop::processNextEvent( int eventType, bool canWait )
{
    XEvent event;
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif

    // we are awake, broadcast it
    emit awake();







    // handle gui and posted events
    if (qt_is_gui_used ) {
	QApplication::sendPostedEvents();

	// Two loops so that posted events accumulate
	while ( XPending(QPaintDevice::x11AppDisplay()) ) {
	    // also flushes output buffer
	    while ( XPending(QPaintDevice::x11AppDisplay()) ) {
		if ( d->exitloop ) {          // quit between events
#if defined(QT_THREAD_SUPPORT)
		    qApp->unlock(FALSE);
#endif

		    return FALSE;
		}

		XNextEvent( QPaintDevice::x11AppDisplay(), &event );	// get next event
		nevents++;

		if ( x11ProcessEvent( &event ) == 1 ) {
#if defined(QT_THREAD_SUPPORT)
		    qApp->unlock(FALSE);
#endif

		    return TRUE;
		}
	    }

	    QApplication::sendPostedEvents();
	}
    }

    if ( d->exitloop ) {			// break immediately
#if defined(QT_THREAD_SUPPORT)
	qApp->unlock(FALSE);
#endif

	return FALSE;
    }

    QApplication::sendPostedEvents();













    // return the maximum time we can wait for an event.
    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or X event
    if ( !canWait ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }













    // return the highest fd we can wait for input on
    if ( sn_highest >= 0 ) {			// has socket notifier(s)
	if ( sn_read )
	    app_readfds = sn_readfds;
	else
	    FD_ZERO( &app_readfds );
	if ( sn_write )
	    app_writefds = sn_writefds;
	if ( sn_except )
	    app_exceptfds = sn_exceptfds;
    } else {
	FD_ZERO( &app_readfds );
    }

    int highest = sn_highest;
    if ( qt_is_gui_used ) {
	FD_SET( d->xfd, &app_readfds );
	highest = QMAX( highest, d->xfd );
	XFlush( QPaintDevice::x11AppDisplay() );
    }
    int nsel;

#if defined(Q_OS_UNIX)
    FD_SET( d->thread_pipe[0], &app_readfds );
    highest = QMAX( highest, d->thread_pipe[0] );
#endif









    /*
      ### TODO - a cleaner way to do preselect handlers should be invented...
      virtual functions?

      if ( qt_preselect_handler ) {
      QVFuncList::Iterator end = qt_preselect_handler->end();
      for ( QVFuncList::Iterator it = qt_preselect_handler->begin(); it != end; ++it )
      (**it)();
      }
    */







    // unlock the GUI mutex and select.  when we return from this function, there is
    // something for us to do
#if defined(QT_THREAD_SUPPORT)
    qApp->unlock(FALSE);
#endif

    nsel = select( highest + 1,
		   &app_readfds,
		   sn_write  ? &app_writefds  : 0,
		   sn_except ? &app_exceptfds : 0,
		   tm );

#undef FDCAST

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif












    // some other thread woke us up... consume the data on the thread pipe so that
    // select doesn't immediately return next time
#if defined(Q_OS_UNIX)
    if ( nsel > 0 && FD_ISSET( d->thread_pipe[0], &app_readfds ) ) {
	char c;
	::read( d->thread_pipe[0], &c, 1 );
    }
#endif











    /*
      ### TODO - a cleaner way to do postselect handlers should be invented...
      virtual functions?

      if ( qt_postselect_handler ) {
      QVFuncList::Iterator end = qt_postselect_handler->end();
      for ( QVFuncList::Iterator it = qt_postselect_handler->begin(); it != end; ++it )
      (**it)();
      }
    */





    // activate socket notifiers
    if ( nsel == -1 ) {
	if ( errno == EINTR || errno == EAGAIN ) {
	    errno = 0;

#if defined(QT_THREAD_SUPPORT)
	    qApp->unlock(FALSE);
#endif

	    return (nevents > 0);
	} else {
	    ; // select error
	}
    } else if ( nsel > 0 && sn_highest >= 0 ) {
	nevents += sn_activate();
    }






    // activate timers
    nevents += activateTimers();








    // color approx. optimization
    qt_reset_color_avail();








#if defined(QT_THREAD_SUPPORT)
    qApp->unlock(FALSE);
#endif

    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventLoop::hasPendingEvents( int eventType )
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return ( qGlobalPostedEventsCount() || XPending( QPaintDevice::x11AppDisplay() ) );
}

void QEventLoop::processEvents( int eventType, int maxTime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( ! d->quitnow && processNextEvent( eventType, FALSE ) ) {
	now = QTime::currentTime();
	if ( start.msecsTo( now ) > maxTime )
	    break;
    }
}

int QEventLoop::timeToWait()
{
    timeval *tm = qt_wait_timer();
    if ( ! tm )	// no active timers
	return -1;
    return (tm->tv_sec*1000) + (tm->tv_usec/1000);
}

int QEventLoop::activateTimers()
{
    return qt_activate_timers();
}
