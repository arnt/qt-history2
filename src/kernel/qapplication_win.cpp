/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_win.cpp#454 $
**
** Implementation of Win32 startup routines and event handling
**
** Created : 931203
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qapplication.h"
#include "qapplication_p.h"
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qsessionmanager.h"
#include "qmime.h"
#include "qguardedptr.h"
#include "qwhatsthis.h" // ######## dependency
#include <ctype.h>
#include "qt_windows.h"
#include <windowsx.h>
#include <limits.h>
#if defined(__CYGWIN32__)
#define __INSIDE_CYGWIN32__
#include <mywinsock.h>
#endif

static UINT WM95_MOUSEWHEEL = 0;

#ifndef QT_MAKEDLL
#include "qtmain_win.cpp"
#endif

extern void qt_dispatchEnterLeave( QWidget*, QWidget* ); // qapplication.cpp

/*
  Internal functions.
*/

Q_EXPORT
void qt_draw_tiled_pixmap( HDC, int, int, int, int,
			   const QPixmap *, int, int );

void qt_erase_background( HDC hdc, int x, int y, int w, int h,
			  const QColor &bg_color,
			  const QPixmap *bg_pixmap, int off_x, int off_y )
{
    if ( bg_pixmap && bg_pixmap->isNull() )	// empty background
	return;
    HPALETTE oldPal = 0;
    if ( QColor::hPal() ) {
	oldPal = SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    }
    if ( bg_pixmap ) {
	qt_draw_tiled_pixmap( hdc, x, y, w, h, bg_pixmap, off_x, off_y );
    } else {
	HBRUSH brush = CreateSolidBrush( bg_color.pixel() );
	HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, brush );
	PatBlt( hdc, x, y, w, h, PATCOPY );
	SelectObject( hdc, oldBrush );
	DeleteObject( brush );
    }
    if ( QColor::hPal() ) {
	SelectPalette( hdc, oldPal, TRUE );
	RealizePalette( hdc );
    }
}




#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x020A
#endif

QRgb qt_colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}


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
//#define USE_HEARTBEAT
#if defined(USE_HEARTBEAT)
static int	 heartBeat	= 0;		// heatbeat timer
#endif

// Session management
static bool	sm_blockUserInput    = FALSE;
static bool	sm_smActive	     = FALSE;
static bool	sm_interactionActive = FALSE;
static QSessionManager* win_session_manager = 0;
static bool	sm_cancel;

// one day in the future we will be able to have static objects in libraries....
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static bool replayPopupMouseEvent = FALSE; // replay handling when popups close

#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif

static bool	app_do_modal	   = FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;
static QWidget *popupButtonFocus   = 0;
static bool	popupCloseDownMode = FALSE;
static bool	qt_try_modal( QWidget *, MSG *, int& ret );

#if defined (QT_THREAD_SUPPORT)
static unsigned long qt_gui_thread = 0;
#endif

QWidget	       *qt_button_down = 0;		// widget got last button-down

static HWND	autoCaptureWnd = 0;
static void	setAutoCapture( HWND );		// automatic capture
static void	releaseAutoCapture();

typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

// VFPTR qt_set_preselect_handler( VFPTR );
static VFPTR qt_preselect_handler = 0;
// VFPTR qt_set_postselect_handler( VFPTR );
static VFPTR qt_postselect_handler = 0;
Q_EXPORT VFPTR qt_set_preselect_handler( VFPTR handler )
{
    VFPTR old_handler = qt_preselect_handler;
    qt_preselect_handler = handler;
    return old_handler;
}
Q_EXPORT VFPTR qt_set_postselect_handler( VFPTR handler )
{
    VFPTR old_handler = qt_postselect_handler;
    qt_postselect_handler = handler;
    return old_handler;
}

typedef int (*QWinEventFilter) (MSG*);
QWinEventFilter qt_set_win_event_filter (QWinEventFilter filter);

static QWinEventFilter qt_win_event_filter = 0;
QWinEventFilter qt_set_win_event_filter (QWinEventFilter filter)
{
    QWinEventFilter old_filter = qt_win_event_filter;
    qt_win_event_filter = filter;
    return old_filter;
}
static bool qt_winEventFilter( MSG* msg )
{
    if ( qt_win_event_filter && qt_win_event_filter( msg )  )
	return TRUE;
    return qApp->winEventFilter( msg );
}

static void	msgHandler( QtMsgType, const char* );
static void     unregWinClasses();

// Simpler timers are needed when Qt does not have the event loop,
// such as for plugins.
Q_EXPORT bool	qt_win_use_simple_timers = FALSE;
void CALLBACK   qt_simple_timer_func( HWND, UINT, UINT, DWORD );

static void	initTimers();
static void	cleanupTimers();
static bool	dispatchTimer( uint, MSG * );
static bool	activateTimer( uint );
static void	activateZeroTimers();

static int	translateKeyCode( int );

Qt::WindowsVersion qt_winver = Qt::WV_NT;

QObject	       *qt_clipboard   = 0;

extern QCursor *qt_grab_cursor();

#if defined(Q_WS_WIN)
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
	const char *p = strrchr( appName, '\\' );	// skip path
	if ( p )
	    memmove( appName, p+1, qstrlen(p) );
	int l = qstrlen( appName );
	if ( (l > 4) && !qstricmp( appName + l - 4, ".exe" ) )
	    appName[l-4] = '\0';		// drop .exe extension
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
#if defined(QT_CHECK_STATE)
	qWarning( "Qt internal error: qWinMain should be called only once" );
#endif
	return;
    }
    already_called = TRUE;

  // Install default debug handler

#if defined(Q_CC_MSVC)
    qInstallMsgHandler( msgHandler );
#endif

  // Create command line

    set_winapp_name();

    char *p = cmdParam;
    char *p_end = p + strlen(p);

    argc = 1;
    argv[0] = appName;

    while ( *p && p < p_end ) {				// parse cmd line arguments

	while ( isspace(*p) )			// skip white space
	    p++;

	if ( *p && p < p_end ) {				// arg starts
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
	    while ( *p && p < p_end ) {
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
		if ( p )
		    *r++ = *p++;
	    }
	    if ( *p && p < p_end )
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

static void qt_show_system_menu( QWidget* tlw)
{
    HMENU menu = GetSystemMenu( tlw->winId(), FALSE );
    if ( !menu )
	return; // no menu for this window

#define enabled (MF_BYCOMMAND | MF_ENABLED)
#define disabled (MF_BYCOMMAND | MF_GRAYED)

    EnableMenuItem( menu, SC_MINIMIZE, enabled);
    bool maximized  = IsZoomed( tlw->winId() );

    EnableMenuItem( menu, SC_MAXIMIZE, maximized?disabled:enabled);
    EnableMenuItem( menu, SC_RESTORE, maximized?enabled:disabled);

    EnableMenuItem( menu, SC_SIZE, enabled);
    EnableMenuItem( menu, SC_MOVE, enabled);
    EnableMenuItem( menu, SC_CLOSE, enabled);
    EnableMenuItem( menu, SC_MINIMIZE, enabled);

#undef enabled
#undef disabled

    int ret = TrackPopupMenuEx( menu,
				TPM_LEFTALIGN  | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
				tlw->geometry().x(), tlw->geometry().y(),
				tlw->winId(),
				0);
    if (ret)
	if ( qt_winver & Qt::WV_NT_based )
	    DefWindowProc(tlw->winId(), WM_SYSCOMMAND, ret, 0);
	else
	    DefWindowProcA(tlw->winId(), WM_SYSCOMMAND, ret, 0);
}

extern QFont qt_LOGFONTtoQFont(LOGFONT& lf,bool scale);

// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();

static void qt_set_windows_resources()
{
    QFont menuFont;
    QFont messageFont;
    QFont statusFont;

    if ( qt_winver & Qt::WV_NT_based ) {
	// W or A version
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof( ncm );
	SystemParametersInfo( SPI_GETNONCLIENTMETRICS,
			      sizeof( ncm ), &ncm, NULL);
	menuFont = qt_LOGFONTtoQFont(ncm.lfMenuFont,TRUE);
	messageFont = qt_LOGFONTtoQFont(ncm.lfMessageFont,TRUE);
	statusFont = qt_LOGFONTtoQFont(ncm.lfStatusFont,TRUE);
    } else {
	// A version
	NONCLIENTMETRICSA ncm;
	ncm.cbSize = sizeof( ncm );
	SystemParametersInfoA( SPI_GETNONCLIENTMETRICS,
			      sizeof( ncm ), &ncm, NULL);
	menuFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMenuFont,TRUE);
	messageFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMessageFont,TRUE);
	statusFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfStatusFont,TRUE);
    }

    QApplication::setFont( menuFont, TRUE, "QPopupMenu");
    QApplication::setFont( menuFont, TRUE, "QMenuBar");
    QApplication::setFont( messageFont, TRUE, "QMessageBox");
    QApplication::setFont( statusFont, TRUE, "QTipLabel");
    QApplication::setFont( statusFont, TRUE, "QStatusBar");

    if ( qt_std_pal && *qt_std_pal != QApplication::palette() )
	return;

    // Do the color settings

    QColorGroup cg;
    cg.setColor( QColorGroup::Foreground,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))) );
    cg.setColor( QColorGroup::Button,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))) );
    cg.setColor( QColorGroup::Light,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))) );
    cg.setColor( QColorGroup::Midlight,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DLIGHT))) );
    cg.setColor( QColorGroup::Dark,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNSHADOW))) );
    cg.setColor( QColorGroup::Mid, cg.button().dark( 150 ) );
    cg.setColor( QColorGroup::Text,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))) );
    cg.setColor( QColorGroup::BrightText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))) );
    cg.setColor( QColorGroup::ButtonText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNTEXT))) );
    cg.setColor( QColorGroup::Base,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOW))) );
    cg.setColor( QColorGroup::Background,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))) );
    cg.setColor( QColorGroup::Shadow,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DDKSHADOW))) );
    cg.setColor( QColorGroup::Highlight,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
    cg.setColor( QColorGroup::HighlightedText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

    if ( qt_winver == Qt::WV_2000 || 
	 qt_winver == Qt::WV_98 || 
	 qt_winver == Qt::WV_XP ) {
	if ( cg.midlight() == cg.button() )
	    cg.setColor( QColorGroup::Midlight, cg.button().light(110) );
    }

    QColor disabled( (cg.foreground().red()+cg.button().red())/2,
		     (cg.foreground().green()+cg.button().green())/2,
		     (cg.foreground().blue()+cg.button().blue())/2);
    QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
		     disabled, cg.brightText(), cg.base(), cg.background() );
    dcg.setColor( QColorGroup::Highlight,
		  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
    dcg.setColor( QColorGroup::HighlightedText,
		  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

    QColorGroup icg = cg;
    if ( qt_winver == Qt::WV_2000 || 
	 qt_winver == Qt::WV_98 || 
	 qt_winver == Qt::WV_XP ) {
	if ( icg.background() != icg.base() ) {
	    icg.setColor( QColorGroup::Highlight, icg.background() );
	    icg.setColor( QColorGroup::HighlightedText, icg.text() );
	}
    }

    QPalette pal( cg, dcg, icg );
    QApplication::setPalette( pal, TRUE );
    *qt_std_pal = pal;

    QColor menu(qt_colorref2qrgb(GetSysColor(COLOR_MENU)));
    QColor menuText(qt_colorref2qrgb(GetSysColor(COLOR_MENUTEXT)));
    {
	// we might need a special color group for the menu.
	cg.setColor( QColorGroup::Button, menu );
	cg.setColor( QColorGroup::Text, menuText );
	cg.setColor( QColorGroup::Foreground, menuText );
	cg.setColor( QColorGroup::ButtonText, menuText );
	QColor disabled( (cg.foreground().red()+cg.button().red())/2,
			 (cg.foreground().green()+cg.button().green())/2,
			 (cg.foreground().blue()+cg.button().blue())/2);
	QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
			 disabled, cg.brightText(), cg.base(), cg.background() );
	dcg.setColor( QColorGroup::Highlight,
		      QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
	dcg.setColor( QColorGroup::HighlightedText,
		      QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

	icg = cg;
	if ( qt_winver == Qt::WV_2000 || 
	     qt_winver == Qt::WV_98 || 
	     qt_winver == Qt::WV_XP ) {
	    icg.setColor( QColorGroup::ButtonText, icg.dark() );
	}
	QPalette menu(cg, dcg, icg);
	QApplication::setPalette( menu, TRUE, "QPopupMenu");
	QApplication::setPalette( menu, TRUE, "QMenuBar");
    }

    QColor ttip(qt_colorref2qrgb(GetSysColor(COLOR_INFOBK)));
    QColor ttipText(qt_colorref2qrgb(GetSysColor(COLOR_INFOTEXT)));
    {
	cg.setColor( QColorGroup::Button, ttip );
	cg.setColor( QColorGroup::Background, ttip );
	cg.setColor( QColorGroup::Text, ttipText );
	cg.setColor( QColorGroup::Foreground, ttipText );
	cg.setColor( QColorGroup::ButtonText, ttipText );
	QColor disabled( (cg.foreground().red()+cg.button().red())/2,
			 (cg.foreground().green()+cg.button().green())/2,
			 (cg.foreground().blue()+cg.button().blue())/2);
	QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
			 disabled, Qt::white, Qt::white, cg.background() );

	QPalette pal(cg, dcg, cg);
	QApplication::setPalette( pal, TRUE, "QTipLabel");
    }
}

/*****************************************************************************
  qt_init() - initializes Qt for Windows
 *****************************************************************************/

void qt_init( int *argcptr, char **argv, QApplication::Type )
{
    // Detect the Windows version
    (void) QApplication::winVersion();

#if defined(QT_DEBUG)
    int argc = *argcptr;
    int i, j;

  // Get command line params

    j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( arg == "-nograb" )
	    appNoGrab = !appNoGrab;
	else
	    argv[j++] = argv[i];
    }
    *argcptr = j;
#else
    Q_UNUSED( argcptr );
    Q_UNUSED( argv );
#endif // QT_DEBUG


    // Get the application name/instance if qWinMain() was not invoked
    set_winapp_name();
    if ( appInst == 0 ) {
	if ( qt_winver & Qt::WV_NT_based )
	    appInst = GetModuleHandle( 0 );
	else
	    appInst = GetModuleHandleA( 0 );
    }

    // Tell tools/ modules.
    qt_winunicode = (qt_winver & Qt::WV_NT_based);

    // Initialize OLE/COM
    //	 S_OK means success and S_FALSE means that it has already
    //	 been initialized
    HRESULT r;
    r = OleInitialize(0);
    if ( r != S_OK && r != S_FALSE ) {
#if defined(QT_CHECK_STATE)
	qWarning( "Qt: Could not initialize OLE (error %x)", r );
#endif
    }

    // Misc. initialization

    QWindowsMime::initialize();
    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
    qApp->setName( appName );

    // default font
    {
	HFONT hfont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
	QFont f("MS Sans Serif",8);
	if ( qt_winver & Qt::WV_NT_based ) {
	    LOGFONT lf;
	    if ( GetObject( hfont, sizeof(lf), &lf ) )
		f = qt_LOGFONTtoQFont((LOGFONT&)lf,TRUE);
	} else {
	    LOGFONTA lf;
	    if ( GetObjectA( hfont, sizeof(lf), &lf ) )
		f = qt_LOGFONTtoQFont((LOGFONT&)lf,TRUE);
	}
	QApplication::setFont( f );
    }

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
    // QFont::locale_init();  ### Uncomment when it does something on Windows

    if ( !qt_std_pal )
	qt_create_std_palette();
    if ( QApplication::desktopSettingsAware() )
	qt_set_windows_resources();

#if defined(UNICODE)
    if ( qt_winver & Qt::WV_NT_based )
	WM95_MOUSEWHEEL = RegisterWindowMessage(L"MSWHEEL_ROLLMSG");
    else
#endif
	WM95_MOUSEWHEEL = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
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

    delete activeBeforePopup;
    activeBeforePopup = 0;
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
	Q_CHECK_PTR( postRList );
    }
    postRList->prepend( p );
}


Q_EXPORT const char *qAppName()			// get application name
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
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}


static QAsciiDict<int> *winclassNames = 0;

const char* qt_reg_winclass( int flags )	// register window class
{
    if ( !winclassNames ) {
	winclassNames = new QAsciiDict<int>;
	Q_CHECK_PTR( winclassNames );
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

    if ( qt_winver & Qt::WV_NT_based ) {
	WNDCLASS wc;
	wc.style	= style;
	wc.lpfnWndProc	= (WNDPROC)QtWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= (HINSTANCE)qWinAppInst();
	if ( icon ) {
	    TCHAR* irc = (TCHAR*)qt_winTchar( QString::fromLatin1("IDI_ICON1"),
					      TRUE );
	    wc.hIcon = LoadIcon( appInst, irc );
	    if ( !wc.hIcon )
		wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
	}
	else {
	    wc.hIcon = 0;
	}
	wc.hCursor	= 0;
	wc.hbrBackground= 0;
	wc.lpszMenuName	= 0;
	wc.lpszClassName= (TCHAR*)qt_winTchar(QString::fromLatin1(cname),TRUE);
	RegisterClass( &wc );
    } else {
	WNDCLASSA wc;
	wc.style	= style;
	wc.lpfnWndProc	= (WNDPROC)QtWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= (HINSTANCE)qWinAppInst();
	if ( icon ) {
	    wc.hIcon = LoadIconA( appInst, (char*)"IDI_ICON1" );
	    if ( !wc.hIcon )
		wc.hIcon = LoadIconA( 0, (char*)IDI_APPLICATION );
	}
	else {
	    wc.hIcon = 0;
	}
	wc.hCursor	= 0;
	wc.hbrBackground= 0;
	wc.lpszMenuName	= 0;
	wc.lpszClassName= cname;
	RegisterClassA( &wc );
    }

    winclassNames->insert( cname, (int*)1 );
    return cname;
}

static void unregWinClasses()
{
    if ( !winclassNames )
	return;
    QAsciiDictIterator<int> it(*winclassNames);
    const char *k;
    while ( (k = it.currentKey()) ) {
	if ( qt_winver & Qt::WV_NT_based ) {
	    UnregisterClass( (TCHAR*)qt_winTchar(QString::fromLatin1(k),TRUE),
			     (HINSTANCE)qWinAppInst() );
	} else {
	    UnregisterClassA( k, (HINSTANCE)qWinAppInst() );
	}
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

Qt::WindowsVersion QApplication::winVersion()
{
    return qt_winver = (Qt::WindowsVersion)qWinVersion();
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	Q_CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    Q_CHECK_PTR( app_cursor );
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

#endif

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
    if ( !child && !w->isTopLevel() )
        w = w->topLevelWidget();
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
    sn_msg = RegisterWindowMessageA( "QtSNEvent" );
    sn_win = qApp->mainWidget();		// use main widget, if any
    if ( !sn_win ) {				// create internal widget
	sn_win = new QWidget(0,"QtSocketNotifier_Internal_Widget");
	Q_CHECK_PTR( sn_win );
    }
    for ( int i=0; i<3; i++ ) {
	*sn_vec[i] = new QSNDict;
	Q_CHECK_PTR( *sn_vec[i] );
	(*sn_vec[i])->setAutoDelete( TRUE );
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

    QSNDict  *dict = *sn_vec[type];

    if ( !dict && QApplication::closingDown() )
	return FALSE; // after sn_cleanup, don't reinitialize.

    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( sn_win == 0 ) {
	    sn_init();
	    dict = *sn_vec[type];
	}
	sn = new QSockNot;
	Q_CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
#if defined(QT_CHECK_STATE)
	if ( dict->find(sockfd) ) {
	    static const char *t[] = { "read", "write", "exception" };
	    qWarning( "QSocketNotifier: Multiple socket notifiers for "
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
		      const QPixmap *pixmap, int xOffset, int yOffset )
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while( yPos < y + h ) {
	drawH = pixmap->height() - yOff;	// Cropping first row
	if ( yPos + drawH > y + h )		// Cropping last row
	    drawH = y + h - yPos;
	xPos = x;
	xOff = xOffset;
	while( xPos < x + w ) {
	    drawW = pixmap->width() - xOff;	// Cropping first column
	    if ( xPos + drawW > x + w )		// Cropping last column
		drawW = x + w - xPos;
	    BitBlt( hdc, xPos, yPos, drawW, drawH, pixmap->handle(),
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
    if ( qt_winver & Qt::WV_NT_based ) {
	// NT has no brush size limitation, so this is straight-forward
	// Note: Since multi cell pixmaps are not used under NT, we can
	// safely access the hbm() parameter of the pixmap.
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
    } else {
	// For Windows 9x, we must do everything ourselves.
	QPixmap *tile = 0;
	QPixmap *pm;
	int  sw = bg_pixmap->width(), sh = bg_pixmap->height();
	if ( sw*sh < 8192 && sw*sh < 16*w*h ) {
	    int tw = sw, th = sh;
	    while ( tw*th < 32678 && tw < w/2 )
		tw *= 2;
	    while ( tw*th < 32678 && th < h/2 )
		th *= 2;
	    tile = new QPixmap( tw, th, bg_pixmap->depth(),
				QPixmap::NormalOptim );
	    qt_fill_tile( tile, *bg_pixmap );
	    pm = tile;
	} else {
	    if ( bg_pixmap->isMultiCellPixmap() )
		off_y += bg_pixmap->multiCellOffset();
	    pm = (QPixmap*)bg_pixmap;
	}
	drawTile( hdc, x, y, w, h, pm, off_x, off_y );
	if ( tile )
	    delete tile;
    }
}



/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    quit_now = FALSE;
    quit_code = 0;
#if defined(QT_THREAD_SUPPORT)
    qt_gui_thread = GetCurrentThreadId();
    qApp->unlock();
#endif
    enter_loop();

    return quit_code;
}

static bool winPeekMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
		     UINT wMsgFilterMax, UINT wRemoveMsg )
{
    if ( qt_winver & Qt::WV_NT_based )
	return PeekMessage( msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
    else
	return PeekMessageA( msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg );
}

static bool winGetMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
		     UINT wMsgFilterMax )
{
    if ( qt_winver & Qt::WV_NT_based )
	return GetMessage( msg, hWnd, wMsgFilterMin, wMsgFilterMax );
    else
	return GetMessageA( msg, hWnd, wMsgFilterMin, wMsgFilterMax );
}

static bool winPostMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( qt_winver & Qt::WV_NT_based )
	return PostMessage( hWnd, msg, wParam, lParam );
    else
	return PostMessageA( hWnd, msg, wParam, lParam );
}

bool QApplication::processNextEvent( bool canWait )
{
    MSG	 msg;

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif
    emit guiThreadAwake();

    sendPostedEvents();
#if defined(QT_THREAD_SUPPORT)
    qApp->unlock( FALSE );
#endif

    if ( canWait ) {				// can wait if necessary
	if ( numZeroTimers ) {			// activate full-speed timers
	    int ok = FALSE;
	    while ( numZeroTimers &&
		!(ok=winPeekMessage(&msg,0,0,0,PM_REMOVE)) ) {
#if defined(QT_THREAD_SUPPORT)
		qApp->lock();
#endif
		activateZeroTimers();
#if defined(QT_THREAD_SUPPORT)
		qApp->unlock( FALSE );
#endif
	    }
	    if ( !ok )	{			// no event
		return FALSE;
	    }
	} else {
	    if ( !winGetMessage(&msg,0,0,0) ) {
#if defined(QT_THREAD_SUPPORT)
		qApp->lock();
#endif
		quit();				// WM_QUIT received
#if defined(QT_THREAD_SUPPORT)
		qApp->unlock( FALSE );
#endif
		return FALSE;
	    }
	}
    } else {					// no-wait mode
	if ( !winPeekMessage(&msg,0,0,0,PM_REMOVE) ) { // no pending events
	    if ( numZeroTimers > 0 ) {		// there are 0-timers
#if defined(QT_THREAD_SUPPORT)
		qApp->lock();
#endif
		activateZeroTimers();
#if defined(QT_THREAD_SUPPORT)
		qApp->unlock( FALSE );
#endif
	    }
	    return FALSE;
	}
    }

    if ( msg.message == WM_TIMER ) {		// timer message received
#if defined(QT_THREAD_SUPPORT)
	qApp->lock();
#endif
	const bool handled = dispatchTimer( msg.wParam, &msg );
#if defined(QT_THREAD_SUPPORT)
	qApp->unlock( FALSE );
#endif
	if ( handled )
	    return TRUE;
    }
    TranslateMessage( &msg );			// translate to WM_CHAR

#if defined(QT_THREAD_SUPPORT)
    qApp->lock();
#endif
    if ( qt_winver & Qt::WV_NT_based )
	DispatchMessage( &msg );		// send to QtWndProc
    else
	DispatchMessageA( &msg );		// send to QtWndProc
    if ( configRequests )			// any pending configs?
	qWinProcessConfigRequests();
    sendPostedEvents();
#if defined(QT_THREAD_SUPPORT)
    qApp->unlock( FALSE );
#endif

    return TRUE;
}


void QApplication::processEvents( int maxtime )
{
    uint ticks = (uint)GetTickCount();
    while ( !app_exit_loop && processNextEvent(FALSE) ) {
	if ( (uint)GetTickCount() - ticks > (uint)maxtime )
	    break;
    }
}

extern uint qGlobalPostedEventsCount();

bool QApplication::hasPendingEvents()
{
    return qGlobalPostedEventsCount() || winPeekMessage( NULL, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD );
}

void QApplication::wakeUpGuiThread()
{
#if defined(QT_THREAD_SUPPORT)
    PostThreadMessage( qt_gui_thread, WM_USER+666, 0, 0 );
#else
    winPostMessage( mainWidget()->winId(), WM_USER+666, 0, 0 );
#endif
}

bool QApplication::winEventFilter( MSG * )	// Windows event filter
{
    return FALSE;
}


void QApplication::winFocus( QWidget *widget, bool gotFocus )
{
    if ( inPopupMode() ) // some delayed focus event to ignore
	return;
    if ( gotFocus ) {
	setActiveWindow( widget );
	if ( active_window && active_window->testWFlags( WStyle_Dialog ) ) {
	    // raise the entire application, not just the dialog
	    QWidget* mw = active_window;
	    while( mw->parentWidget() && mw->testWFlags( WStyle_Dialog) )
		mw = mw->parentWidget()->topLevelWidget();
	    if ( mw != active_window )
		SetWindowPos( mw->winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
	}
    } else {
	setActiveWindow( 0 );
    }
}


//
// QtWndProc() receives all messages from the main event loop
//

static bool inLoop = FALSE;

#define RETURN(x) { inLoop=FALSE;return x; }

extern "C"
LRESULT CALLBACK QtWndProc( HWND hwnd, UINT message, WPARAM wParam,
			    LPARAM lParam )
{
    
    if ( inLoop ) {
	qApp->sendPostedEvents( 0, QEvent::ShowWindowRequest );
    }
    inLoop = TRUE;

    bool result = TRUE;
    QEvent::Type evt_type = QEvent::None;
    QETWidget *widget;

    if ( !qApp )				// unstable app state
	goto do_default;

    MSG msg;
    msg.hwnd = hwnd;				// create MSG structure
    msg.message = message;			// time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;

    if ( qt_winEventFilter(&msg) )		// send through app filter
	RETURN(0);

    switch ( message ) {
    case WM_QUERYENDSESSION: {
	if ( sm_smActive ) // bogus message from windows
	    RETURN(TRUE);

	sm_smActive = TRUE;
	sm_blockUserInput = TRUE; // prevent user-interaction outside interaction windows
	sm_cancel = FALSE;
	qApp->commitData( *win_session_manager );
	if ( lParam == (LPARAM)ENDSESSION_LOGOFF ) {
	    //### should call something like fsync() for all
	    //file descriptors being closed?
	}
	RETURN(!sm_cancel);
    }

    case WM_ENDSESSION: {
	sm_smActive = FALSE;
	sm_blockUserInput = FALSE;
	bool endsession = (bool) wParam;

	if ( endsession ) {
	    qApp->quit();
	}

	RETURN(0);
    }

    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
	if ( QApplication::desktopSettingsAware() )
	    qt_set_windows_resources();
	break;
    default:
	break;
    }

    widget = (QETWidget*)QWidget::find( hwnd );
    if ( !widget )				// don't know this widget
	goto do_default;

    if ( app_do_modal )	{			// modal event handling
	int ret = 0;
	if ( !qt_try_modal(widget, &msg, ret ) )
	    RETURN(ret);
    }

    if ( widget->winEvent(&msg) )		// send through widget filter
	RETURN(0);

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
	if ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST
	     && message != WM_MOUSEWHEEL ) {
	    if ( qApp->activePopupWidget() != 0) { // in popup mode
		POINT curPos;
		GetCursorPos( &curPos );

		QWidget* w = QApplication::widgetAt( curPos.x, curPos.y, TRUE );
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
		if ( w->focusPolicy() & QWidget::ClickFocus ) {
		    QFocusEvent::setReason( QFocusEvent::Mouse);
		    w->setFocus();
		    QFocusEvent::resetReason();
		}
	    }
	    widget->translateMouseEvent( msg );	// mouse event
	} else if ( message == WM95_MOUSEWHEEL ) {
	    result = widget->translateWheelEvent( msg );
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
		result = TRUE;			// consume event
		break;

	    case WM_MOUSEWHEEL:
		result = widget->translateWheelEvent( msg );
		break;

	    case WM_NCMOUSEMOVE:
		{
		    // span the application wide cursor over the
		    // non-client area.
		    QCursor *c = qt_grab_cursor();
		    if ( !c )
			c = QApplication::overrideCursor();
		    if ( c )	// application cursor defined
			SetCursor( c->handle() );
		    else
			result = FALSE;
		    // generate leave event also when the caret enters
		    // the non-client area.
		    qt_dispatchEnterLeave( 0, QWidget::find(curWin) );
		    curWin = 0;
		}
		break;

	    case WM_SYSCOMMAND:
		if ( wParam == SC_CONTEXTHELP ) {
		    // What's This? Windows wants to do something for
		    // us....naaa
		    QWhatsThis::enterWhatsThisMode();
		    if ( qt_winver & Qt::WV_NT_based )
			DefWindowProc( hwnd, WM_NCPAINT, 1, 0 );
		    else
			DefWindowProcA( hwnd, WM_NCPAINT, 1, 0 );
		} else
		    result = FALSE;
		break;

	    case WM_PAINT:				// paint event
		result = widget->translatePaintEvent( msg );
		break;

	    case WM_ERASEBKGND:			// erase window background
		{
		    int ox = 0;
		    int oy = 0;
		    RECT r;
		    if ( !widget->isTopLevel() ) {
			if ( widget->backgroundOrigin() == QWidget::ParentOrigin ) {
			    ox = widget->x();
			    oy = widget->y();
			} else if ( widget->backgroundOrigin() == QWidget::WindowOrigin ) {
			    QPoint p = widget->mapTo( widget->topLevelWidget(), QPoint(0,0) );
			    ox = p.x();
			    oy = p.y();
			}
		    }
		    GetClientRect( hwnd, &r );
		    qt_erase_background
			( (HDC)wParam, r.left, r.top,
			  r.right-r.left, r.bottom-r.top,
			  widget->backgroundColor(),
			  widget->backgroundPixmap(), ox, oy );
		    RETURN(TRUE);
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
		    while ( (popup=QApplication::activePopupWidget()) &&
			    maxiter-- )
			popup->hide();
		}
		qApp->winFocus( widget, LOWORD(wParam) == WA_INACTIVE ? 0 : 1 );
		break;

	    case WM_PALETTECHANGED:			// our window changed palette
		if ( QColor::hPal() && (WId)wParam == widget->winId() )
		    RETURN(0);			// otherwise: FALL THROUGH!
		// FALL THROUGH
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
		    RETURN(n);
		}
		break;

	    case WM_CLOSE:				// close window
		widget->translateCloseEvent( msg );
		RETURN(0);				// always handled

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
		    if ( x->maxw < QWIDGETSIZE_MAX )
			mmi->ptMaxTrackSize.x = x->maxw + f.width()	 - s.width();
		    if ( x->maxh < QWIDGETSIZE_MAX )
			mmi->ptMaxTrackSize.y = x->maxh + f.height() - s.height();
		    RETURN(0);
		}
		break;

	    case WM_CONTEXTMENU:
		{
		    // it's not VK_APPS or Shift+F10, but a click in the NC area
		    if ( lParam != 0xffffffff ) {
			result = FALSE;
			break;
		    }
		    QWidget *fw = qApp->focusWidget();
		    if ( fw ) {
			QContextMenuEvent e( QContextMenuEvent::Keyboard, QPoint( 5, 5 ), fw->mapToGlobal( QPoint( 5, 5 ) ) );
			result = QApplication::sendEvent( fw, &e );
		    }
		}
		break;

	    case WM_CHANGECBCHAIN:
	    case WM_DRAWCLIPBOARD:
	    case WM_RENDERFORMAT:
	    case WM_RENDERALLFORMATS:
		if ( qt_clipboard ) {
		    QCustomEvent e( QEvent::Clipboard, &msg );
		    QApplication::sendEvent( qt_clipboard, &e );
		    RETURN(0);
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
    if ( result )
	RETURN(FALSE);

do_default:
    if ( qt_winver & Qt::WV_NT_based )
	RETURN( DefWindowProc(hwnd,message,wParam,lParam) )
    else
	RETURN( DefWindowProcA(hwnd,message,wParam,lParam) )
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
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	Q_CHECK_PTR( qt_modal_stack );
    }
    releaseAutoCapture();
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
    qt_dispatchEnterLeave( 0, QWidget::find( (WId)curWin  ) ); // send synthetic leave event
    curWin = 0;
}


void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	    QPoint p( QCursor::pos() );
	    app_do_modal = FALSE; // necessary, we may get recursively into qt_try_modal below
	    QWidget* w = QApplication::widgetAt( p.x(), p.y(), TRUE );
	    qt_dispatchEnterLeave( w, QWidget::find( curWin ) ); // send synthetic enter event
	    curWin = w? w->winId() : 0;
	}
    }
    app_do_modal = qt_modal_stack != 0;
}

static bool qt_blocked_modal( QWidget *widget )
{
    if ( !app_do_modal )
	return FALSE;
    if ( qApp->activePopupWidget() )
	return FALSE;
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return FALSE;

    QWidget *modal=0, *top=qt_modal_stack->getFirst();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )				// don't block event
	return FALSE;
    return TRUE;
}

static bool qt_try_modal( QWidget *widget, MSG *msg, int& ret )
{
    if ( qApp->activePopupWidget() )
	return TRUE;
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=QApplication::activeModalWidget();
    int	 type  = msg->message;

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

    bool block_event = FALSE;
    if ( type == WM_NCHITTEST ) {
      //block_event = TRUE;
	// QApplication::beep();
    } else if ( (type >= WM_MOUSEFIRST && type <= WM_MOUSELAST) ||
	 (type >= WM_KEYFIRST	&& type <= WM_KEYLAST) || type == WM_NCMOUSEMOVE ) {
      if ( type == WM_MOUSEMOVE || type == WM_NCMOUSEMOVE )
	  SetCursor( Qt::arrowCursor.handle() );
      block_event = TRUE;
    } else if ( type == WM_MOUSEACTIVATE ){
      if ( !top->isActiveWindow() )
	top->setActiveWindow();
      else
	QApplication::beep();
      block_event = TRUE;
      ret = MA_NOACTIVATEANDEAT;
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
    if ( popupWidgets->count() == 1 && !qt_nograb() )
	setAutoCapture( popup->winId() );	// grab mouse/keyboard
    // Popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason( QFocusEvent::Popup );
    if ( popup->focusWidget())
	popup->focusWidget()->setFocus();
    else
	popup->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;
    popupWidgets->removeRef( popup );
    POINT curPos;
    GetCursorPos( &curPos );
    replayPopupMouseEvent = !popup->geometry().contains( QPoint(curPos.x, curPos.y) );
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() )			// grabbing not disabled
	    releaseAutoCapture();
	active_window = (*activeBeforePopup);	// windows does not have
	// A reasonable focus handling for our popups => we have
	// to restore the focus manually.
	if ( active_window ) {
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	    QFocusEvent::resetReason();
	}
    } else {
	// Popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	QFocusEvent::setReason( QFocusEvent::Popup );
	QWidget* aw = popupWidgets->getLast();
	if (aw->focusWidget())
	    aw->focusWidget()->setFocus();
	else
	    aw->setFocus();
	QFocusEvent::resetReason();
    }
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

static bool dispatchTimer( uint timerId, MSG *msg )
{
#if defined(USE_HEARTBEAT)
    if ( timerId != (WPARAM)heartBeat ) {
	if ( !msg || !qApp || !qt_winEventFilter(msg) )
	    return activateTimer( timerId );
    } else if ( curWin && qApp ) {		// process heartbeat
	POINT p;
	GetCursorPos( &p );
	HWND newWin = WindowFromPoint(p);
	if ( newWin != curWin && QWidget::find(newWin) == 0 ) {
	    qt_dispatchEnterLeave( 0, QWidget::find(curWin) );
	    curWin = 0;
	}
    }
#else
    if ( !msg || !qApp || !qt_winEventFilter(msg) )
	return activateTimer( timerId );
#endif
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



// In DnD, the mouse release event never appears, so the
// mouse button state machine must be manually reset
void QApplication::winMouseButtonUp()
{
    qt_button_down = 0;
    releaseAutoCapture();
}

bool QETWidget::translateMouseEvent( const MSG &msg )
{
    static QPoint pos;
    static POINT gpos={-1,-1};
    QEvent::Type type;				// event parameters
    int	   button;
    int	   state;
    int	   i;

    if ( sm_blockUserInput ) //block user interaction during session management
	return TRUE;

    for ( i=0; (UINT)mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3 )
	;
    if ( !mouseTbl[i] )
	return FALSE;
    type   = (QEvent::Type)mouseTbl[++i];	// event type
    button = mouseTbl[++i];			// which button
    state  = translateButtonState( msg.wParam, type, button ); // button state

    if ( type == QEvent::MouseMove ) {
	if ( !(state & (LeftButton | MidButton | RightButton) ) )
	    qt_button_down = 0;
	QCursor *c = qt_grab_cursor();
	if ( !c )
	    c = QApplication::overrideCursor();
	if ( c )				// application cursor defined
	    SetCursor( c->handle() );
	else					// use widget cursor
	    SetCursor( cursor().handle() );
	if ( curWin != winId() ) {		// new current window
	    qt_dispatchEnterLeave( this, QWidget::find(curWin) );
	    curWin = winId();
	}

	POINT curPos;
	DWORD ol_pos = GetMessagePos();
	curPos.x = GET_X_LPARAM(ol_pos);
	curPos.y = GET_Y_LPARAM(ol_pos);

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
	DWORD ol_pos = GetMessagePos();
	gpos.x = GET_X_LPARAM(ol_pos);
	gpos.y = GET_Y_LPARAM(ol_pos);

	pos = mapFromGlobal( QPoint(gpos.x, gpos.y) );

	if ( type == QEvent::MouseButtonPress ) {	// mouse button pressed
	    // Magic for masked widgets
	    qt_button_down = findChildWidget( this, pos );
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
		pos = popup->mapFromGlobal( QPoint(gpos.x, gpos.y) );
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
	} else if ( popupChild ){
	    QMouseEvent e( type,
		popupChild->mapFromGlobal(QPoint(gpos.x,gpos.y)),
		QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendEvent( popupChild, &e );
	} else {
	    QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendEvent( popupChild ? popupChild : popup, &e );
	}

	if ( releaseAfter )
	    qt_button_down = 0;

	if ( type == QEvent::MouseButtonPress
	     && qApp->activePopupWidget() != activePopupWidget
	     && replayPopupMouseEvent ) {
	    // the popup dissappeared. Replay the event
	    QWidget* w = QApplication::widgetAt( gpos.x, gpos.y, TRUE );
	    if (w && w->rect().contains( gpos.x, gpos.y ) && !qt_blocked_modal( w ) ) {
		if ( w->isEnabled() ) {
		    QWidget* tw = w;
		    while ( tw->focusProxy() )
			tw = tw->focusProxy();
		    if ( tw->focusPolicy() & QWidget::ClickFocus ) {
			QFocusEvent::setReason( QFocusEvent::Mouse);
			w->setFocus();
			QFocusEvent::resetReason();
		    }
		}
		if ( QWidget::mouseGrabber() == 0 )
		    setAutoCapture( w->winId() );
		winPostMessage( w->winId(), msg.message, msg.wParam, msg.lParam );
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

	QWidget *widget = this;
	QWidget *w = QWidget::mouseGrabber();
	if ( !w )
	    w = qt_button_down;
	if ( w && w != this ) {
	    widget = w;
	    pos = w->mapFromGlobal(QPoint(gpos.x, gpos.y));
	}

	if ( type == QEvent::MouseButtonRelease &&
	     (state & (~button) & ( LeftButton |
				    MidButton |
				    RightButton)) == 0 ) {
	    qt_button_down = 0;
	}

	if ( type == QEvent::MouseButtonRelease && button == RightButton && ( ( state & (~button) ) == 0 ) ) {
	    QContextMenuEvent e( QContextMenuEvent::Mouse, pos, QPoint(gpos.x,gpos.y) );
	    QApplication::sendEvent( widget, &e );
	    if ( !e.isAccepted() ) { // Only send mouse event when context event has not been processed
		QMouseEvent e2( type, pos, QPoint(gpos.x,gpos.y), button, state );
		QApplication::sendEvent( widget, &e2 );
	    }
	} else {
	    QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendEvent( widget, &e );
	}

	if ( type != QEvent::MouseMove )
	    pos.rx() = pos.ry() = -9999;	// init for move compression
    }
    return TRUE;
}


//
// Keyboard event translation
//

static const ushort KeyTbl[] = {		// keyboard mapping table
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
#if defined(QT_CHECK_RANGE)
	qWarning( "Qt: Internal keyboard buffer overflow" );
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

static
QChar wmchar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.

    if ( qt_winver & Qt::WV_NT_based ) {
	ushort uc = (ushort)c;
	return QChar(uc&0xff,(uc>>8)&0xff);
    } else {
	char mb[2];
	mb[0] = c&0xff;
	mb[1] = 0;
	WCHAR wc[1];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
	    mb, -1, wc, 1);
	return QChar(wc[0]);
    }
}

static
QChar imechar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.

    if ( qt_winver & Qt::WV_NT_based ) {
	ushort uc = (ushort)c;
	return QChar(uc&0xff,(uc>>8)&0xff);
    } else {
	char mb[3];
	mb[0] = (c>>8)&0xff;
	mb[1] = c&0xff;
	mb[2] = 0;
	WCHAR wc[1];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
	    mb, -1, wc, 1);
	return QChar(wc[0]);
    }
}

bool QETWidget::translateKeyEvent( const MSG &msg, bool grab )
{
    bool k0=FALSE, k1=FALSE;
    int  state = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= Qt::ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= Qt::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= Qt::AltButton;
    //TODO: if it is a pure shift/ctrl/alt keydown, invert state logic, like X
    if ( msg.lParam & 0xc0000000 ) {
	state |= Qt::Keypad;
	//qDebug(  "YES! %x", msg.lParam );
    } else {
	//qDebug(  "NO! %x", msg.lParam );
    }

    if ( msg.message == WM_CHAR ) {
	// a multi-character key not found by our look-ahead
	QString s;
	QChar ch = wmchar_to_unicode(msg.wParam);
	if (!ch.isNull())
	    s += ch;
	k0 = sendKeyEvent( QEvent::KeyPress, 0, msg.wParam, state, grab, s );
	k1 = sendKeyEvent( QEvent::KeyRelease, 0, msg.wParam, state, grab, s );
    }
    else if ( msg.message == WM_IME_CHAR ) {
	// input method characters not found by our look-ahead
	QString s;
	QChar ch = imechar_to_unicode(msg.wParam);
	if (!ch.isNull())
	    s += ch; 
	k0 = sendKeyEvent( QEvent::KeyPress, 0, ch.row() ? 0 : ch.cell(), state, grab, s );
	k1 = sendKeyEvent( QEvent::KeyRelease, 0, ch.row() ? 0 : ch.cell(), state, grab, s );
    }
    else {
	int code = translateKeyCode( msg.wParam );
	// If the bit 24 of lParm is set you received a enter,
	// otherwise a Return. (This is the extended key bit)
	if ((code == Qt::Key_Return) && (msg.lParam & 0x1000000)) {
	    code = Qt::Key_Enter;
	}

	if ( !(msg.lParam & 0x1000000) ) {	// All cursor keys without extended bit
	    switch ( code ) {
	    case Key_Left:
	    case Key_Right:
	    case Key_Up:
	    case Key_Down:
	    case Key_PageUp:
	    case Key_PageDown:
	    case Key_Home:
	    case Key_End:
	    case Key_Insert:
	    case Key_Delete:
	    case Key_Asterisk:
	    case Key_Plus:
	    case Key_Minus:
	    case Key_Period:
	    case Key_0:
	    case Key_1:
	    case Key_2:
	    case Key_3:
	    case Key_4:
	    case Key_5:
	    case Key_6:
	    case Key_7:
	    case Key_8:
	    case Key_9:
		state |= Keypad;
	    default:
		if ( (uint)msg.lParam == 0x004c0001 ||
		     (uint)msg.lParam == 0xc04c0001 )
		    state |= Keypad;
		break;
	    }
	} else {				// And some with extended bit
	    switch ( code ) {
	    case Key_Enter:
	    case Key_Slash:
	    case Key_NumLock:
		state |= Keypad;
	    default:
		break;
	    }
	}

	int t = msg.message;
	if ( t == WM_KEYDOWN || t == WM_IME_KEYDOWN || t == WM_SYSKEYDOWN ) {
	    // KEYDOWN
	    KeyRec* rec = find_key_rec( msg.wParam, FALSE );
	    // Find uch
	    QChar uch;
	    MSG wm_char;
	    UINT charType = ( t == WM_KEYDOWN ? WM_CHAR :
			      t == WM_IME_KEYDOWN ? WM_IME_CHAR : WM_SYSCHAR );
	    if ( winPeekMessage(&wm_char, 0, charType, charType, PM_REMOVE) ) {
		// Found a XXX_CHAR
		uch = charType == WM_IME_CHAR
			? imechar_to_unicode(wm_char.wParam)
			: wmchar_to_unicode(wm_char.wParam);
		if ( t == WM_SYSKEYDOWN &&
		     uch.isLetter() && (msg.lParam & KF_ALTDOWN) ) {
		    // (See doc of WM_SYSCHAR)
		    uch = uch.lower(); //Alt-letter
		}
		if ( !code && !uch.row() )
		    code = asciiToKeycode(uch.cell(), state);
	    } else {
		// No XXX_CHAR; deduce uch from XXX_KEYDOWN params
		if ( msg.wParam == VK_DELETE )
		    uch = QChar((char)0x7f); // Windows doesn't know this one.
		else {
		    if (t != WM_SYSKEYDOWN) {
			UINT map;
			if ( qt_winver & Qt::WV_NT_based ) {
			    map = MapVirtualKey( msg.wParam, 2 );
			} else {
			    map = MapVirtualKeyA( msg.wParam, 2 );
			    // High-order bit is 0x8000 on '95
			    if ( map & 0x8000 )
				map = (map^0x8000)|0x80000000;
			}
			// If the high bit of the return value of
			// MapVirtualKey is set, the key is a deadkey.
			if ( !(map & 0x80000000) ) {
			    uch = wmchar_to_unicode((DWORD)map);
			}
		    }
		}
		if ( !code && !uch.row() )
		    code = asciiToKeycode( uch.cell(), state);
	    }

	    if ( state == QMouseEvent::AltButton ) {
		// Special handling of global Windows hotkeys
		switch ( code ) {
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Enter:
		case Qt::Key_F4:
		    return FALSE;		// Send the event on to Windows
		case Qt::Key_Space:
		    // do not pass this key to windows, we will process it ourselves
		    qt_show_system_menu( topLevelWidget() );
		    return TRUE;
		default:
		    break;
		}
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
		if ( !uch.isNull() )
		    text += uch;
		char a = uch.row() ? 0 : uch.cell();
		store_key_rec( msg.wParam, a, text );
		k0 = sendKeyEvent( QEvent::KeyPress, code, a,
				   state, grab, text );
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
		if ( code == Qt::Key_Alt )
		    k0 = TRUE; // don't let window see the meta key
	    }
	}
    }

    return k0 || k1;
}


bool QETWidget::translateWheelEvent( const MSG &msg )
{
    int  state = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= QMouseEvent::ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= QMouseEvent::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= QMouseEvent::AltButton;

    int delta;
    if ( msg.message == WM_MOUSEWHEEL )
	delta = (short) HIWORD ( msg.wParam );
    else
	delta = (int) msg.wParam;

    QPoint globalPos;

    globalPos.rx() = LOWORD ( msg.lParam );
    globalPos.ry() = HIWORD ( msg.lParam );

    QWidget* w = QApplication::widgetAt( globalPos, TRUE );
    if ( !w)
	w = this;

    while ( w->focusProxy() )
	w = w->focusProxy();
    if ( w->focusPolicy() == QWidget::WheelFocus ) {
	QFocusEvent::setReason( QFocusEvent::Mouse);
	w->setFocus();
	QFocusEvent::resetReason();
    }

    // send the event to the widget or its ancestors
    {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w != popup )
	    popup->hide();
	QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state );
	if ( QApplication::sendEvent( w, &e ) )
	    return TRUE;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    if ( w != qApp->focusWidget() && ( w = qApp->focusWidget() ) ) {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w != popup )
	    popup->hide();
	QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state );
	if ( QApplication::sendEvent( w, &e ) )
	    return TRUE;
    }
    return FALSE;
}


static bool isModifierKey(int code)
{
    return code >= Qt::Key_Shift && code <= Qt::Key_ScrollLock;
}

bool QETWidget::sendKeyEvent( QEvent::Type type, int code, int ascii,
			      int state, bool grab, const QString& text,
			      bool autor )
{
    if ( type == QEvent::KeyPress && !grab ) {	// send accel events
	QKeyEvent aa( QEvent::AccelOverride, code, ascii, state, text, autor );
	aa.ignore();
	QApplication::sendEvent( this, &aa );
	if ( !aa.isAccepted() ) {
	    QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor );
	    a.ignore();
	    QApplication::sendEvent( topLevelWidget(), &a );
	    if ( a.isAccepted() )
		return TRUE;
	}
    }
    if ( !isEnabled() )
	return FALSE;
    QKeyEvent e( type, code, ascii, state, text, autor );
    QApplication::sendEvent( this, &e );
    if ( !isModifierKey(code) && state == QMouseEvent::AltButton
	 && ((code>=Key_A && code<=Key_Z) || (code>=Key_0 && code<=Key_9))
	 && type == QEvent::KeyPress && !e.isAccepted() )
	QApplication::beep();  // emulate windows behavioar
    return e.isAccepted();
}


//
// Paint event translation
//
bool QETWidget::translatePaintEvent( const MSG & )
{
    PAINTSTRUCT ps;
    QRegion rgn(0,0,1,1); // trigger handle
    int res = GetUpdateRgn(winId(), (HRGN) rgn.handle(), FALSE);
    setWState( WState_InPaintEvent );
    hdc = BeginPaint( winId(), &ps );
    if ( res != COMPLEXREGION )
	rgn = QRect(QPoint(ps.rcPaint.left,ps.rcPaint.top),
		    QPoint(ps.rcPaint.right-1,ps.rcPaint.bottom-1));
    QPaintEvent e(rgn);
    QApplication::sendEvent( this, (QEvent*) &e );
    hdc = 0;
    EndPaint( winId(), &ps );
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
    QRect cr = geometry();
    if ( msg.message == WM_SIZE ) {		// resize event
	WORD a = LOWORD(msg.lParam);
	WORD b = HIWORD(msg.lParam);
	QSize oldSize = size();
	QSize newSize( a, b );
	cr.setSize( newSize );
	if ( msg.wParam != SIZE_MINIMIZED )
	    crect = cr;
	if ( isTopLevel() ) {			// update caption/icon text
	    createTLExtra();
	    if ( msg.wParam == SIZE_MINIMIZED ) {
		// being "hidden"
		extra->topextra->iconic = 1;
		if ( isVisible() ) {
		    clearWState( WState_Visible );
		    QHideEvent e( TRUE );
		    QApplication::sendEvent( this, &e );
		    sendHideEventsToChildren( TRUE );
		}
	    } else if ( extra->topextra->iconic ) {
		// being shown
		extra->topextra->iconic = 0;
		if ( !isVisible() ) {
		    setWState( WState_Visible );
		    clearWState( WState_ForceHide );
		    sendShowEventsToChildren( TRUE );
		    QShowEvent e( TRUE );
		    QApplication::sendEvent( this, &e );
		}
	    }
	    QString txt;
	    if ( IsIconic(winId()) && !!iconText() )
		txt = iconText();
	    else if ( !caption().isNull() )
		txt = caption();

	    if ( !!txt ) {
		if ( qt_winver & Qt::WV_NT_based )
		    SetWindowText( winId(), (TCHAR*)qt_winTchar(txt,TRUE) );
		else
		    SetWindowTextA( winId(), txt.local8Bit() );
	    }
	}
	if ( msg.wParam != SIZE_MINIMIZED ) {
	    if ( isVisible() ) {
		QResizeEvent e( newSize, oldSize );
		QApplication::sendEvent( this, &e );
	    } else {
		QResizeEvent *e = new QResizeEvent( newSize, oldSize );
		QApplication::postEvent( this, e );
	    }
	}
	if ( !testWFlags( WNorthWestGravity ) )
	    repaint( visibleRect(), !testWFlags(WResizeNoErase) );
    } else if ( msg.message == WM_MOVE ) {	// move event
	int a = (int) (short) LOWORD(msg.lParam);
	int b = (int) (short) HIWORD(msg.lParam);
	QPoint oldPos = geometry().topLeft();
	// Ignore silly Windows move event to wild pos after iconify.
	if ( !isMinimized() ) {
	    QPoint newCPos( a, b );
	    cr.moveTopLeft( newCPos );
	    crect = cr;
	    if ( isVisible() ) {
		QMoveEvent e( newCPos, oldPos );  // cpos (client position)
		QApplication::sendEvent( this, &e );
	    } else {
		QMoveEvent * e = new QMoveEvent( newCPos, oldPos );
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

void  QApplication::setCursorFlashTime( int msecs )
{
    SetCaretBlinkTime( msecs / 2 );
    cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    int blink = GetCaretBlinkTime();
    if ( blink != 0 )
	return 2*blink;
    return cursor_flash_time;
}

void QApplication::setDoubleClickInterval( int ms )
{
    SetDoubleClickTime( ms );
    mouse_double_click_time = ms;
}


int QApplication::doubleClickInterval()
{
    int ms = GetDoubleClickTime();
    if ( ms != 0 )
	return ms;
    return mouse_double_click_time;
}

void QApplication::setWheelScrollLines( int n )
{
#ifdef SPI_SETWHEELSCROLLLINES
    if ( n < 0 )
	n = 0;
    SystemParametersInfo( SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0 );
#else
    wheel_scroll_lines = n;
#endif
}

int QApplication::wheelScrollLines()
{
#ifdef SPI_GETWHEELSCROLLLINES
    uint i = 3;
    SystemParametersInfo( SPI_GETWHEELSCROLLLINES, sizeof( uint ), &i, 0 );
    if ( i > INT_MAX )
	i = INT_MAX;
    return i;
#else
    return wheel_scroll_lines;
#endif
}

void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    switch (effect) {
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeMenu:
	fade_menu = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_FadeTooltip:
	fade_tooltip = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }

    if ( desktopSettingsAware() && ( qt_winver == WV_98 || qt_winver == WV_2000 || qt_winver == Qt::WV_XP ) ) {
	// we know that they can be used when we are here
	UINT api;
	switch (effect) {
	case UI_AnimateMenu:
	    api = SPI_SETMENUANIMATION;
	    break;
	case UI_FadeMenu:
	    if ( qt_winver == WV_98 )
		return;
	    api = SPI_SETMENUFADE;
	    break;
	case UI_AnimateCombo:
	    api = SPI_SETCOMBOBOXANIMATION;
	    break;
	case UI_AnimateTooltip:
	    api = SPI_SETTOOLTIPANIMATION;
	    break;
	case UI_FadeTooltip:
	    if ( qt_winver == WV_98 )
		return;
	    api = SPI_SETTOOLTIPFADE;
	    break;
	default:
	   api = SPI_SETUIEFFECTS;
	break;
	}
	BOOL onoff = enable;
	if ( qt_winver & WV_NT_based )
	    SystemParametersInfo( api, 0, &onoff, 0 );
	else
	    SystemParametersInfoA( api, 0, &onoff, 0 );
    }
}

bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( desktopSettingsAware() && ( qt_winver == WV_98 || qt_winver == WV_2000 || qt_winver == Qt::WV_XP ) ) {
    // we know that they can be used when we are here
	BOOL enabled = FALSE;
	UINT api;
	switch (effect) {
	case UI_AnimateMenu:
	    api = SPI_GETMENUANIMATION;
	    break;
	case UI_FadeMenu:
	    if ( qt_winver == WV_98 )
		return FALSE;
	    api = SPI_GETMENUFADE;
	    break;
	case UI_AnimateCombo:
	    api = SPI_GETCOMBOBOXANIMATION;
	    break;
	case UI_AnimateTooltip:
	    if ( qt_winver == WV_98 )
		api = SPI_GETMENUANIMATION;
	    else
		api = SPI_GETTOOLTIPANIMATION;
	    break;
	case UI_FadeTooltip:
	    if ( qt_winver == WV_98 )
		return FALSE;
	    api = SPI_GETTOOLTIPFADE;
	    break;
	default:
	    api = SPI_GETUIEFFECTS;
	    break;
	}
	if ( qt_winver & WV_NT_based )
	    SystemParametersInfo( api, 0, &enabled, 0 );
	else
	    SystemParametersInfoA( api, 0, &enabled, 0 );
	return enabled;
    } else {
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
}

/*****************************************************************************
  Poor man's session management support on MS-Windows
 *****************************************************************************/


class QSessionManager::Data
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication *app, QString &session )
    : QObject( app, "session manager")
{
    d = new Data;
    d->sessionId = session;
    win_session_manager = this;
}

QSessionManager::~QSessionManager()
{
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

bool QSessionManager::allowsInteraction()
{
    sm_interactionActive = TRUE;
    sm_blockUserInput = FALSE;
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    sm_interactionActive = TRUE;
    return TRUE;
}

void QSessionManager::release()
{
    sm_interactionActive = FALSE;
    if ( sm_smActive )
	sm_blockUserInput = TRUE;
}

void QSessionManager::cancel()
{
    sm_cancel = TRUE;
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
