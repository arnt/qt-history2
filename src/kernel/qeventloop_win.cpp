/****************************************************************************
**
** Implementation of Win32 startup routines and event handling.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qeventloop_p.h"
#include "qeventloop.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qevent.h"
#include <private/qinputcontext_p.h>

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

extern uint qGlobalPostedEventsCount();
extern bool qt_winEventFilter( MSG* msg, long &result );

static DWORD qt_gui_thread = 0;
// Simpler timers are needed when Qt does not have the event loop,
// such as for plugins.
Q_EXPORT bool	qt_win_use_simple_timers = TRUE;
void CALLBACK   qt_simple_timer_func( HWND, UINT, UINT, DWORD );

static void	initTimers();
static void	cleanupTimers();
static bool	dispatchTimer( uint, MSG * );
static bool	activateTimer( uint );
static void	activateZeroTimers();

static int	 numZeroTimers	= 0;		// number of full-speed timers

bool winPeekMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
		     UINT wMsgFilterMax, UINT wRemoveMsg )
{
    QT_WA( {
	return PeekMessage( msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
    } , {
	return PeekMessageA( msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
    } );
}

bool winPostMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    QT_WA( {
	return PostMessage( hWnd, msg, wParam, lParam );
    } , {
	return PostMessageA( hWnd, msg, wParam, lParam );
    } );
}

static bool winGetMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
		     UINT wMsgFilterMax )
{
    QT_WA( {
	return GetMessage( msg, hWnd, wMsgFilterMin, wMsgFilterMax );
    } , {
	return GetMessageA( msg, hWnd, wMsgFilterMin, wMsgFilterMax );
    } );
}


/*****************************************************************************
  Safe configuration (move,resize,setGeometry) mechanism to avoid
  recursion when processing messages.
 *****************************************************************************/

#include "qptrqueue.h"

struct QWinConfigRequest {
    WId	 id;					// widget to be configured
    int	 req;					// 0=move, 1=resize, 2=setGeo
    int	 x, y, w, h;				// request parameters
};

static QPtrQueue<QWinConfigRequest> *configRequests = 0;

void qWinRequestConfig( WId id, int req, int x, int y, int w, int h )
{
    if ( !configRequests )			// create queue
	configRequests = new QPtrQueue<QWinConfigRequest>;
    QWinConfigRequest *r = new QWinConfigRequest;
    r->id = id;					// create new request
    r->req = req;
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    configRequests->enqueue( r );		// store request in queue
}

Q_EXPORT void qWinProcessConfigRequests()		// perform requests in queue
{
    if ( !configRequests )
	return;
    QWinConfigRequest *r;
    for ( ;; ) {
	if ( configRequests->isEmpty() )
	    break;
	r = configRequests->dequeue();
	QWidget *w = QWidget::find( r->id );
	if ( w ) {				// widget exists
	    if ( w->testWState(Qt::WState_ConfigPending) )
		return;				// biting our tail
	    if ( r->req == 0 )
		w->move( r->x, r->y );
	    else if ( r->req == 1 )
		w->resize( r->w, r->h );
	    else
		w->setGeometry( r->x, r->y, r->w, r->h );
	}
	delete r;
    }
    delete configRequests;
    configRequests = 0;
}


/*****************************************************************************
  Timer handling; Our routines depend on Windows timer functions, but we
  need some extra handling to activate objects at timeout.

  Implementation note: There are two types of timer identifiers. Windows
  timer ids (internal use) are stored in TimerInfo.  Qt timer ids are
  indexes (+1) into the timerVec vector.

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

#include "qptrvector.h"
#include "qintdict.h"

struct TimerInfo {				// internal timer info
    uint     ind;				// - Qt timer identifier - 1
    uint     id;				// - Windows timer identifier
    bool     zero;				// - zero timing
    QObject *obj;				// - object to receive events
};
typedef QPtrVector<TimerInfo>  TimerVec;		// vector of TimerInfo structs
typedef QIntDict<TimerInfo> TimerDict;		// fast dict of timers

static TimerVec  *timerVec  = 0;		// timer vector
static TimerDict *timerDict = 0;		// timer dict


void CALLBACK qt_simple_timer_func( HWND, UINT, UINT idEvent, DWORD )
{
    dispatchTimer( idEvent, 0 );
}


// Activate a timer, used by both event-loop based and simple timers.

static bool dispatchTimer( uint timerId, MSG *msg )
{
    long res = 0;
    if ( !msg || !qApp || !qt_winEventFilter(msg,res) )
	return activateTimer( timerId );
    return TRUE;
}


//
// Timer activation (called from the event loop when WM_TIMER arrives)
//

static bool activateTimer( uint id )		// activate timer
{
    if ( !timerVec )				// should never happen
	return FALSE;
    register TimerInfo *t = timerDict->find( id );
    if ( !t )					// no such timer id
	return FALSE;
    QTimerEvent e( t->ind + 1 );
    QApplication::sendEvent( t->obj, &e );	// send event
    return TRUE;				// timer event was processed
}

static void activateZeroTimers()		// activate full-speed timers
{
    if ( !timerVec )
	return;
    uint i=0;
    register TimerInfo *t = 0;
    int n = numZeroTimers;
    while ( n-- ) {
	for ( ;; ) {
	    t = timerVec->at(i++);
	    if ( t && t->zero )
		break;
	    else if ( i == timerVec->size() )		// should not happen
		return;
	}
	QTimerEvent e( t->ind + 1 );
	QApplication::sendEvent( t->obj, &e );
    }
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerVec = new TimerVec( 128 );
    Q_CHECK_PTR( timerVec );
    timerVec->setAutoDelete( TRUE );
    timerDict = new TimerDict( 29 );
    Q_CHECK_PTR( timerDict );
}

static void cleanupTimers()			// remove pending timers
{
    register TimerInfo *t;
    if ( !timerVec )				// no timers were used
	return;
    for ( uint i=0; i<timerVec->size(); i++ ) {		// kill all pending timers
	t = timerVec->at( i );
	if ( t && !t->zero )
	    KillTimer( 0, t->id );
    }
    delete timerDict;
    timerDict = 0;
    delete timerVec;
    timerVec  = 0;

    if ( qt_win_use_simple_timers ) {
	// Dangerous to leave WM_TIMER events in the queue if they have our
	// timerproc (eg. Qt-based DLL plugins may be unloaded)
	MSG msg;
	while (winPeekMessage( &msg, (HWND)-1, WM_TIMER, WM_TIMER, PM_REMOVE ))
	    continue;
    }
}


//
// Main timer functions for starting and killing timers
//


int qStartTimer( int interval, QObject *obj )
{
    register TimerInfo *t;
    if ( !timerVec )				// initialize timer data
	initTimers();
    int ind = timerVec->findRef( 0 );		// get free timer
    if ( ind == -1 || !obj ) {
	ind = timerVec->size();			// increase the size
	timerVec->resize( ind * 4 );
    }
    t = new TimerInfo;				// create timer entry
    Q_CHECK_PTR( t );
    t->ind  = ind;
    t->obj  = obj;

    if ( qt_win_use_simple_timers ) {
	t->zero = FALSE;
	t->id = SetTimer( 0, 0, (uint)interval,
			  (TIMERPROC)qt_simple_timer_func );
    } else {
	t->zero = interval == 0;
	if ( t->zero ) {			// add zero timer
	    t->id = (uint)50000 + ind;		// unique, high id ##########
	    numZeroTimers++;
	} else {
	    t->id = SetTimer( 0, 0, (uint)interval, 0 );
	}
    }
    if ( t->id == 0 ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "qStartTimer: Failed to create a timer." );
#endif
	delete t;				// could not set timer
	return 0;
    }
    timerVec->insert( ind, t );			// store in timer vector
    timerDict->insert( t->id, t );		// store in dict
    return ind + 1;				// return index in vector
}

bool qKillTimer( int ind )
{
    if ( !timerVec || ind <= 0 || (uint)ind > timerVec->size() )
	return FALSE;
    register TimerInfo *t = timerVec->at(ind-1);
    if ( !t )
	return FALSE;
    if ( t->zero )
	numZeroTimers--;
    else
	KillTimer( 0, t->id );
    timerDict->remove( t->id );
    timerVec->remove( ind-1 );
    return TRUE;
}

bool qKillTimer( QObject *obj )
{
    if ( !timerVec )
	return FALSE;
    register TimerInfo *t;
    for ( uint i=0; i<timerVec->size(); i++ ) {
	t = timerVec->at( i );
	if ( t && t->obj == obj ) {		// object found
	    if ( t->zero )
		numZeroTimers--;
	    else
		KillTimer( 0, t->id );
	    timerDict->remove( t->id );
	    timerVec->remove( i );
	}
    }
    return TRUE;
}

/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

typedef QIntDict<QSockNot> QSNDict;

static QSNDict *sn_read	  = 0;
static QSNDict *sn_write  = 0;
static QSNDict *sn_except = 0;

static QSNDict**sn_vec[3] = { &sn_read, &sn_write, &sn_except };

uint	qt_sn_msg	  = 0;			// socket notifier message
static QWidget *sn_win	  = 0;			// win msg via this window


static void sn_cleanup()
{
    delete sn_win;
    sn_win = 0;
    for ( int i=0; i<3; i++ ) {
	delete *sn_vec[i];
	*sn_vec[i] = 0;
    }
}

static void sn_init()
{
    if ( sn_win )
	return;
    qAddPostRoutine( sn_cleanup );
#ifdef Q_OS_TEMP
    qt_sn_msg = RegisterWindowMessage(L"QtSNEvent");
#else
    qt_sn_msg = RegisterWindowMessageA( "QtSNEvent" );
#endif
    sn_win = new QWidget(0,"QtSocketNotifier_Internal_Widget");
    Q_CHECK_PTR( sn_win );
    for ( int i=0; i<3; i++ ) {
	*sn_vec[i] = new QSNDict;
	Q_CHECK_PTR( *sn_vec[i] );
	(*sn_vec[i])->setAutoDelete( TRUE );
    }
}

void qt_sn_activate_fd( int sockfd, int type )
{
    QSNDict  *dict = *sn_vec[type];
    QSockNot *sn   = dict ? dict->find(sockfd) : 0;
    if ( sn ) {
	QApplication::eventLoop()->setSocketNotifierPending( sn->obj );
    }
}

/*****************************************************************************
  QEventLoop Implementation
 *****************************************************************************/

void QEventLoop::init()
{
    qt_gui_thread = GetCurrentThreadId();
}

void QEventLoop::cleanup()
{
    // cleanup the common parts of the event loop
    cleanupTimers();
    sn_cleanup();
}

void QEventLoop::registerSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return;
    }

    QSNDict  *dict = *sn_vec[type];

    if ( !dict && QApplication::closingDown() )
	return; // after sn_cleanup, don't reinitialize.

    QSockNot *sn;
    if ( sn_win == 0 ) {
	sn_init();
	dict = *sn_vec[type];
    }
    sn = new QSockNot;
    Q_CHECK_PTR( sn );
    sn->obj = notifier;
    sn->fd  = sockfd;
#if defined(QT_CHECK_STATE)
    if ( dict->find(sockfd) ) {
	static const char *t[] = { "read", "write", "exception" };
	qWarning( "QSocketNotifier: Multiple socket notifiers for "
		    "same socket %d and type %s", sockfd, t[type] );
    }
#endif
    dict->insert( sockfd, sn );

#ifndef Q_OS_TEMP
    int sn_event = 0;
    if ( sn_read && sn_read->find(sockfd) )
	sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if ( sn_write && sn_write->find(sockfd) )
	sn_event |= FD_WRITE | FD_CONNECT;
    if ( sn_except && sn_except->find(sockfd) )
	sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect( sockfd, sn_win->winId(), sn_event ? qt_sn_msg : 0, sn_event );
#else
/*
	fd_set	rd,wt,ex;
	FD_ZERO(&rd);
	FD_ZERO(&wt);
	FD_ZERO(&ex);
    if ( sn_read && sn_read->find(sockfd) )
		FD_SET( sockfd, &rd );
    if ( sn_write && sn_write->find(sockfd) )
		FD_SET( sockfd, &wt );
    if ( sn_except && sn_except->find(sockfd) )
		FD_SET( sockfd, &ex );
//	select( 1, &rd, &wt, &ex, NULL );
*/
#endif

}

void QEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return;
    }

    QSNDict  *dict = *sn_vec[type];

    if ( !dict && QApplication::closingDown() )
	return; // after sn_cleanup, don't reinitialize.

    if ( dict == 0 )
	return;

    QSockNot *sn = dict->find(sockfd);
    if ( !sn )
	return;
    d->sn_pending_list.removeRef( sn );		// remove from activation list

    if ( !dict->remove(sockfd) )		// did not find sockfd
	return;

#ifndef Q_OS_TEMP // ### This probably needs fixing
    int sn_event = 0;
    if ( sn_read && sn_read->find(sockfd) )
	sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if ( sn_write && sn_write->find(sockfd) )
	sn_event |= FD_WRITE | FD_CONNECT;
    if ( sn_except && sn_except->find(sockfd) )
	sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect( sockfd, sn_win->winId(), sn_event ? qt_sn_msg : 0, sn_event );
#else
/*
	fd_set	rd,wt,ex;
	FD_ZERO(&rd);
	FD_ZERO(&wt);
	FD_ZERO(&ex);
    if ( sn_read && sn_read->find(sockfd) )
		FD_SET( sockfd, &rd );
    if ( sn_write && sn_write->find(sockfd) )
		FD_SET( sockfd, &wt );
    if ( sn_except && sn_except->find(sockfd) )
		FD_SET( sockfd, &ex );
//	select( 1, &rd, &wt, &ex, NULL );
*/
#endif


}

void QEventLoop::setSocketNotifierPending( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return;
    }

    QSNDict  *dict = *sn_vec[type];
    QSockNot *sn   = dict ? dict->find(sockfd) : 0;
    if ( !sn )
	return;

    if ( d->sn_pending_list.findRef( sn ) >= 0 )
	return;
    d->sn_pending_list.append( sn );
}

bool QEventLoop::hasPendingEvents() const
{
    MSG msg;
    return qGlobalPostedEventsCount() || winPeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
}

bool QEventLoop::processEvents( ProcessEventsFlags flags )
{
    MSG	 msg;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker( QApplication::qt_mutex );
#endif
    emit awake();
    emit qApp->guiThreadAwake();

    QApplication::sendPostedEvents();

    if ( flags & ExcludeUserInput ) {
	while ( winPeekMessage(&msg,0,0,0,PM_NOREMOVE) ) {
	    if ( (msg.message >= WM_KEYFIRST &&
		 msg.message <= WM_KEYLAST) ||
		 (msg.message >= WM_MOUSEFIRST &&
		 msg.message <= WM_MOUSELAST) ) {
		winPeekMessage(&msg,0,0,0,PM_REMOVE);
		continue;
	    }
	    break;
	}
    }

    bool canWait = d->exitloop || d->quitnow ? FALSE : (flags & WaitForMore);

    if ( canWait ) {				// can wait if necessary
	if ( numZeroTimers ) {			// activate full-speed timers
	    int ok = FALSE;
	    while ( numZeroTimers &&
		!(ok=winPeekMessage(&msg,0,0,0,PM_REMOVE)) ) {
		activateZeroTimers();
	    }
	    if ( !ok )	{			// no event
		return FALSE;
	    }
	} else {
	    emit aboutToBlock();
#ifdef QT_THREAD_SUPPORT
	    locker.mutex()->unlock();
#endif // QT_THREAD_SUPPORT
	    if ( !winGetMessage(&msg,0,0,0) ) {
#ifdef QT_THREAD_SUPPORT
		locker.mutex()->lock();
#endif // QT_THREAD_SUPPORT
		exit( 0 );				// WM_QUIT received
		return FALSE;
	    }
#ifdef QT_THREAD_SUPPORT
	    locker.mutex()->lock();
#endif // QT_THREAD_SUPPORT
	}
    } else {					// no-wait mode
	if ( !winPeekMessage(&msg,0,0,0,PM_REMOVE) ) { // no pending events
	    if ( numZeroTimers > 0 ) {		// there are 0-timers
		activateZeroTimers();
	    }
	    return FALSE;
	}
    }

    bool handled = FALSE;
    if ( msg.message == WM_TIMER ) {		// timer message received
	if ( dispatchTimer( msg.wParam, &msg ) )
	    return TRUE;
    } else if ( msg.message && (!msg.hwnd || !QWidget::find(msg.hwnd)) ) {
	long res = 0;
	handled = qt_winEventFilter( &msg, res );
    }

    if ( !handled ) {
	QInputContext::TranslateMessage( &msg );			// translate to WM_CHAR

	QT_WA( {
	    DispatchMessage( &msg );		// send to QtWndProc
	} , {
	    DispatchMessageA( &msg );		// send to QtWndProc
	} );
    }

    if ( !(flags & ExcludeSocketNotifiers ) )
	activateSocketNotifiers();

    if ( configRequests )			// any pending configs?
	qWinProcessConfigRequests();
    QApplication::sendPostedEvents();

    return TRUE;
}

void QEventLoop::wakeUp()
{
    if ( GetCurrentThreadId() != qt_gui_thread )
	QT_WA( {
	    PostThreadMessageW( qt_gui_thread, WM_NULL, 0, 0 );
	} , {
	    PostThreadMessageA( qt_gui_thread, WM_NULL, 0, 0 );
	} );
}

int QEventLoop::timeToWait() const
{
    return -1;
}

int QEventLoop::activateTimers()
{
    return 0;
}

int QEventLoop::activateSocketNotifiers()
{
    if ( d->sn_pending_list.isEmpty() )
	return 0; // nothing to do

    int n_act = 0;
    QEvent event( QEvent::SockAct );
    QPtrListIterator<QSockNot> it( d->sn_pending_list );
    QSockNot *sn;
    while ( (sn=it.current()) ) {
	++it;
	d->sn_pending_list.removeRef( sn );
	QApplication::sendEvent( sn->obj, &event );
	n_act++;
    }

    return n_act;
}
