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

#include "qplatformdefs.h"
#include "qeventloop.h"
#include "qapplication.h"
#include "qbitarray.h"
#include "qwscommand_qws.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "qwindowsystem_qws.h"
#include "qptrqueue.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#include <errno.h>

#include "qeventloop_p.h"
#define d d_func()
#define q q_func()

// from qapplication.cpp
extern bool qt_is_gui_used;

// from qeventloop_unix.cpp
extern timeval *qt_wait_timer();
extern void cleanupTimers();

// from qapplication_qws.cpp
extern QWSDisplay* qt_fbdpy; // QWS `display'
QLock *QWSDisplay::lock = 0;

bool qt_disable_lowpriority_timers=FALSE;

// ### this needs to go away at some point...
typedef void (*VFPTR)();
typedef QList<VFPTR> QVFuncList;
void qt_install_preselect_handler( VFPTR );
void qt_remove_preselect_handler( VFPTR );
static QVFuncList *qt_preselect_handler = 0;
void qt_install_postselect_handler( VFPTR );
void qt_remove_postselect_handler( VFPTR );
static QVFuncList *qt_postselect_handler = 0;
void qt_install_preselect_handler( VFPTR handler )
{
    if ( !qt_preselect_handler )
	qt_preselect_handler = new QVFuncList;
    qt_preselect_handler->append( handler );
}
void qt_remove_preselect_handler( VFPTR handler )
{
    if ( qt_preselect_handler ) {
	QVFuncList::Iterator it = qt_preselect_handler->find( handler );
	if ( it != qt_preselect_handler->end() )
		qt_preselect_handler->remove( it );
    }
}
void qt_install_postselect_handler( VFPTR handler )
{
    if ( !qt_postselect_handler )
	qt_postselect_handler = new QVFuncList;
    qt_postselect_handler->prepend( handler );
}
void qt_remove_postselect_handler( VFPTR handler )
{
    if ( qt_postselect_handler ) {
	QVFuncList::Iterator it = qt_postselect_handler->find( handler );
	if ( it != qt_postselect_handler->end() )
		qt_postselect_handler->remove( it );
    }
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
    // process events from the QWS server
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker( QApplication::qt_mutex );
#endif

    // handle gui and posted events
    if (qt_is_gui_used ) {
	QApplication::sendPostedEvents();

	while ( qt_fbdpy->eventPending() ) {	// also flushes output buffer
	    if ( d->shortcut ) {
		return FALSE;
	    }

	    QWSEvent *event = qt_fbdpy->getEvent();	// get next event
	    nevents++;

	    bool ret = qApp->qwsProcessEvent( event ) == 1;
	    delete event;
	    if ( ret ) {
		return TRUE;
	    }
	}
    }

    if ( d->shortcut ) {
	return FALSE;
    }

    extern QPtrQueue<QWSCommand> *qt_get_server_queue();
    if ( !qt_get_server_queue()->isEmpty() ) {
	QWSServer::processEventQueue();
    }

    QApplication::sendPostedEvents();

    // don't block if exitLoop() or exit()/quit() has been called.
    bool canWait = d->exitloop || d->quitnow ? FALSE : (flags & WaitForMore);

    // Process timers and socket notifiers - the common UNIX stuff

    // return the maximum time we can wait for an event.
    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or event
    if ( !canWait ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }

    int highest = 0;
    if ( ! ( flags & ExcludeSocketNotifiers ) ) {
	// return the highest fd we can wait for input on
	if ( d->sn_highest >= 0 ) {                     // has socket notifier(s)
	    if (!d->sn_vec[0].list.isEmpty())
		d->sn_vec[0].select_fds = d->sn_vec[0].enabled_fds;
	    else
		FD_ZERO( &d->sn_vec[0].select_fds );

	    if (!d->sn_vec[1].list.isEmpty())
		d->sn_vec[1].select_fds = d->sn_vec[1].enabled_fds;
	    else
		FD_ZERO( &d->sn_vec[1].select_fds );

	    if (!d->sn_vec[2].list.isEmpty())
		d->sn_vec[2].select_fds = d->sn_vec[2].enabled_fds;
	    else
		FD_ZERO( &d->sn_vec[2].select_fds );
	} else {
	    FD_ZERO( &d->sn_vec[0].select_fds );
	    FD_ZERO( &d->sn_vec[1].select_fds );
	    FD_ZERO( &d->sn_vec[2].select_fds );
	}

	highest = d->sn_highest;
    } else {
	FD_ZERO( &d->sn_vec[0].select_fds );
	FD_ZERO( &d->sn_vec[1].select_fds );
	FD_ZERO( &d->sn_vec[2].select_fds );
    }

    FD_SET( d->thread_pipe[0], &d->sn_vec[0].select_fds );
    highest = QMAX( highest, d->thread_pipe[0] );

    if ( canWait )
	emit aboutToBlock();

    if ( qt_preselect_handler ) {
	QVFuncList::Iterator it, end = qt_preselect_handler->end();
	for ( it = qt_preselect_handler->begin(); it != end; ++it )
	    (**it)();
    }

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

    if ( qt_postselect_handler ) {
	QVFuncList::Iterator it, end = qt_postselect_handler->end();
	for ( it = qt_postselect_handler->begin(); it != end; ++it )
	    (**it)();
    }

    // activate socket notifiers
    if ( ! ( flags & ExcludeSocketNotifiers ) && nsel > 0 && d->sn_highest >= 0 ) {
	// if select says data is ready on any socket, then set the socket notifier
	// to pending
	int i;
	for ( i=0; i<3; i++ ) {
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
    nevents += activateTimers();

    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return qGlobalPostedEventsCount() || qt_fbdpy->eventPending();
}
