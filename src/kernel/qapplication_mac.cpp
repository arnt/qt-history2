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
#include "qmenubar.h"

#ifdef Q_WS_MACX
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#endif

#define	 GC GC_QQQ

#if defined(QT_THREAD_SUPPORT)
#include "qthread.h"
#endif

#if defined( Q_WS_MAC9 ) && 0
extern "C" int main(int argc, char **argv) { return 0; }
#endif

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static int mouse_button_state = 0;
static bool	app_do_modal	= FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;		// stack of modal widgets
static char    *appName;                        // application name
static Cursor *currentCursor;                  //current cursor
QObject	       *qt_clipboard = 0;
QWidget	       *qt_button_down	 = 0;		// widget got last button-down
QWidget        *qt_mouseover = 0;

void qt_mac_destroy_widget(QWidget *w)
{
    if(qt_button_down == w)
	qt_button_down = NULL;
    if(qt_mouseover == w)
	qt_mouseover = NULL;
}

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping(QPaintDevice *dev);


static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;

typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

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

void qt_init( int* /* argcptr */, char **argv, QApplication::Type )
{
    // Set application name
    char *p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

    if ( qt_is_gui_used ) {
        qApp->setName( appName );
        QColor::initialize();
        QFont::initialize();
        QCursor::initialize();
        QPainter::initialize();

	QWidget *tlw = new QWidget(NULL, "empty_widget", Qt::WDestructiveClose);
	tlw->hide();

	static EventTypeSpec events[] = {
	    { kEventClassMouse, kEventMouseWheelMoved },
	    { kEventClassMouse, kEventMouseDown },
	    { kEventClassMouse, kEventMouseUp }, 
	    { kEventClassMouse, kEventMouseDragged },
	    { kEventClassMouse, kEventMouseMoved },

	    { kEventClassKeyboard, kEventRawKeyUp },
	    { kEventClassKeyboard, kEventRawKeyDown },
	    { kEventClassKeyboard, kEventRawKeyRepeat },

	    { kEventClassWindow, kEventWindowUpdate },
	    { kEventClassWindow, kEventWindowActivated },
	    { kEventClassWindow, kEventWindowDeactivated },
	    { kEventClassWindow, kEventWindowShown },
	    { kEventClassWindow, kEventWindowHidden },

	    { kEventClassApplication, kEventAppActivated }, 
	    { kEventClassApplication, kEventAppDeactivated }
	};
	InstallEventHandler( GetApplicationEventTarget(), 
			     NewEventHandlerUPP(QApplication::globalEventProcessor), 
			     GetEventTypeCount(events), events, 
			     (void *)qApp, NULL);
    }
}



/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( postRList ) {
	QVFuncList::Iterator it = postRList->begin();
	while ( it != postRList->end() ) {	// call post routines
	    (**it)();
	    postRList->remove( it );
	    it = postRList->begin();
	}
	delete postRList;
	postRList = 0;
    }
    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
    QFont::cleanup();
    QColor::cleanup();
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

void qAddPostRoutine( Q_CleanUpFunction p)
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	Q_CHECK_PTR( postRList );
    }
    postRList->prepend( p );
}


void qRemovePostRoutine( Q_CleanUpFunction p )
{
    if ( !postRList ) return;

    QVFuncList::Iterator it = postRList->begin();

    while ( it != postRList->end() ) {
	if ( *it == p ) {
	    postRList->remove( it );
	    it = postRList->begin();
	}
    }
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
	Q_CHECK_PTR( cursorStack );
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
	    if(curwidg->isVisible() && !curwidg->isTopLevel()) {
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
    if(!wp)
	return NULL; //oh well, not my widget!

    //get that widget
    QWidget * widget=QWidget::find((WId)wp);
    if(!widget) {
	qWarning("Couldn't find %d",(int)wp);
	return 0;
    }

    //find the child
    if(child) {
	QMacSavedPortInfo savedInfo;
	SetPortWindowPort( wp );
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

/* timer code */
struct TimerInfo {
    QObject *obj;
    EventLoopTimerRef id;
};
typedef QList<TimerInfo> TimerList;	// list of TimerInfo structs
static TimerList *timerList	= 0;		// timer list

/* timer call back */
Q_PASCAL static void qt_activate_timers(EventLoopTimerRef, void *data)
{
    TimerInfo *t = (TimerInfo *)data;
    QTimerEvent e( (int)t->id );
    QApplication::sendEvent( t->obj, &e );	// send event
}

//
// Timer initialization and cleanup routines
//
static void initTimers()			// initialize timers
{
    timerList = new TimerList;
    Q_CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	timerList = 0;
    }
}

//
// Main timer functions for starting and killing timers
//
int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    TimerInfo *t = new TimerInfo;		// create timer
    Q_CHECK_PTR( t );
    EventTimerInterval mint = (((EventTimerInterval)interval) / 1000) + 0.00001;
if(! InstallEventLoopTimer(GetMainEventLoop(), mint, mint,
			       NewEventLoopTimerUPP(qt_activate_timers), t, &t->id) ) {
	t->obj = obj;
	timerList->append(t);
	return (int)t->id;
    }
    delete t;
    return 0;
}

bool qKillTimer( int id )
{
    if ( !timerList || id <= 0)
	return FALSE;				// not init'd or invalid timer
    register TimerInfo *t = timerList->first();
    while ( t && t->id != (EventLoopTimerRef)id ) // find timer info in list
	t = timerList->next();
    if ( t ) { 					// id found
	RemoveEventLoopTimer(t->id);
	return timerList->remove();
    }
    return FALSE; // id not found
}

bool qKillTimer( QObject *obj )
{
    if ( !timerList )				// not initialized
	return FALSE;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    RemoveEventLoopTimer(t->id);
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}

#ifdef Q_OS_MACX
//socket stuff
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
#else
bool qt_set_socket_handler( int, int, QObject *, bool )
{
    return FALSE;
}
#warning "need to implement sockets on mac9"
#endif


bool QApplication::processNextEvent( bool canWait )
{
    int	   nevents = 0;

    if(qt_is_gui_used) {
	sendPostedEvents();

	/* this gives QD a chance to flush buffers, I don't like doing it 
	   FIXME!! */
	EventRecord ev;
	EventAvail(everyEvent, &ev);

	EventRef event;
	OSStatus ret;
	do {
	    ret = ReceiveNextEvent( 0, 0, kEventDurationNoWait, TRUE, &event );
	    if(ret == eventLoopTimedOutErr || ret == eventLoopQuitErr) 
		break;
	    ret = SendEventToApplication(event);
	    ReleaseEvent(event);
	    if(ret != noErr)
		break;
	    nevents++;
	} while(1);
	sendPostedEvents();
    }

#ifndef QT_NO_CLIPBOARD
    //manufacture an event so the clipboard can see if it has changed
    if(qt_clipboard) {
	QEvent ev(QEvent::Clipboard);
	QApplication::sendEvent(qt_clipboard, &ev);
    }
#endif

    if ( quit_now || app_exit_loop )
	return FALSE;
    sendPostedEvents();

    if ( qt_preselect_handler ) {
	QVFuncList::Iterator end = qt_preselect_handler->end();
	for ( QVFuncList::Iterator it = qt_preselect_handler->begin(); 
	      it != end; ++it )
	    (**it)();
    }

#ifdef Q_OS_MACX
    timeval tm;
    tm.tv_sec  = 0;			// no time to wait
    tm.tv_usec = 0;
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
    int nsel = select( sn_highest + 1, (&app_readfds), 
		       (sn_write  ? &app_writefds  : 0), 
		       (sn_except ? &app_exceptfds : 0), &tm );
#else
#warning "need to implement sockets on mac9"
#endif

    if ( qt_postselect_handler ) {
	QVFuncList::Iterator end = qt_postselect_handler->end();
	for ( QVFuncList::Iterator it = qt_postselect_handler->begin(); 
	      it != end; ++it )
	    (**it)();
    }

#ifdef Q_OS_MACX
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
#else
#warning "need to implement sockets on mac9"
#endif

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
//    qDebug("Falling back to ::%d::", key);
    return key;
}


bool QApplication::do_mouse_down( Point *pt )
{
    WindowPtr wp;
    short windowPart;
    windowPart = FindWindow( *pt, &wp );
    QWidget *widget = QWidget::find( (WId)wp );
    int growWindowSize = 0;
    bool in_widget = FALSE;

    switch( windowPart ) {
    case inGoAway:
	if( widget ) {
	    widget->close();
	} else {
	    qWarning("Close for unknown widget");
	}
	break;
    case inDrag:
    {
	DragWindow( wp, *pt, 0 );

	int ox = widget->x(), oy = widget->y();
	QMacSavedPortInfo savedInfo;
	SetPortWindowPort( wp );
	Point p = { 0, 0 };
	LocalToGlobal(&p);
	widget->setCRect( QRect( p.h, p.v, widget->width(), widget->height() ) );
	QMoveEvent qme( QPoint( widget->x(), widget->y() ), QPoint( ox, oy) );
	QApplication::sendEvent( widget, &qme );
    }
    break;
    case inContent:
	in_widget = TRUE;
	break;
    case inGrow: 
	Rect limits;

	if( widget ) {
	    if ( widget->extra ) {
		SetRect( &limits, widget->extra->minw, widget->extra->minh,
			 widget->extra->maxw, widget->extra->maxh);
	    }
	}
	growWindowSize = GrowWindow( wp, *pt, &limits);
	if( growWindowSize) {
	    // nw/nh might not match the actual size if setSizeIncrement
	    // is used
	    int nw = LoWord( growWindowSize );
	    int nh = HiWord( growWindowSize );

	    if( nw < desktop()->width() && nw > 0 && nh < desktop()->height() && nh > 0 ) {
		if( widget ) 
		    widget->resize( nw, nh );
	    }
	}
	break;
    case inCollapseBox:
	if( TrackBox( wp, *pt, windowPart ) == true ) {
	    if(widget) 
		widget->showMinimized();
	}
	break;
    case inZoomIn:
	if( TrackBox( wp, *pt, windowPart ) == true ) {
	    if(widget) 
		widget->showNormal();
	}
	break;
    case inZoomOut:
	if( TrackBox( wp, *pt, windowPart ) == true ) {
	    if(widget) 
		widget->showMaximized();
	}
	break;
    }
    return in_widget;
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

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	Q_CHECK_PTR( qt_modal_stack );
    }
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	}
    }
    app_do_modal = qt_modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, EventRef event )
{
   if ( qApp->activePopupWidget() )
	return TRUE;
    // a bit of a hack: use WStyle_Tool as a general ignore-modality
    // flag, also for complex widgets with children.
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=QApplication::activeModalWidget();

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )			// don't block event
	return TRUE;

    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader ) )
	groupLeader = groupLeader->parentWidget();

    if ( groupLeader ) {
	// Does groupLeader have a child in qt_modal_stack?
	bool unrelated = TRUE;
	modal = qt_modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != groupLeader && !p->testWFlags( Qt::WGroupLeader) ) {
		p = p->parentWidget();
	    }
	    modal = qt_modal_stack->next();
	    if ( p == groupLeader ) unrelated = FALSE;
	}

	if ( unrelated )
	    return TRUE;		// don't block event
    }

    bool block_event  = FALSE;
    bool paint_event = FALSE;

    UInt32 ekind = GetEventKind(event), eclass=GetEventClass(event);
    switch(eclass) {
    case kEventClassMouse:
	block_event = ekind != kEventMouseMoved;
	break;
    case kEventClassKeyboard:
	block_event = TRUE;
	break;
    case kEventClassWindow:
	paint_event = ekind == kEventWindowUpdate;
	break;
    }

    if ( top->parentWidget() == 0 && (block_event || paint_event) )
	top->raise();

    return !block_event;
}


Q_PASCAL OSStatus 
QApplication::globalEventProcessor(EventHandlerCallRef ref, EventRef event, void *data)
{
    QApplication *app = (QApplication *)data;
    if ( app->macEventFilter( event ) )
	return 1;

    QWidget *widget = NULL;

    UInt32 ekind = GetEventKind(event);
    switch(GetEventClass(event))
    {
    case kEventClassMouse:
    {
	if( (ekind == kEventMouseDown && mouse_button_state ) ||
	    (ekind == kEventMouseUp && !mouse_button_state) )
	    return 0;

	QEvent::Type etype = QEvent::None;

	int button=QEvent::NoButton, state=0, wheel_delta=0;
	if(ekind == kEventMouseDown || ekind == kEventMouseUp) {
	    EventMouseButton mb;
	    GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, 
			      sizeof(mb), NULL, &mb);
	    if(mb == kEventMouseButtonPrimary)
		button = QMouseEvent::LeftButton;
	    else if(mb == kEventMouseButtonSecondary)
	        button = QMouseEvent::RightButton;
	    else 
		button = QMouseEvent::MidButton;
	}

	int keys;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, 
			  sizeof(keys), NULL, &keys);
	keys = get_modifiers(keys);
	
	Point where;
	GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, 
			  sizeof(where), NULL, &where);

	switch(ekind) {
	case kEventMouseDown:
	{

	    UInt32 count;
	    GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL, 
			      sizeof(count), NULL, &count);
	    if(count == 2) 
		etype = QEvent::MouseButtonDblClick;
	    else
		etype = QEvent::MouseButtonPress;
	    mouse_button_state = button;
	    break;
	}
	case kEventMouseUp:
	    etype = QEvent::MouseButtonRelease;
	    state = mouse_button_state;
	    mouse_button_state = 0;
	    break;
	case kEventMouseDragged:
	case kEventMouseMoved:
	    etype = QEvent::MouseMove;
	    state = mouse_button_state;
	    break;
	case kEventMouseWheelMoved:
	{
	    long int mdelt;
	    GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, NULL, 
			      sizeof(mdelt), NULL, &mdelt);
	    wheel_delta = mdelt * 100;
	    state = mouse_button_state;
	    break;
	}
	}

	//handle popup's first
	QWidget *popupwidget = NULL;
	if( app->inPopupMode() ) {
	    QMacSavedPortInfo savedInfo;

	    WindowPtr wp;
	    FindWindow(where,&wp);
	    if(wp) {
		QWidget *clt=QWidget::find((WId)wp);
		if(clt && clt->isPopup())
		    popupwidget = clt;
	    }
	    if(!popupwidget)
		popupwidget = activePopupWidget();
	    SetPortWindowPort((WindowPtr)popupwidget->handle());
	    Point gp = where;
	    GlobalToLocal( &gp ); //now map it to the window
	    popupwidget = recursive_match(popupwidget, gp.h, gp.v);

	    QPoint p( where.h, where.v );
	    QPoint plocal(popupwidget->mapFromGlobal( p ));
	    if(wheel_delta) {
		QWheelEvent qwe( plocal, p, wheel_delta, state | keys);
		QApplication::sendEvent( popupwidget, &qwe);
	    } else {
		QMouseEvent qme( etype, plocal, p, button | keys, state | keys );
		QApplication::sendEvent( popupwidget, &qme );
	    }
	}

	//figure out which widget to send it to
	if( ekind != kEventMouseDown && qt_button_down )
	    widget = qt_button_down;
	else if( mac_mouse_grabber )
	    widget = mac_mouse_grabber;
	else
	    widget = QApplication::widgetAt( where.h, where.v, true );

	switch(ekind) {
	case kEventMouseDragged:
	case kEventMouseMoved:
	{
	    //set the cursor up
	    Cursor *n = NULL;
	    if(!widget) //not over the app, don't set a cursor..
		;
	    else if(widget->extra && widget->extra->curs)
		n = (Cursor *)widget->extra->curs->handle();
	    else if(cursorStack)
		n = (Cursor *)app_cursor->handle();
	    if(!n)
		n = (Cursor *)arrowCursor.handle(); //I give up..
	    if(currentCursor != n)
		SetCursor(currentCursor = n);
	    if ( qt_mouseover != widget ) {
		qt_dispatchEnterLeave( widget, qt_mouseover );
		qt_mouseover = widget;
	    }
	    break;
	}
	case kEventMouseDown:
	    qt_button_down = widget;
	    break;     
	case kEventMouseUp:
	    qt_button_down = NULL;
	    break;     
	}

	//finally send the event to the widget if its not the popup
	if ( widget && widget != popupwidget ) {
	    if ( app_do_modal && !qt_try_modal(widget, event) )
		return 1;

	    if(ekind == kEventMouseDown) {
		QWidget* w = widget;
		while ( w->focusProxy() )
		    w = w->focusProxy();
		if ( w->focusPolicy() & QWidget::ClickFocus ) {
		    QFocusEvent::setReason( QFocusEvent::Mouse);
		    w->setFocus();
		    QFocusEvent::resetReason();
		}
		if(QWidget *tlw = widget->topLevelWidget()) {
		    tlw->raise();
		    if(tlw->isTopLevel() && !tlw->isPopup() && 
		       (tlw->isModal() || !tlw->isDialog())) {
			if(IsMenuBarVisible()) {
			    if(QObject *mb = tlw->child(0, "QMenuBar", FALSE)) {
				QMenuBar *bar = (QMenuBar *)mb;
				if(bar->isTopLevel() && bar->isVisible()) 
				    HideMenuBar();
			    }
			}
			app->setActiveWindow(tlw);
		    }
		}

		if(!app->do_mouse_down( &where )) {
		    qt_button_down = NULL;
		    mouse_button_state = 0;
		    return 0;
		}
	    }

#if 0
	    char *desc = NULL;
	    switch(ekind) {
	    case kEventMouseDown: desc = "MouseButtonPress"; break;
	    case kEventMouseUp: desc = "MouseButtonRelease"; break;
	    case kEventMouseDragged: 
	    case kEventMouseMoved: desc = "MouseMove"; break;
	    case kEventMouseWheelMoved: desc = "MouseWheelMove"; break;
	    }
	    qDebug("Would send (%s) event to %s %s (%d %d %d)", desc, 
		   widget->name(), widget->className(), button|keys, state|keys, 
		   wheel_delta);
#endif
	    QPoint p( where.h, where.v );
	    QPoint plocal(widget->mapFromGlobal( p ));
	    if(wheel_delta) {
		QWheelEvent qwe( plocal, p, wheel_delta, state | keys);
		QApplication::sendEvent( widget, &qwe);
	    } else {
		QMouseEvent qme( etype, plocal, p, button | keys, state | keys );
		QApplication::sendEvent( widget, &qme );
	    }
	}
	break;
    }
    case kEventClassKeyboard:
    {
	UInt32 modif;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modif), NULL, &modif);
	int modifiers = get_modifiers(modif);

	char chr;
	GetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(chr), NULL, &chr);
	int mychar=get_key(chr);
	QString mystr = QChar(mychar);

	QEvent::Type etype = (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress;

	if( mac_keyboard_grabber )
	    widget = mac_keyboard_grabber;
	else if(focus_widget)
	    widget = focus_widget;

	if(widget) {
	    if ( app_do_modal && !qt_try_modal(widget, event) )
		return 1;

	    bool isAccel = FALSE;
	    if(etype == QEvent::KeyPress && !mac_keyboard_grabber) {
		QKeyEvent aa(QEvent::AccelOverride, mychar, mychar, modifiers, mystr, ekind == kEventRawKeyRepeat,
			     mystr.length());
		aa.ignore();
		QApplication::sendEvent( widget, &aa );
		if ( !aa.isAccepted() ) {
		    QKeyEvent a(QEvent::Accel, mychar, mychar, modifiers, mystr, ekind == kEventRawKeyRepeat,
				mystr.length());
		    a.ignore();
		    QApplication::sendEvent( widget->topLevelWidget(), &a );
		    if ( a.isAccepted() ) 
			isAccel = TRUE;
		}
	    }
	    if(!isAccel) {
		QKeyEvent ke(etype,mychar, mychar, modifiers, mystr, ekind == kEventRawKeyRepeat, mystr.length());
		QApplication::sendEvent(widget,&ke);
	    }
	} 
	break;
    }
    case kEventClassWindow:
    {
	WindowRef wid;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, 
			  sizeof(WindowRef), NULL, &wid);
	widget = QWidget::find( (WId)wid );
	if(!widget) {
	    qWarning("Couldn't find EventClasWindow widget for %d", (int)wid);
        break;
    }
    
	if(ekind == kEventWindowUpdate) {
		int metricWidth = widget->metric (QPaintDeviceMetrics::PdmWidth );
		int metricHeight = widget->metric( QPaintDeviceMetrics::PdmHeight );
		widget->crect.setWidth( metricWidth - 1 );
		widget->crect.setHeight( metricHeight - 1 );
		
		QMacSavedPortInfo savedInfo;
		BeginUpdate((WindowPtr)widget->handle());
		widget->propagateUpdates();
		EndUpdate((WindowPtr)widget->handle());
	} else if(ekind == kEventWindowActivated) {
		if(widget) {
		    widget->raise();
		    QWidget *tlw = widget->topLevelWidget();
		    if(tlw->isTopLevel() && !tlw->isPopup() && 
		       (tlw->isModal() || !tlw->isDialog())) {
				if(IsMenuBarVisible()) {
			    	if(QObject *mb = tlw->child(0, "QMenuBar", FALSE)) {
						QMenuBar *bar = (QMenuBar *)mb;
						if(bar->isTopLevel() && bar->isVisible()) 
				   		 	HideMenuBar();	
			    	}
				}
				app->setActiveWindow(tlw);
			}
			if (widget->focusWidget())
				widget->focusWidget()->setFocus();
			else
				widget->setFocus();
		}
	} else if(ekind == kEventWindowDeactivated) {
		if(active_window && widget == active_window) 
		    app->setActiveWindow(NULL);
		while(app->inPopupMode())
		    app->activePopupWidget()->close();
	} else if(ekind == kEventWindowShown || ekind == kEventWindowHidden) {

	}
    break;
    case kEventClassApplication:
	if(ekind == kEventAppActivated)
	    app->clipboard()->loadScrap(FALSE);
	else if(ekind == kEventAppDeactivated) {
	    app->clipboard()->saveScrap();
	    app->setActiveWindow(NULL);
	}
	break;
    }
    }
    return noErr;
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

bool QApplication::macEventFilter( EventRef )
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
	Q_CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
    }
    popupWidgets->append( popup );		// add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    active_window = popup;
    QFocusEvent::setReason( QFocusEvent::Popup );
    if (active_window->focusWidget())
	active_window->focusWidget()->setFocus();
    else
	active_window->setFocus();
    QFocusEvent::resetReason();
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
	// restore the former active window immediately, although
	// we'll get a focusIn later
	active_window = (*activeBeforePopup);
	QFocusEvent::setReason( QFocusEvent::Popup );
	if ( active_window )
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	QFocusEvent::resetReason();
    } else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	active_window = popupWidgets->getLast();
	QFocusEvent::setReason( QFocusEvent::Popup );
	if (active_window->focusWidget())
	    active_window->focusWidget()->setFocus();
	else
	    active_window->setFocus();
	QFocusEvent::resetReason();
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

bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( !animate_ui )
	return FALSE;

    switch( effect ) {
    case UI_AnimateMenu:
	return animate_menu;
    case UI_FadeMenu:
	return fade_menu;
    case UI_AnimateCombo:
	return animate_combo;
    case UI_AnimateTooltip:
	return animate_tooltip;
    case UI_FadeTooltip:
	return fade_tooltip;
    default:
	return animate_ui;
    }
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

#include <qapplication_mac.moc>

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
