/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp_os2.cpp#18 $
**
** Implementation of OS/2 PM startup routines and event handling
**
** Created : 940707
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qevent.h"
#include "qview.h"
#include "qobjcoll.h"
#include "qwininfo.h"
#include <ctype.h>
#define	 INCL_DOSNMPIPES			// for debug output
#define	 INCL_PM
#include <os2.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qapp_os2.cpp#18 $");


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static char    *appName;			// application name
static HAB	appInst;			// handle to app instance
static HMQ	msgQueue;			// handle to message queue
static HWND	curWin = 0;			// current window
static QWidget *curWidget;
#if defined(USE_HEARTBEAT)
static int	heartBeat = 0;			// heatbeat timer
#endif
static int	pmNumButtons;			// number of mouse buttons
static bool	pmSwappedButtons;		// TRUE if swapped buttons

typedef void  (*VFPTR)();
static VFPTR   *cleanupRvec = 0;		// vector of cleanup routines
static uint	cleanupRcount = 0;

static void	debugHandler( const char * );	// default debug handler
static void	initDebug();
static void	cleanupDebug();

static void	initTimers();
static void	cleanupTimers();
static bool	activateTimer( uint );

static int translateKeyCode( int );

class QETWidget : public QWidget {		// event translator widget
public:
    bool translateMouseEvent( const QMSG & );
    bool translateKeyEvent( const QMSG & );
    bool translatePaintEvent( const QMSG & );
    bool translateConfigEvent( const QMSG & );
    bool translateCloseEvent( const QMSG & );
    void setWFlags( WFlags n )	 { setWFlags(n); }
    void clearWFlags( WFlags n ) { clearWFlags(n); }
};


/*****************************************************************************
  main() - initializes OS/2 PM and calls user's startup function qMain()
 *****************************************************************************/

int main( int argc, char **argv )
{
#if defined(TRACE_FS)
    startFSTrace();
#endif

  // Install default debug handler

    initDebug();

  // Set application name

    char *p = strrchr( argv[0], '\\' );
    appName = p ? p + 1 : argv[0];

  // Get PM parameters

    appInst = WinInitialize( 0 );

  // Create message queue

    msgQueue = WinCreateMsgQueue( appInst, 0 );

  // Misc. initialization

    pmNumButtons = WinQuerySysValue( HWND_DESKTOP, SV_CMOUSEBUTTONS );
    pmSwappedButtons = WinQuerySysValue( HWND_DESKTOP, SV_SWAPBUTTON );

    QWinInfo::initialize();
    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
#if defined(USE_HEARTBEAT)
    heartBeat = WinStartTimer( appInst, 0, 0, 100 );
#endif

  // Call user-provided main routine

    int returnCode = qMain( argc, argv );

  // Cleanup

    if ( cleanupRvec ) {
	while ( cleanupRcount )			// call cleanup routines
	    (*(cleanupRvec[--cleanupRcount]))();
	delete [] cleanupRvec;
    }
    if ( qApp )
	delete qApp;
    QApplication::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
    cleanupTimers();
#if defined(USE_HEARTBEAT)
    WinStopTimer( appInst, 0, heartBeat );
#endif

    WinDestroyMsgQueue( msgQueue );
    WinTerminate( appInst );

    cleanupDebug();

#if defined(TRACE_FS)
    stopFSTrace();
#endif
    return returnCode;
}


/*****************************************************************************
  The default debug handler in Qt writes output to a named pipe.
  Run the qdbgos2.exe program to see the debug output!
 *****************************************************************************/

static HFILE debugChannel;
static char debugChannelName[] = "\\PIPE\\QTDEBUGT";

static void initDebug()				// init debug system
{
    APIRET r;
    ULONG  action;
    int flag = FILE_OPEN;
    int mode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |
	       OPEN_FLAGS_WRITE_THROUGH;

    r = DosOpen( debugChannelName, &debugChannel, &action,
		 0, 0, flag, mode, 0 );
    if ( r != 0 )				// cannot open named pipe
	debugChannel = 0;
    else
	installDebugHandler( (void (*)(char*))debugHandler );
}

static void cleanupDebug()
{
    if ( debugChannel ) {
	debug( "QT-DEBUG-QUIT" );		// ask server to disconnect
	DosClose( debugChannel );
	debugChannel = 0;
    }
}

static void debugHandler( const char *str )	// print debug message
{
    if ( !debugChannel )
	return;
    APIRET r;
    ULONG bytes_written;
    QString buf = str;
    buf += '\n';
    DosWrite( debugChannel, (void *)buf.data(), buf.size(), &bytes_written );
}


void qAddCleanupRoutine( void (*p)() )		// add cleanup routine
{
    if ( !cleanupRvec )
	cleanupRvec = new VFPTR[16];		// plenty of room
    cleanupRvec[cleanupRcount++] = p;		// save pointer to routine
}


/*****************************************************************************
  Global functions that export important data
 *****************************************************************************/

char *qAppName()				// get application name
{
    return appName;
}

HAB qPMAppInst()				// get PM app handle
{
    return appInst;
}


/*****************************************************************************
  Safe configuration (move,resize,changeGeometry) mechanism to avoids
  recursion in processing messages.
 *****************************************************************************/

#include "qqueue.h"

struct QWinConfigRequest {
    WId	 id;					// widget to be configured
    int	 req;					// 0=move, 1=resize, 2=changeG
    int	 x, y, w, h;				// request parameters
};

Q_DECLARE(QQueueM,QWinConfigRequest);
static QQueue(QWinConfigRequest) *configRequests = 0;

void qWinRequestConfig( WId id, int req, int x, int y, int w, int h )
{
    if ( !configRequests )			// create queue
	configRequests = new QQueue(QWinConfigRequest);
    QWinConfigRequest *r = new QWinConfigRequest;
    r->id = id;					// create new request
    r->req = req;
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    configRequests->enqueue( r );		// store request in queue
}

static void qWinProcessConfigRequests()		// perform requests in queue
{
    if ( !configRequests )
	return;
    QWinConfigRequest *r;
    while ( 1 ) {
	if ( configRequests->isEmpty() )
	    break;
	r = configRequests->dequeue();
	QWidget *w = QWidget::find( r->id );
	if ( w ) {				// widget exists
	    if ( w->testWFlags( WWin_Config ) ) // biting our tail
		return;
	    if ( r->req == 0 )
		w->move( r->x, r->y );
	    else
	    if ( r->req == 1 )
		w->resize( r->w, r->h );
	    else
		w->changeGeometry( r->x, r->y, r->w, r->h );
	}
	delete r;
    }
    delete configRequests;
    configRequests = 0;
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec( QWidget *mainWidget )	// main event loop
{
    QMSG msg;
    main_widget = mainWidget;			// set main widget
    while ( WinGetMsg(appInst, &msg, 0, 0, 0) ) {
	if ( pmEventFilter( &msg ) )		// pass through event filter
	    continue;
#if defined(USE_HEARTBEAT)
	if ( msg.msg == WM_TIMER ) {		// timer message received
	    if ( msg.wParam != heartBeat )
		activateTimer( SHORT1FROMMP(msg.mp1) );
	    else
	    if ( curWin && qApp ) {		// process heartbeat
		POINTL p;
		WinQueryPointerPos( HWND_DESKTOP, &p );
NOTE!!! Check if WinWindowFromPoint is ok
		if ( WinWindowFromPoint( 0, p, FALSE ) != curWin ) {
		    QEvent leave( Event_Leave );
		    SEND_EVENT( curWidget, &leave );
		    curWin = 0;
		}
	    }
	    continue;
	}
#else
	if ( msg.msg == WM_TIMER ) {		// timer message received
	    if ( activateTimer( SHORT1FROMMP(msg.mp1) ) )
		continue;
	}
#endif
	WinDispatchMsg( appInst, &msg );	// send to WndProc
	if ( configRequests )			// any pending configs?
	    qWinProcessConfigRequests();
	if ( quit_now )				// request to quit application
	    break;
    }
    return quit_code;
}


/*****************************************************************************
  WndProc receives all messages from the main event loop
 *****************************************************************************/

extern "C" MRESULT EXPENTRY
WndProc( HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2 )
{
    if ( !qApp )				// unstable app state
	return WinDefWindowProc(hwnd,message,mp1,mp2);

    QETWidget *widget = (QETWidget*)QWidget::find( hwnd );
    if ( !widget )				// don't know this widget
	return WinDefWindowProc(hwnd,message,mp1,mp2);

    QMSG msg;
    msg.hwnd = hwnd;				// create QMSG structure
    msg.msg = message;				// time and pt fields ignored
    msg.mp1 = mp1;
    msg.mp2 = mp2;

    int evt_type = Event_None;
    bool result = TRUE;

    if ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST )
	widget->translateMouseEvent( msg );	// mouse event
    else
    switch ( message ) {

	case WM_CHAR:
	    result = widget->translateKeyEvent( msg );
	    break;

	case WM_PAINT:				// paint event
	    result = widget->translatePaintEvent( msg );
	    break;

	case WM_MOVE:				// move window
	case WM_SIZE:				// resize window
	    result = widget->translateConfigEvent( msg );
	    break;

	case WM_ACTIVATE:			// activate/deactivate
	    if ( SHORT1FROMMP(mp1) )
		widget->setF( WState_Active );
	    else
		widget->clearF( WState_Active );
	    result = TRUE;
	    break;

	case WM_CLOSE:				// close window
	    if ( widget->translateCloseEvent( msg ) )
		delete widget;
	    return 0;				// always handled

	case WM_DESTROY:			// destroy window
	    if ( hwnd == curWin ) {
		QEvent leave( Event_Leave );
		SEND_EVENT( widget, &leave );
		curWin = 0;
	    }
	    result = FALSE;
	    break;

	default:
//	    debug( "MSG #%x", message );
	    result = FALSE;			// event was not processed
	    break;

    }

    if ( evt_type != Event_None ) {		// simple event
	QEvent evt( evt_type );
	result = SEND_EVENT(widget, &evt);
    }
    return result ? 0 : WinDefWindowProc(hwnd,message,mp1,mp2);
}


bool QApplication::pmEventFilter( QMSG * )	// OS/2 PM event filter
{
    return FALSE;
}


/*****************************************************************************
  Timer handling; Our routines depend on OS/2 PM timer functions, but we
  need some extra handling to activate objects at timeout.
  We also keep an internal countdown variable to have longer timeouts.
  Max timeout is around 25 days.  PM is limited to max 65 seconds.

  Implementation note: There are two types of timer identifiers.  PM
  timer ids (internal use) are stored in TimerInfo.  Qt timer ids are
  indexes (+1) into the timerVec vector.

  NOTE: These functions are for internal use. QObject::startTimer() and
	 QObject::killTimer() are for public use.
	 The QTimer class provides a high-level interface which translates
	 timer events into signals.

  qStartTimer( interval, obj )
	Starts a timer which will run until it is killed with qKillTimer()
	Arguments:
	    long interval	timer interval in milliseconds
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

/*****************************************************************************
  Internal data structure for timers
 *****************************************************************************/

#include "qvector.h"
#include "qintdict.h"

struct TimerInfo {				// internal timer info
    uint     ind;				// - Qt timer identifier - 1
    uint     id;				// - PM timer identifier
    uint     maxcount;				// - max count for countdown
    uint     countdown;				// - countdown variable
    QObject *obj;				// - object to receive events
};
typedef Q_DECLARE(QVectorM,TimerInfo) TimerVec;	// vector of TimerInfo structs
typedef Q_DECLARE(QIntDictM,TimerInfo) TimerDict; // fast dict of timers

static TimerVec	 *timerVec  = 0;		// timer vector
static TimerDict *timerDict = 0;		// timer dict


/*****************************************************************************
  Timer activation (called from the event loop)
 *****************************************************************************/

static bool activateTimer( uint id )		// activate timer
{
    if ( !timerVec )				// should never happen
	return FALSE;
    register TimerInfo *t = timerDict->find( id );
    if ( !t )					// no such timer id
	return FALSE;
    if ( t->countdown )				// we're not ready yet
	t->countdown--;
    else {
	t->countdown = t->maxcount;		// start counting again
	QTimerEvent evt( t->ind + 1 );
	SEND_EVENT( t->obj, &evt );		// send event
    }
    return TRUE;				// timer event was processed
}


/*****************************************************************************
  Timer initialization and cleanup routines
 *****************************************************************************/

static void initTimers()			// initialize timers
{
    timerVec = new TimerVec( MaxTimers );
    CHECK_PTR( timerVec );
    timerVec->setAutoDelete( TRUE );
    timerDict = new TimerDict( 29 );
    CHECK_PTR( timerDict );
}

static void cleanupTimers()			// remove pending timers
{
    register TimerInfo *t;
    if ( !timerVec )				// no timers were used
	return;
    for ( int i=0; i<MaxTimers; i++ ) {		// kill all pending timers
	t = timerVec->at( i );
	if ( t )
	    WinStopTimer( appInst, 0, t->id );
    }
    delete timerDict;
    delete timerVec;
}


/*****************************************************************************
  Main timer functions for starting and killing timers
 *****************************************************************************/

int qStartTimer( long interval, QObject *obj )	// start timer
{
    register TimerInfo *t;
    if ( !timerVec )				// initialize timer data
	initTimers();
    int ind = timerVec->findRef( 0 ) + 1;	// get free timer
    if ( ind == 0 || !obj )			// cannot create timer
	return 0;
    t = new TimerInfo;				// create timer entry
    CHECK_PTR( t );
    t->ind = ind;
    if ( interval > 65535 ) {			// use long intervals
	t->maxcount = (uint)(interval >> 16);
	interval /= t->maxcount + 1;
    }
    else
	t->maxcount = 0;
    t->countdown = t->maxcount;
    t->obj = obj;
    t->id = WinStartTimer( appInst, 0, 0, (uint)interval );
    if ( t->id == 0 ) {
	delete t;				// could not set timer
	return 0;
    }
    timerVec->insert( ind-1, t );		// store in timer vector
    timerDict->insert( t->id, t );		// store in dict
    return ind;					// return index in vector
}

bool qKillTimer( int ind )			// kill timer with id
{
    if ( !timerVec || ind <= 0 || ind > MaxTimers || timerVec->at(ind-1) == 0 )
	return FALSE;
    WinStopTimer( appInst, 0, timerVec->at(ind-1)->id );
    timerDict->remove( timerVec->at(ind-1)->id );
    timerVec->remove( ind-1 );
    return TRUE;
}

bool qKillTimer( QObject *obj )			// kill timer for obj
{
    if ( !timerVec )
	return FALSE;
    register TimerInfo *t;
    for ( int i=0; i<MaxTimers; i++ ) {
	t = timerVec->at( i );
	if ( t && t->obj == obj ) {		// object found
	    WinStopTimer( appInst, 0, t->id );
	    timerDict->remove( t->id );
	    timerVec->remove( i );
	}
    }
    return TRUE;
}


/*****************************************************************************
  Mouse event translation
 *****************************************************************************/

static ushort mouseTbl[] = {
    WM_MOUSEMOVE,	Event_MouseMove,		0,
    WM_BUTTON1DOWN,	Event_MouseButtonPress,		LeftButton,
    WM_BUTTON1UP,	Event_MouseButtonRelease,	LeftButton,
    WM_BUTTON1DBLCLK,	Event_MouseButtonDblClick,	LeftButton,
    WM_BUTTON3DOWN,	Event_MouseButtonPress,		RightButton,
    WM_BUTTON3UP,	Event_MouseButtonRelease,	RightButton,
    WM_BUTTON3DBLCLK,	Event_MouseButtonDblClick,	RightButton,
    WM_BUTTON2DOWN,	Event_MouseButtonPress,		MidButton,
    WM_BUTTON2UP,	Event_MouseButtonRelease,	MidButton,
    WM_BUTTON2DBLCLK,	Event_MouseButtonDblClick,	MidButton,
    0,			0,				0
};

static int translateButtonState( int s )
{
    int bst = 0;
    if ( WinGetKeyState(HWND_DESKTOP, VK_BUTTON1) & 0x8000 )
	bst |= LeftButton;
    if ( WinGetKeyState(HWND_DESKTOP, VK_BUTTON2) & 0x8000 ) {
	if ( pmNumButtons == 2 ) {
	    bst |= pmSwappedButtons ?		// two-button mice
		   LeftButton :
		   RightButton;
	}
	else
	    bst |= MidButton;
    }
    if ( WinGetKeyState(HWND_DESKTOP, VK_BUTTON3) & 0x8000 )
	bst |= RightButton;
    if ( s & KC_SHIFT )
	bst |= ShiftButton;
    if ( s & KC_CTRL )
	bst |= ControlButton;
    if ( s & KC_ALT )
	bst |= AltButton;
    return bst;
}

bool QETWidget::translateMouseEvent( const QMSG &msg )
{
    static bool capture = FALSE;
    int	   type;				// event parameters
    QPoint pos;
    int	   button;
    int	   state;

    pos.rx() = SHORT1FROMMP(msg.mp1);		// get position
    pos.ry() = clientSize().height() - SHORT2FROMMP(msg.mp1) - 1;
    for ( int i=0; mouseTbl[i] != msg.msg || !mouseTbl[i]; i += 3 ) ;
    if ( !mouseTbl[i] )
	return FALSE;
    type = mouseTbl[++i];			// event type
    button = mouseTbl[++i];			// which button
    if ( button == MidButton && pmNumButtons == 2 )
	button = pmSwappedButtons ?		// two-button mice
		 LeftButton :
		 RightButton;
    state = translateButtonState( SHORT2FROMMP(msg.mp2) );
    if ( type == Event_MouseMove ) {
	WinSetPointer( HWND_DESKTOP, cursor().handle() );
	if ( curWin != id() ) {			// new current window
	    if ( curWin ) {			// send leave event
		QEvent leave( Event_Leave );
		SEND_EVENT( curWidget, &leave );
	    }
	    curWin = id();
	    curWidget = this;
	    QEvent enter( Event_Enter );	// send enter event
	    SEND_EVENT( this, &enter );
	}
	if ( (state == 0 || !capture) && !testWFlags(WEtc_MouMove) )
	    return TRUE;			// no button
    }
    else {					// mouse button click
	if ( !testWFlags(WState_Active) && testWFlags(WType_Overlap) ) {
	    WinSetActiveWindow( HWND_DESKTOP, msg.hwnd );
	    return FALSE;
	}
    }
    int bs = state & (LeftButton | RightButton | MidButton);
    if ( (type == Event_MouseButtonPress ||
	  type == Event_MouseButtonDblClick) && bs == button ) {
	WinSetCapture( HWND_DESKTOP, id() );
	capture = TRUE;
    }
    else
    if ( type == Event_MouseButtonRelease && bs == 0 ) {
	WinSetCapture( HWND_DESKTOP, 0 );
	capture = FALSE;
    }
    QMouseEvent evt( type, pos, button, state );
    return SEND_EVENT( this, &evt );		// send event
}


/*****************************************************************************
  Keyboard event translation
 *****************************************************************************/

#include "qkeycode.h"

static ushort KeyTbl[] = {			// keyboard mapping table
    VK_ESC,		Key_Escape,		// misc keys
    VK_TAB,		Key_Tab,
    VK_BACKSPACE,	Key_Backspace,
    VK_ENTER,		Key_Return,
    VK_INSERT,		Key_Insert,
    VK_DELETE,		Key_Delete,
    VK_PAUSE,		Key_Pause,
    VK_PRINTSCRN,	Key_Print,
    VK_HOME,		Key_Home,		// cursor movement
    VK_END,		Key_End,
    VK_LEFT,		Key_Left,
    VK_UP,		Key_Up,
    VK_RIGHT,		Key_Right,
    VK_DOWN,		Key_Down,
    VK_PAGEUP,		Key_Prior,
    VK_PAGEDOWN,	Key_Next,
    VK_SHIFT,		Key_Shift,		// modifiers
    VK_CTRL,		Key_Control,
    VK_ALT,		Key_Alt,
    VK_ALTGRAF,		Key_Alt,
    VK_MENU,		Key_Alt,
    VK_CAPSLOCK,	Key_CapsLock,
    VK_NUMLOCK,		Key_NumLock,
    VK_SCRLLOCK,	Key_ScrollLock,
    0,			0
};

static int translateKeyCode( int key )		// get Key_... code
{
    int code = 0;
    if ( key >= VK_F1 && key <= VK_F24 )	// function keys
	code = Key_F1 + (key - VK_F1);		// assumes contiguous codes!
    else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] && code == 0 ) {
	    if ( key == (int)KeyTbl[i] )
		code = KeyTbl[i+1];
	    else
		i += 2;
	}
    }
    return code;
}

bool QETWidget::translateKeyEvent( const QMSG &msg )
{
    int type;
    int code = 0;
    int ascii = 0;
    int state = 0;
    int fs = SHORT1FROMMP( msg.mp1 );
    int chr = SHORT1FROMMP( msg.mp2 );
    int vkey = SHORT2FROMMP( msg.mp2 );
    if ( fs & KC_VIRTUALKEY ) {
	if (vkey == VK_SPACE )
	    code = ascii = Key_Space;
	else {
	    code = translateKeyCode( vkey );
	    if ( code == 0 )
		return FALSE;
	}
    }
    else {
	ascii = chr;
	int a = toupper(ascii);
	if ( a >= Key_Space && a <= Key_AsciiTilde )
	    code = a;
    }
    state = translateButtonState( fs );
    type = fs & KC_KEYUP ? Event_KeyRelease : Event_KeyPress;
    QKeyEvent evt( type, code, ascii, state );
    return SEND_EVENT( this, &evt );		// send event
}


/*****************************************************************************
  Paint event translation
 *****************************************************************************/

bool QETWidget::translatePaintEvent( const QMSG & )
{
    QPaintEvent evt( clientRect() );
    setWFlags( WState_Paint );
    RECTL r;
    WinQueryWindowRect( id(), &r );
    hps = WinBeginPaint( id(), 0, 0 );
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, 0 );
    WinFillRect( hps, &r, backgroundColor().pixel() );
    bool res = SEND_EVENT( this, &evt );
    WinEndPaint( hps );
    hps = 0;
    clearWFlags( WState_Paint );
    return res;
}


/*****************************************************************************
  Window move and resize (configure) events
 *****************************************************************************/

bool QETWidget::translateConfigEvent( const QMSG &msg )
{
    setWFlags( WWin_Config );			// set config flag
    QRect r = clientGeometry();			// get widget geometry
    if ( msg.msg == WM_SIZE ) {			// resize event
	int w = SHORT1FROMMP(msg.mp2);		// new window width
	int h = SHORT2FROMMP(msg.mp2);		// new window height
	QSize newSize( w, h );
	r.setSize( newSize );
	setRect( r );
	QResizeEvent evt( geometry().size() );
	SEND_EVENT( this, &evt );
	if ( isParentType() ) {			// parent type, i.e. QView
	    debug( "starting to reposition" );
	    QView *v = (QView *)this;
	    if ( WinIsWindowVisible(v->id()) && v->iconText() )
		WinSetWindowText( v->id(), v->caption() );
	    else
		WinSetWindowText( v->id(), v->iconText() );
	}
	else
	if ( !testWFlags(WType_Overlap) )	// manual redraw
	    update();
    }
    else if ( msg.msg == WM_MOVE ) {		// move event
	SWP p;
	WinQueryWindowPos( id(), &p );
	QPoint newPos( p.cx, p.cy );
	r.moveTopLeft( newPos );
//	setRect( r );
	QMoveEvent evt( geometry().topLeft() );
	SEND_EVENT( this, &evt );
    }
    clearWFlags( WWin_Config );			// clear config flag
    return TRUE;
}


/*****************************************************************************
  Close window event translation
 *****************************************************************************/

bool QETWidget::translateCloseEvent( const QMSG & )
{
    QEvent evt( Event_Close );
    if ( SEND_EVENT( this, &evt ) ) {		// close widget
	hide();
	if ( qApp->mainWidget() == this || nWidgets() == 1 )
	    qApp->quit();
	else
	    return TRUE;			// delete widget
    }
    return FALSE;
}


/*****************************************************************************
  QWinInfo::initialize() - Gets window system specific attributes
 *****************************************************************************/

bool QWinInfo::initialize()
{
    HDC gdc;					// global DC
    long val;

    gdc = WinOpenWindowDC( HWND_DESKTOP );
    DevQueryCaps( gdc, CAPS_COLORS, 1, &val );
    nColors = val;

    if ( nColors < 3 )
	dispType = MonochromeDisplay;
    else
	dispType = ColorDisplay;		// well, could be LCD...

    int width  = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    int height = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
    dispSize = QSize( width, height );

    DevQueryCaps( gdc, CAPS_HORIZONTAL_RESOLUTION, 1, &val );
    width = val/dispSize.width();
    DevQueryCaps( gdc, CAPS_VERTICAL_RESOLUTION, 1, &val );
    height = val/dispSize.height();
    dispSizeMM = QSize( width, height );

    return TRUE;
}
