/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_win.cpp#247 $
**
** Implementation of Win32 startup routines and event handling
**
** Created : 931203
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qapplication.h"
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include <ctype.h>
#include "qt_windows.h"

#if defined(__CYGWIN32__)
#define __INSIDE_CYGWIN32__
#include <mywinsock.h>
#endif

#if !defined(QT_MAKEDLL)
#define QAPPLICATION_WIN_CPP
#include "qtinit_win.cpp"
#include "qtmain_win.cpp"
#endif


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x020A
#endif


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static char	 appName[120];			// application name
static HINSTANCE appInst	= 0;		// handle to app instance
static HINSTANCE appPrevInst	= 0;		// handle to prev app instance
static int	 appCmdShow	= 0;		// main window show command
static int	 numZeroTimers	= 0;		// number of full-speed timers
static HWND	 curWin		= 0;		// current window
static HDC	 displayDC	= 0;		// display device context
static QWidget	*desktopWidget	= 0;		// desktop window widget
#define USE_HEARTBEAT
#if defined(USE_HEARTBEAT)
static int	 heartBeat	= 0;		// heatbeat timer
#endif

#if defined(DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif

static bool	app_do_modal	= FALSE;	// modal mode
static bool	app_exit_loop	= FALSE;	// flag to exit local loop

static QWidgetList *modal_stack = 0;		// stack of modal widgets
static QWidget	   *popupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;

static HWND	autoCaptureWnd = 0;
static void	setAutoCapture( HWND );		// automatic capture
static void	releaseAutoCapture();

typedef void  (*VFPTR)();
typedef QList<void> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

static void	msgHandler( QtMsgType, const char* );

static void     unregWinClasses();

// Simpler timers are needed when Qt does not have the
// event loop (such as for plugins).
Q_EXPORT bool	qt_win_use_simple_timers = TRUE; //FALSE;
void CALLBACK   qt_simple_timer_func( HWND, UINT, UINT, DWORD );

static void	initTimers();
static void	cleanupTimers();
static void	dispatchTimer( uint, MSG * );
static bool	activateTimer( uint );
static void	activateZeroTimers();

Q_EXPORT Qt::WindowsVersion qt_winver = Qt::WV_NT;

QObject	       *qt_clipboard = 0;

static bool	qt_try_modal( QWidget *, MSG * );

static int	translateKeyCode( int );

void		qt_init_windows_mime();		// qdnd_win.cpp

QWidget*	qt_button_down	     = 0;	// the widget getting last button-down

#if defined(_WS_WIN32_)
#define __export
#endif

extern "C" LRESULT CALLBACK QtWndProc( HWND, UINT, WPARAM, LPARAM );

class QETWidget : public QWidget		// event translator widget
{
public:
    void	setWFlags( WFlags f )	{ QWidget::setWFlags(f); }
    void	clearWFlags( WFlags f ) { QWidget::clearWFlags(f); }
    QWExtra    *xtra()			{ return QWidget::extraData(); }
    bool	winEvent( MSG *m )	{ return QWidget::winEvent(m); }
    bool	translateMouseEvent( const MSG &msg );
    bool	translateKeyEvent( const MSG &msg, bool grab );
    bool	translateWheelEvent( const MSG &msg );
    bool	sendKeyEvent( QEvent::Type type, int code, int ascii,
			      int state, bool grab, const QString& text,
			      bool autor=FALSE );
    bool	translatePaintEvent( const MSG &msg );
    bool	translateConfigEvent( const MSG &msg );
    bool	translateCloseEvent( const MSG &msg );
};


static void set_winapp_name()
{
    static bool already_set = FALSE;
    if ( !already_set ) {
	already_set = TRUE;
	GetModuleFileNameA( 0, appName, sizeof(appName) );
	char *p = strrchr( appName, '\\' );	// skip path
	if ( p )
	    memmove( appName, p+1, strlen(p) );
    }
}


/*****************************************************************************
  qWinMain() - Initializes Windows. Called from WinMain() in qtmain_win.cpp
 *****************************************************************************/

Q_EXPORT
void qWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
	       int cmdShow, int &argc, QArray<pchar> &argv )
{
    static bool already_called = FALSE;

    if ( already_called ) {
#if defined(CHECK_STATE)
	warning( "Qt internal error: qWinMain should be called only once" );
#endif
	return;
    }
    already_called = TRUE;

  // Install default debug handler

    qInstallMsgHandler( msgHandler );

  // Create command line

    set_winapp_name();

    char *p = cmdParam;
    argc = 1;
    argv[0] = appName;

    while ( *p ) {				// parse cmd line arguments

	while ( isspace(*p) )			// skip white space
	    p++;

	if ( *p ) {				// arg starts
	    int quote;
	    char *start, *r;
	    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
		quote = *p;
		start = ++p;
	    } else {
		quote = 0;
		start = p;
	    }
	    r = start;
	    while ( *p ) {
		if ( *p == '\\' ) {		// escape char?
		    p++;
		    if ( *p == '\"' || *p == '\'' )
			;			// yes
		    else
			p--;			// treat \ literally
		} else if ( quote ) {
		    if ( *p == quote ) {
			p++;
			if ( isspace(*p) )
			    break;
			quote = 0;
		    }
		} else {
		    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
			quote = *p++;
			continue;
		    } else if ( isspace(*p) )
			break;
		}
		*r++ = *p++;
	    }
	    if ( *p )
		p++;
	    *r = '\0';

	    if ( argc >= (int)argv.size() )	// expand array
		argv.resize( argv.size()*2 );
	    argv[argc++] = start;
	}
    }

  // Get Windows parameters

    appInst = instance;
    appPrevInst = prevInstance;
    appCmdShow = cmdShow;
}

static void outColor(const char* s, const QColor& col) {
    debug("%s is %d %d %d", s, col.red(), col.green(), col.blue());
}
static void qt_set_windows_resources()
{
    // windows supports special fonts for the menus
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof( NONCLIENTMETRICS );
    SystemParametersInfo( SPI_GETNONCLIENTMETRICS,
			  sizeof( NONCLIENTMETRICS),
			  &ncm,
			  NULL);

    QString menuFontName = qt_winQString(ncm.lfMenuFont.lfFaceName);
    QFont menuFont(menuFontName);
    if (ncm.lfMenuFont.lfItalic)
	menuFont.setItalic( TRUE );
    if (ncm.lfMenuFont.lfWeight != FW_DONTCARE) {
	menuFont.setWeight(ncm.lfMenuFont.lfWeight*99/900);
    }
    double mps = ((double) -ncm.lfMenuFont.lfHeight*72)/
		    ((double) GetDeviceCaps(qt_display_dc(), LOGPIXELSY));
    menuFont.setPointSize(int(mps+0.5));

    if (menuFont != QFont::defaultFont()) {
	QApplication::setFont( menuFont, FALSE, "QPopupMenu");
 	QApplication::setFont( menuFont, TRUE, "QMenuBar");
    }

    QString messageFontName = qt_winQString(ncm.lfMessageFont.lfFaceName);
    QFont messageFont(messageFontName);
    if (ncm.lfMessageFont.lfItalic)
	messageFont.setItalic( TRUE );
    if (ncm.lfMessageFont.lfWeight != FW_DONTCARE) {
	messageFont.setWeight(ncm.lfMessageFont.lfWeight*99/900);
    }
    mps = ((double) -ncm.lfMessageFont.lfHeight*72)/
	  ((double) GetDeviceCaps(qt_display_dc(), LOGPIXELSY));
    messageFont.setPointSize(int(mps+0.5));

    if (messageFont != QFont::defaultFont()) {
 	QApplication::setFont( messageFont, TRUE, "QMessageBoxLabel");
    }

    // Same technique could apply to set the statusbar or tooltip
    // font, but since windows does not allow to change them, we do
    // not care for now.

    // Do the color settings

    QColorGroup cg;
    cg.setColor( QColorGroup::Foreground,
		 QColor(GetSysColor(COLOR_WINDOWTEXT)) );
    cg.setColor( QColorGroup::Button,
		 QColor(GetSysColor(COLOR_BTNFACE)) );
    cg.setColor( QColorGroup::Light,
		 QColor(GetSysColor(COLOR_BTNHIGHLIGHT)) );
    cg.setColor( QColorGroup::Midlight,
		 QColor(GetSysColor(COLOR_3DLIGHT)) );
    cg.setColor( QColorGroup::Dark,
		 QColor(GetSysColor(COLOR_BTNSHADOW)) );
    cg.setColor( QColorGroup::Mid,
		 QColor(GetSysColor(COLOR_3DDKSHADOW)) );
    cg.setColor( QColorGroup::Text,
		 QColor(GetSysColor(COLOR_WINDOWTEXT)) );
    cg.setColor( QColorGroup::BrightText,
		 QColor(GetSysColor(COLOR_BTNHIGHLIGHT)) );
    cg.setColor( QColorGroup::ButtonText,
		 QColor(GetSysColor(COLOR_WINDOWTEXT)) );
    cg.setColor( QColorGroup::Base,
		 QColor(GetSysColor(COLOR_WINDOW)) );
    cg.setColor( QColorGroup::Background,
		 QColor(GetSysColor(COLOR_BTNFACE)) );
    cg.setColor( QColorGroup::Shadow,
		 QColor(GetSysColor(COLOR_3DDKSHADOW)) );
    cg.setColor( QColorGroup::Highlight,
		 QColor(GetSysColor(COLOR_HIGHLIGHT)) );
    cg.setColor( QColorGroup::HighlightedText,
		 QColor(GetSysColor(COLOR_HIGHLIGHTTEXT)) );
	
    QColor disabled( (cg.foreground().red()+cg.button().red())/2,
		     (cg.foreground().green()+cg.button().green())/2,
		     (cg.foreground().blue()+cg.button().blue())/2);
    QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
		     disabled, Qt::white, Qt::white, cg.background() );
	
    QPalette pal( cg, dcg, cg );
    QApplication::setPalette( pal, TRUE );

    QColor menu(GetSysColor(COLOR_MENU));
    QColor menuText(GetSysColor(COLOR_MENUTEXT));
    if (menu != cg.button() || menuText != cg.text()) {
	// we need a special color group for the menu.
	cg.setColor( QColorGroup::Button, menu );
	cg.setColor( QColorGroup::Text, menuText );
	cg.setColor( QColorGroup::Foreground, menuText );
	cg.setColor( QColorGroup::ButtonText, menuText );
	QColor disabled( (cg.foreground().red()+cg.button().red())/2,
			 (cg.foreground().green()+cg.button().green())/2,
			 (cg.foreground().blue()+cg.button().blue())/2);
	QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
			 disabled, Qt::white, Qt::white, cg.background() );
	
	QPalette pal(cg, dcg, cg);
 	QApplication::setPalette( pal, FALSE, "QPopupMenu");
 	QApplication::setPalette( pal, TRUE, "QMenuBar");
    }

}

/*****************************************************************************
  qt_init() - initializes Qt for Windows
 *****************************************************************************/

void qt_init( int *argcptr, char **argv )
{
#if defined(DEBUG)
    int argc = *argcptr;
    int i, j;

  // Get command line params

    j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QString arg = argv[i];
	if ( arg == "-nograb" )
	    appNoGrab = !appNoGrab;
	else
	    argv[j++] = argv[i];
    }
    *argcptr = j;
#endif // DEBUG

  // Get the application name/instance if qWinMain() was not invoked

    set_winapp_name();
    if ( appInst == 0 )
	appInst = GetModuleHandle( 0 );

  // Detect the Windows version

#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s	    0
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS  1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT	    2
#endif

    OSVERSIONINFO osver;
    osver.dwOSVersionInfoSize = sizeof(osver);
    GetVersionEx( &osver );
    switch ( osver.dwPlatformId ) {
	case VER_PLATFORM_WIN32s:
	    qt_winver = Qt::WV_32s;
	    break;
	case VER_PLATFORM_WIN32_WINDOWS:
	    if ( osver.dwMinorVersion == 10 )
		qt_winver = Qt::WV_98;
	    else
		qt_winver = Qt::WV_95;
	    break;
	default:
	    qt_winver = Qt::WV_NT;
    }

  // Initialize OLE/COM
  //   S_OK means success and S_FALSE means that it has already
  //   been initialized
    HRESULT r;
    r = OleInitialize(0);
    if ( r != S_OK && r != S_FALSE ) {
#if defined(CHECK_STATE)
	warning( "Qt: Could not initialize OLE (error %x)", r );
#endif
    }

  // Misc. initialization

    qt_init_windows_mime();

    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
    qApp->setName( appName );

    qt_set_windows_resources();

#if defined(USE_HEARTBEAT)
    /*
      ########
      The heartbeat timer should be slowed down if we get no user input
      for some time and we should wake it up immediately when we get the
      first user event (any mouse or keyboard event).
      ########
    */
    heartBeat = SetTimer( 0, 0, 200,
	qt_win_use_simple_timers ?
	(TIMERPROC)qt_simple_timer_func : 0 );
#endif
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( postRList ) {
	VFPTR f = (VFPTR)postRList->first();
	while ( f ) {				// call post routines
	    (*f)();
	    postRList->remove();
	    f = (VFPTR)postRList->first();
	}
	delete postRList;
    }
#if defined(USE_HEARTBEAT)
    KillTimer( 0, heartBeat );
#endif
    cleanupTimers();
    unregWinClasses();
    QPixmapCache::clear();
    QPainter::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
    if ( displayDC )
	ReleaseDC( 0, displayDC );

  // Deinitialize OLE/COM
    OleUninitialize();
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

static void msgHandler( QtMsgType t, const char* str )
{
    if ( !str )
	str = "(null)";
    QCString s = str;
    s += "\n";
    OutputDebugStringA( s.data() );
    if ( t == QtFatalMsg )
        ExitProcess( 1 );
}


Q_EXPORT void qAddPostRoutine( Q_CleanUpFunction p )
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	CHECK_PTR( postRList );
    }
    postRList->insert( 0, (void *)p );		// store at list head
}


Q_EXPORT char *qAppName()			// get application name
{
    return appName;
}

Q_EXPORT HINSTANCE qWinAppInst()			// get Windows app handle
{
    return appInst;
}

Q_EXPORT HINSTANCE qWinAppPrevInst()		// get Windows prev app handle
{
    return appPrevInst;
}

Q_EXPORT int qWinAppCmdShow()			// get main window show command
{
    return appCmdShow;
}


Q_EXPORT HDC qt_display_dc()			// get display DC
{
    if ( !displayDC )
	displayDC = GetDC( 0 );
    return displayDC;
}

bool qt_nograb()				// application no-grab option
{
    return appNoGrab;
}


static QDict<int> *winclassNames = 0;

const char* qt_reg_winclass( int flags )	// register window class
{
    if ( !winclassNames ) {
	winclassNames = new QDict<int>;
	CHECK_PTR( winclassNames );
    }
    uint style;
    bool icon;
    const char *cname;
    if ( (flags & (Qt::WType_Popup|Qt::WStyle_Tool)) == 0 ) {
	cname = "QWidget";
	style = CS_DBLCLKS;
	icon  = TRUE;
    } else {
	cname = "QPopup";
	style = CS_DBLCLKS | CS_SAVEBITS;
	icon  = FALSE;
    }

    if ( winclassNames->find(cname) )		// already registered
	return cname;

    WNDCLASS wc;
    wc.style		= style;
    wc.lpfnWndProc	= (WNDPROC)QtWndProc;
    wc.cbClsExtra	= 0;
    wc.cbWndExtra	= 0;
    wc.hInstance	= (HINSTANCE)qWinAppInst();
    wc.hIcon		= icon ? LoadIcon(0,IDI_APPLICATION) : 0;
    wc.hCursor		= 0;
    wc.hbrBackground	= 0;
    wc.lpszMenuName	= 0;
    wc.lpszClassName	= (TCHAR*)qt_winTchar(cname,TRUE);
    RegisterClass( &wc );

    winclassNames->insert( cname, (int*)1 );
    return cname;
}

static void unregWinClasses()
{
    if ( !winclassNames )
	return;
    QDictIterator<int> it(*winclassNames);
    const char* k;
    while ( (k = (const char*)(void*)it.currentKeyLong()) ) {
	UnregisterClass( (TCHAR*)qt_winTchar(k,TRUE),
			 (HINSTANCE)qWinAppInst() );
	++it;
    }
    delete winclassNames;
    winclassNames = 0;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;			// set main widget
}


QWidget *QApplication::desktop()
{
    if ( !desktopWidget ) {			// not created yet
	desktopWidget = new QWidget( 0, "desktop", WType_Desktop );
	CHECK_PTR( desktopWidget );
    }
    return desktopWidget;
}

Qt::WindowsVersion QApplication::winVersion()
{
    return qt_winver;
}


/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
    SetCursor( app_cursor->handle() );
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    if ( app_cursor ) {
	SetCursor( app_cursor->handle() );
    } else {
	delete cursorStack;
	cursorStack = 0;
	QWidget *w = QWidget::find( curWin );
	if ( w )
	    SetCursor( w->cursor().handle() );
	else
	    SetCursor( arrowCursor.handle() );
    }
}

/*
  Internal function called from QWidget::setCursor()
*/

void qt_set_cursor( QWidget *w, const QCursor& c )
{
    if ( w->winId() == curWin )			// set immediately
	SetCursor( c.handle() );
}


void QApplication::setGlobalMouseTracking( bool enable )
{
    if ( enable ) {
	++app_tracking;
    } else {
	--app_tracking;
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	QWidget *w;
	QObjectListIt it( *p->children() );
	it.toLast();
	while ( it.current() ) {
	    if ( it.current()->isWidgetType() ) {
		w = (QWidget*)it.current();
		if ( w->isVisible() && w->geometry().contains(pos) ) {
		    QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		    return c ? c : w;
		}
	    }
	    --it;
	}
    }
    return 0;
}

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    POINT p;
    HWND  win;
    QWidget *w;
    p.x = x;
    p.y = y;
    win = WindowFromPoint( p );
    if ( !win )
	return 0;
    w = QWidget::find( win );
    if ( !w )
	return 0;
    if ( child ) {
	HWND cwin = ChildWindowFromPoint( win, p );
	if ( cwin && cwin != win )
	    return QWidget::find( cwin );
    }
    return w;
}


void QApplication::beep()
{
    MessageBeep( MB_OK );
}


/*****************************************************************************
  Safe configuration (move,resize,setGeometry) mechanism to avoid
  recursion when processing messages.
 *****************************************************************************/

#include "qqueue.h"

struct QWinConfigRequest {
    WId	 id;					// widget to be configured
    int	 req;					// 0=move, 1=resize, 2=setGeo
    int	 x, y, w, h;				// request parameters
};

static QQueue<QWinConfigRequest> *configRequests = 0;

void qWinRequestConfig( WId id, int req, int x, int y, int w, int h )
{
    if ( !configRequests )			// create queue
	configRequests = new QQueue<QWinConfigRequest>;
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
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

struct QSockNot {
    QObject *obj;
    int	     fd;
};

typedef QIntDict<QSockNot> QSNDict;

static QSNDict *sn_read	  = 0;
static QSNDict *sn_write  = 0;
static QSNDict *sn_except = 0;

static QSNDict**sn_vec[3] = { &sn_read, &sn_write, &sn_except };

static uint	sn_msg	  = 0;			// socket notifier message
static QWidget *sn_win	  = 0;			// win msg via this window


static void sn_cleanup()
{
    for ( int i=0; i<3; i++ )
	delete *sn_vec[i];
}

static void sn_init()
{
    if ( sn_win )
	return;
    qAddPostRoutine( sn_cleanup );
    sn_msg = RegisterWindowMessageA( "QtSNEvent" );
    sn_win = qApp->mainWidget();		// use main widget, if any
    if ( !sn_win ) {				// create internal widget
	sn_win = new QWidget(0,"QtSocketNotifier_Internal_Widget");
	CHECK_PTR( sn_win );
    }
    for ( int i=0; i<3; i++ ) {
	*sn_vec[i] = new QSNDict;
	CHECK_PTR( *sn_vec[i] );
	(*sn_vec[i])->setAutoDelete( TRUE );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNDict  *dict = *sn_vec[type];
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( sn_win == 0 ) {
	    sn_init();
	    dict = *sn_vec[type];
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
#if defined(DEBUG)
	if ( dict->find(sockfd) ) {
	    static const char *t[] = { "read", "write", "exception" };
	    warning( "QSocketNotifier: Multiple socket notifiers for "
		     "same socket %d and type %s", sockfd, t[type] );
	}
#endif
	dict->insert( sockfd, sn );
    } else {					// disable notifier
	if ( dict == 0 )
	    return FALSE;
	if ( !dict->remove(sockfd) )		// did not find sockfd
	    return FALSE;
    }
    int sn_event = 0;
    if ( sn_read && sn_read->find(sockfd) )
	sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if ( sn_write && sn_write->find(sockfd) )
	sn_event |= FD_WRITE | FD_CONNECT;
    if ( sn_except && sn_except->find(sockfd) )
	sn_event |= FD_OOB;
  // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
  // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect( sockfd, sn_win->winId(), sn_event ? sn_msg : 0, sn_event );
    return TRUE;
}


static void sn_activate_fd( int sockfd, int type )
{
    QSNDict  *dict = *sn_vec[type];
    QSockNot *sn   = dict ? dict->find(sockfd) : 0;
    if ( sn ) {
	QEvent event( QEvent::SockAct );
	QApplication::sendEvent( sn->obj, &event );
    }
}


/*****************************************************************************
  Windows-specific drawing used here
 *****************************************************************************/

static void drawTile( HDC hdc, int x, int y, int w, int h,
		      const QPixmap &pixmap, int xOffset, int yOffset )
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while( yPos < y + h ) {
	drawH = pixmap.height() - yOff;    // Cropping first row
	if ( yPos + drawH > y + h )	   // Cropping last row
	    drawH = y + h - yPos;
	xPos = x;
	xOff = xOffset;
	while( xPos < x + w ) {
	    drawW = pixmap.width() - xOff; // Cropping first column
	    if ( xPos + drawW > x + w )	   // Cropping last column
		drawW = x + w - xPos;
	    BitBlt( hdc, xPos, yPos, drawW, drawH, pixmap.handle(),
		    xOff, yOff, SRCCOPY );
	    xPos += drawW;
	    xOff = 0;
	}
	yPos += drawH;
	yOff = 0;
    }
}

Q_EXPORT
void qt_fill_tile( QPixmap *tile, const QPixmap &pixmap )
{
    bitBlt( tile, 0, 0, &pixmap, 0, 0, -1, -1, Qt::CopyROP, TRUE );
    int x = pixmap.width();
    while ( x < tile->width() ) {
	bitBlt( tile, x, 0, tile, 0, 0, x, pixmap.height(), Qt::CopyROP, TRUE);
	x *= 2;
    }
    int y = pixmap.height();
    while ( y < tile->height() ) {
	bitBlt( tile, 0, y, tile, 0, 0, tile->width(), y, Qt::CopyROP, TRUE );
	y *= 2;
    }
}

Q_EXPORT
void qt_draw_tiled_pixmap( HDC hdc, int x, int y, int w, int h,
			   const QPixmap *bg_pixmap,
			   int off_x, int off_y )
{
    if ( qt_winver == Qt::WV_NT ) {		// no brush size limitation
	HBRUSH brush = CreatePatternBrush( bg_pixmap->hbm() );
	HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, brush );
	if ( off_x || off_y ) {
	    POINT p;
	    SetBrushOrgEx( hdc, -off_x, -off_y, &p );
	    PatBlt( hdc, x, y, w, h, PATCOPY );
	    SetBrushOrgEx( hdc, p.x, p.y, 0 );
	} else {
	    PatBlt( hdc, x, y, w, h, PATCOPY );
	}
	SelectObject( hdc, oldBrush );
	DeleteObject( brush );
    } else {					// Windows 95 & 98
	QPixmap tile = *bg_pixmap;
	int sw = tile.width(), sh = tile.height();
	if ( sw*sh < 8192 && sw*sh < 16*w*h ) {
	    int tw = sw, th = sh;
	    while ( tw*th < 32678 && tw < w/2 )
		tw *= 2;
	    while ( tw*th < 32678 && th < h/2 )
		th *= 2;
	    tile.resize( tw, th );
	    tile.setOptimization( QPixmap::BestOptim );
	    qt_fill_tile( &tile, *bg_pixmap );
	}
	bool tmp_dc = tile.handle() == 0;
	if ( tmp_dc )
	    tile.allocMemDC();
	drawTile( hdc, x, y, w, h, tile, off_x, off_y );
	if ( tmp_dc )
	    tile.freeMemDC();
    }
}


#if defined(QT_BASEAPP)
typedef (*qt_ebg_fn)( HDC, int, int, int, int, const QColor &,
		      const QPixmap *, int, int );

static qt_ebg_fn qt_ebg_inst = 0;

Q_EXPORT void qt_ebg( void *p )
{
    qt_ebg_inst = (qt_ebg_fn)p;
}

void qt_erase_bg( HDC hdc, int x, int y, int w, int h,
		  const QColor &bg_color,
		  const QPixmap *bg_pixmap, int off_x, int off_y )
{
    if ( qt_ebg_inst )
	(*qt_ebg_inst)( hdc, x, y, w, h, bg_color, bg_pixmap, off_x, off_y );
}

#else

#define QT_ERASE_BACKGROUND

void qt_erase_bg( HDC hdc, int x, int y, int w, int h,
		  const QColor &bg_color,
		  const QPixmap *bg_pixmap, int off_x, int off_y )
{
    qt_erase_background( hdc, x, y, w, h, bg_color, bg_pixmap, off_x, off_y );
}

#endif


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    quit_now  = FALSE;
    quit_code = 0;
    enter_loop();
    return quit_code;
}


bool QApplication::processNextEvent( bool canWait )
{
    MSG	 msg;

    sendPostedEvents();

    if ( canWait ) {				// can wait if necessary
	if ( numZeroTimers ) {			// activate full-speed timers
	    int ok;
	    while ( numZeroTimers && !(ok=PeekMessage(&msg,0,0,0,PM_REMOVE)) )
		activateZeroTimers();
	    if ( !ok )				// no event
		return FALSE;
	} else {
	    if ( !GetMessage(&msg,0,0,0) ) {
		quit();				// WM_QUIT received
		return FALSE;
	    }
	}
    } else {					// no-wait mode
	if ( !PeekMessage(&msg,0,0,0,PM_REMOVE) ) { // no pending events
	    if ( numZeroTimers > 0 )		// there are 0-timers
		activateZeroTimers();
	    return FALSE;
	}
    }

    if ( msg.message == WM_TIMER ) {		// timer message received
	dispatchTimer( msg.wParam, &msg );
	return TRUE;
    }


    TranslateMessage( &msg );			// translate to WM_CHAR
    DispatchMessage( &msg );			// send to QtWndProc
    if ( configRequests )			// any pending configs?
	qWinProcessConfigRequests();

    sendPostedEvents();

    return TRUE;
}


void QApplication::processEvents( int maxtime )
{
    uint ticks = (uint)GetTickCount();
    while ( !quit_now && processNextEvent(FALSE) ) {
	if ( (uint)GetTickCount() - ticks > (uint)maxtime )
	    break;
    }
}


int QApplication::enter_loop()
{
    loop_level++;
    quit_now = FALSE;

    bool old_app_exit_loop = app_exit_loop;
    app_exit_loop = FALSE;

    while ( !quit_now && !app_exit_loop )
	processNextEvent( TRUE );

    app_exit_loop = old_app_exit_loop;
    loop_level--;

    return 0;
}

void QApplication::exit_loop()
{
    app_exit_loop = TRUE;
}


bool QApplication::winEventFilter( MSG * )	// Windows event filter
{
    return FALSE;
}


void QApplication::winFocus( QWidget *widget, bool gotFocus )
{
    if ( gotFocus ) {
	if ( inPopupMode() ) // some delayed focus event to ignore
	    return;
	active_window = widget->topLevelWidget();
	QWidget *w = widget->focusWidget();
	if (w && (w->isFocusEnabled() || w->isTopLevel() ) )
	    w->setFocus();
	else {
	    // set focus to some arbitrary widget with WTabToFocus
	    widget->focusNextPrevChild( TRUE );
	}
    } else {
	if ( focus_widget && !inPopupMode() ) {
	    QFocusEvent out( QEvent::FocusOut );
	    QWidget *widget = focus_widget;
	    focus_widget = 0;
	    QApplication::sendEvent( widget, &out );
	}
    }
}


//
// QtWndProc() receives all messages from the main event loop
//

extern "C"
LRESULT CALLBACK QtWndProc( HWND hwnd, UINT message, WPARAM wParam,
			    LPARAM lParam )
{
    if ( !qApp )				// unstable app state
	return DefWindowProc(hwnd,message,wParam,lParam);

    MSG msg;
    msg.hwnd = hwnd;				// create MSG structure
    msg.message = message;			// time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;

    if ( qApp->winEventFilter(&msg) )		// send through app filter
	return 0;

    QETWidget *widget = (QETWidget*)QWidget::find( hwnd );
    if ( !widget )				// don't know this widget
	return DefWindowProc(hwnd,message,wParam,lParam);

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, &msg) )
	    return 0;

    if ( widget->winEvent(&msg) )		// send through widget filter
	return 0;

    QEvent::Type evt_type = QEvent::None;
    bool result = TRUE;

    if ( sn_msg && message == sn_msg ) {	// socket notifier message
	int type = -1;
	switch ( WSAGETSELECTEVENT(lParam) ) {
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
	if ( type >= 0 )
	    sn_activate_fd( wParam, type );
    } else
    if ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST ) {
	if ( qApp->activePopupWidget() != 0) { // in popup mode
	    POINT curPos;
	    GetCursorPos( &curPos );
	    QWidget* w = QApplication::widgetAt(curPos.x, curPos.y);
	    if (w && w->testWFlags(Qt::WType_Popup))
		widget = (QETWidget*)w;
	}
	if ( widget->isEnabled() &&
	     (message == WM_LBUTTONDOWN ||
	      message == WM_MBUTTONDOWN ||
	      message == WM_RBUTTONDOWN) ) {
	    QWidget* w = widget;
	    while ( w->focusProxy() )
		w = w->focusProxy();
	    if ( w->focusPolicy() & QWidget::ClickFocus )
		w->setFocus();
	}
	widget->translateMouseEvent( msg );	// mouse event
    } else
    switch ( message ) {

	case WM_KEYDOWN:			// keyboard event
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_IME_CHAR:
	case WM_IME_KEYDOWN:
	case WM_CHAR: {
	    QWidget *g = QWidget::keyboardGrabber();
	    if ( g )
		widget = (QETWidget*)g;
	    else if ( qApp->focusWidget() )
		widget = (QETWidget*)qApp->focusWidget();
	    else
		widget = (QETWidget*)widget->topLevelWidget();
	    if ( widget->isEnabled() )
		result = widget->translateKeyEvent( msg, g != 0 );
	    break;
	  }
	case WM_SYSCHAR:
	    result = TRUE;                      // consume event
	    break;

	case WM_MOUSEWHEEL:
	    result = widget->translateWheelEvent( msg );
	    break;

	case WM_PAINT:				// paint event
	    result = widget->translatePaintEvent( msg );
	    break;

	case WM_ERASEBKGND: {			// erase window background
	    RECT r;
	    GetClientRect( hwnd, &r );
#if defined(QT_ERASE_BACKGROUND)
	    qt_erase_background
#else
	   qt_erase_bg
#endif
		    ( (HDC)wParam, r.left, r.top,
		      r.right-r.left, r.bottom-r.top,
		      widget->backgroundColor(),
		      widget->backgroundPixmap(), 0, 0 );
	    }
	    break;

	case WM_MOVE:				// move window
	case WM_SIZE:				// resize window
	    result = widget->translateConfigEvent( msg );
	    break;

	case WM_ACTIVATE:
	    if ( QApplication::activePopupWidget() && LOWORD(wParam) == WA_INACTIVE &&
		 QWidget::find((HWND)lParam) == 0 ) {
		// Another application was activated while our popups are open,
		// then close all popups.  In case some popup refuses to close,
		// we give up after 1024 attempts (to avoid an infinite loop).
		int maxiter = 1024;
		QWidget *popup;
		while ( (popup=QApplication::activePopupWidget()) && maxiter-- )
		    popup->hide();
	    }
	    qApp->winFocus( widget, LOWORD(wParam) == WA_INACTIVE ? 0 : 1 );
	    break;

	case WM_PALETTECHANGED:			// our window changed palette
	    if ( QColor::hPal() && (WId)wParam == widget->winId() )
		return 0;			// otherwise: FALL THROUGH!

	case WM_SETTINGCHANGE:
	    qt_set_windows_resources();
	    break;
	case WM_QUERYNEWPALETTE:		// realize own palette
	    if ( QColor::hPal() ) {
		HDC hdc = GetDC( widget->winId() );
		HPALETTE hpalOld = SelectPalette( hdc, QColor::hPal(), FALSE );
		uint n = RealizePalette( hdc );
		if ( n )
		    InvalidateRect( widget->winId(), 0, TRUE );
		SelectPalette( hdc, hpalOld, TRUE );
		RealizePalette( hdc );
		ReleaseDC( widget->winId(), hdc );
		return n;
	    }
	    break;

	case WM_CLOSE:				// close window
	    widget->translateCloseEvent( msg );
	    return 0;				// always handled

	case WM_DESTROY:			// destroy window
	    if ( hwnd == curWin ) {
		QEvent leave( QEvent::Leave );
		QApplication::sendEvent( widget, &leave );
		curWin = 0;
	    }
	    result = FALSE;
	    break;

	case WM_GETMINMAXINFO:
	    if ( widget->xtra() ) {
		MINMAXINFO *mmi = (MINMAXINFO *)lParam;
		QWExtra	   *x = widget->xtra();
		QRect	   f  = widget->frameGeometry();
		QSize	   s  = widget->size();
		if ( x->minw > 0 )
		    mmi->ptMinTrackSize.x = x->minw + f.width()	 - s.width();
		if ( x->minh > 0 )
		    mmi->ptMinTrackSize.y = x->minh + f.height() - s.height();
		if ( x->maxw < QCOORD_MAX )
		    mmi->ptMaxTrackSize.x = x->maxw + f.width()	 - s.width();
		if ( x->maxh < QCOORD_MAX )
		    mmi->ptMaxTrackSize.y = x->maxh + f.height() - s.height();
		return 0;
	    }
	    break;

	case WM_CHANGECBCHAIN:
	case WM_DRAWCLIPBOARD:
	    if ( qt_clipboard ) {
		QCustomEvent e( QEvent::Clipboard, &msg );
		QApplication::sendEvent( qt_clipboard, &e );
		return 0;
	    }
						// NOTE: fall-through!
	default:
	    result = FALSE;			// event was not processed
	    break;

    }

    if ( evt_type != QEvent::None ) {		// simple event
	QEvent e( evt_type );
	result = QApplication::sendEvent(widget, &e);
    }
    return result ? 0 : DefWindowProc(hwnd,message,wParam,lParam);
}


/*****************************************************************************
  Modal widgets; We have implemented our own modal widget mechanism
  to get total control.
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
    if ( !modal_stack ) {			// create modal stack
	modal_stack = new QWidgetList;
	CHECK_PTR( modal_stack );
    }
    releaseAutoCapture();
    modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( modal_stack && modal_stack->removeRef(widget) ) {
	if ( modal_stack->isEmpty() ) {
	    delete modal_stack;
	    modal_stack = 0;
	}
    }
    app_do_modal = modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, MSG *msg )
{
    if ( qApp->activePopupWidget() )
	return TRUE;
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=modal_stack->getFirst();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(Qt::WType_Modal) )	// widget is modal
	modal = widget;
    if ( modal == top )				// don't block event
	return TRUE;

#ifdef ALLOW_NON_APPLICATION_MODAL
    if ( top && top->parentWidget() ) {
	// Not application-modal
	// Does widget have a child in modal_stack?
	bool unrelated = TRUE;
	modal = modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != widget ) {
		p = p->parentWidget();
	    }
	    modal = modal_stack->next();
	    if ( p ) unrelated = FALSE;
	}
	if ( unrelated ) return TRUE;		// don't block event
    }
#endif

    bool block_event = FALSE;
    int	 type  = msg->message;

    if ( type == WM_NCHITTEST ) {
	block_event = TRUE;
	// QApplication::beep();
    } else if ( (type >= WM_MOUSEFIRST && type <= WM_MOUSELAST) ||
	 (type >= WM_KEYFIRST	&& type <= WM_KEYLAST) ) {
	block_event = TRUE;
    }

    if ( top->parentWidget() == 0 &&
	 (msg->message == WM_ACTIVATE && LOWORD(msg->wParam) != WA_INACTIVE) ){
	top->raise();
	block_event = TRUE;
    }

    return !block_event;
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

static QWidget *activeBeforePopup = 0;

void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
	activeBeforePopup = active_window;
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 && !qt_nograb() )
	setAutoCapture( popup->winId() );	// grab mouse/keyboard
    // Popups are not focus-handled by the window system (the first
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
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() )			// grabbing not disabled
	    releaseAutoCapture();
	active_window = activeBeforePopup;	// windows does not have
	// A reasonable focus handling for ours popups => we have
	// to restore the focus manually.
	if (active_window && active_window->focusWidget())
	    active_window->focusWidget()->setFocus();
	else
	    active_window->setFocus();
    }
     else {
 	// Popups are not focus-handled by the window system (the
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
  Functions returning the active popup and modal widgets.
 *****************************************************************************/


QWidget *QApplication::activePopupWidget()
{
    return popupWidgets ? popupWidgets->getLast() : 0;
}


QWidget *QApplication::activeModalWidget()
{
    return modal_stack ? modal_stack->getLast() : 0;
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

#include "qvector.h"
#include "qintdict.h"

struct TimerInfo {				// internal timer info
    uint     ind;				// - Qt timer identifier - 1
    uint     id;				// - Windows timer identifier
    bool     zero;				// - zero timing
    QObject *obj;				// - object to receive events
};
typedef QVector<TimerInfo>  TimerVec;		// vector of TimerInfo structs
typedef QIntDict<TimerInfo> TimerDict;		// fast dict of timers

static TimerVec  *timerVec  = 0;		// timer vector
static TimerDict *timerDict = 0;		// timer dict


void CALLBACK qt_simple_timer_func( HWND, UINT, UINT idEvent, DWORD )
{
    dispatchTimer( idEvent, 0 );
}


// Activate a timer, used by both event-loop based and simple timers.

static void dispatchTimer( uint timerId, MSG *msg )
{
#if defined(USE_HEARTBEAT)
    if ( timerId != (WPARAM)heartBeat ) {
	if ( !msg || !qApp || !qApp->winEventFilter(msg) )
	    activateTimer( timerId );
    } else if ( curWin && qApp ) {		// process heartbeat
	POINT p;
	GetCursorPos( &p );
	HWND newWin = WindowFromPoint(p);
	if ( newWin != curWin && QWidget::find(newWin) == 0 ) {
	    QWidget *curWidget = QWidget::find(curWin);
	    if ( curWidget ) {
		QEvent leave( QEvent::Leave );
		QApplication::sendEvent( curWidget, &leave );
	    }
	    curWin = 0;
	}
    }
#else
    if ( !msg || !qApp || !qApp->winEventFilter(msg) )
	activateTimer( timerId );
#endif
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
    register TimerInfo *t;
    int n = numZeroTimers;
    while ( n-- ) {
	while ( TRUE ) {
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
	while (PeekMessage( &msg, (HWND)-1, WM_TIMER, WM_TIMER, PM_REMOVE ))
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
    CHECK_PTR( t );
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
#if defined(DEBUG)
	warning( "qStartTimer: No more Windows timers" );
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
  Event translation; translates Windows events to Qt events
 *****************************************************************************/

//
// Auto-capturing for mouse press and mouse release
//

static void setAutoCapture( HWND h )
{
    if ( autoCaptureWnd )
	releaseAutoCapture();
    autoCaptureWnd = h;
    SetCapture( h );
}

static void releaseAutoCapture()
{
    if ( autoCaptureWnd ) {
	ReleaseCapture();
	autoCaptureWnd = 0;
    }
}


//
// Mouse event translation
//
// Non-client mouse messages are not translated
//

static ushort mouseTbl[] = {
    WM_MOUSEMOVE,	QEvent::MouseMove,		0,
    WM_LBUTTONDOWN,	QEvent::MouseButtonPress,	QMouseEvent::LeftButton,
    WM_LBUTTONUP,	QEvent::MouseButtonRelease,	QMouseEvent::LeftButton,
    WM_LBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	QMouseEvent::LeftButton,
    WM_RBUTTONDOWN,	QEvent::MouseButtonPress,	QMouseEvent::RightButton,
    WM_RBUTTONUP,	QEvent::MouseButtonRelease,	QMouseEvent::RightButton,
    WM_RBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	QMouseEvent::RightButton,
    WM_MBUTTONDOWN,	QEvent::MouseButtonPress,	QMouseEvent::MidButton,
    WM_MBUTTONUP,	QEvent::MouseButtonRelease,	QMouseEvent::MidButton,
    WM_MBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	QMouseEvent::MidButton,
    0,			0,				0
};

static int translateButtonState( int s, int type, int button )
{
    int bst = 0;
    if ( s & MK_LBUTTON )
	bst |= QMouseEvent::LeftButton;
    if ( s & MK_MBUTTON )
	bst |= QMouseEvent::MidButton;
    if ( s & MK_RBUTTON )
	bst |= QMouseEvent::RightButton;
    if ( s & MK_SHIFT )
	bst |= QMouseEvent::ShiftButton;
    if ( s & MK_CONTROL )
	bst |= QMouseEvent::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	bst |= QMouseEvent::AltButton;

    // Translate from Windows-style "state after event"
    // to X-style "state before event"
    if ( type == QEvent::MouseButtonPress ||
	 type == QEvent::MouseButtonDblClick )
	bst &= ~button;
    else if ( type == QEvent::MouseButtonRelease )
	bst |= button;

    return bst;
}


extern QCursor *qt_grab_cursor();

bool QETWidget::translateMouseEvent( const MSG &msg )
{
    static QPoint pos;
    static POINT gpos={-1,-1};
    QEvent::Type type;				// event parameters
    int	   button;
    int	   state;
    int	   i;

    for ( i=0; (UINT)mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3 )
	;
    if ( !mouseTbl[i] )
	return FALSE;
    type   = (QEvent::Type)mouseTbl[++i];	// event type
    button = mouseTbl[++i];			// which button
    state  = translateButtonState( msg.wParam, type, button ); // button state

    if ( type == QEvent::MouseMove ) {
	QCursor *c = qt_grab_cursor();
	if ( !c )
	    c = QApplication::overrideCursor();
	if ( c )				// application cursor defined
	    SetCursor( c->handle() );
	else					// use widget cursor
	    SetCursor( cursor().handle() );
	if ( curWin != winId() ) {		// new current window
	    if ( curWin ) {			// send leave event
		QWidget *curWidget = QWidget::find(curWin);
		if ( curWidget ) {
		    QEvent leave( QEvent::Leave );
		    QApplication::sendEvent( curWidget, &leave );
		}
	    }
	    curWin = winId();
	    QEvent enter( QEvent::Enter );	// send enter event
	    QApplication::sendEvent( this, &enter );
	}

	POINT curPos;
	GetCursorPos( &curPos );		// compress mouse move
	if ( curPos.x == gpos.x && curPos.y == gpos.y )
	    return TRUE;			// same global position
	gpos = curPos;

	if ( state == 0 && autoCaptureWnd == 0 && !hasMouseTracking() &&
	     !QApplication::hasGlobalMouseTracking() )
	    return TRUE;			// no button

	ScreenToClient( winId(), &curPos );

	pos.rx() = (short)curPos.x;
	pos.ry() = (short)curPos.y;
    } else {
	// ignore LOWORD(msg.lParam) and pos.ry() = HIWORD(msg.lParam)
	// since they might be for the wrong widget. Use the global cursor pos
	// instead.
	GetCursorPos( &gpos );
	pos = mapFromGlobal( QPoint(gpos.x, gpos.y) );
	
	if ( type == QEvent::MouseButtonPress ) {	// mouse button pressed
	    qt_button_down = findChildWidget( this, pos );	//magic for masked widgets
	    if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
		qt_button_down = this;
	}
    }

    if ( qApp->inPopupMode() ) {			// in popup mode
	QWidget* activePopupWidget = qApp->activePopupWidget();
	QWidget *popup = activePopupWidget;
	
	if ( popup != this ) {
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( mapToGlobal(pos) );
	}
	QWidget *popupChild = findChildWidget( popup, pos );
	bool releaseAfter = FALSE;
	switch ( type ) {
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonDblClick:
		popupButtonFocus = popupChild;
		break;
	    case QEvent::MouseButtonRelease:
		releaseAfter = TRUE;
		break;
	    default:
		break;				// nothing for mouse move
	}

	if ( popupButtonFocus ) {
	    QMouseEvent e( type,
		popupButtonFocus->mapFromGlobal(QPoint(gpos.x,gpos.y)),
		QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendEvent( popupButtonFocus, &e );
	    if ( releaseAfter ) {
		popupButtonFocus = 0;
	    }
	} else {
	    QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendEvent( popup, &e );
	}
	
	if ( releaseAfter )
	    qt_button_down = 0;
	
	if ( type == QEvent::MouseButtonPress &&
	     (qApp->activePopupWidget() != activePopupWidget) ){
	    // the popup dissappeared. Replay the event
	    QWidget* w = QApplication::widgetAt(gpos.x, gpos.y);
	    if (w) {
		QMouseEvent* e = new QMouseEvent( type, w->mapFromGlobal(QPoint(gpos.x, gpos.y)),
			   QPoint(gpos.x,gpos.y), button, state );
		QApplication::postEvent( w, e );
	    }
	}
    } else {					// not popup mode
	int bs = state & (QMouseEvent::LeftButton
			| QMouseEvent::RightButton
			| QMouseEvent::MidButton);
	if ( (type == QEvent::MouseButtonPress ||
	      type == QEvent::MouseButtonDblClick) && bs == 0 ) {
	    if ( QWidget::mouseGrabber() == 0 )
		setAutoCapture( winId() );
	} else if ( type == QEvent::MouseButtonRelease && bs == button ) {
	    if ( QWidget::mouseGrabber() == 0 )
		releaseAutoCapture();
	}
	QWidget* widget = this;
	if ( QWidget::mouseGrabber() == 0 && qt_button_down) {
	    widget = qt_button_down;
	    pos = mapToGlobal( pos );
	    pos = widget->mapFromGlobal( pos );
	}
	QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	QApplication::sendEvent( widget, &e );	// send event
	if ( type != QEvent::MouseMove )
	    pos.rx() = pos.ry() = -9999;	// init for move compression
	if ( type == QEvent::MouseButtonRelease &&
	     (state & (~button) & ( LeftButton |
				    MidButton |
				    RightButton)) == 0 ) {
	    qt_button_down = 0;
	}
    }
    return TRUE;
}


//
// Keyboard event translation
//

#include "qkeycode.h"

static ushort KeyTbl[] = {			// keyboard mapping table
    VK_ESCAPE,		Qt::Key_Escape,		// misc keys
    VK_TAB,		Qt::Key_Tab,
    VK_BACK,		Qt::Key_Backspace,
    VK_RETURN,		Qt::Key_Return,
    VK_INSERT,		Qt::Key_Insert,
    VK_DELETE,		Qt::Key_Delete,
//  VK_CLEAR,		Qt::Key_Clear,
    VK_PAUSE,		Qt::Key_Pause,
    VK_SNAPSHOT,	Qt::Key_Print,
    VK_HOME,		Qt::Key_Home,		// cursor movement
    VK_END,		Qt::Key_End,
    VK_LEFT,		Qt::Key_Left,
    VK_UP,		Qt::Key_Up,
    VK_RIGHT,		Qt::Key_Right,
    VK_DOWN,		Qt::Key_Down,
    VK_PRIOR,		Qt::Key_Prior,
    VK_NEXT,		Qt::Key_Next,
    VK_SHIFT,		Qt::Key_Shift,		// modifiers
    VK_CONTROL,		Qt::Key_Control,
//  VK_????,		Qt::Key_Meta,
//  VK_????,		Qt::Key_Meta,
    VK_MENU,		Qt::Key_Alt,
    VK_CAPITAL,		Qt::Key_CapsLock,
    VK_NUMLOCK,		Qt::Key_NumLock,
    VK_SCROLL,		Qt::Key_ScrollLock,
    VK_NUMPAD0,		Qt::Key_0,			// numeric keypad
    VK_NUMPAD1,		Qt::Key_1,
    VK_NUMPAD2,		Qt::Key_2,
    VK_NUMPAD3,		Qt::Key_3,
    VK_NUMPAD4,		Qt::Key_4,
    VK_NUMPAD5,		Qt::Key_5,
    VK_NUMPAD6,		Qt::Key_6,
    VK_NUMPAD7,		Qt::Key_7,
    VK_NUMPAD8,		Qt::Key_8,
    VK_NUMPAD9,		Qt::Key_9,
    VK_MULTIPLY,	Qt::Key_Asterisk,
    VK_ADD,		Qt::Key_Plus,
    VK_SEPARATOR,	Qt::Key_Comma,
    VK_SUBTRACT,	Qt::Key_Minus,
    VK_DECIMAL,		Qt::Key_Period,
    VK_DIVIDE,		Qt::Key_Slash,
    VK_APPS,		Qt::Key_Menu,			
    0,			0
};

static int translateKeyCode( int key )		// get Qt::Key_... code
{
    int code;
    if ( (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') ) {
	code = 0;
    } else if ( key >= VK_F1 && key <= VK_F24 ) {
	code = Qt::Key_F1 + (key - VK_F1);		// function keys
    } else {
	int i = 0;				// any other keys
	code = 0;
	while ( KeyTbl[i] ) {
	    if ( key == (int)KeyTbl[i] ) {
		code = KeyTbl[i+1];
		break;
	    }
	    i += 2;
	}
    }
    return code;
}

struct KeyRec {
    KeyRec(int c, int a, const QString& t) : code(c), ascii(a), text(t) { }
    KeyRec() { }
    int code, ascii;
    QString text;
};

static const int maxrecs=64; // User has LOTS of fingers...
static KeyRec key_rec[maxrecs];
static int nrecs=0;

static KeyRec* find_key_rec( int code, bool remove )
{
    KeyRec *result = 0;
    for (int i=0; i<nrecs; i++) {
	if (key_rec[i].code == code) {
	    if (remove) {
		static KeyRec tmp;
		tmp = key_rec[i];
		while (i+1 < nrecs) {
		    key_rec[i] = key_rec[i+1];
		    i++;
		}
		nrecs--;
		result = &tmp;
	    } else {
		result = &key_rec[i];
	    }
	    break;
	}
    }
    return result;
}

static void store_key_rec( int code, int ascii, const QString& text )
{
    if ( nrecs == maxrecs ) {
#if defined(CHECK_RANGE)
	warning( "Qt: Internal keyboard buffer overflow" );
#endif
	return;
    }

    key_rec[nrecs++] = KeyRec(code,ascii,text);
}

static int asciiToKeycode(char a, int state)
{
    if ( a >= 'a' && a <= 'z' )
	a = toupper( a );
    if ( (state & QMouseEvent::ControlButton) != 0 ) {
	if ( a >= 1 && a <= 26 )	// Ctrl+'A'..'Z'
	    a += 'A' - 1;
    }
    return a;
}


bool QETWidget::translateKeyEvent( const MSG &msg, bool grab )
{
    bool k0=FALSE, k1=FALSE;
    int  state = 0;

    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= QMouseEvent::ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= QMouseEvent::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= QMouseEvent::AltButton;
    //TODO: if it is a pure shift/ctrl/alt keydown, invert state logic, like X

    if ( msg.message == WM_CHAR ) {
	// a multi-character key not found by our look-ahead
	QString s;
	s += (char)msg.wParam;
	k0 = sendKeyEvent( QEvent::KeyPress, 0, msg.wParam, state, grab, s );
	k1 = sendKeyEvent( QEvent::KeyRelease, 0, msg.wParam, state, grab, s );
    }
    else if ( msg.message == WM_IME_CHAR ) {
	debug("IME"); //###
	// input method characters not found by our look-ahead
	QString s;
	ushort uc = (ushort)msg.wParam;
	s += QChar(uc&0xff,(uc>>8)&0xff);
	k0 = sendKeyEvent( QEvent::KeyPress, 0, msg.wParam, state, grab, s );
	k1 = sendKeyEvent( QEvent::KeyRelease, 0, msg.wParam, state, grab, s );
    }
    else {
	int code = translateKeyCode( msg.wParam );
	int t = msg.message;
        if ( t == WM_KEYDOWN || t == WM_IME_KEYDOWN || t == WM_SYSKEYDOWN ) {
	    // KEYDOWN
	    KeyRec* rec = find_key_rec( msg.wParam, FALSE );
	    // Find uch
	    QChar uch;
	    MSG wm_char;
	    UINT charType = ( t == WM_KEYDOWN ? WM_CHAR :
			      t == WM_IME_KEYDOWN ? WM_IME_CHAR : WM_SYSCHAR );
	    if ( PeekMessage(&wm_char, 0, charType, charType, PM_REMOVE) ) {
		// Found a XXX_CHAR
		uch = QChar(wm_char.wParam & 0xff, (wm_char.wParam>>8) & 0xff);
		if ( t == WM_SYSKEYDOWN && !uch.row &&
		     isalpha(uch.cell) && (msg.lParam & KF_ALTDOWN) ) {
		    // (See doc of WM_SYSCHAR)
    		    uch = QChar((char)tolower(uch.cell)); //Alt-letter
		}
		if ( !code && !uch.row )
		    code = asciiToKeycode(uch.cell, state);
	    }
	    else {
		// No XXX_CHAR; deduce uch from XXX_KEYDOWN params
		if ( msg.wParam == VK_DELETE )
		    uch = QChar((char)0x7f); // Windows doesn't know this one.
		else
		    uch = QChar((char)MapVirtualKey( msg.wParam, 2 ));
		if ( !code )
		    code = asciiToKeycode( uch.cell, state);
	    }

	    if ( rec ) {
		// it is already down (so it is auto-repeating)
		if ( code < Key_Shift || code > Key_ScrollLock ) {
		    k0 = sendKeyEvent( QEvent::KeyRelease, code, rec->ascii,
				       state, grab, rec->text, TRUE);
		    k1 = sendKeyEvent( QEvent::KeyPress, code, rec->ascii,
				       state, grab, rec->text, TRUE);
		}
	    } else {
		QString text;
		if ( uch != QChar::null )
		    text += uch;
		char a = uch.row ? 0 : uch.cell;
		store_key_rec( msg.wParam, a, text );
		k0 = sendKeyEvent( QEvent::KeyPress, code, a,
				   state, grab, text );
	    }
	
	    if ( state == QMouseEvent::AltButton ) {
		// Special handling of global Windows hotkeys
		switch ( code ) {
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Enter:
		case Qt::Key_F4:
		case Qt::Key_Space:
		    k0 = FALSE;		// Send the event on to Windows
		    k1 = FALSE;
		    break;
		default:
		    break;
		}
	    }

        } else {
	    // Must be KEYUP
	    KeyRec* rec = find_key_rec( msg.wParam, TRUE );
	    if ( !rec ) {
		// Someone ate the key down event
	    } else {
		if ( !code )
		    code = asciiToKeycode(rec->ascii ? rec->ascii : msg.wParam,
				state);
		k0 = sendKeyEvent( QEvent::KeyRelease, code, rec->ascii,
				    state, grab, rec->text);
	    }
        }
    }
    return k0 || k1;
}


bool QETWidget::translateWheelEvent( const MSG &msg )
{
    int  state = 0;

    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= QMouseEvent::ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= QMouseEvent::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= QMouseEvent::AltButton;

    int delta =	(short) HIWORD ( msg.wParam );
    QPoint globalPos;

    globalPos.rx() = LOWORD ( msg.lParam );
    globalPos.ry() = HIWORD ( msg.lParam );

    QWheelEvent e( globalPos, delta, state );
    e.ignore();	

    // send the event to the widget that has the focus or its ancestors
    QWidget* w = qApp->focus_widget;
    if (w){
	do {
	    ((QPoint)e.pos()) = w->mapFromGlobal(globalPos); // local coordinates
	    QApplication::sendEvent( w, &e );
	    if ( e.isAccepted() )
		return TRUE;
	    w = w->parentWidget();
	} while (w);
    }

    return TRUE;
}


static bool isModifierKey(int code)
{
    return code >= Qt::Key_Shift && code <= Qt::Key_ScrollLock;
}

bool QETWidget::sendKeyEvent( QEvent::Type type, int code, int ascii,
			      int state, bool grab, const QString& text,
			      bool autor )
{
    if ( type == QEvent::KeyPress && !grab ) {	// send accel event to tlw
	QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	if ( a.isAccepted() )
	    return TRUE;
    }
    if ( !isEnabled() )
	return FALSE;
    QKeyEvent e( type, code, ascii, state, text, autor );
    QApplication::sendEvent( this, &e );
    if ( !isModifierKey(code) && state == QMouseEvent::AltButton
      && type == QEvent::KeyPress && !e.isAccepted() )
	QApplication::beep();  // emulate windows behavior
    return e.isAccepted();
}


//
// Paint event translation
//

bool QETWidget::translatePaintEvent( const MSG & )
{
    PAINTSTRUCT ps;
    RECT rect;
    GetUpdateRect( winId(), &rect, FALSE );
    QRect r( QPoint(rect.left,rect.top), QPoint(rect.right,rect.bottom) );
    QPaintEvent e( r );

#if 0
    // Use real update region later
    HRGN reg = CreateRectRgn(0,0,1,1); // empty would do
    GetUpdateRgn( winId(), reg, FALSE );
    QPaintEvent e( reg );
#endif

    setWState( WState_InPaintEvent );
    hdc = BeginPaint( winId(), &ps );
    QApplication::sendEvent( this, (QEvent*) &e );
    EndPaint( winId(), &ps );
    hdc = 0;
    clearWState( WState_InPaintEvent );
    return TRUE;
}

//
// Window move and resize (configure) events
//

bool QETWidget::translateConfigEvent( const MSG &msg )
{
    if ( !testWState(WState_Created) )		// in QWidget::create()
	return TRUE;
    setWState( WState_ConfigPending );		// set config flag
    QRect r = geometry();
    WORD a = LOWORD(msg.lParam);
    WORD b = HIWORD(msg.lParam);
    if ( msg.message == WM_SIZE ) {		// resize event
	QSize oldSize = size();
	QSize newSize( a, b );
	r.setSize( newSize );
	if ( msg.wParam != SIZE_MINIMIZED )
	    setCRect( r );
	if ( isTopLevel() ) {			// update caption/icon text
	    createTLExtra();
	    if ( msg.wParam == SIZE_MINIMIZED ) {
		// being "hidden"
		if ( isVisible() ) {
		    clearWState( WState_Visible );
		    QHideEvent e(TRUE);
		    QApplication::sendEvent( this, &e );
		    extra->topextra->iconic = 1;
		}
	    } else if ( extra->topextra->iconic ) {
		// being shown
		if ( !isVisible() ) {
		    setWState( WState_Visible );
		    QShowEvent e(TRUE);
		    QApplication::sendEvent( this, &e );
		}
		extra->topextra->iconic = 0;
	    }
	    if ( IsIconic(winId()) && iconText() )
		SetWindowText( winId(), (TCHAR*)qt_winTchar(iconText(),TRUE) );
	    else if ( !caption().isNull() )
		SetWindowText( winId(), (TCHAR*)qt_winTchar(caption(),TRUE) );
	}
	if ( isVisible() ) {
	    QResizeEvent e( newSize, oldSize );
	    QApplication::sendEvent( this, &e );
	} else {
	    QResizeEvent * e = new QResizeEvent( newSize, oldSize );
	    QApplication::postEvent( this, e );
	}
	if ( !testWFlags(WResizeNoErase) )
	    repaint( TRUE );
    } else if ( msg.message == WM_MOVE ) {	// move event
	QPoint oldPos = pos();
	// Ignore silly Windows move event to wild pos after iconify.
	if ( a <= QCOORD_MAX && b <= QCOORD_MAX ) {
	    QPoint newPos( a, b );
	    r.moveTopLeft( newPos );
	    setCRect( r );
	    if ( isVisible() ) {
		QMoveEvent e( newPos, oldPos );
		QApplication::sendEvent( this, &e );
	    } else {
		QMoveEvent * e = new QMoveEvent( newPos, oldPos );
		QApplication::postEvent( this, e );
	    }
	}
    }
    clearWState( WState_ConfigPending );		// clear config flag
    return TRUE;
}


//
// Close window event translation.
//
// This class is a friend of QApplication because it needs to emit the
// lastWindowClosed() signal when the last top level widget is closed.
//

bool QETWidget::translateCloseEvent( const MSG & )
{
    return close(FALSE);
}
