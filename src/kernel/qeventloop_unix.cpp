/****************************************************************************
**
** Implementation of QEventLoop class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qeventloop_p.h" // includes qplatformdefs.h
#include "qeventloop.h"
#include "qapplication.h"
#include "qbitarray.h"
#include "qevent.h"
#include "qmutex.h"
#include <stdlib.h>
#include <errno.h>
#define d d_func()
#define q q_func()


/*****************************************************************************
  Timer handling; UNIX has no application timer support so we'll have to
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
void cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval		*qt_wait_timer();
timeval	*qt_wait_timer_max = 0;

bool qt_disable_lowpriority_timers=FALSE;

//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline bool operator==( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec;
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

// Timer initialization
static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    timerList->setAutoDelete( TRUE );
    gettimeofday( &watchtime, 0 );
}

// Timer cleanup
void cleanupTimers()
{
    delete timerList;
    timerList = 0;
    delete timerBitVec;
    timerBitVec = 0;
}

// Main timer functions for starting and killing timers
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
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
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
 Socket notifier type
 *****************************************************************************/
QSockNotType::QSockNotType()
{
    list.setAutoDelete(true);
    FD_ZERO( &select_fds );
    FD_ZERO( &enabled_fds );
    FD_ZERO( &pending_fds );
}


/*****************************************************************************
 QEventLoop implementations for UNIX
 *****************************************************************************/
void QEventLoop::registerSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
	qWarning( "QSocketNotifier: Internal error" );
	return;
    }

    QList<QSockNot *>  &list = d->sn_vec[type].list;
    fd_set *fds  = &d->sn_vec[type].enabled_fds;
    QSockNot *sn;

    sn = new QSockNot;
    sn->obj = notifier;
    sn->fd = sockfd;
    sn->queue = &d->sn_vec[type].pending_fds;

    int i;
    for (i = 0; i < list.size(); ++i) {
	QSockNot *p = list.at(i);
	if (p->fd < sockfd )
	    break;
	if ( p->fd == sockfd ) {
	    static const char *t[] = { "read", "write", "exception" };
	    qWarning( "QSocketNotifier: Multiple socket notifiers for "
		      "same socket %d and type %s", sockfd, t[type] );
	}
    }
    list.insert( i, sn );

    FD_SET( sockfd, fds );
    d->sn_highest = QMAX( d->sn_highest, sockfd );
}

void QEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
	qWarning( "QSocketNotifier: Internal error" );
	return;
    }

    QList<QSockNot *> &list = d->sn_vec[type].list;
    fd_set *fds  =  &d->sn_vec[type].enabled_fds;
    QSockNot *sn;
    int i;
    for (i = 0; i < list.size(); ++i) {
	sn = list.at(i);
	if(sn->obj == notifier && sn->fd == sockfd)
	    break;
    }
    if (i == list.size()) // not found
	return;

    FD_CLR( sockfd, fds );			// clear fd bit
    FD_CLR( sockfd, sn->queue );
    d->sn_pending_list.remove(sn);		// remove from activation list
    list.removeAt(i);				// remove notifier found above

    if ( d->sn_highest == sockfd ) {		// find highest fd
	d->sn_highest = -1;
	for ( int i=0; i<3; i++ ) {
	    if ( !d->sn_vec[i].list.isEmpty() )
		d->sn_highest = QMAX( d->sn_highest,  // list is fd-sorted
				      d->sn_vec[i].list.first()->fd );
	}
    }
}

void QEventLoop::setSocketNotifierPending( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
	qWarning( "QSocketNotifier: Internal error" );
	return;
    }

    QList<QSockNot *> &list = d->sn_vec[type].list;
    QSockNot *sn;
    int i;
    for (i = 0; i < list.size(); ++i) {
	sn = list.at(i);
	if(sn->obj == notifier && sn->fd == sockfd)
	    break;
    }
    if ( i == list.size() ) // not found
	return;

    // We choose a random activation order to be more fair under high load.
    // If a constant order is used and a peer early in the list can
    // saturate the IO, it might grab our attention completely.
    // Also, if we're using a straight list, the callback routines may
    // delete other entries from the list before those other entries are
    // processed.
    if ( ! FD_ISSET( sn->fd, sn->queue ) ) {
	d->sn_pending_list.insert( (rand() & 0xff) %
				   (d->sn_pending_list.count()+1), sn );
	FD_SET( sn->fd, sn->queue );
    }
}

void QEventLoop::wakeUp()
{
    /*
      Apparently, there is not consistency among different operating
      systems on how to use FIONREAD.

      FreeBSD, Linux and Solaris all expect the 3rd argument to
      ioctl() to be an int, which is normally 32-bit even on 64-bit
      machines.

      IRIX, on the other hand, expects a size_t, which is 64-bit on
      64-bit machines.

      So, the solution is to use size_t initialized to zero to make
      sure all bits are set to zero, preventing underflow with the
      FreeBSD/Linux/Solaris ioctls.
    */
    size_t nbytes = 0;
    char c = 0;
    if ( ::ioctl( d->thread_pipe[0], FIONREAD, (char*)&nbytes ) >= 0 && nbytes == 0 ) {
	::write(  d->thread_pipe[1], &c, 1  );
    }
}

int QEventLoop::timeToWait() const
{
    timeval *tm = qt_wait_timer();
    if ( ! tm )	// no active timers
	return -1;
    return (tm->tv_sec*1000) + (tm->tv_usec/1000);
}

int QEventLoop::activateTimers()
{
    if ( qt_disable_lowpriority_timers || !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int n_act = 0, maxCount = timerList->count();
    TimerInfo *begin = 0;
    register TimerInfo *t;

    for ( ;; ) {
	if ( ! maxCount )
	    break;
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
	if ( ! begin ) {
	    begin = t;
	} else if ( begin == t ) {
	    // avoid sending the same timer multiple times
	    break;
	} else if ( t->interval <  begin->interval || t->interval == begin->interval ) {
	    begin = t;
	}
	timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	else
	    maxCount--;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
	if ( timerList->findRef( begin ) == -1 )
	    begin = 0;
    }
    return n_act;
}

int QEventLoop::activateSocketNotifiers()
{
    if ( d->sn_pending_list.isEmpty() )
	return 0;

    // activate entries
    int n_act = 0;
    QEvent event( QEvent::SockAct );
    while (!d->sn_pending_list.isEmpty()) {
	QSockNot *sn = d->sn_pending_list.takeAt(0);
	if ( FD_ISSET(sn->fd, sn->queue) ) {
	    FD_CLR( sn->fd, sn->queue );
	    QApplication::sendEvent( sn->obj, &event );
	    n_act++;
	}
    }

    return n_act;
}





void QEventLoop::init()
{
    // initialize the common parts of the event loop
    pipe( d->thread_pipe );
    fcntl(d->thread_pipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(d->thread_pipe[1], F_SETFD, FD_CLOEXEC);

    d->sn_highest = -1;
}

void QEventLoop::cleanup()
{
    // cleanup the common parts of the event loop
    close( d->thread_pipe[0] );
    close( d->thread_pipe[1] );
    cleanupTimers();
}

bool QEventLoop::processEvents( ProcessEventsFlags flags )
{
    int nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker( QApplication::qt_mutex );
#endif

    if ( d->shortcut )
	return FALSE;

    QApplication::sendPostedEvents();

    // don't block if exitLoop() or exit()/quit() has been called.
    bool canWait = d->exitloop || d->quitnow ? FALSE : (flags & WaitForMore);

    // Process timers and socket notifiers - the common UNIX stuff

    // return the maximum time we can wait for an event.
    static timeval zerotm;
    timeval *tm = 0;
    if ( ! ( flags & 0x08 ) ) {			// 0x08 == ExcludeTimers for X11 only
	tm = qt_wait_timer();			// wait for timer or X event
	if ( !canWait ) {
	    if ( !tm )
		tm = &zerotm;
	    tm->tv_sec  = 0;			// no time to wait
	    tm->tv_usec = 0;
	}
    }

    int highest = 0;
    FD_ZERO( &d->sn_vec[0].select_fds );
    FD_ZERO( &d->sn_vec[1].select_fds );
    FD_ZERO( &d->sn_vec[2].select_fds );
    if ( ! ( flags & ExcludeSocketNotifiers ) && (d->sn_highest >= 0) ) {
	// return the highest fd we can wait for input on
	if ( !d->sn_vec[0].list.isEmpty() )
	    d->sn_vec[0].select_fds = d->sn_vec[0].enabled_fds;
	if ( !d->sn_vec[1].list.isEmpty() )
	    d->sn_vec[1].select_fds = d->sn_vec[1].enabled_fds;
	if ( !d->sn_vec[2].list.isEmpty() )
	    d->sn_vec[2].select_fds = d->sn_vec[2].enabled_fds;
	highest = d->sn_highest;
    }

#ifdef Q_WS_X11
    if ( d->xfd != -1 ) {
	// select for events on the event socket - only on X11
	FD_SET( d->xfd, &d->sn_vec[0].select_fds );
	highest = QMAX( highest, d->xfd );
    }
#endif

    FD_SET( d->thread_pipe[0], &d->sn_vec[0].select_fds );
    highest = QMAX( highest, d->thread_pipe[0] );

    if ( canWait )
	emit aboutToBlock();

    // unlock the GUI mutex and select.  when we return from this function, there is
    // something for us to do
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->unlock();
#endif

    int nsel;
    nsel = select( highest + 1,
		   &d->sn_vec[0].select_fds,
		   &d->sn_vec[1].select_fds,
		   &d->sn_vec[2].select_fds,
		   tm );

    // relock the GUI mutex before processing any pending events
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->lock();
#endif

    // we are awake, broadcast it
    emit awake();

    if ( nsel == -1 ) {
	if ( errno != EINTR && errno != EAGAIN )
	    perror( "select" );
	return (nevents > 0);
    }

#undef FDCAST

    // some other thread woke us up... consume the data on the thread pipe so that
    // select doesn't immediately return next time
    if ( nsel > 0 && FD_ISSET( d->thread_pipe[0], &d->sn_vec[0].select_fds ) ) {
	char c;
	::read( d->thread_pipe[0], &c, 1 );
    }

    // activate socket notifiers
    if ( ! ( flags & ExcludeSocketNotifiers ) && nsel > 0 && d->sn_highest >= 0 ) {
	// if select says data is ready on any socket, then set the socket notifier
	// to pending
	for (int i=0; i<3; i++ ) {
	    QList<QSockNot *> &list = d->sn_vec[i].list;
	    for (int j = 0; j < list.size(); ++j) {
		QSockNot *sn = list.at(j);
		if ( FD_ISSET( sn->fd, &d->sn_vec[i].select_fds ) )
		    setSocketNotifierPending( sn->obj );
	    }
	}

	nevents += activateSocketNotifiers();
    }

    // activate timers
    if ( ! ( flags & 0x08 ) ) {
	// 0x08 == ExcludeTimers for X11 only
	nevents += activateTimers();
    }

    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return qGlobalPostedEventsCount();
}
