/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_mac.cpp
**
** Implementation of Mac startup routines and event handling
**
** Created : 001018
**
** Copyrigght (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// NOT REVISED
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "qglobal.h"
#include "qt_mac.h"

#include "qapplication.h"
#include "qapplication_p.h"
#include "qcolor_p.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "qobjectlist.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qtextcodec.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qsocketnotifier.h"
#include "qsessionmanager.h"
#include "qvaluelist.h"
#include "qdict.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qwhatsthis.h" // ######## dependency
#include "qwindowsstyle.h" // ######## dependency
#include "qmotifplusstyle.h" // ######## dependency
#include "qpaintdevicemetrics.h"


#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>

#define	 GC GC_QQQ

#if defined(QT_THREAD_SUPPORT)
#include "qthread.h"
#endif


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static Qt::ButtonState mouse_button_state = Qt::NoButton;
static char    *appName;                        // application name
static Cursor *currentCursor;                  //current cursor
QObject	       *qt_clipboard = 0;
QWidget	       *qt_button_down	 = 0;		// widget got last button-down
static unsigned int qt_last_mouse_down = 0;

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;

typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;

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

//timer stuff
static void	initTimers();
static void	cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval		*qt_wait_timer();
timeval 	*qt_wait_timer_max = 0;
int 		qt_activate_timers();

struct QSockNot {
    QObject *obj;
    int	     fd;
    fd_set  *queue;
};

typedef QList<QSockNot> QSNList;
typedef QListIterator<QSockNot> QSNListIt;

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

static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

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


// one day in the future we will be able to have static objects in libraries....
struct QScrollInProgress {
    static long serial;
    QScrollInProgress( QWidget* w, int x, int y ) :
    id( serial++ ), scrolled_widget( w ), dx( x ), dy( y ) {}
    long id;
    QWidget* scrolled_widget;
    int dx, dy;
};
long QScrollInProgress::serial=0;

class QETWidget : public QWidget		// event translator widget
{
public:
    void setWState( WFlags f )		{ QWidget::setWState(f); }
    void clearWState( WFlags f )	{ QWidget::clearWState(f); }
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
};


UnsignedWide thesecs;

void qt_init( int* /* argcptr */, char **argv, QApplication::Type )
{
    char *p;

    // Set application name
   
    p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

// FIXME: which of these are needed and why?
//	InitGraf(&qd.thePort);
//	InitWindows();
	InitCursor();
//	InitMenus();
//	InitFonts();
    if ( qt_is_gui_used ) {
        QColor::initialize();
        QFont::initialize();
        QCursor::initialize();
        QPainter::initialize();
    }
    Microseconds( &thesecs );
    if ( qt_is_gui_used ) {
        qApp->setName( appName );   
    }

    //my desktop hacks, trying to figure out how to get a desktop, this doesn't work
    //but I'm going to leave it for now so I can test some more FIXME!!!
    WindowRef foo = GetFrontWindowOfClass(kDesktopWindowClass, TRUE);
    qDebug("Found something: %d", foo);
}



/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    cleanupTimers();
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
}

void qt_updated_rootinfo()
{

}

bool qt_wstate_iconified( WId )
{
    return FALSE;
}

void qAddPostRoutine( Q_CleanUpFunction )
{
}


void qRemovePostRoutine( Q_CleanUpFunction )
{
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

extern QWidget * mac_mouse_grabber;
extern QWidget * mac_keyboard_grabber;

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor> QCursorList;
static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace)
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor(cursor);
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    if(cursorStack->isEmpty()) {
	app_cursor = NULL;
	delete cursorStack;
	cursorStack = NULL;
    }
}

#endif

void QApplication::setGlobalMouseTracking( bool b)
{
    if(b)
	app_tracking++;
    else
	app_tracking--;
}


static QWidget *recursive_match(QWidget *widg, int x, int y)
{
    // Keep looking until we find ourselves in a widget with no kiddies
    // where the x,y is
    if(!widg) 
	return 0;

    const QObjectList *objl=widg->children();
    if(!objl) // No children 
	return widg;

    QObjectListIt it(*objl);
    for(it.toLast(); it.current(); --it) {
	if((*it)->isWidgetType()) {
	    QWidget *curwidg=(QWidget *)(*it);
	    if(curwidg->isVisible()) {
		int wx=curwidg->x(), wy=curwidg->y();
		int wx2=wx+curwidg->width(), wy2=wy+curwidg->height();
		if(x>=wx && y>=wy && x<=wx2 && y<=wy2) {
		    return recursive_match(curwidg,x-wx,y-wy);
		} 
	    }
	}
    }
    // If we get here, it's within a widget that has children, but isn't in any
    // of the children
    return widg;
}

QWidget *QApplication::widgetAt( int x, int y, bool child)
{
  //find the tld
  Point p;
  p.h=x;
  p.v=y;
  WindowPtr wp;
  FindWindow(p,&wp);

  //get that widget
  QWidget * widget=QWidget::find((WId)wp);
  if(!widget) {
    qWarning("Couldn't find %d",(int)wp);
    return 0;
  }

  //find the child
  if(child) {
    GlobalToLocal( &p ); //now map it to the window
    widget = recursive_match(widget, p.h, p.v);
  }
  return widget;
}

void QApplication::beep()
{
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    quit_now = FALSE;
    quit_code = 0;

#if defined(QT_THREAD_SUPPORT)
    qApp->unlock(FALSE);
#endif

    enter_loop();

    return quit_code;
}


//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef QList<TimerInfo> TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list


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
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
	t = timerList->next();
	index++;
    }
    timerList->insert( index, ti );		// inserts sorted
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
    CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
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
    CHECK_PTR( t );
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
	CHECK_PTR( sn_act_list );
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
	    CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
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


bool QApplication::processNextEvent( bool canWait )
{
  int	   nevents = 0;

  if(qt_is_gui_used) {
    sendPostedEvents();
    EventRecord event;    
    do {
	do {
	if(app_exit_loop)
	  return FALSE;

	GetNextEvent(everyEvent, &event);
	nevents++;
	if(macProcessEvent( (MSG *)(&event) ) == 1)
	  return TRUE;

      } while(EventAvail(everyEvent, &event));
     } while(EventAvail(everyEvent, &event));
    sendPostedEvents(); //let them accumulate
  } 

  if ( quit_now || app_exit_loop )
    return FALSE;
  sendPostedEvents();

  static timeval zerotm;
  timeval *tm = qt_wait_timer();		// wait for timer or X event
  if ( !canWait ) {
    if ( !tm )
      tm = &zerotm;
    tm->tv_sec  = 0;			// no time to wait
    tm->tv_usec = 0;
  }
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

  if ( qt_preselect_handler ) {
    QVFuncList::Iterator end = qt_preselect_handler->end();
    for ( QVFuncList::Iterator it = qt_preselect_handler->begin(); it != end; ++it )
      (**it)();
  }

  //we don't need this for now, when we want to to socket's and stuff we'll need to massage this some.
#if 0
  printf("Doing select..\n");
  int nsel = select( sn_highest + 1,
		     (&app_readfds),
		     (sn_write  ? &app_writefds  : 0),
		     (sn_except ? &app_exceptfds : 0),
		     tm );
  printf("Done selecting..\n");
#else
  int nsel = 0;
#endif


  if ( qt_postselect_handler ) {
    QVFuncList::Iterator end = qt_postselect_handler->end();
    for ( QVFuncList::Iterator it = qt_postselect_handler->begin(); it != end; ++it )
      (**it)();
  }

  if ( nsel == -1 ) {
    if ( errno == EINTR || errno == EAGAIN ) {
      errno = 0;
      return (nevents > 0);
    } else {
      ; // select error
    }
  } else if ( nsel > 0 && sn_highest >= 0 ) {
    nevents += sn_activate();
  }

  nevents += qt_activate_timers();		// activate timers
  //  qt_reset_color_avail();			// color approx. optimization
  
  return (nevents > 0);
}

/* key maps */
struct key_sym
{
    int mac_code;
    int qt_code;
    const char *desc;
};

static key_sym modifier_syms[] = {
{ shiftKey, Qt::ShiftButton, "Qt::Shift" },
{ controlKey, Qt::ControlButton, "Qt::ControlButton" },
{ rightControlKey, Qt::ControlButton, "Qt::ControlButton" },
{ optionKey, Qt::AltButton, "Qt::AltButton" },
{ rightOptionKey, Qt::AltButton, "Qt::AltButton" },
{   0, 0, NULL }
};
static int get_modifiers(int key)
{
    int ret = 0;
    for(int i = 0; modifier_syms[i].desc; i++) {
	if(key & modifier_syms[i].mac_code) {
//	    qDebug("got modifier: %s", modifier_syms[i].desc);
	    ret |= modifier_syms[i].qt_code;
	}
    }
    return ret;
}

static key_sym key_syms[] = {
{ kHomeCharCode, Qt::Key_Home, "Qt::Home" },
{ kEnterCharCode, Qt::Key_Enter, "Qt::Key_Enter" },
{ kEndCharCode, Qt::Key_End, "Qt::Key_End" },
{ kBackspaceCharCode, Qt::Key_Backspace, "Qt::Backspace" },
{ kTabCharCode, Qt::Key_Tab, "Qt::Tab" },
{ kPageUpCharCode, Qt::Key_PageUp, "Qt::PageUp" },
{ kPageDownCharCode, Qt::Key_PageDown, "Qt::PageDown" },
{ kReturnCharCode, Qt::Key_Return, "Qt::Key_Return" },
//function keys?
{ kEscapeCharCode, Qt::Key_Escape, "Qt::Key_Escape" },
{ kLeftArrowCharCode, Qt::Key_Left, "Qt::Key_Left" },
{ kRightArrowCharCode, Qt::Key_Right, "Qt::Key_Right" },
{ kUpArrowCharCode, Qt::Key_Up, "Qt::Key_Up" },
{ kDownArrowCharCode, Qt::Key_Down, "Qt::Key_Down" },
{ kDeleteCharCode, Qt::Key_Delete, "Qt::Key_Delete" }
};
static int get_key(int key)
{
    for(int i = 0; key_syms[i].desc; i++) {
	if(key_syms[i].mac_code == key) {
//	    qDebug("got key: %s", key_syms[i].desc);
	    return key_syms[i].qt_code;
	}
    }
    //qDebug("Falling back to ::%c::", key);
    return key;
}


bool mouse_down=false;
extern WId myactive;

bool QApplication::do_mouse_down( EventRecord* es )
{
  EventRecord *er = (EventRecord *)es;
  WindowPtr wp;
  short windowPart;
  Point wherePoint = er->where;
  windowPart = FindWindow( er->where, &wp );
  QWidget *widget;
  int growWindowSize = 0;
  bool in_widget = FALSE;

  switch( windowPart ) {
  case inGoAway:
    widget=QWidget::find( (WId)wp );
    if( widget ) {
      if( widget->close( FALSE ) ) {
	widget->hide();
      } else {
	qDebug( "do_mouse_down: widget not found" );
      }
    } else {
      qWarning("Close for unknown widget");
    }
    break;
  case inDrag:
    DragWindow( wp, er->where, 0 );
    break;
  case inContent:
    in_widget = TRUE;
    break;
  case inGrow:
    Rect limits;
    widget = QWidget::find( (WId)wp );
   
    if( widget ) {
      if ( widget->extra ) {
	SetRect( &limits, widget->extra->minw, widget->extra->minh,
		 widget->extra->maxw, widget->extra->maxh);
      }
    }
    growWindowSize = GrowWindow( wp, wherePoint, &limits);
    if( growWindowSize) {
      // nw/nh might not match the actual size if setSizeIncrement
      // is used
      int nw = LoWord( growWindowSize );
      int nh = HiWord( growWindowSize );

      // FIXME: Replace hard coded constants with display size 
      if( nw < 1600 && nw > 0 && nh < 1200 && nh > 0 ) {
	if( widget ) {
	  int ow = widget->width(), oh = widget->height();
	  widget->resize( nw, nh );

	  QResizeEvent qre( QSize( widget->width(), widget->height() ), QSize( ow, oh) );
	  QApplication::sendEvent( widget, &qre );
	  widget->resizeEvent( &qre );
	}
      }
    }
    break;
  case inZoomIn:
  case inZoomOut:
    if( TrackBox( wp, er->where, windowPart ) == true ) {
      Rect bounds;
      SetPortWindowPort( wp );
      GetPortBounds( GetWindowPort( wp ), &bounds );
      EraseRect( &bounds );
      ZoomWindow( wp, windowPart, false);
      InvalWindowRect( wp, &bounds );
    }
    break;
  }
  return in_widget;
}

/* almost all of this is bogus, it needs to be fixed on the next pass. Basically it needs to use
   a qetwidget for event translation, mouse events and popups are wrong too, FIXME!! */
int QApplication::macProcessEvent(MSG * m)
{
    QWidget *widget;
    WindowPtr wp;
    EventRecord *er = (EventRecord *)m;
    if(!er->what)
	return 0;

    if ( er->what == updateEvt ) {
	widget = QWidget::find( (WId)er->message );

	if(!widget) {
	    qWarning("Couldn't find paint widget for %d!",(int)wp);
	} else {
	    int metricWidth = widget->metric (QPaintDeviceMetrics::PdmWidth );
	    int metricHeight = widget->metric( QPaintDeviceMetrics::PdmHeight );
	    widget->crect.setWidth( metricWidth - 1 );
	    widget->crect.setHeight( metricHeight - 1 );

	    BeginUpdate((WindowPtr)widget->handle());
	    qt_set_paintevent_clipping( widget, QRegion( 0, 0, widget->width(), widget->height() ));
	    widget->propagateUpdates( 0, 0, widget->width(), widget->height() );
	    qt_clear_paintevent_clipping();
	    EndUpdate((WindowPtr)widget->handle());
	}
    } else if(er->what == keyDown) {
	if( mac_keyboard_grabber ) 
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;
	else //last ditch effort
	    widget = QApplication::widgetAt(er->where.h, er->where.v, true);
    
	if(widget) {
	    int mychar=get_key(er->message & charCodeMask);
	    QKeyEvent ke(QEvent::KeyPress,mychar, mychar, 
			 get_modifiers(er->modifiers), QString(QChar(mychar)));
	    QApplication::sendEvent(widget,&ke);
	} 
    } else if(er->what == keyUp) {
	if( mac_keyboard_grabber ) 
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;
	else //last ditch effort
	    widget = QApplication::widgetAt(er->where.h, er->where.v, true);
    
	if(widget) {
	    int mychar=get_key(er->message & charCodeMask);
	    QKeyEvent ke(QEvent::KeyRelease,mychar, mychar, 
			 get_modifiers(er->modifiers), QString(QChar(mychar)));
	    QApplication::sendEvent(widget,&ke);
	} 
    } else if(er->what == activateEvt) {
	widget = QWidget::find( (WId)er->message );	
	if(widget && (er->modifiers & 0x01)) 
	    setActiveWindow(widget);
    } else if( er->what == mouseDown  || er->what == mouseUp ) {
	short part = FindWindow( er->where, &wp );
	if( part == inContent ) {
	    QEvent::Type etype = QEvent::None;
	    int button=0, state=0;
	    if(er->what == mouseDown) {
		//check if this is the second click, there must be a way to make the
		//mac do this for us, FIXME!!
		if(qt_last_mouse_down && (er->when - qt_last_mouse_down <= 
					  (uint)mouse_double_click_time)) {
		    etype = QEvent::MouseButtonDblClick;
		    qt_last_mouse_down = 0;
		} else {
		    etype = QEvent::MouseButtonPress;
		    qt_last_mouse_down = er->when;
		}

		//gross, need to handle more mouse buttons, FIXME
		button = mouse_button_state = QMouseEvent::LeftButton; 
	    } else {
		etype = QEvent::MouseButtonRelease;

		//gross, need to handle more mouse buttons, FIXME
		button = state = mouse_button_state;
		mouse_button_state = Qt::NoButton;
	    }

	    //handle popup's first
	    QWidget *popupwidget = NULL;
	    if( inPopupMode() ) {
		popupwidget = activePopupWidget();
		SetPortWindowPort((WindowPtr)widget->handle());
		Point gp = er->where;
		GlobalToLocal( &gp ); //now map it to the window
		popupwidget = recursive_match(popupwidget, gp.h, gp.v);

		QPoint p( er->where.h, er->where.v );
		QPoint plocal(popupwidget->mapFromGlobal( p ));
		QMouseEvent qme( etype, plocal, p, button, state );
		QApplication::sendEvent( popupwidget, &qme );
	    } 
	    
	    //figure out which widget to send it to
	    if( er->what == mouseUp && qt_button_down )
	     	widget = qt_button_down;
            else if( mac_mouse_grabber )
		widget = mac_mouse_grabber;
	    else 
		widget = QApplication::widgetAt( er->where.h, er->where.v, true );

	    //setup the saved widget
	    qt_button_down = er->what == mouseDown ? widget : NULL;

	    //finally send the event to the widget if its not the popup
	    if ( widget && widget != popupwidget ) {
		if(er->what == mouseDown) {
		    QWidget* w = widget;
		    while ( w->focusProxy() )
			w = w->focusProxy();
		    if ( w->focusPolicy() & QWidget::ClickFocus ) {
			QFocusEvent::setReason( QFocusEvent::Mouse);
			w->setFocus();
			QFocusEvent::resetReason();
		    }
		}

		QPoint p( er->where.h, er->where.v );
		QPoint plocal(widget->mapFromGlobal( p ));
		QMouseEvent qme( etype, plocal, p, button, state );
		QApplication::sendEvent( widget, &qme );
//		qDebug("Would send event to %s %s", widget->name(), widget->className());
	    }
	}
	//mousedown's will effect stuff outside InContent as well
	else if(er->what == mouseDown) {
	    do_mouse_down( er ); //do resize/move stuff
	}
    } else if(er->what == osEvt) {
	if(((er->message >> 24) & 0xFF) == mouseMovedMessage) {
	    short part = FindWindow( er->where, &wp );
	    if( part == inContent ) {
		if( inPopupMode() ) {
		    widget = activePopupWidget();
		    SetPortWindowPort((WindowPtr)widget->handle());
		    Point p = er->where;
		    GlobalToLocal( &p ); //now map it to the window
		    widget = recursive_match(widget, p.h, p.v);
		} else if( qt_button_down ) {
		    widget = qt_button_down;
		} else if( mac_mouse_grabber ) {
		    widget = mac_mouse_grabber;
		} else {
		    widget = QApplication::widgetAt( er->where.h, er->where.v, true );
		}
		if ( widget ) {
		    //set the cursor up
		    Cursor *n = NULL;
		    if(widget->extra && widget->extra->curs) 
			n = (Cursor *)widget->extra->curs->handle();
		    else if(cursorStack)
			n = (Cursor *)app_cursor->handle();
		    if(!n)
			n = (Cursor *)arrowCursor.handle(); //I give up..
		    if(currentCursor != n) 
			SetCursor(currentCursor = n);
		
		    //ship the event
		    QPoint p( er->where.h, er->where.v );
		    QPoint plocal(widget->mapFromGlobal( p ));
		    QMouseEvent qme( QEvent::MouseMove, plocal, p, 
				     QMouseEvent::NoButton, mouse_button_state);
		    QApplication::sendEvent( widget, &qme );
		}
	    }
	}
	else 
	    printf("Damn!\n");
    } else {
	qWarning("  Type %d",er->what);
    }
    return 0;
}

void QApplication::wakeUpGuiThread()
{
}

/*****************************************************************************
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  qt_enter_modal()
	Enters modal state
	Arguments:
	    QWidget *widget	A modal widget

  qt_leave_modal()
	Leaves modal state for a widget
	Arguments:
	    QWidget *widget	A modal widget
 *****************************************************************************/

void qt_enter_modal( QWidget * )
{
}


void qt_leave_modal( QWidget * )
{
}


void QApplication::processEvents( int maxtime)
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !app_exit_loop && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}

bool QApplication::macEventFilter( void ** )
{
    return 0;
}



/*****************************************************************************
  Popup widget mechanism

  openPopup()
	Adds a widget to the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be added

  closePopup()
	Removes a widget from the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be removed
 *****************************************************************************/

void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
    }
    popupWidgets->append( popup );		// add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    active_window = popup;
    if (active_window->focusWidget())
	active_window->focusWidget()->setFocus();
    else
	active_window->setFocus();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;

    popupWidgets->removeRef( popup );
    if (popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	active_window = (*activeBeforePopup);
	// restore the former active window immediately, although
	// we'll get a focusIn later
	if ( active_window )
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
    } else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	 active_window = popupWidgets->getLast();
	 if (active_window->focusWidget())
	     active_window->focusWidget()->setFocus();
	 else
	     active_window->setFocus();
     }
}

/*****************************************************************************
  Event translation; translates X11 events to Qt events
 *****************************************************************************/


void  QApplication::setCursorFlashTime( int msecs )
{
    cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

void QApplication::setDoubleClickInterval( int ms )
{
    mouse_double_click_time = ms;
}


//FIXME: What is the default value on the Mac?
int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}


void QApplication::setWheelScrollLines( int n )
{
    wheel_scroll_lines = n;
}

int QApplication::wheelScrollLines()
{
    return wheel_scroll_lines;
}

void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    switch (effect) {
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeMenu:
	if ( enable )
	    animate_menu = TRUE;
	fade_menu = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_FadeTooltip:
	if ( enable )
	    animate_tooltip = TRUE;
	fade_tooltip = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }
}

bool QApplication::isEffectEnabled( Qt::UIEffect )
{
    return FALSE;
}



/*****************************************************************************
  Session management support
 *****************************************************************************/

#ifdef QT_NO_SM_SUPPORT

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "qt_sessionmanager" )
{
    d = new QSessionManagerData;
    d->sessionId = session;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

void* QSessionManager::handle() const
{
    return 0;
}

bool QSessionManager::allowsInteraction()
{
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return TRUE;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}

void QSessionManager::setRestartHint( QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand( const QStringList& command)
{
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand( const QStringList& command)
{
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    return d->discardCommand;
}

void QSessionManager::setManagerProperty( const QString&, const QString&)
{
}

void QSessionManager::setManagerProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return FALSE;
}

void QSessionManager::requestPhase2()
{
}

#else // QT_NO_SM_SUPPORT


class QSmSocketReceiver : public QObject
{
    Q_OBJECT
public:
    QSmSocketReceiver( int socket )
	: QObject(0,0)
	{
	    QSocketNotifier* sn = new QSocketNotifier( socket, QSocketNotifier::Read, this );
	    connect( sn, SIGNAL( activated(int) ), this, SLOT( socketActivated(int) ) );
	}

public slots:
     void socketActivated(int);
};


// workaround for broken libsm, see below
struct QT_smcConn {
    unsigned int save_yourself_in_progress : 1;
    unsigned int shutdown_in_progress : 1;
};

void QSmSocketReceiver::socketActivated(int)
{
}

#include "qapplication_mac.moc"

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData;
    d->sessionId = session;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    delete d;
}

QString QSessionManager::sessionId() const
{
    return "";
}

bool QSessionManager::allowsInteraction()
{
    return FALSE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return FALSE;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}

void QSessionManager::setRestartHint( QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand( const QStringList& )
{
}

QStringList QSessionManager::restartCommand() const
{
    return QStringList();
}

void QSessionManager::setDiscardCommand( const QStringList& )
{
}

QStringList QSessionManager::discardCommand() const
{
    return QStringList();
}

void QSessionManager::setManagerProperty( const QString&, const QString& )
{
}

void QSessionManager::setManagerProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return true;
}

void QSessionManager::requestPhase2()
{
}


#endif // QT_NO_SM_SUPPORT
