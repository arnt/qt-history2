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
#include "qcoreapplication.h"
#include "qhash.h"
#include "qsocketnotifier.h"
#include <private/qinputcontext_p.h>
#define d d_func()
#define q q_func()

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

static LRESULT CALLBACK win_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static HWND qt_create_sn_window();

extern uint qGlobalPostedEventsCount();

static DWORD qt_gui_thread = 0;
// Simpler timers are needed when Qt does not have the event loop,
// such as for plugins.
#ifndef Q_OS_TEMP
extern Q_CORE_EXPORT bool	qt_win_use_simple_timers = TRUE;
#else
extern Q_CORE_EXPORT bool	qt_win_use_simple_timers = FALSE;
#endif
void CALLBACK   qt_simple_timer_func( HWND, UINT, UINT, DWORD );
extern Q_CORE_EXPORT bool qt_winEventFilter(MSG* msg);

static TimerVec  *timerVec = 0;
static TimerDict *timerDict = 0;

Q_CORE_EXPORT bool qt_dispatch_timer( uint, MSG * );
Q_CORE_EXPORT bool activateTimer( uint );
Q_CORE_EXPORT void activateZeroTimers();

extern Q_CORE_EXPORT int	 numZeroTimers	= 0;		// number of full-speed timers

Q_CORE_EXPORT bool winPeekMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
		     UINT wMsgFilterMax, UINT wRemoveMsg )
{
    QT_WA( {
	return PeekMessage( msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
    } , {
	return PeekMessageA( msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
    } );
}

Q_CORE_EXPORT bool winPostMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    QT_WA( {
	return PostMessage( hWnd, msg, wParam, lParam );
    } , {
	return PostMessageA( hWnd, msg, wParam, lParam );
    } );
}

Q_CORE_EXPORT bool winGetMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
		     UINT wMsgFilterMax )
{
    QT_WA( {
	return GetMessage( msg, hWnd, wMsgFilterMin, wMsgFilterMax );
    } , {
	return GetMessageA( msg, hWnd, wMsgFilterMin, wMsgFilterMax );
    } );
}


//
// Internal data structure for timers
//

void CALLBACK qt_simple_timer_func( HWND, UINT, UINT idEvent, DWORD )
{
    qt_dispatch_timer( idEvent, 0 );
}


// Activate a timer, used by both event-loop based and simple timers.

bool qt_dispatch_timer( uint timerId, MSG *msg )
{
    if ( !msg || !QCoreApplication::instance() || !qt_winEventFilter(msg))
	return activateTimer(timerId);
    return TRUE;
}


//
// Timer activation (called from the event loop when WM_TIMER arrives)
//

bool activateTimer( uint id )		// activate timer
{
    if ( !timerVec )				// should never happen
	return FALSE;
    register TimerInfo *t = timerDict->value(id);
    if ( !t )					// no such timer id
	return FALSE;
    QTimerEvent e( t->ind + 1 );
    QCoreApplication::sendEvent( t->obj, &e );	// send event
    return TRUE;				// timer event was processed
}

void activateZeroTimers()		// activate full-speed timers
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
	QCoreApplication::sendEvent( t->obj, &e );
    }
}


//
// Main timer functions for starting and killing timers
//


int QEventLoop::registerTimer( int interval, QObject *obj )
{
    register TimerInfo *t;
    if ( !timerVec ) {				// initialize timer data
	timerVec = new TimerVec;
	timerDict = new TimerDict;
    }
    int ind = timerVec->size();
    t = new TimerInfo;				// create timer entry
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
	qSystemWarning( "registerTimer: Failed to create a timer" );
	delete t;				// could not set timer
	return 0;
    }
    timerVec->insert( ind, t );			// store in timer vector
    timerDict->insert( t->id, t );		// store in dict
    return ind + 1;				// return index in vector
}

bool QEventLoop::unregisterTimer( int ind )
{
    if ( !timerVec || ind <= 0 || ind > timerVec->size() )
	return FALSE;
    register TimerInfo *t = timerVec->at(ind-1);
    if ( !t )
	return FALSE;
    if ( t->zero )
	numZeroTimers--;
    else
	KillTimer( 0, t->id );
    timerDict->remove( t->id );
    timerVec->removeAt( ind-1 );
    delete t;
    return TRUE;
}

bool QEventLoop::unregisterTimers( QObject *obj )
{
    if ( !timerVec )
	return FALSE;
    register TimerInfo *t;
    for ( int i=0; i<timerVec->size(); i++ ) {
	t = timerVec->at( i );
	if ( t && t->obj == obj ) {		// object found
	    if ( t->zero )
		numZeroTimers--;
	    else
		KillTimer( 0, t->id );
	    timerDict->remove( t->id );
	    timerVec->removeAt( i );
            delete t;
	}
    }
    return TRUE;
}

/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

typedef QHash<int, QSockNot*> QSNDict;

static QSNDict *sn_read	  = 0;
static QSNDict *sn_write  = 0;
static QSNDict *sn_except = 0;

static QSNDict**sn_vec[3] = { &sn_read, &sn_write, &sn_except };

uint	qt_sn_msg	  = 0;			// socket notifier message
static HWND sn_win	  = 0;			// win msg via this window


static void sn_cleanup()
{
    DestroyWindow(sn_win);
    sn_win = 0;
    for ( int i=0; i<3; i++ ) {
	for(QSNDict::Iterator it = (*sn_vec[i])->begin(); it != (*sn_vec[i])->end(); ++it)
	    delete (*it);
	delete *sn_vec[i];
	*sn_vec[i] = 0;
    }
}

struct q_table_item
{
    int msg;
    const char *name;
};

static q_table_item qws_names[] = {
    { 0x0001, "WM_CREATE" },
    { 0x0002, "WM_DESTROY" },
    { 0x0003, "WM_MOVE" },
    { 0x0005, "WM_SIZE" },
    { 0x0006, "WM_ACTIVATE" },
    { 0x0007, "WM_SETFOCUS" },
    { 0x0008, "WM_KILLFOCUS" },
    { 0x001c, "WM_ACTIVATEAPP" },
    { 0x0018, "WM_SHOWWINDOW" },
    { 0x001f, "WM_SETCURSOR" },
    { 0x0024, "WM_GETMINMAXINFO" },
    { 0x0046, "WM_WINDOWPOSCHANGING" },
    { 0x0047, "WM_WINDOWPOSCHANGED" },
    { 0x007f, "WM_GETICON" },
    { 0x0081, "WM_NCCREATE" },
    { 0x0082, "WM_NCDESTROY" },
    { 0x0083, "WM_CALCSIZE" },
    { 0x0086, "WM_NCACTIVATE" },
    { 0x0100, "WM_KEYDOWN" },
    { 0x0101, "WM_KEYUP" },
    { 0x0102, "WM_CHAR" },
    { 0x0104, "WM_SYSKEYDOWN" },
    { 0x0105, "WM_SYSKEYUP" },
    { 0x0281, "WM_IME_SETCONTEXT" },
    { 0x0282, "WM_IME_NOTIFY" },
    { 0, 0 }
};

static const char *mapMessageName(int msg)
{
    for (int i=0; qws_names[i].msg; ++i) {
	if (qws_names[i].msg == msg)
	    return qws_names[i].name;
    }
    return "-";
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
    sn_win = qt_create_sn_window();
    for ( int i=0; i<3; i++ )
	*sn_vec[i] = new QSNDict;
}

void qt_sn_activate_fd( int sockfd, int type )
{
    QSNDict *dict = *sn_vec[type];
    QSockNot *sn = dict ? dict->value(sockfd) : 0;
    if ( sn )
	QCoreApplication::eventLoop()->setSocketNotifierPending( sn->obj );
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
    if(timerVec) { //cleanup timers
	register TimerInfo *t;
	for ( int i=0; i<timerVec->size(); i++ ) {		// kill all pending timers
	    t = timerVec->at( i );
	    if ( t && !t->zero )
		KillTimer( 0, t->id );
	    delete t;
	}
	delete timerDict;
	timerDict = 0;
	delete timerVec;
	timerVec = 0;

	if ( qt_win_use_simple_timers ) {
	    // Dangerous to leave WM_TIMER events in the queue if they have our
	    // timerproc (eg. Qt-based DLL plugins may be unloaded)
	    MSG msg;
	    while (winPeekMessage( &msg, (HWND)-1, WM_TIMER, WM_TIMER, PM_REMOVE ))
		continue;
	}
    }
    // cleanup the common parts of the event loop
    sn_cleanup();
}

void QEventLoop::registerSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
	qWarning( "QSocketNotifier: Internal error" );
	return;
    }

    QSNDict *dict = *sn_vec[type];

    if ( !dict && QCoreApplication::closingDown() )
	return; // after sn_cleanup, don't reinitialize.

    QSockNot *sn;
    if ( sn_win == 0 ) {
	sn_init();
	dict = *sn_vec[type];
    }

    sn = new QSockNot;
    sn->obj = notifier;
    sn->fd  = sockfd;
    if ( dict->find(sockfd) ) {
	static const char *t[] = { "read", "write", "exception" };
	qWarning( "QSocketNotifier: Multiple socket notifiers for "
		    "same socket %d and type %s", sockfd, t[type] );
    }
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
    WSAAsyncSelect( sockfd, sn_win, sn_event ? qt_sn_msg : 0, sn_event );
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
    select( 1, &rd, &wt, &ex, NULL );
*/
#endif
}

void QEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
	qWarning( "QSocketNotifier: Internal error" );
	return;
    }

    QSNDict *dict = *sn_vec[type];
    if (!dict)
	return;

    QSockNot *sn = dict->value(sockfd);
    if (!sn)
	return;

    d->sn_pending_list.remove( sn );		// remove from activation list

    dict->remove(sockfd);
    delete sn;

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
    WSAAsyncSelect( sockfd, sn_win, sn_event ? qt_sn_msg : 0, sn_event );
#else
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
    select( 1, &rd, &wt, &ex, NULL );
#endif
}

void QEventLoop::setSocketNotifierPending( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
	qWarning( "QSocketNotifier: Internal error" );
	return;
    }

    QSNDict *dict = *sn_vec[type];
    QSockNot *sn   = dict ? (*dict)[sockfd] : 0;
    if (!sn)
	return;

    if ( d->sn_pending_list.indexOf(sn) >= 0 )
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

    emit awake();

    QCoreApplication::sendPostedEvents();

    if ( flags & ExcludeUserInput ) {
	while ( winPeekMessage(&msg,0,0,0,PM_NOREMOVE) ) {
	    if ( (msg.message >= WM_KEYFIRST &&
		 msg.message <= WM_KEYLAST) ||
		 (msg.message >= WM_MOUSEFIRST &&
		 msg.message <= WM_MOUSELAST) ||
		 msg.message == WM_MOUSEWHEEL ) {
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
	    if (!winPeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
		emit aboutToBlock();
	    if ( !winGetMessage(&msg,0,0,0) ) {
		exit( 0 );				// WM_QUIT received
		return FALSE;
	    }
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
	if ( qt_dispatch_timer( msg.wParam, &msg ) )
	    return TRUE;
    }

    if ( !handled ) {
	QT_WA( {
	    DispatchMessage( &msg );		// send to QtWndProc
	} , {
	    DispatchMessageA( &msg );		// send to QtWndProc
	} );
    }

    if ( !(flags & ExcludeSocketNotifiers ) )
	activateSocketNotifiers();

    QCoreApplication::sendPostedEvents();

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
    while (!d->sn_pending_list.isEmpty()) {
	QSockNot *sn = d->sn_pending_list.takeAt(0);
	QCoreApplication::sendEvent( sn->obj, &event );
	n_act++;
    }

    return n_act;
}

static LRESULT CALLBACK win_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if ( msg == qt_sn_msg ) {	// socket notifier message
	int type = -1;
#ifndef Q_OS_TEMP
	switch ( WSAGETSELECTEVENT(lp) ) {
	case FD_READ:
	case FD_CLOSE:
	case FD_ACCEPT:
	    type = 0;
	    break;
	case FD_WRITE:
	case FD_CONNECT:
	    type = 1;
	    break;
	case FD_OOB:
	    type = 2;
	    break;
	}
#endif
	if ( type >= 0 )
	    qt_sn_activate_fd( wp, type );
	return 0;
    } else {
	return  DefWindowProc(hwnd, msg, wp, lp);
    }
}

static HWND qt_create_sn_window()
{
    HINSTANCE hi = qWinAppInst();
    WNDCLASSA wc;
    wc.style = 0;
    wc.lpfnWndProc = win_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "QtSocketNotifier_Internal_Widget";
    if (!RegisterClassA(&wc)) {
	qWarning("Failed to register class: %d\n", GetLastError());
	return 0;
    }
    HWND wnd = CreateWindowA(wc.lpszClassName, 	// classname
			     wc.lpszClassName,	// window name
			     0,                	// style
			     0, 0, 0, 0, 	// geometry
			     0,		       	// parent
			     0,       		// menu handle
			     hi,	 	// application
			     0);		// windows creation data.
    if (!wnd) {
	qWarning("Failed to create socket notifier receiver window: %d\n", GetLastError());
    }
    return wnd;
}
