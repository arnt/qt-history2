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


static timeval	watchtime;			// watch if time is turned back
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

//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//
void QEventLoopPrivate::timerInsert( const TimerInfo *ti )	// insert timer info into list
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

void QEventLoopPrivate::timerRepair( const timeval &time )	// repair broken timer
{
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.
*/
timeval *QEventLoopPrivate::timerWait() 
{
    if ( !timerList || !timerList->count() )
	return 0;

    timeval currentTime;
    getTime( currentTime );
    if ( currentTime < watchtime )	// clock was turned back
	timerRepair( currentTime );
    watchtime = currentTime;

    static timeval ret;
    TimerInfo *t = timerList->first();	// first waiting timer
    if ( currentTime < t->timeout ) {	// time to wait
	ret = t->timeout - currentTime;
    } else {
	ret.tv_sec  = 0;			// no time to wait
	ret.tv_usec = 0;
    }
    return &ret;
}

// Main timer functions for starting and killing timers
int QEventLoop::registerTimer( int interval, QObject *obj )
{
    if ( !d->timerList ) {				// initialize timer data
	d->timerBitVec = new QBitArray( 128 );
	int i = d->timerBitVec->size();
	while( i-- > 0 )
	    d->timerBitVec->clearBit( i );
	d->timerList = new TimerList;
	d->timerList->setAutoDelete( TRUE );
	gettimeofday( &watchtime, 0 );
    }

    int id = 0;
    {
	int i = d->timerBitVec->size()-1;
	while ( i >= 0 && (*d->timerBitVec)[i] )
	    i--;
	if ( i < 0 ) {
	    i = d->timerBitVec->size();
	    d->timerBitVec->resize( 4 * i );
	    for( int j=d->timerBitVec->size()-1; j > i; j-- )
		d->timerBitVec->clearBit( j );
	}
	d->timerBitVec->setBit( i );
	id = i+1;
    }
    if ( id <= 0 ||
	 id > (int)d->timerBitVec->size() || !obj )// cannot create timer
	return 0;
    d->timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    d->timerInsert( t );				// put timer in list
    return id;
}

bool QEventLoop::unregisterTimer( int id )
{
    register TimerInfo *t;
    if ( !d->timerList || id <= 0 ||
	 id > (int)d->timerBitVec->size() || !d->timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = d->timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = d->timerList->next();
    if ( t ) {					// id found
	d->timerBitVec->clearBit( id-1 );		// set timer inactive
	return d->timerList->remove();
    }
    return FALSE; // id not found
}

bool QEventLoop::unregisterTimers( QObject *obj )
{
    register TimerInfo *t;
    if ( !d->timerList )				// not initialized
	return FALSE;
    t = d->timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    d->timerBitVec->clearBit( t->id-1 );
	    d->timerList->remove();
	    t = d->timerList->current();
	} else {
	    t = d->timerList->next();
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
    timeval *tm = ((QEventLoop*)this)->d->timerWait();
    if ( ! tm )	// no active timers
	return -1;
    return (tm->tv_sec*1000) + (tm->tv_usec/1000);
}

int QEventLoop::activateTimers()
{
    if ( qt_disable_lowpriority_timers || !d->timerList || !d->timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int n_act = 0, maxCount = d->timerList->count();
    TimerInfo *begin = 0;
    register TimerInfo *t;

    for ( ;; ) {
	if ( ! maxCount )
	    break;
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		d->timerRepair( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = d->timerList->first();
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
	d->timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	d->timerInsert( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	else
	    maxCount--;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
	if ( d->timerList->findRef( begin ) == -1 )
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
    d->timerList  = 0;
    d->timerBitVec = 0;
}

void QEventLoop::cleanup()
{
    // cleanup the common parts of the event loop
    close( d->thread_pipe[0] );
    close( d->thread_pipe[1] );

    //cleanup timer
    delete d->timerList;
    d->timerList = 0;
    delete d->timerBitVec;
    d->timerBitVec = 0;
}

int QEventLoopPrivate::eventloopSelect(uint flags, timeval *t)
{
    // Process timers and socket notifiers - the common UNIX stuff
    int highest = 0;
    FD_ZERO( &sn_vec[0].select_fds );
    FD_ZERO( &sn_vec[1].select_fds );
    FD_ZERO( &sn_vec[2].select_fds );
    if ( ! ( flags & QEventLoop::ExcludeSocketNotifiers ) && (sn_highest >= 0) ) {
	// return the highest fd we can wait for input on
	if ( !sn_vec[0].list.isEmpty() )
	    sn_vec[0].select_fds = sn_vec[0].enabled_fds;
	if ( !sn_vec[1].list.isEmpty() )
	    sn_vec[1].select_fds = sn_vec[1].enabled_fds;
	if ( !sn_vec[2].list.isEmpty() )
	    sn_vec[2].select_fds = sn_vec[2].enabled_fds;
	highest = sn_highest;
    }

#ifdef Q_WS_X11
    if ( xfd != -1 ) {
	// select for events on the event socket - only on X11
	FD_SET( xfd, &sn_vec[0].select_fds );
	highest = QMAX( highest, xfd );
    }
#endif

    FD_SET( thread_pipe[0], &sn_vec[0].select_fds );
    highest = QMAX( highest, thread_pipe[0] );
    int nsel = select(highest + 1, &sn_vec[0].select_fds, &sn_vec[1].select_fds,
		      &sn_vec[2].select_fds, t);

    int ret = 0;
    if ( nsel == -1 ) {
	if ( errno != EINTR && errno != EAGAIN )
	    perror( "select" );
    } else {
	// some other thread woke us up... consume the data on the thread pipe so that
	// select doesn't immediately return next time
	if ( FD_ISSET( thread_pipe[0], &sn_vec[0].select_fds ) ) {
	    char c;
	    ::read( thread_pipe[0], &c, 1 );
	}

	// activate socket notifiers
	if ( ! ( flags & QEventLoop::ExcludeSocketNotifiers ) && nsel > 0 && sn_highest >= 0 ) {
	    // if select says data is ready on any socket, then set the socket notifier
	    // to pending
	    for (int i=0; i<3; i++ ) {
		QList<QSockNot *> &list = sn_vec[i].list;
		for (int j = 0; j < list.size(); ++j) {
		    QSockNot *sn = list.at(j);
		    if ( FD_ISSET( sn->fd, &sn_vec[i].select_fds ) ) 
			q->setSocketNotifierPending( sn->obj );
		}
	    }
	}
	ret += q->activateSocketNotifiers();
    }
    return ret;
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

    if ( canWait )
	emit aboutToBlock();

    // unlock the GUI mutex and select.  when we return from this function, there is
    // something for us to do
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->unlock();
#endif

    // relock the GUI mutex before processing any pending events
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->lock();
#endif

    // return the maximum time we can wait for an event.
    static timeval zerotm;
    timeval *tm = NULL;
    if ( !(flags & 0x08)) {			// 0x08 == ExcludeTimers for X11 only
	tm = d->timerWait();			// wait for timer or X event
	if ( !canWait ) {
	    if ( !tm )
		tm = &zerotm;
	    tm->tv_sec  = 0;			// no time to wait
	    tm->tv_usec = 0;
	}
    }
    nevents += d->eventloopSelect(flags, tm);

    // we are awake, broadcast it
    emit awake();

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
