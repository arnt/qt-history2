/****************************************************************************
**
** Implementation of QEventLoop class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qguieventloop_p.h"
#include "qcoreapplication.h"
#include <private/qeventloop_p.h>
#include "qmutex.h"
#include "qwidget.h"
#include "qinputcontext_p.h"

#define d d_func()
#define q q_func()

extern bool qt_winEventFilter(MSG* msg);
Q_CORE_EXPORT bool activateTimer( uint id );		// activate timer
Q_CORE_EXPORT void activateZeroTimers();
Q_CORE_EXPORT bool winPeekMessage( MSG*, HWND, UINT, UINT, UINT);
Q_CORE_EXPORT bool winGetMessage( MSG*, HWND, UINT, UINT);

Q_CORE_EXPORT int numZeroTimers;

void QGuiEventLoop::init()
{

}

void QGuiEventLoop::cleanup()
{

}

extern Q_CORE_EXPORT bool qt_dispatch_timer( uint timerId, MSG *msg );

/*****************************************************************************
  Safe configuration (move,resize,setGeometry) mechanism to avoid
  recursion when processing messages.
 *****************************************************************************/

struct QWinConfigRequest {
    WId	 id;					// widget to be configured
    int	 req;					// 0=move, 1=resize, 2=setGeo
    int	 x, y, w, h;				// request parameters
};

static QList<QWinConfigRequest*> *configRequests = 0;

void qWinRequestConfig( WId id, int req, int x, int y, int w, int h )
{
    if (!configRequests)			// create queue
	configRequests = new QList<QWinConfigRequest*>;
    QWinConfigRequest *r = new QWinConfigRequest;
    r->id = id;					// create new request
    r->req = req;
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    configRequests->append(r);		// store request in queue
}

Q_GUI_EXPORT void qWinProcessConfigRequests()		// perform requests in queue
{
    if ( !configRequests )
	return;
    QWinConfigRequest *r;
    for ( ;; ) {
	if ( configRequests->isEmpty() )
	    break;
	r = configRequests->takeLast();
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

bool QGuiEventLoop::processEvents(ProcessEventsFlags flags)
{
    MSG	 msg;

    emit awake();

    QCoreApplication::sendPostedEvents();

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
    } else if ( msg.message && (!msg.hwnd || !QWidget::find(msg.hwnd)) ) {
	handled = qt_winEventFilter(&msg);
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
    QCoreApplication::sendPostedEvents();

    return TRUE;
}

void QGuiEventLoop::flush()
{

}


