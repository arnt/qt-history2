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

#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qguardedptr.h"
#include "qhash.h"
#include "qlibrary.h"
#include "qmetaobject.h"
#include "qmime.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qsessionmanager.h"
#include "qstyle.h"
#include "qwhatsthis.h" // ######## dependency
#include "qwidget.h"
#include "qt_windows.h"
#if defined(QT_NON_COMMERCIAL)
#include "qnc_win.h"
#endif

#ifdef QT_THREAD_SUPPORT
#include "qthread.h"
#include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#include "private/qapplication_p.h"
#include "private/qinternal_p.h"
#include "private/qinputcontext_p.h"

#include <windowsx.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef Q_OS_TEMP
#include <sipapi.h>
#endif

#if defined(QT_TABLET_SUPPORT)
#define PACKETDATA  ( PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | \
		      PK_ORIENTATION | PK_CURSOR )
#define PACKETMODE  0

extern bool chokeMouse;
// Wacom isn't being nice by not providing updated headers for this. so
// define it here so we can do things correctly according to their
// software implementation guide, and make it somewhat standard.
#if !defined(CSR_TYPE)
#define CSR_TYPE ( (UINT) 20 )
#endif

#include <wintab.h>
#include <pktdef.h>
#include <math.h>

typedef HCTX	( API *PtrWTOpen )(HWND, LPLOGCONTEXT, BOOL);
typedef BOOL	( API *PtrWTClose )(HCTX);
typedef UINT	( API *PtrWTInfo )(UINT, UINT, LPVOID);
typedef BOOL	( API *PtrWTEnable )(HCTX, BOOL);
typedef BOOL	( API *PtrWTOverlap )(HCTX, BOOL);
typedef int	( API *PtrWTPacketsGet )(HCTX, int, LPVOID);
typedef BOOL	( API *PtrWTGet )(HCTX, LPLOGCONTEXT);
typedef int     ( API *PtrWTQueueSizeGet )(HCTX);
typedef BOOL    ( API *PtrWTQueueSizeSet )(HCTX, int);

static PtrWTInfo	 ptrWTInfo = 0;
static PtrWTEnable	 ptrWTEnable = 0;
static PtrWTOverlap	 ptrWTOverlap = 0;
static PtrWTPacketsGet	 ptrWTPacketsGet = 0;
static PtrWTGet		 ptrWTGet = 0;

static const double PI = 3.14159265359;

static PACKET localPacketBuf[QT_TABLET_NPACKETQSIZE];  // our own tablet packet queue.
HCTX qt_tablet_context;  // the hardware context for the tablet ( like a window handle )
bool qt_tablet_tilt_support;
static void prsInit(HCTX hTab);
static UINT prsAdjust(PACKET p, HCTX hTab);
static void initWinTabFunctions();	// resolve the WINTAB api functions
#endif

extern bool winPeekMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
			    UINT wMsgFilterMax, UINT wRemoveMsg );
extern bool winPostMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

#if defined(__CYGWIN32__)
#define __INSIDE_CYGWIN32__
#include <mywinsock.h>
#endif

#if defined(QT_ACCESSIBILITY_SUPPORT)
#include <qaccessible.h>
#if defined(Q_CC_GNU)
#include <winuser.h>
#else
#include <winable.h>
#endif
#include <oleacc.h>
#ifndef WM_GETOBJECT
#define WM_GETOBJECT                    0x003D
#endif

extern IAccessible *qt_createWindowsAccessible( QAccessibleInterface *object );
#endif // QT_ACCESSIBILITY_SUPPORT

// support for on-the-fly changes of the XP theme engine
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif
#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT		29
#define COLOR_MENUBAR			30
#endif

// support for xbuttons
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C
#define WM_XBUTTONDBLCLK                0x020D
#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))
#define GET_XBUTTON_WPARAM(wParam)      (HIWORD(wParam))
#define XBUTTON1      0x0001
#define XBUTTON2      0x0002
#define MK_XBUTTON1         0x0020
#define MK_XBUTTON2         0x0040
#endif

// support for multi-media-keys on ME/2000/XP
#ifndef WM_APPCOMMAND
#define WM_APPCOMMAND                   0x0319

#define FAPPCOMMAND_MOUSE 0x8000
#define FAPPCOMMAND_KEY   0
#define FAPPCOMMAND_OEM   0x1000
#define FAPPCOMMAND_MASK  0xF000
#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
#define GET_DEVICE_LPARAM(lParam)     ((WORD)(HIWORD(lParam) & FAPPCOMMAND_MASK))
#define GET_MOUSEORKEY_LPARAM         GET_DEVICE_LPARAM
#define GET_FLAGS_LPARAM(lParam)      (LOWORD(lParam))
#define GET_KEYSTATE_LPARAM(lParam)   GET_FLAGS_LPARAM(lParam)

#define APPCOMMAND_BROWSER_BACKWARD       1
#define APPCOMMAND_BROWSER_FORWARD        2
#define APPCOMMAND_BROWSER_REFRESH        3
#define APPCOMMAND_BROWSER_STOP           4
#define APPCOMMAND_BROWSER_SEARCH         5
#define APPCOMMAND_BROWSER_FAVORITES      6
#define APPCOMMAND_BROWSER_HOME           7
#define APPCOMMAND_VOLUME_MUTE            8
#define APPCOMMAND_VOLUME_DOWN            9
#define APPCOMMAND_VOLUME_UP              10
#define APPCOMMAND_MEDIA_NEXTTRACK        11
#define APPCOMMAND_MEDIA_PREVIOUSTRACK    12
#define APPCOMMAND_MEDIA_STOP             13
#define APPCOMMAND_MEDIA_PLAY_PAUSE       14
#define APPCOMMAND_LAUNCH_MAIL            15
#define APPCOMMAND_LAUNCH_MEDIA_SELECT    16
#define APPCOMMAND_LAUNCH_APP1            17
#define APPCOMMAND_LAUNCH_APP2            18
#define APPCOMMAND_BASS_DOWN              19
#define APPCOMMAND_BASS_BOOST             20
#define APPCOMMAND_BASS_UP                21
#define APPCOMMAND_TREBLE_DOWN            22
#define APPCOMMAND_TREBLE_UP              23
#endif

static UINT WM95_MOUSEWHEEL = 0;

#if(_WIN32_WINNT < 0x0400)
// This struct is defined in winuser.h if the _WIN32_WINNT >= 0x0400 -- in the
// other cases we have to define it on our own.
typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
    HWND  hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;
#endif
#ifndef WM_MOUSELEAVE
#define WM_MOUSELEAVE                   0x02A3
#endif

#include "private/qwidget_p.h"

extern void qt_dispatchEnterLeave( QWidget*, QWidget* ); // qapplication.cpp
static int translateButtonState( int s, int type, int button );

/*
  Internal functions.
*/

Q_EXPORT
void qt_draw_tiled_pixmap( HDC, int, int, int, int,
			   const QPixmap *, int, int );

void qt_erase_background( HDC hdc, int x, int y, int w, int h,
			  const QBrush &brush, int off_x, int off_y )
{
    if ( brush.pixmap() && brush.pixmap()->isNull() )	// empty background
	return;
    HPALETTE oldPal = 0;
    if ( QColor::hPal() ) {
	oldPal = SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    }
    if ( brush.pixmap() ) {
	qt_draw_tiled_pixmap( hdc, x, y, w, h, brush.pixmap(), off_x, off_y );
    } else {
	HBRUSH hbrush = CreateSolidBrush( brush.color().pixel() );
	HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, hbrush );
	PatBlt( hdc, x, y, w, h, PATCOPY );
	SelectObject( hdc, oldBrush );
	DeleteObject( hbrush );
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

static char	 appName[256];			// application name
static char	 appFileName[256];		// application file name
static HINSTANCE appInst	= 0;		// handle to app instance
static HINSTANCE appPrevInst	= 0;		// handle to prev app instance
static int	 appCmdShow	= 0;		// main window show command
static HWND	 curWin		= 0;		// current window
static HDC	 displayDC	= 0;		// display device context
static UINT	 appUniqueID	= 0;		// application id used by WinCE

// Session management
static bool	sm_blockUserInput    = FALSE;
static bool	sm_smActive	     = FALSE;
extern QSessionManager* qt_session_manager_self;
static bool	sm_cancel;

static bool replayPopupMouseEvent = FALSE; // replay handling when popups close

static bool ignoreNextMouseReleaseEvent = FALSE; // ignore the next release event if
						 // return from a modal widget
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif

static bool	app_do_modal	   = FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;
extern QDesktopWidget *qt_desktopWidget;
static QWidget *popupButtonFocus   = 0;
static bool	popupCloseDownMode = FALSE;
static bool	qt_try_modal( QWidget *, MSG *, int& ret );

QWidget	       *qt_button_down = 0;		// widget got last button-down

extern bool qt_tryAccelEvent( QWidget*, QKeyEvent* ); // def in qaccel.cpp

static HWND	autoCaptureWnd = 0;
static void	setAutoCapture( HWND );		// automatic capture
static void	releaseAutoCapture();

// ### Remove 4.0 [start] -------------------
typedef void (*VFPTR)();
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
static QWinEventFilter qt_win_event_filter = 0;

Q_EXPORT QWinEventFilter qt_set_win_event_filter (QWinEventFilter filter)
{
    QWinEventFilter old_filter = qt_win_event_filter;
    qt_win_event_filter = filter;
    return old_filter;
}

typedef bool (*QWinEventFilterEx)(MSG*,long&);
static QWinEventFilterEx qt_win_event_filter_ex = 0;

Q_EXPORT QWinEventFilterEx qt_set_win_event_filter_ex( QWinEventFilterEx filter )
{
    QWinEventFilterEx old = qt_win_event_filter_ex;
    qt_win_event_filter_ex = filter;
    return old;
}

// ### Remove 4.0 [end] --------------------
bool qt_winEventFilter( MSG* msg, long &result )
{
    result = 0;
    if ( qt_win_event_filter && qt_win_event_filter( msg )  )
	return TRUE;
    if ( qt_win_event_filter_ex && qt_win_event_filter_ex( msg, result ) )
        return TRUE;
    return qApp->winEventFilter( msg );
}

static void	msgHandler( QtMsgType, const char* );
static void     unregWinClasses();

static int	translateKeyCode( int );

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
    QWExtra    *xtra()			{ return d->extraData(); }
    bool	winEvent( MSG *m )	{ return QWidget::winEvent(m); }
#if defined(Q_CC_GNU)
    void	markFrameStrutDirty()	{ fstrut_dirty = 1; }
#else
    void	markFrameStrutDirty()	{ QWidget::fstrut_dirty = 1; }
#endif
    bool	translateMouseEvent( const MSG &msg );
    bool	translateKeyEvent( const MSG &msg, bool grab );
    bool	translateWheelEvent( const MSG &msg );
    bool	sendKeyEvent( QEvent::Type type, int code, int ascii,
			      int state, bool grab, const QString& text,
			      bool autor=FALSE );
    bool	translatePaintEvent( const MSG &msg );
    bool	translateConfigEvent( const MSG &msg );
    bool	translateCloseEvent( const MSG &msg );
#if defined(QT_TABLET_SUPPORT)
	bool	translateTabletEvent( const MSG &msg, PACKET *localPacketBuf,
		                          int numPackets );
#endif
    void	repolishStyle( QStyle &style ) { styleChange( style ); }

    HDC setHdc(HDC h) { HDC tmp = hdc; hdc = h; return tmp; }

};

static void set_winapp_name()
{
    static bool already_set = FALSE;
    if ( !already_set ) {
	already_set = TRUE;
#ifndef Q_OS_TEMP
	GetModuleFileNameA( 0, appFileName, sizeof(appFileName) );
#else
	QString afm;
	afm.setLength( 256 );
	afm.setLength( GetModuleFileName( 0, (unsigned short*)afm.unicode(), 255 ) );
	strncpy( appFileName, afm.latin1(), afm.length() );
#endif
	const char *p = strrchr( appFileName, '\\' );	// skip path
	if ( p )
	    memcpy( appName, p+1, qstrlen(p) );
	int l = qstrlen( appName );
	if ( (l > 4) && !qstricmp( appName + l - 4, ".exe" ) )
	    appName[l-4] = '\0';		// drop .exe extension
    }
}


/*****************************************************************************
  qWinMain() - Initializes Windows. Called from WinMain() in qtmain_win.cpp
 *****************************************************************************/

#if defined( Q_OS_TEMP )
Q_EXPORT void __cdecl qWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
	       int cmdShow, int &argc, QVector<pchar> &argv )
#else
Q_EXPORT
void qWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
	       int cmdShow, int &argc, QVector<pchar> &argv )
#endif
{
    static bool already_called = FALSE;

    if ( already_called ) {
	qWarning( "Qt internal error: qWinMain should be called only once" );
	return;
    }
    already_called = TRUE;

  // Install default debug handler

    qInstallMsgHandler( msgHandler );

  // Create command line

    set_winapp_name();

    char *p = cmdParam;
    char *p_end = p + strlen(p);

    argc = 1;
    argv[0] = appFileName;

    while ( *p && p < p_end ) {				// parse cmd line arguments
	while ( isspace((uchar) *p) )			// skip white space
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
			if ( isspace((uchar) *p) )
			    break;
			quote = 0;
		    }
		} else {
		    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
			quote = *p++;
			continue;
		    } else if ( isspace((uchar) *p) )
			break;
		}
		if ( p )
		    *r++ = *p++;
	    }
	    if ( *p && p < p_end )
		p++;
	    *r = '\0';

	    if ( argc >= (int)argv.size()-1 )	// expand array
		argv.resize( argv.size()*2 );
	    argv[argc++] = start;
	}
    }
    argv[argc] = 0;
  // Get Windows parameters

    appInst = instance;
    appPrevInst = prevInstance;
    appCmdShow = cmdShow;

#ifdef Q_OS_TEMP
    TCHAR uniqueAppID[256];
    GetModuleFileName( 0, uniqueAppID, 255 );
    appUniqueID = RegisterWindowMessage(
		  QString::fromUcs2(uniqueAppID)
		  .lower().remove('\\').ucs2() );
#endif
}

static void qt_show_system_menu( QWidget* tlw)
{
    HMENU menu = GetSystemMenu( tlw->winId(), FALSE );
    if ( !menu )
	return; // no menu for this window

#define enabled (MF_BYCOMMAND | MF_ENABLED)
#define disabled (MF_BYCOMMAND | MF_GRAYED)

#ifndef Q_OS_TEMP
    EnableMenuItem( menu, SC_MINIMIZE, enabled);
    bool maximized  = IsZoomed( tlw->winId() );

    EnableMenuItem( menu, SC_MAXIMIZE, maximized?disabled:enabled);
    EnableMenuItem( menu, SC_RESTORE, maximized?enabled:disabled);

    EnableMenuItem( menu, SC_SIZE, maximized?disabled:enabled);
    EnableMenuItem( menu, SC_MOVE, maximized?disabled:enabled);
    EnableMenuItem( menu, SC_CLOSE, enabled);
#endif

#undef enabled
#undef disabled

    int ret = TrackPopupMenuEx( menu,
				TPM_LEFTALIGN  | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
				tlw->geometry().x(), tlw->geometry().y(),
				tlw->winId(),
				0);
    if (ret)
#ifdef Q_OS_TEMP
	DefWindowProc(tlw->winId(), WM_SYSCOMMAND, ret, 0);
#else
	QtWndProc(tlw->winId(), WM_SYSCOMMAND, ret, 0);
#endif
}

extern QFont qt_LOGFONTtoQFont(LOGFONT& lf,bool scale);

// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();

static void qt_set_windows_resources()
{
#ifndef Q_OS_TEMP
    QFont menuFont;
    QFont messageFont;
    QFont statusFont;
    QFont titleFont;
    QFont smallTitleFont;

    QT_WA( {
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof( ncm );
	SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0 );
	menuFont = qt_LOGFONTtoQFont(ncm.lfMenuFont,TRUE);
	messageFont = qt_LOGFONTtoQFont(ncm.lfMessageFont,TRUE);
	statusFont = qt_LOGFONTtoQFont(ncm.lfStatusFont,TRUE);
	titleFont = qt_LOGFONTtoQFont(ncm.lfCaptionFont,TRUE);
	smallTitleFont = qt_LOGFONTtoQFont(ncm.lfSmCaptionFont,TRUE);
    } , {
	// A version
	NONCLIENTMETRICSA ncm;
	ncm.cbSize = sizeof( ncm );
	SystemParametersInfoA( SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	menuFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMenuFont,TRUE);
	messageFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMessageFont,TRUE);
	statusFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfStatusFont,TRUE);
	titleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfCaptionFont,TRUE);
	smallTitleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfSmCaptionFont,TRUE);
    } );

    QApplication::setFont( menuFont, TRUE, "QPopupMenu" );
    QApplication::setFont( menuFont, TRUE, "QMenuBar" );
    QApplication::setFont( messageFont, TRUE, "QMessageBox" );
    QApplication::setFont( statusFont, TRUE, "QTipLabel" );
    QApplication::setFont( statusFont, TRUE, "QStatusBar" );
    QApplication::setFont( titleFont, TRUE, "QTitleBar" );
    QApplication::setFont( smallTitleFont, TRUE, "QDockWindowTitleBar" );
#else
    LOGFONT lf;
    HGDIOBJ stockFont = GetStockObject( SYSTEM_FONT );
    GetObject( stockFont, sizeof(lf), &lf );
    QApplication::setFont( qt_LOGFONTtoQFont( lf, TRUE ) );
#endif// Q_OS_TEMP

    if ( qt_std_pal && *qt_std_pal != QApplication::palette() )
	return;

    // Do the color settings
    QPalette pal;
    pal.setColor( QPalette::Foreground,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))) );
    pal.setColor( QPalette::Button,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))) );
    pal.setColor( QPalette::Light,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))) );
    pal.setColor( QPalette::Dark,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNSHADOW))) );
    pal.setColor( QPalette::Mid, pal.button().color().dark( 150 ) );
    pal.setColor( QPalette::Text,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))) );
    pal.setColor( QPalette::BrightText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))) );
    pal.setColor( QPalette::Base,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOW))) );
    pal.setColor( QPalette::Background,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))) );
    pal.setColor( QPalette::ButtonText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNTEXT))) );
    pal.setColor( QPalette::Midlight,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DLIGHT))) );
    pal.setColor( QPalette::Shadow,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DDKSHADOW))) );
    pal.setColor( QPalette::Highlight,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
    pal.setColor( QPalette::HighlightedText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );
    // ### hardcoded until I find out how to get it from the system settings.
    pal.setColor( QPalette::Link, Qt::blue );
    pal.setColor( QPalette::LinkVisited, Qt::magenta );

    if ( qt_winver != Qt::WV_NT && qt_winver != Qt::WV_95 ) {
	if ( pal.midlight() == pal.button() )
	    pal.setColor( QPalette::Midlight, pal.button().color().light(110) );
	if ( pal.background() != pal.base() ) {
	    pal.setColor( QPalette::Inactive, QPalette::Highlight, pal.color(QPalette::Inactive, QPalette::Background) );
	    pal.setColor( QPalette::Inactive, QPalette::HighlightedText, pal.color(QPalette::Inactive, QPalette::Text) );
	}
    }

    const QColor fg = pal.foreground().color(), btn = pal.button().color();
    QColor disabled( (fg.red()+btn.red())/2,(fg.green()+btn.green())/2,
		     (fg.blue()+btn.blue())/2);
    pal.setColor( QPalette::Disabled, QPalette::Foreground, disabled );
    pal.setColor( QPalette::Disabled, QPalette::Text, disabled );
    pal.setColor( QPalette::Disabled, QPalette::Highlight,
		  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
    pal.setColor( QPalette::Disabled, QPalette::HighlightedText,
		  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

    QApplication::setPalette( pal, TRUE );
    *qt_std_pal = pal;

    QColor menuCol(qt_colorref2qrgb(GetSysColor(COLOR_MENU)));
    QColor menuText(qt_colorref2qrgb(GetSysColor(COLOR_MENUTEXT)));
    {
	QPalette menu(pal);
	// we might need a special color group for the menu.
	menu.setColor( QPalette::Active, QPalette::Button, menuCol );
	menu.setColor( QPalette::Active, QPalette::Text, menuText );
	menu.setColor( QPalette::Active, QPalette::Foreground, menuText );
	menu.setColor( QPalette::Active, QPalette::ButtonText, menuText );
	const QColor fg = menu.foreground().color(), btn = menu.button().color();
	QColor disabled( (fg.red()+btn.red())/2,(fg.green()+btn.green())/2,
			 (fg.blue()+btn.blue())/2);
	menu.setColor( QPalette::Disabled, QPalette::Foreground, disabled );
	menu.setColor( QPalette::Disabled, QPalette::Text, disabled );
	menu.setColor( QPalette::Disabled, QPalette::Highlight,
		      QColor(qt_colorref2qrgb(GetSysColor(
						  qt_winver == Qt::WV_XP ? COLOR_MENUHILIGHT :
						  COLOR_HIGHLIGHTTEXT))) );
	menu.setColor( QPalette::Disabled, QPalette::HighlightedText,
		      QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

	menu.setColor( QPalette::Inactive, QPalette::Button,
		      menu.color(QPalette::Active, QPalette::Button));
	menu.setColor( QPalette::Inactive, QPalette::Text,
		      menu.color(QPalette::Active, QPalette::Text));
	menu.setColor( QPalette::Inactive, QPalette::Foreground,
		      menu.color(QPalette::Active, QPalette::Foreground));
	menu.setColor( QPalette::Inactive, QPalette::ButtonText,
		      menu.color(QPalette::Active, QPalette::ButtonText));
	if ( qt_winver != Qt::WV_NT && qt_winver != Qt::WV_95 )
	    menu.setColor( QPalette::Inactive, QPalette::Button,
			  pal.color(QPalette::Inactive, QPalette::Dark));
	QApplication::setPalette( menu, TRUE, "QPopupMenu");

	if ( qt_winver == Qt::WV_XP ) {
	    BOOL isFlat;
	    SystemParametersInfo( 0x1022 /*SPI_GETFLATMENU*/, 0, &isFlat, 0 );
	    if ( isFlat ) {
		QColor menubar(qt_colorref2qrgb(GetSysColor(COLOR_MENUBAR)));
		menu.setColor( QPalette::Active, QPalette::Button, menubar );
		menu.setColor( QPalette::Disabled, QPalette::Button, menubar );
		menu.setColor( QPalette::Inactive, QPalette::Button, menubar );
	    }
	}
	QApplication::setPalette( menu, TRUE, "QMenuBar");
    }

    QColor ttip(qt_colorref2qrgb(GetSysColor(COLOR_INFOBK)));
    QColor ttipText(qt_colorref2qrgb(GetSysColor(COLOR_INFOTEXT)));
    {
	QPalette tiplabel(pal);
	tiplabel.setColor( QPalette::Active, QPalette::Button, ttip );
	tiplabel.setColor( QPalette::Active, QPalette::Background, ttip );
	tiplabel.setColor( QPalette::Active, QPalette::Text, ttipText );
	tiplabel.setColor( QPalette::Active, QPalette::Foreground, ttipText );
	tiplabel.setColor( QPalette::Active, QPalette::ButtonText, ttipText );
	tiplabel.setColor( QPalette::Active, QPalette::Button, ttip );
	tiplabel.setColor( QPalette::Active, QPalette::Background, ttip );
	tiplabel.setColor( QPalette::Active, QPalette::Text, ttipText );
	tiplabel.setColor( QPalette::Active, QPalette::Foreground, ttipText );
	tiplabel.setColor( QPalette::Active, QPalette::ButtonText, ttipText );
	const QColor fg = tiplabel.foreground().color(), btn = tiplabel.button().color();
	QColor disabled( (fg.red()+btn.red())/2,(fg.green()+btn.green())/2,
			 (fg.blue()+btn.blue())/2);
	tiplabel.setColor( QPalette::Disabled, QPalette::Foreground, disabled );
	tiplabel.setColor( QPalette::Disabled, QPalette::Text, disabled );
	tiplabel.setColor( QPalette::Disabled, QPalette::Base, Qt::white );
	tiplabel.setColor( QPalette::Disabled, QPalette::BrightText, Qt::white );
	QApplication::setPalette( tiplabel, TRUE, "QTipLabel");
    }
}

/*****************************************************************************
  qt_init() - initializes Qt for Windows
 *****************************************************************************/

// need to get default font?
extern bool qt_app_has_font;

void qt_init( int *argcptr, char **argv, QApplication::Type )
{

#if defined(QT_DEBUG)
    int argc = *argcptr;
    int i, j;

  // Get command line params

    j = argc ? 1 : 0;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	if ( qstrcmp(argv[i], "-nograb") == 0 )
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
	QT_WA( {
	    appInst = GetModuleHandle( 0 );
	}, {
	    appInst = GetModuleHandleA( 0 );
	} );
    }

#ifndef Q_OS_TEMP
    // Initialize OLE/COM
    //	 S_OK means success and S_FALSE means that it has already
    //	 been initialized
    HRESULT r;
    r = OleInitialize(0);
    if ( r != S_OK && r != S_FALSE ) {
	qWarning( "Qt: Could not initialize OLE (error %x)", r );
    }
#endif

    // Misc. initialization

    QWindowsMime::initialize();
    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
#if defined(QT_THREAD_SUPPORT)
    QThread::initialize();
#endif
    qApp->setName( appName );

    // default font
    if ( !qt_app_has_font ) {
	HFONT hfont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
	QFont f("MS Sans Serif",8);
	QT_WA( {
	    LOGFONT lf;
	    if ( GetObject( hfont, sizeof(lf), &lf ) )
		f = qt_LOGFONTtoQFont((LOGFONT&)lf,TRUE);
	} , {
	    LOGFONTA lf;
	    if ( GetObjectA( hfont, sizeof(lf), &lf ) )
		f = qt_LOGFONTtoQFont((LOGFONT&)lf,TRUE);
	} );
	QApplication::setFont( f );
    }

    // QFont::locale_init();  ### Uncomment when it does something on Windows

    if ( !qt_std_pal )
	qt_create_std_palette();
    if ( QApplication::desktopSettingsAware() )
	qt_set_windows_resources();

    QT_WA( {
	WM95_MOUSEWHEEL = RegisterWindowMessage(L"MSWHEEL_ROLLMSG");
    } , {
	WM95_MOUSEWHEEL = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
    } );
#if defined(QT_TABLET_SUPPORT)
    initWinTabFunctions();
#endif
    QInputContext::init();
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    unregWinClasses();
    QPixmapCache::clear();
    QPainter::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
#if defined(QT_THREAD_SUPPORT)
    QThread::cleanup();
#endif
    if ( displayDC ) {
	ReleaseDC( 0, displayDC );
	displayDC = 0;
    }

    QInputContext::shutdown();

#ifndef Q_OS_TEMP
  // Deinitialize OLE/COM
    OleUninitialize();
#endif
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

#if defined(Q_CC_MSVC) && !defined(Q_OS_TEMP)
#include <crtdbg.h>
#endif

static void msgHandler( QtMsgType t, const char* str )
{
#if defined(QT_THREAD_SUPPORT)
    // OutputDebugString is not threadsafe.
    static QMutex staticMutex;
#endif

    if ( !str )
	str = "(null)";

#if defined(QT_THREAD_SUPPORT)
    staticMutex.lock();
#endif
    QT_WA( {
	QString s(str);
	s += "\n";
	OutputDebugStringW( (TCHAR*)s.ucs2() );
    }, {
	QByteArray s(str);
	s += "\n";
	OutputDebugStringA( s.data() );
    } )
#if defined(QT_THREAD_SUPPORT)
    staticMutex.unlock();
#endif
    if ( t == QtFatalMsg )
#ifndef Q_OS_TEMP
#if defined(Q_CC_MSVC) && defined(_DEBUG)
	_CrtDbgReport( _CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, str );
#else
	ExitProcess( 1 );
#endif
#else
	exit(1);
#endif
}

Q_EXPORT const char *qAppFileName()		// get application file name
{
    return appFileName;
}

Q_EXPORT const char *qAppName()			// get application name
{
    if ( !appName[0] )
	set_winapp_name();
    return appName;
}

Q_EXPORT HINSTANCE qWinAppInst()		// get Windows app handle
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

const QString qt_reg_winclass( int flags )	// register window class
{
    if ( !winclassNames ) {
	winclassNames = new QAsciiDict<int>;
    }
    uint style;
    bool icon;
    QString cname;
    if ( flags & Qt::WWinOwnDC ) {
	cname = "QWidgetOwnDC";
#ifndef Q_OS_TEMP
	style = CS_OWNDC | CS_DBLCLKS;
#else
	style = CS_DBLCLKS;
#endif
	icon  = TRUE;
    } else if ( (flags & (Qt::WType_Popup|Qt::WStyle_Tool)) == 0 ) {
	cname = "QWidget";
	style = CS_DBLCLKS;
	icon  = TRUE;
    } else {
	cname = "QPopup";
#ifndef Q_OS_TEMP
	style = CS_DBLCLKS | CS_SAVEBITS;
#else
	style = CS_DBLCLKS;
#endif
	if ( qt_winver == Qt::WV_XP )
	    style |= 0x00020000;		// CS_DROPSHADOW
	icon  = FALSE;
    }

#ifdef Q_OS_TEMP
    // We need to register the classes with the
    // unique ID on WinCE to make sure we can
    // move the windows to the front when starting
    // a second instance.
    cname = QString::number( appUniqueID );
#endif

    if ( winclassNames->find(cname.latin1()) )		// already registered
	return cname;

#ifndef Q_OS_TEMP
    QT_WA( {
	WNDCLASS wc;
	wc.style	= style;
	wc.lpfnWndProc	= (WNDPROC)QtWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= (HINSTANCE)qWinAppInst();
	if ( icon ) {
	    wc.hIcon = LoadIcon( appInst, L"IDI_ICON1" );
	    if ( !wc.hIcon )
		wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
	}
	else
	{
	    wc.hIcon = 0;
	}
	wc.hCursor	= 0;
	wc.hbrBackground= 0;
	wc.lpszMenuName	= 0;
	wc.lpszClassName= (TCHAR*)cname.ucs2();
	RegisterClass( &wc );
    } , {
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
	wc.lpszClassName= cname.latin1();
	RegisterClassA( &wc );
    } );
#else
	WNDCLASS wc;
	wc.style	= style;
	wc.lpfnWndProc	= (WNDPROC)QtWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= (HINSTANCE)qWinAppInst();
	if ( icon ) {
	    wc.hIcon = LoadIcon( appInst, L"IDI_ICON1" );
//	    if ( !wc.hIcon )
//		wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
	}
	else
	{
	    wc.hIcon = 0;
	}
	wc.hCursor	= 0;
	wc.hbrBackground= 0;
	wc.lpszMenuName	= 0;
	wc.lpszClassName= (TCHAR*)cname.ucs2();
	RegisterClass( &wc );

#endif

    winclassNames->insert( cname.latin1(), (int*)1 );
    return cname;
}

static void unregWinClasses()
{
    if ( !winclassNames )
	return;
    QAsciiDictIterator<int> it(*winclassNames);
    const char *k;
    while ( (k = it.currentKey()) ) {
	QT_WA( {
	    UnregisterClass( (TCHAR*)QString::fromLatin1(k).ucs2(), (HINSTANCE)qWinAppInst() );
	} , {
	    UnregisterClassA( k, (HINSTANCE)qWinAppInst() );
	} );
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
    main_widget = mainWidget;
}

Qt::WindowsVersion QApplication::winVersion()
{
    return qt_winver;
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QPtrList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
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

void qt_set_cursor( QWidget *w, const QCursor& /* c */)
{
    if ( !curWin )
	return;
    QWidget* cW = QWidget::find( curWin );
    if ( !cW || cW->topLevelWidget() != w->topLevelWidget() ||
	 !cW->isVisible() || !cW->hasMouse() || cursorStack )
	return;

    SetCursor( cW->cursor().handle() );
}



/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    QObjectList children = p->children();
    for(int i = children.size(); i > 0 ; ) {
	--i;
	QObject *o = children.at(i);
	if ( o->isWidgetType() ) {
	    QWidget *w = (QWidget*)o;
	    if ( w->isVisible() && w->geometry().contains(pos) ) {
		QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		return c ? c : w;
	    }
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
    while ( !w && win ) {
	win = GetParent( win );
	w = QWidget::find( win );
    }
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
	    if ( pixmap->isMultiCellPixmap() ) {
		BitBlt( hdc, xPos, yPos,
			drawW, drawH, pixmap->multiCellHandle(),
			xOff, yOff+pixmap->multiCellOffset(), SRCCOPY );
	    } else {
		BitBlt( hdc, xPos, yPos,
			drawW, drawH, pixmap->handle(),
			xOff, yOff, SRCCOPY );
	    }
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
    copyBlt( tile, 0, 0, &pixmap, 0, 0, -1, -1);
    int x = pixmap.width();
    while ( x < tile->width() ) {
	copyBlt( tile, x, 0, tile, 0, 0, x, pixmap.height() );
	x *= 2;
    }
    int y = pixmap.height();
    while ( y < tile->height() ) {
	copyBlt( tile, 0, y, tile, 0, 0, tile->width(), y );
	y *= 2;
    }
}

Q_EXPORT
void qt_draw_tiled_pixmap( HDC hdc, int x, int y, int w, int h,
			   const QPixmap *bg_pixmap,
			   int off_x, int off_y )
{
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
	pm = (QPixmap*)bg_pixmap;
    }
    drawTile( hdc, x, y, w, h, pm, off_x, off_y );
    if ( tile )
	delete tile;
}



/*****************************************************************************
  Main event loop
 *****************************************************************************/

extern uint qGlobalPostedEventsCount();

/*!
    The message procedure calls this function for every message
    received. Reimplement this function if you want to process window
    messages \e msg that are not processed by Qt. If you don't want
    the event to be processed by Qt, then return TRUE; otherwise
    return FALSE.
*/
bool QApplication::winEventFilter( MSG * /*msg*/ )	// Windows event filter
{
    return FALSE;
}

/*!
    If \a gotFocus is TRUE, \a widget will become the active window.
    Otherwise the active window is reset to NULL.
*/
void QApplication::winFocus( QWidget *widget, bool gotFocus )
{
    if ( inPopupMode() ) // some delayed focus event to ignore
	return;
    if ( gotFocus ) {
	setActiveWindow( widget );
	if ( active_window && active_window->testWFlags( WType_Dialog ) ) {
	    // raise the entire application, not just the dialog
	    QWidget* mw = active_window;
	    while( mw->parentWidget() && mw->testWFlags( WType_Dialog) )
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
static int inputcharset = CP_ACP;
extern uint qt_sn_msg;
extern void qt_sn_activate_fd( int sockfd, int type );

#define RETURN(x) { inLoop=FALSE;return x; }

bool qt_sendSpontaneousEvent( QObject *receiver, QEvent *event )
{
    return QApplication::sendSpontaneousEvent( receiver, event );
}

extern "C"
LRESULT CALLBACK QtWndProc( HWND hwnd, UINT message, WPARAM wParam,
			    LPARAM lParam )
{
    bool result = TRUE;
    long filterRes = 0;
    QEvent::Type evt_type = QEvent::None;
    QETWidget *widget = 0;

#if defined(QT_TABLET_SUPPORT)
	// there is no need to process pakcets from tablet unless
	// it is actually on the tablet, a flag to let us know...
	int nPackets;	// the number of packets we get from the queue
#endif

    if ( !qApp )				// unstable app state
	goto do_default;

    // make sure we update widgets also when the user resizes
    if ( inLoop && qApp->loopLevel() )
	qApp->sendPostedEvents( 0, QEvent::Paint );

    inLoop = TRUE;

    MSG msg;
    msg.hwnd = hwnd;				// create MSG structure
    msg.message = message;			// time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.pt.x = GET_X_LPARAM( lParam );
    msg.pt.y = GET_Y_LPARAM( lParam );
    ClientToScreen( msg.hwnd, &msg.pt ); 	// the coords we get are client coords

    /*
    // sometimes the autograb is not released, so the clickevent is sent
    // to the wrong window. We ignore this for now, because it doesn't
    // cause any problems.
    if ( msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_MBUTTONDOWN ) {
	HWND handle = WindowFromPoint( msg.pt );
	if ( msg.hwnd != handle ) {
	    msg.hwnd = handle;
	    hwnd = handle;
	}
    }
    */

#if defined(QT_NON_COMMERCIAL)
    QT_NC_WNDPROC
#endif

    if ( qt_winEventFilter(&msg, filterRes) )		// send through app filter
	RETURN(filterRes);

    switch ( message ) {
#ifndef Q_OS_TEMP
    case WM_QUERYENDSESSION: {
	if ( sm_smActive ) // bogus message from windows
	    RETURN(TRUE);

	sm_smActive = TRUE;
	sm_blockUserInput = TRUE; // prevent user-interaction outside interaction windows
	sm_cancel = FALSE;
	if ( qt_session_manager_self )
	    qApp->commitData( *qt_session_manager_self );
	if ( lParam == (LPARAM)ENDSESSION_LOGOFF ) {
	    _flushall();
	}
	RETURN(!sm_cancel);
    }
    case WM_ENDSESSION: {
	sm_smActive = FALSE;
	sm_blockUserInput = FALSE;
	bool endsession = (bool) wParam;

	if ( endsession ) {
	    // since the process will be killed immediately quit() has no real effect
	    int index = QApplication::staticMetaObject.indexOfSignal( "aboutToQuit()" );
	    qApp->qt_metacall(QMetaObject::EmitSignal, index,0);
	    qApp->quit();
	}

	RETURN(0);
    }
    case WM_DISPLAYCHANGE:
	if ( qApp->type() == QApplication::Tty )
	    break;
	if ( qt_desktopWidget ) {
	    int x = GetSystemMetrics( 76 );
	    int y = GetSystemMetrics( 77 );
	    QMoveEvent mv( QPoint(x, y), qt_desktopWidget->pos() );
	    QApplication::sendEvent( qt_desktopWidget, &mv );
	    x = GetSystemMetrics( 78 );
	    y = GetSystemMetrics( 79 );
	    QResizeEvent re( QSize(x, y), qt_desktopWidget->size() );
	    QApplication::sendEvent( qt_desktopWidget, &re );
	}
	break;
#endif

    case WM_SETTINGCHANGE:
#ifdef Q_OS_TEMP
	// CE SIP hide/show
	if ( wParam == SPI_SETSIPINFO ) {
	    QResizeEvent re( QSize(0, 0), QSize(0, 0) ); // Calculated by QDesktopWidget
	    QApplication::sendEvent( qt_desktopWidget, &re );
	    break;
	}
#endif
	// ignore spurious XP message when user logs in again after locking
	if ( qApp->type() == QApplication::Tty )
	    break;
	if ( QApplication::desktopSettingsAware() && wParam != SPI_SETWORKAREA ) {
	    widget = (QETWidget*)QWidget::find( hwnd );
	    if ( widget ) {
		widget->markFrameStrutDirty();
		if ( !widget->parentWidget() )
		    qt_set_windows_resources();
	    }
	}
	break;
    case WM_SYSCOLORCHANGE:
	if ( qApp->type() == QApplication::Tty )
	    break;
	if ( QApplication::desktopSettingsAware() ) {
	    widget = (QETWidget*)QWidget::find( hwnd );
	    if ( widget && !widget->parentWidget() )
		qt_set_windows_resources();
	}
	break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
	if ( ignoreNextMouseReleaseEvent )
	    ignoreNextMouseReleaseEvent = FALSE;
	break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
	if ( ignoreNextMouseReleaseEvent ) {
	    ignoreNextMouseReleaseEvent = FALSE;
	    if ( qt_button_down && qt_button_down->winId() == autoCaptureWnd ) {
		releaseAutoCapture();
		qt_button_down = 0;
	    }

	    RETURN(0);
	}
	break;

    default:
	break;
    }

    if ( !widget )
	widget = (QETWidget*)QWidget::find( hwnd );
    if ( !widget )				// don't know this widget
	goto do_default;

    if ( app_do_modal )	{			// modal event handling
	int ret = 0;
	if ( !qt_try_modal(widget, &msg, ret ) )
	    return ret;
    }

    if ( widget->winEvent(&msg) )		// send through widget filter
	RETURN(0);

    if ( qt_sn_msg && msg.message == qt_sn_msg ) {	// socket notifier message
	int type = -1;
#ifndef Q_OS_TEMP
	switch ( WSAGETSELECTEVENT(msg.lParam) ) {
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
	    qt_sn_activate_fd( msg.wParam, type );
    } else if ( ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST ||
	    message >= WM_XBUTTONDOWN && message <= WM_XBUTTONDBLCLK )
	    && message != WM_MOUSEWHEEL ) {
	if ( qApp->activePopupWidget() != 0) { // in popup mode
	    POINT curPos = msg.pt;
	    QWidget* w = QApplication::widgetAt(curPos.x, curPos.y, TRUE );
	    if ( w )
		widget = (QETWidget*)w;
	}

#if defined(QT_TABLET_SUPPORT)
	if ( !chokeMouse ) {
#endif
	    widget->translateMouseEvent( msg );	// mouse event
#if defined(QT_TABLET_SUPPORT)
	} else {
	    // Sometimes we only get a WM_MOUSEMOVE message
	    // and sometimes we get both a WM_MOUSEMOVE and
	    // a WM_LBUTTONDOWN/UP, this creates a spurious mouse
	    // press/release event, using the winPeekMessage
	    // will help us fix this.  This leaves us with a
	    // question:
	    //    This effectively kills using the mouse AND the
	    //    tablet simultaneously, well creates wacky input.
	    //    Is this going to be a problem? (probably not)
	    bool next_is_button = FALSE;
	    bool is_mouse_move = (message == WM_MOUSEMOVE);
	    if ( is_mouse_move ) {
		MSG msg1;
		if ( winPeekMessage(&msg1, msg.hwnd, WM_MOUSEFIRST,
				    WM_MOUSELAST, PM_NOREMOVE) )
		    next_is_button = ( msg1.message == WM_LBUTTONUP
				       || msg1.message == WM_LBUTTONDOWN );
	    }
	    if ( !is_mouse_move || (is_mouse_move && !next_is_button) )
	    	chokeMouse = FALSE;
	}
#endif
    } else if ( message == WM95_MOUSEWHEEL ) {
	result = widget->translateWheelEvent( msg );
    } else {
	switch ( message ) {
	case WM_KEYDOWN:			// keyboard event
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_IME_CHAR:
	case WM_IME_KEYDOWN:
	case WM_CHAR: {
	    MSG msg1;
	    winPeekMessage(&msg1, msg.hwnd, 0, 0, PM_NOREMOVE);
	    if ( msg1.message == WM_DEADCHAR ) {
		result = TRUE; // consume event since there is a dead char next
		break;
	    }
	    QWidget *g = QWidget::keyboardGrabber();
	    if ( g )
		widget = (QETWidget*)g;
	    else if ( qApp->focusWidget() )
		widget = (QETWidget*)qApp->focusWidget();
	    else if ( !widget || widget->winId() == GetFocus() ) // We faked the message to go to exactly that widget.
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

	case WM_APPCOMMAND:
	    {
		uint cmd = GET_APPCOMMAND_LPARAM(lParam);
		uint uDevice = GET_DEVICE_LPARAM(lParam);
		uint dwKeys = GET_KEYSTATE_LPARAM(lParam);

		int state = translateButtonState( dwKeys, QEvent::KeyPress, 0 );

		switch ( uDevice ) {
		case FAPPCOMMAND_KEY:
		    {
			int key = 0;

			switch( cmd ) {
			case APPCOMMAND_BASS_BOOST:
			    key = Qt::Key_BassBoost;
			    break;
			case APPCOMMAND_BASS_DOWN:
			    key = Qt::Key_BassDown;
			    break;
			case APPCOMMAND_BASS_UP:
			    key = Qt::Key_BassUp;
			    break;
			case APPCOMMAND_BROWSER_BACKWARD:
			    key = Qt::Key_Back;
			    break;
			case APPCOMMAND_BROWSER_FAVORITES:
			    key = Qt::Key_Favorites;
			    break;
			case APPCOMMAND_BROWSER_FORWARD:
			    key = Qt::Key_Forward;
			    break;
			case APPCOMMAND_BROWSER_HOME:
			    key = Qt::Key_HomePage;
			    break;
			case APPCOMMAND_BROWSER_REFRESH:
			    key = Qt::Key_Refresh;
			    break;
			case APPCOMMAND_BROWSER_SEARCH:
			    key = Qt::Key_Search;
			    break;
			case APPCOMMAND_BROWSER_STOP:
			    key = Qt::Key_Stop;
			    break;
			case APPCOMMAND_LAUNCH_APP1:
			    key = Qt::Key_Launch0;
			    break;
			case APPCOMMAND_LAUNCH_APP2:
			    key = Qt::Key_Launch1;
			    break;
			case APPCOMMAND_LAUNCH_MAIL:
			    key = Qt::Key_LaunchMail;
			    break;
			case APPCOMMAND_LAUNCH_MEDIA_SELECT:
			    key = Qt::Key_LaunchMedia;
			    break;
			case APPCOMMAND_MEDIA_NEXTTRACK:
			    key = Qt::Key_MediaNext;
			    break;
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
			    key = Qt::Key_MediaPlay;
			    break;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
			    key = Qt::Key_MediaPrev;
			    break;
			case APPCOMMAND_MEDIA_STOP:
			    key = Qt::Key_MediaStop;
			    break;
			case APPCOMMAND_TREBLE_DOWN:
			    key = Qt::Key_TrebleDown;
			    break;
			case APPCOMMAND_TREBLE_UP:
			    key = Qt::Key_TrebleUp;
			    break;
			case APPCOMMAND_VOLUME_DOWN:
			    key = Qt::Key_VolumeDown;
			    break;
			case APPCOMMAND_VOLUME_MUTE:
			    key = Qt::Key_VolumeMute;
			    break;
			case APPCOMMAND_VOLUME_UP:
			    key = Qt::Key_VolumeUp;
			    break;
			default:
			    break;
			}
			if ( key ) {
			    bool res = FALSE;
			    QWidget *g = QWidget::keyboardGrabber();
			    if ( g )
				widget = (QETWidget*)g;
			    else if ( qApp->focusWidget() )
				widget = (QETWidget*)qApp->focusWidget();
			    else
				widget = (QETWidget*)widget->topLevelWidget();
			    if ( widget->isEnabled() )
				res = ((QETWidget*)widget)->sendKeyEvent( QEvent::KeyPress, key, 0, state, FALSE, QString::null, g != 0 );
			    if ( res )
				return TRUE;
			}
		    }
		    break;

		default:
		    break;
		}

		result = FALSE;
	    }
	    break;

#ifndef Q_OS_TEMP
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
#endif

	case WM_SYSCOMMAND:
#ifndef Q_OS_TEMP
	    switch( wParam ) {
	    case SC_CONTEXTHELP:
#ifndef QT_NO_WHATSTHIS
		QWhatsThis::enterWhatsThisMode();
#endif
		QT_WA( {
		    DefWindowProc( hwnd, WM_NCPAINT, 1, 0 );
		} , {
		    DefWindowProcA( hwnd, WM_NCPAINT, 1, 0 );
		} );
		break;
#if defined(QT_NON_COMMERCIAL)
		QT_NC_SYSCOMMAND
#endif
	    case SC_MAXIMIZE:
		QApplication::postEvent( widget, new QEvent( QEvent::ShowMaximized ) );
		result = FALSE;
		break;
	    case SC_MINIMIZE:
		QApplication::postEvent( widget, new QEvent( QEvent::ShowMinimized ) );
		result = FALSE;
		break;
	    case SC_RESTORE:
		QApplication::postEvent( widget, new QEvent( QEvent::ShowNormal ) );
		result = FALSE;
		break;
	    default:
		result = FALSE;
		break;
	    }
#endif
	    break;

	case WM_SETTINGCHANGE:
	    if ( !msg.wParam ) {
		QString area = QT_WA_INLINE( QString::fromUcs2( (unsigned short *)msg.lParam ),
					     QString::fromLocal8Bit( (char*)msg.lParam ) );
		if ( area == "intl" )
		    QApplication::postEvent( widget, new QEvent( QEvent::LocaleChange ) );
	    }
	    break;

#ifndef Q_OS_TEMP
	case WM_NCLBUTTONDBLCLK:
	    if ( wParam == HTCAPTION ) {
		if ( widget->isMaximized() )
		    QApplication::postEvent( widget, new QEvent( QEvent::ShowNormal ) );
		else
		    QApplication::postEvent( widget, new QEvent( QEvent::ShowMaximized ) );
	    }
	    result = FALSE;
	    break;
#endif
	case WM_PAINT:				// paint event
	    result = widget->translatePaintEvent( msg );
	    break;

	case WM_ERASEBKGND:			// erase window background
	    {
		HDC oldhdc = widget->setHdc((HDC)wParam);
		widget->erase();
		widget->setHdc(oldhdc);
		RETURN(TRUE);
	    }
	    break;

	case WM_MOVE:				// move window
	case WM_SIZE:				// resize window
	    result = widget->translateConfigEvent( msg );
	    if ( widget->isMaximized() )
		QApplication::postEvent( widget, new QEvent( QEvent::ShowMaximized ) );
	    break;

	case WM_ACTIVATE:
#if defined(QT_TABLET_SUPPORT)
	    if ( ptrWTOverlap && ptrWTEnable ) {
		// cooperate with other tablet applications, but when
		// we get focus, I want to use the tablet...
		if (qt_tablet_context && GET_WM_ACTIVATE_STATE(wParam, lParam)) {
		    if ( ptrWTEnable(qt_tablet_context, TRUE) )
			ptrWTOverlap(qt_tablet_context, TRUE);
		}
	    }
#endif
	    if ( QApplication::activePopupWidget() && LOWORD(wParam) == WA_INACTIVE &&
		QWidget::find((HWND)lParam) == 0 ) {
		// Another application was activated while our popups are open,
		// then close all popups.  In case some popup refuses to close,
		// we give up after 1024 attempts (to avoid an infinite loop).
		int maxiter = 1024;
		QWidget *popup;
		while ( (popup=QApplication::activePopupWidget()) && maxiter-- )
		    popup->close();
	    }
	    qApp->winFocus( widget, LOWORD(wParam) == WA_INACTIVE ? 0 : 1 );
	    break;

#ifndef Q_OS_TEMP
	    case WM_MOUSEACTIVATE:
		{
		    const QWidget *tlw = widget->topLevelWidget();
		    // Do not change activation if the clicked widget is inside a floating dock window
		    if ( tlw->inherits( "QDockWindow" ) && qApp->activeWindow()
			 && !qApp->activeWindow()->inherits("QDockWindow") )
			RETURN(MA_NOACTIVATE);
		}
		result = FALSE;
		break;
#endif
	    case WM_SHOWWINDOW:
#ifndef Q_OS_TEMP
		if ( lParam == SW_PARENTOPENING ) {
		    if ( widget->testWState(Qt::WState_ForceHide) )
			RETURN(0);
		}
#endif
		result = FALSE;
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
	    if ( widget == popupButtonFocus )
		popupButtonFocus = 0;
	    result = FALSE;
	    break;

#ifndef Q_OS_TEMP
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
		if ( lParam != (int)0xffffffff ) {
		    result = FALSE;
		    break;
		}
		QWidget *fw = qApp->focusWidget();
		if ( fw ) {
		    QContextMenuEvent e( QContextMenuEvent::Keyboard, QPoint( 5, 5 ), fw->mapToGlobal( QPoint( 5, 5 ) ), 0 );
		    result = qt_sendSpontaneousEvent( fw, &e );
		}
	    }
	    break;
#endif

	case WM_IME_STARTCOMPOSITION:
    	    result = QInputContext::startComposition();
	    break;
	case WM_IME_ENDCOMPOSITION:
	    result = QInputContext::endComposition();
	    break;
	case WM_IME_COMPOSITION:
	    result = QInputContext::composition( lParam );
	    break;

#ifndef Q_OS_TEMP
	case WM_CHANGECBCHAIN:
	case WM_DRAWCLIPBOARD:
	case WM_RENDERFORMAT:
	case WM_RENDERALLFORMATS:
	case WM_DESTROYCLIPBOARD:
	    if ( qt_clipboard ) {
		QCustomEvent e( QEvent::Clipboard, &msg );
		qt_sendSpontaneousEvent( qt_clipboard, &e );
		RETURN(0);
	    }
	    result = FALSE;
	    break;
#endif
#if defined(QT_ACCESSIBILITY_SUPPORT)
	case WM_GETOBJECT:
	    {
		// Ignoring all requests while starting up
		if ( qApp->startingUp() || !qApp->loopLevel() || lParam != OBJID_CLIENT ) {
		    result = FALSE;
		    break;
		}

		typedef LRESULT (WINAPI *PtrLresultFromObject)(REFIID, WPARAM, LPUNKNOWN );
		static PtrLresultFromObject ptrLresultFromObject = 0;
		static bool oleaccChecked = FALSE;

		if ( !oleaccChecked ) {
		    oleaccChecked = TRUE;
		    ptrLresultFromObject = (PtrLresultFromObject)QLibrary::resolve( "oleacc.dll", "LresultFromObject" );
		}
		if ( ptrLresultFromObject ) {
		    QAccessibleInterface *acc = 0;
		    QAccessible::queryAccessibleInterface( widget, &acc );
		    if ( !acc ) {
			result = FALSE;
			break;
		    }

		    // and get an instance of the IAccessibile implementation
		    IAccessible *iface = qt_createWindowsAccessible( acc );
		    acc->release();
		    LRESULT res = ptrLresultFromObject( IID_IAccessible, wParam, iface );  // ref == 2
		    iface->Release(); // the client will release the object again, and then it will destroy itself

		    if ( res > 0 )
			RETURN(res);
		}
	    }
	    result = FALSE;
	    break;
#endif
#if defined(QT_TABLET_SUPPORT)
	case WT_PACKET:
	    // Get the packets and also don't link against the actual library...
	    if ( ptrWTPacketsGet ) {
		if ( (nPackets = ptrWTPacketsGet( qt_tablet_context, QT_TABLET_NPACKETQSIZE, &localPacketBuf)) ) {
		    result = widget->translateTabletEvent( msg, localPacketBuf, nPackets );
		}
	    }
	    break;
	case WT_PROXIMITY:
	    // flush the QUEUE
	    if ( ptrWTPacketsGet )
		ptrWTPacketsGet( qt_tablet_context, QT_TABLET_NPACKETQSIZE + 1, NULL);
	    if ( chokeMouse )
		chokeMouse = FALSE;
	    break;
#endif
	case WM_KILLFOCUS:
	    if ( !QWidget::find( (HWND)wParam ) ) { // we don't get focus, so unset it now
		if ( !widget->hasFocus() ) // work around Windows bug after minimizing/restoring
		    widget = (QETWidget*)qApp->focusWidget();
		HWND focus = ::GetFocus();
		if ( !widget || (focus && ::IsChild( widget->winId(), focus )) ) {
		    result = FALSE;
		} else {
		    widget->clearFocus();
		    result = TRUE;
		}
	    } else {
		result = FALSE;
	    }
	    break;

	case WM_THEMECHANGED:
	    if ( widget->testWFlags( Qt::WType_Desktop ) || !qApp || qApp->closingDown()
							 || qApp->type() == QApplication::Tty )
		break;

	    if ( widget->testWState(Qt::WState_Polished) )
		qApp->style().unPolish(widget);

	    if ( widget->testWState(Qt::WState_Polished) )
		qApp->style().polish(widget);
	    widget->repolishStyle( qApp->style() );
	    if ( widget->isVisible() )
		widget->update();
	    break;

#ifndef Q_OS_TEMP
	case WM_INPUTLANGCHANGE: {
	    char info[7];
	    if ( !GetLocaleInfoA( MAKELCID(lParam, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, info, 6 ) ) {
		inputcharset = CP_ACP;
	    } else {
		inputcharset = QString( info ).toInt();
	    }
            break;
        }
#else
	case WM_COMMAND:
	    result = (wParam == 0x1);
	    if ( result )
		QApplication::postEvent( widget, new QEvent( QEvent::OkRequest ) );
	    break;
	case WM_HELP:
	    QApplication::postEvent( widget, new QEvent( QEvent::HelpRequest ) );
	    result = TRUE;
	    break;
#endif

	case WM_MOUSELEAVE:
	    // We receive a mouse leave for curWin, meaning
	    // the mouse was moved outside our widgets
	    if ( widget->winId() == curWin ) {
		bool dispatch = !widget->hasMouse();
		// hasMouse is updated when dispatching enter/leave,
		// so test if it is actually up-to-date
		if ( !dispatch ) {
		    QRect geom = widget->geometry();
		    if ( widget->parentWidget() && !widget->isTopLevel() ) {
			QPoint gp = widget->parentWidget()->mapToGlobal( widget->pos() );
			geom.setX( gp.x() );
			geom.setY( gp.y() );
		    }
		    QPoint cpos = QCursor::pos();
		    dispatch = !geom.contains( cpos );
		    if ( !dispatch ) {
			HRGN hrgn = CreateRectRgn(0,0,0,0);
			if ( GetWindowRgn( curWin, hrgn ) != ERROR ) {
			    QPoint lcpos = widget->mapFromGlobal( cpos );
			    dispatch = !PtInRegion( hrgn, lcpos.x(), lcpos.y() );
			}
			DeleteObject(hrgn);
		    }
		}
		if ( dispatch ) {
		    qt_dispatchEnterLeave( 0, QWidget::find( (WId)curWin ) );
		    curWin = 0;
		}
	    }
	    break;

	case WM_CANCELMODE:
	    if ( qApp->focusWidget() ) {
		QFocusEvent::setReason( QFocusEvent::ActiveWindow );
		QFocusEvent e( QEvent::FocusOut );
		QApplication::sendEvent( qApp->focusWidget(), &e );
		QFocusEvent::resetReason();
	    }
	    break;

	default:
	    result = FALSE;			// event was not processed
	    break;
	}
    }

    if ( evt_type != QEvent::None ) {		// simple event
	QEvent e( evt_type );
	result = qt_sendSpontaneousEvent(widget, &e);
    }
    if ( result )
	RETURN(FALSE);

do_default:
    RETURN( QInputContext::DefWindowProc(hwnd,message,wParam,lParam) )
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

Q_EXPORT bool qt_modal_state()
{
    return app_do_modal;
}


Q_EXPORT void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
    }
    releaseAutoCapture();
    qt_dispatchEnterLeave( 0, QWidget::find((WId)curWin));
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
    curWin = 0;
    qt_button_down = 0;
    ignoreNextMouseReleaseEvent = FALSE;
}


Q_EXPORT void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->remove(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	    QPoint p( QCursor::pos() );
	    app_do_modal = FALSE; // necessary, we may get recursively into qt_try_modal below
	    QWidget* w = QApplication::widgetAt( p.x(), p.y(), TRUE );
	    qt_dispatchEnterLeave( w, QWidget::find( curWin ) ); // send synthetic enter event
	    curWin = w? w->winId() : 0;
	}
	ignoreNextMouseReleaseEvent = TRUE;
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

    QWidget *modal=0, *top=qt_modal_stack->first();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )				// don't block event
	return FALSE;
    return TRUE;
}

static bool qt_try_modal( QWidget *widget, MSG *msg, int& ret )
{
    QWidget * top = 0;

    if ( qt_tryModalHelper( widget, &top ) )
	return TRUE;

    int	 type  = msg->message;

    bool block_event = FALSE;
#ifndef Q_OS_TEMP
    if ( type == WM_NCHITTEST ) {
      //block_event = TRUE;
	// QApplication::beep();
    } else
#endif
	if ( (type >= WM_MOUSEFIRST && type <= WM_MOUSELAST) ||
	     type == WM_MOUSEWHEEL || type == (int)WM95_MOUSEWHEEL ||
	     type == WM_MOUSELEAVE ||
	     (type >= WM_KEYFIRST	&& type <= WM_KEYLAST)
#ifndef Q_OS_TEMP
			|| type == WM_NCMOUSEMOVE
#endif
		) {
      if ( type == WM_MOUSEMOVE
#ifndef Q_OS_TEMP
			|| type == WM_NCMOUSEMOVE
#endif
			) {
	QCursor *c = qt_grab_cursor();
	if ( !c )
	    c = QApplication::overrideCursor();
	if ( c )				// application cursor defined
	    SetCursor( c->handle() );
	else
	    SetCursor( Qt::arrowCursor.handle() );
      }
      block_event = TRUE;
    } else if ( type == WM_CLOSE ) {
	block_event = TRUE;
    }
#ifndef Q_OS_TEMP
    else if ( type == WM_MOUSEACTIVATE || type == WM_NCLBUTTONDOWN ){
	if ( !top->isActiveWindow() ) {
	    top->setActiveWindow();
	} else {
	    QApplication::beep();
	}
	block_event = TRUE;
	ret = MA_NOACTIVATEANDEAT;
    } else if ( type == WM_SYSCOMMAND ) {
	if ( !(msg->wParam == SC_RESTORE && widget->isMinimized()) )
	    block_event = TRUE;
    }
#endif

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
    }
    popupWidgets->append( popup );		// add to end of list
    if ( !popup->isEnabled() )
	return;

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
    popupWidgets->remove( popup );
    POINT curPos;
    GetCursorPos( &curPos );
    replayPopupMouseEvent = !popup->geometry().contains( QPoint(curPos.x, curPos.y) );
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !popup->isEnabled() )
	    return;
	if ( !qt_nograb() )			// grabbing not disabled
	    releaseAutoCapture();
	if ( active_window ) {
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    if ( active_window->focusWidget() )
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
	QWidget* aw = popupWidgets->last();
	if ( popupWidgets->count() == 1 )
	    setAutoCapture( aw->winId() );
	if (aw->focusWidget())
	    aw->focusWidget()->setFocus();
	else
	    aw->setFocus();
	QFocusEvent::resetReason();
    }
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
    WM_LBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::LeftButton,
    WM_LBUTTONUP,	QEvent::MouseButtonRelease,	Qt::LeftButton,
    WM_LBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::LeftButton,
    WM_RBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::RightButton,
    WM_RBUTTONUP,	QEvent::MouseButtonRelease,	Qt::RightButton,
    WM_RBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::RightButton,
    WM_MBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::MidButton,
    WM_MBUTTONUP,	QEvent::MouseButtonRelease,	Qt::MidButton,
    WM_MBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::MidButton,
    WM_XBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::MidButton*2, //### Qt::XButton1/2
    WM_XBUTTONUP,	QEvent::MouseButtonRelease,	Qt::MidButton*2,
    WM_XBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::MidButton*2,
    0,			0,				0
};

static int translateButtonState( int s, int type, int button )
{
    int bst = 0;
    if ( s & MK_LBUTTON )
	bst |= Qt::LeftButton;
    if ( s & MK_MBUTTON )
	bst |= Qt::MidButton;
    if ( s & MK_RBUTTON )
	bst |= Qt::RightButton;
    if ( s & MK_SHIFT )
	bst |= Qt::ShiftButton;
    if ( s & MK_CONTROL )
	bst |= Qt::ControlButton;

    if ( s & MK_XBUTTON1 )
	bst |= Qt::MidButton*2;//### Qt::XButton1;
    if ( s & MK_XBUTTON2 )
	bst |= Qt::MidButton*4;//### Qt::XButton2;

    if ( GetKeyState(VK_MENU) < 0 )
	bst |= Qt::AltButton;

    if ( (GetKeyState(VK_LWIN) < 0) ||
	 (GetKeyState(VK_RWIN) < 0) )
 	bst |= Qt::MetaButton;

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
/*! \internal */
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

    // Compress mouse move events
    if ( msg.message == WM_MOUSEMOVE ) {
	MSG mouseMsg;
	while ( winPeekMessage( &mouseMsg, msg.hwnd, WM_MOUSEFIRST,
		WM_MOUSELAST, PM_NOREMOVE ) ) {
	    if ( mouseMsg.message == WM_MOUSEMOVE ) {
#define PEEKMESSAGE_IS_BROKEN 1
#ifdef PEEKMESSAGE_IS_BROKEN
		// Since the Windows PeekMessage() function doesn't
		// correctly return the wParam for WM_MOUSEMOVE events
		// if there is a key release event in the queue
		// _before_ the mouse event, we have to also consider
		// key release events (kls 2003-05-13):
		MSG keyMsg;
		bool done = FALSE;
		while ( winPeekMessage( &keyMsg, 0, WM_KEYFIRST, WM_KEYLAST,
			PM_NOREMOVE ) ) {
		    if ( keyMsg.time < mouseMsg.time ) {
			if ( (keyMsg.lParam & 0xC0000000) == 0x40000000 ) {
			    winPeekMessage( &keyMsg, 0, keyMsg.message,
					    keyMsg.message, PM_REMOVE );
			} else {
			    done = TRUE;
			    break;
			}
		    } else {
			break; // no key event before the WM_MOUSEMOVE event
		    }
		}
		if ( done )
		    break;
#else
		// Actually the following 'if' should work instead of
		// the above key event checking, but apparently
		// PeekMessage() is broken :-(
		if ( mouseMsg.wParam != msg.wParam )
		    break; // leave the message in the queue because
			   // the key state has changed
#endif
		MSG *msgPtr = (MSG *)(&msg);
		// Update the passed in MSG structure with the
		// most recent one.
		msgPtr->lParam = mouseMsg.lParam;
		msgPtr->wParam = mouseMsg.wParam;
		msgPtr->pt = mouseMsg.pt;
		// Remove the mouse move message
		winPeekMessage( &mouseMsg, msg.hwnd, WM_MOUSEMOVE,
				WM_MOUSEMOVE, PM_REMOVE );
	    } else {
		break; // there was no more WM_MOUSEMOVE event
	    }
	}
    }


    for ( i=0; (UINT)mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3 )
	;
    if ( !mouseTbl[i] )
	return FALSE;
    type   = (QEvent::Type)mouseTbl[++i];	// event type
    button = mouseTbl[++i];			// which button
    if ( button > Qt::MidButton ) {
	switch( GET_XBUTTON_WPARAM( msg.wParam ) ) {
	case XBUTTON1:
	    button = Qt::MidButton*2; //### XButton1;
	    break;
	case XBUTTON2:
	    button = Qt::MidButton*4; //### XButton2;
	    break;
	}
    }
    state  = translateButtonState( msg.wParam, type, button ); // button state
    if ( type == QEvent::MouseMove ) {
	if ( !(state & MouseButtonMask) )
	    qt_button_down = 0;
	QCursor *c = qt_grab_cursor();
	if ( !c )
	    c = QApplication::overrideCursor();
	if ( c )				// application cursor defined
	    SetCursor( c->handle() );
	else if ( isEnabled() )		// use widget cursor if widget is enabled
	    SetCursor( cursor().handle() );
	else {
	    QWidget *parent = parentWidget();
	    while ( parent && !parent->isEnabled() )
		parent = parent->parentWidget();
	    if ( parent )
		SetCursor( parent->cursor().handle() );
	}
	if ( curWin != winId() ) {		// new current window
	    qt_dispatchEnterLeave( this, QWidget::find(curWin) );
	    curWin = winId();
#ifndef Q_OS_TEMP
	    static bool trackMouseEventLookup = FALSE;
	    typedef BOOL (WINAPI *PtrTrackMouseEvent)(LPTRACKMOUSEEVENT);
	    static PtrTrackMouseEvent ptrTrackMouseEvent = 0;
	    if ( !trackMouseEventLookup ) {
		trackMouseEventLookup = TRUE;
		ptrTrackMouseEvent = (PtrTrackMouseEvent)QLibrary::resolve( "comctl32", "_TrackMouseEvent" );
	    }
	    if ( ptrTrackMouseEvent && !qApp->inPopupMode() ) {
		// We always have to set the tracking, since
		// Windows detects more leaves than we do..
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = 0x00000002;    // TME_LEAVE
		tme.hwndTrack = curWin;      // Track on window receiving msgs
		tme.dwHoverTime = (DWORD)-1; // HOVER_DEFAULT
		ptrTrackMouseEvent( &tme );
	    }
#endif // Q_OS_TEMP
	}

	POINT curPos = msg.pt;
	if ( curPos.x == gpos.x && curPos.y == gpos.y )
	    return TRUE;			// same global position
	gpos = curPos;

	ScreenToClient( winId(), &curPos );

	pos.rx() = (short)curPos.x;
	pos.ry() = (short)curPos.y;
    } else {
	gpos = msg.pt;
	pos = mapFromGlobal( QPoint(gpos.x, gpos.y) );

	if ( type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick ) {	// mouse button pressed
	    // Magic for masked widgets
	    qt_button_down = findChildWidget( this, pos );
	    if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
		qt_button_down = this;
	}
    }

    if ( qApp->inPopupMode() ) {			// in popup mode
	replayPopupMouseEvent = FALSE;
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
	    QApplication::sendSpontaneousEvent( popupButtonFocus, &e );
	    if ( releaseAfter ) {
		popupButtonFocus = 0;
	    }
	} else if ( popupChild ){
	    QMouseEvent e( type,
		popupChild->mapFromGlobal(QPoint(gpos.x,gpos.y)),
		QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendSpontaneousEvent( popupChild, &e );
	} else {
	    QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendSpontaneousEvent( popupChild ? popupChild : popup, &e );
	}

	if ( releaseAfter )
	    qt_button_down = 0;

	if ( type == QEvent::MouseButtonPress
	     && qApp->activePopupWidget() != activePopupWidget
	     && replayPopupMouseEvent ) {
	    // the popup dissappeared. Replay the event
	    QWidget* w = QApplication::widgetAt( gpos.x, gpos.y, TRUE );
	    if (w && !qt_blocked_modal( w ) ) {
		if ( QWidget::mouseGrabber() == 0 )
		    setAutoCapture( w->winId() );

		POINT widgetpt = gpos;
		ScreenToClient( w->winId(), &widgetpt );
		LPARAM lParam = MAKELPARAM( widgetpt.x, widgetpt.y );
		winPostMessage( w->winId(), msg.message, msg.wParam, lParam );
	    }
	}
    } else {					// not popup mode
	int bs = state & MouseButtonMask;
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
	     (state & (~button) & ( MouseButtonMask )) == 0 ) {
	    qt_button_down = 0;
	}

	QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	QApplication::sendSpontaneousEvent( widget, &e );
	if ( type == QEvent::MouseButtonRelease && button == RightButton ) {
	    QContextMenuEvent e2( QContextMenuEvent::Mouse, pos, QPoint(gpos.x,gpos.y), state );
	    QApplication::sendSpontaneousEvent( widget, &e2 );
	}

	if ( type != QEvent::MouseMove )
	    pos.rx() = pos.ry() = -9999;	// init for move compression
    }
    return TRUE;
}


//
// Keyboard event translation
//

static const uint KeyTbl[] = {		// keyboard mapping table
    VK_ESCAPE,		Qt::Key_Escape,		// misc keys
    VK_TAB,		Qt::Key_Tab,
    VK_BACK,		Qt::Key_Backspace,
    VK_RETURN,		Qt::Key_Return,
    VK_INSERT,		Qt::Key_Insert,
    VK_DELETE,		Qt::Key_Delete,
    VK_CLEAR,		Qt::Key_Clear,
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
    VK_LWIN,		Qt::Key_Meta,
    VK_RWIN,		Qt::Key_Meta,
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
	qWarning( "Qt: Internal keyboard buffer overflow" );
	return;
    }

    key_rec[nrecs++] = KeyRec(code,ascii,text);
}

static int asciiToKeycode(char a, int state)
{
    if ( a >= 'a' && a <= 'z' )
	a = toupper( a );
    if ( (state & Qt::ControlButton) != 0 ) {
	if ( a >= 1 && a <= 26 )	// Ctrl+'A'..'Z'
	    a += 'A' - 1;
    }

    return a & 0xff;
}

static
QChar wmchar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA( {
	return QChar( (ushort)c );
    } , {
	char mb[2];
	mb[0] = c&0xff;
	mb[1] = 0;
	WCHAR wc[1];
	MultiByteToWideChar( inputcharset, MB_PRECOMPOSED, mb, -1, wc, 1);
	return QChar(wc[0]);
    } );
}

static
QChar imechar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA( {
	return QChar( (ushort)c );
    } , {
	char mb[3];
	mb[0] = (c>>8)&0xff;
	mb[1] = c&0xff;
	mb[2] = 0;
	WCHAR wc[1];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
	    mb, -1, wc, 1);
	return QChar(wc[0]);
    } );
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
    if ( (GetKeyState(VK_LWIN) < 0) ||
	 (GetKeyState(VK_RWIN) < 0) )
 	state |= Qt::MetaButton;

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
    } else {
	extern bool qt_use_rtl_extensions;
	if ( qt_use_rtl_extensions ) {
	    // for Directionality changes (BiDi)
	    static int dirStatus = 0;
	    if ( !dirStatus && state == Qt::ControlButton && msg.wParam == VK_CONTROL && msg.message == WM_KEYDOWN ) {
		if ( GetKeyState( VK_LCONTROL ) < 0 ) {
		    dirStatus = VK_LCONTROL;
		} else if ( GetKeyState( VK_RCONTROL ) < 0 ) {
		    dirStatus = VK_RCONTROL;
		}
	    } else if ( dirStatus ) {
		if ( msg.message == WM_KEYDOWN ) {
		    if ( msg.wParam == VK_SHIFT ) {
			if ( dirStatus == VK_LCONTROL && GetKeyState( VK_LSHIFT ) < 0 ) {
				dirStatus = VK_LSHIFT;
			} else if ( dirStatus == VK_RCONTROL && GetKeyState( VK_RSHIFT ) < 0 ) {
    			    dirStatus = VK_RSHIFT;
			}
		    } else {
			dirStatus = 0;
		    }
		} else if ( msg.message == WM_KEYUP ) {
		    if ( dirStatus == VK_LSHIFT &&
			( msg.wParam == VK_SHIFT && GetKeyState( VK_LCONTROL )  ||
			  msg.wParam == VK_CONTROL && GetKeyState( VK_LSHIFT ) ) ) {
			k0 = sendKeyEvent( QEvent::KeyPress, Qt::Key_Direction_L, 0, 0, grab, QString::null );
			k1 = sendKeyEvent( QEvent::KeyRelease, Qt::Key_Direction_L, 0, 0, grab, QString::null );
			dirStatus = 0;
		    } else if ( dirStatus == VK_RSHIFT &&
			( msg.wParam == VK_SHIFT && GetKeyState( VK_RCONTROL ) ||
			  msg.wParam == VK_CONTROL && GetKeyState( VK_RSHIFT ) ) ) {
			k0 = sendKeyEvent( QEvent::KeyPress, Qt::Key_Direction_R, 0, 0, grab, QString::null );
			k1 = sendKeyEvent( QEvent::KeyRelease, Qt::Key_Direction_R, 0, 0, grab, QString::null );
			dirStatus = 0;
		    } else {
			dirStatus = 0;
		    }
		} else {
		    dirStatus = 0;
		}
	    }
	}

	int code = translateKeyCode( msg.wParam );
	// Invert state logic
	if ( code == Key_Alt )
	    state = state^AltButton;
	else if ( code == Key_Control )
	    state = state^ControlButton;
	else if ( code == Key_Shift )
	    state = state^ShiftButton;

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
		    if ( t != WM_SYSKEYDOWN || !code ) {
			UINT map;
			QT_WA( {
			    map = MapVirtualKey( msg.wParam, 2 );
			} , {
			    map = MapVirtualKeyA( msg.wParam, 2 );
			    // High-order bit is 0x8000 on '95
			    if ( map & 0x8000 )
				map = (map^0x8000)|0x80000000;
			} );
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

	    if ( state == Qt::AltButton ) {
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

	    // map shift+tab to shift+backtab, QAccel knows about it
	    // and will handle it
	    if ( code == Key_Tab && ( state & ShiftButton ) == ShiftButton )
		code = Key_BackTab;

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

		// see comment above
		if ( code == Key_Tab && ( state & ShiftButton ) == ShiftButton )
		    code = Key_BackTab;

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
	state |= Qt::ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= Qt::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= Qt::AltButton;
    if ( (GetKeyState(VK_LWIN) < 0) ||
	 (GetKeyState(VK_RWIN) < 0) )
 	state |= Qt::MetaButton;

    int delta;
    if ( msg.message == WM_MOUSEWHEEL )
	delta = (short) HIWORD ( msg.wParam );
    else
	delta = (int) msg.wParam;

    Orientation orient = ( state&AltButton
#if 0 // disabled for now - Trenton's one-wheel mouse makes trouble...
    // "delta" for usual wheels is +-120. +-240 seems to indicate the second wheel
    // see more recent MSDN for WM_MOUSEWHEEL

	 || delta == 240 || delta == -240 )?Horizontal:Vertical;
    if ( delta == 240 || delta == -240 )
	delta /= 2;
#endif
	)? Horizontal:Vertical;

    QPoint globalPos;

    globalPos.rx() = (short)LOWORD ( msg.lParam );
    globalPos.ry() = (short)HIWORD ( msg.lParam );


    // if there is a widget under the mouse and it is not shadowed
    // by modality, we send the event to it first
    int ret = 0;
    QWidget* w = QApplication::widgetAt( globalPos, TRUE );
    if ( !w || !qt_try_modal( w, (MSG*)&msg, ret ) )
	w = this;

    // send the event to the widget or its ancestors
    {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w->topLevelWidget() != popup )
	    popup->close();
	QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state, orient );
	if ( QApplication::sendSpontaneousEvent( w, &e ) )
	    return TRUE;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    if ( w != qApp->focusWidget() && ( w = qApp->focusWidget() ) ) {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w->topLevelWidget() != popup )
	    popup->close();
	QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state, orient );
	if ( QApplication::sendSpontaneousEvent( w, &e ) )
	    return TRUE;
    }
    return FALSE;
}

#if defined(QT_TABLET_SUPPORT)

//
// Windows Wintab to QTabletEvent translation
//

// the following is copied from the wintab syspress example (public domain)
/*------------------------------------------------------------------------------
The functions PrsInit and PrsAdjust make sure that our pressure out can always
reach the full 0-255 range we desire, regardless of the button pressed or the
"pressure button marks" settings.
------------------------------------------------------------------------------*/
/* pressure adjuster local state. */
/* need wOldCsr = -1, so PrsAdjust will call PrsInit first time */
static UINT wActiveCsr = 0,  wOldCsr = (UINT)-1;
static BYTE wPrsBtn;
static UINT prsYesBtnOrg, prsYesBtnExt, prsNoBtnOrg, prsNoBtnExt;
/* -------------------------------------------------------------------------- */
static void prsInit( HCTX hTab)
{
    /* browse WinTab's many info items to discover pressure handling. */
    if ( ptrWTInfo && ptrWTGet ) {
	AXIS np;
	LOGCONTEXT lc;
	BYTE logBtns[32];
	UINT btnMarks[2];
	UINT size;

	/* discover the LOGICAL button generated by the pressure channel. */
	/* get the PHYSICAL button from the cursor category and run it */
	/* through that cursor's button map (usually the identity map). */
	wPrsBtn = (BYTE)-1;
	ptrWTInfo(WTI_CURSORS + wActiveCsr, CSR_NPBUTTON, &wPrsBtn);
	size = ptrWTInfo(WTI_CURSORS + wActiveCsr, CSR_BUTTONMAP, &logBtns);
	if ((UINT)wPrsBtn < size)
	    wPrsBtn = logBtns[wPrsBtn];

	/* get the current context for its device variable. */
	ptrWTGet(hTab, &lc);

	/* get the size of the pressure axis. */
	ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_NPRESSURE, &np);
	prsNoBtnOrg = (UINT)np.axMin;
	prsNoBtnExt = (UINT)np.axMax - (UINT)np.axMin;

	/* get the button marks (up & down generation thresholds) */
	/* and calculate what pressure range we get when pressure-button is down. */
	btnMarks[1] = 0; /* default if info item not present. */
	ptrWTInfo(WTI_CURSORS + wActiveCsr, CSR_NPBTNMARKS, btnMarks);
	prsYesBtnOrg = btnMarks[1];
	prsYesBtnExt = (UINT)np.axMax - btnMarks[1];
    }
}
/* -------------------------------------------------------------------------- */
static UINT prsAdjust(PACKET p, HCTX hTab)
{
    UINT wResult;

    wActiveCsr = p.pkCursor;
    if (wActiveCsr != wOldCsr) {

	/* re-init on cursor change. */
	prsInit( hTab );
	wOldCsr = wActiveCsr;
    }

    /* scaling output range is 0-255 */

    if (p.pkButtons & (1 << wPrsBtn)) {
	/* if pressure-activated button is down, */
	/* scale pressure output to compensate btn marks */
	wResult = p.pkNormalPressure - prsYesBtnOrg;
	wResult = MulDiv(wResult, 255, prsYesBtnExt);
    } else {
	/* if pressure-activated button is not down, */
	/* scale pressure output directly */
	wResult = p.pkNormalPressure - prsNoBtnOrg;
	wResult = MulDiv(wResult, 255, prsNoBtnExt);
    }

    return wResult;
}


bool QETWidget::translateTabletEvent( const MSG &msg, PACKET *localPacketBuf,
				      int numPackets )
{
    POINT ptNew;
    static DWORD btnNew, btnOld, btnChange;
    UINT prsNew;
    ORIENTATION ort;
    static bool button_pressed = FALSE;
    int i,
	dev,
	tiltX,
	tiltY;
    bool sendEvent;
    QEvent::Type t;


    // the most typical event that we get...
    t = QEvent::TabletMove;
    for ( i = 0; i < numPackets; i++ ) {
	if ( localPacketBuf[i].pkCursor == 2 ) {
	    dev = QTabletEvent::Eraser;
	} else if ( localPacketBuf[i].pkCursor == 1 ){
	    dev = QTabletEvent::Stylus;
	} else {
	    dev = QTabletEvent::NoDevice;
	}
	btnOld = btnNew;
	btnNew = localPacketBuf[i].pkButtons;
	btnChange = btnOld ^ btnNew;

	if (btnNew & btnChange) {
	    button_pressed = TRUE;
	    t = QEvent::TabletPress;
	}
	ptNew.x = (UINT)localPacketBuf[i].pkX;
	ptNew.y = (UINT)localPacketBuf[i].pkY;
	prsNew = 0;
	if ( btnNew ) {
	    prsNew = prsAdjust( localPacketBuf[i], qt_tablet_context );
	} else if ( button_pressed ) {
	    // One button press, should only give one button release
	    t = QEvent::TabletRelease;
	    button_pressed = FALSE;
	}
	QPoint globalPos( ptNew.x, ptNew.y );

	// make sure the tablet event get's sent to the proper widget...
	QWidget *w = QApplication::widgetAt( globalPos, TRUE );
	if ( w == NULL )
	    w = this;
	QPoint localPos = w->mapFromGlobal( globalPos );
	if ( !qt_tablet_tilt_support )
	    tiltX = tiltY = 0;
	else {
	    ort = localPacketBuf[i].pkOrientation;
	    // convert from azimuth and altitude to x tilt and y tilt
	    // what follows is the optimized version.  Here are the equations
	    // I used to get to this point (in case things change :)
	    // X = sin(azimuth) * cos(altitude)
	    // Y = cos(azimuth) * cos(altitude)
	    // Z = sin(altitude)
	    // X Tilt = arctan(X / Z)
	    // Y Tilt = arctan(Y / Z)
	    double radAzim = ( ort.orAzimuth / 10 ) * ( PI / 180 );
	    //double radAlt = abs( ort.orAltitude / 10 ) * ( PI / 180 );
	    double tanAlt = tan( (abs(ort.orAltitude / 10)) * ( PI / 180 ) );

	    double degX = atan( sin(radAzim) / tanAlt );
	    double degY = atan( cos(radAzim) / tanAlt );
	    tiltX = degX * ( 180 / PI );
	    tiltY = -degY * ( 180 / PI );
	}
        // get the unique ID of the device...
        int csr_type,
	    csr_physid;
	ptrWTInfo( WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_TYPE, &csr_type );
	ptrWTInfo( WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_PHYSID, &csr_physid );
	QPair<int,int> llId( csr_type, csr_physid );
	QTabletEvent e( t, localPos, globalPos, dev, prsNew, tiltX, tiltY, llId );
	sendEvent = QApplication::sendSpontaneousEvent( w, &e );
    }
    return sendEvent;
}

extern bool qt_is_gui_used;
static void initWinTabFunctions()
{
    if ( !qt_is_gui_used )
	return;
    QLibrary library( "wintab32" );
    library.setAutoUnload( FALSE );
    QT_WA( {
	ptrWTInfo = (PtrWTInfo)library.resolve( "WTInfoW" );
	ptrWTGet = (PtrWTGet)library.resolve( "WTGetW" );
    } , {
	ptrWTInfo = (PtrWTInfo)library.resolve( "WTInfoA" );
	ptrWTGet = (PtrWTGet)library.resolve( "WTGetA" );
    } );

    ptrWTEnable = (PtrWTEnable)library.resolve( "WTEnable" );
    ptrWTOverlap = (PtrWTEnable)library.resolve( "WTOverlap" );
    ptrWTPacketsGet = (PtrWTPacketsGet)library.resolve( "WTPacketsGet" );
}
#endif

static bool isModifierKey(int code)
{
    return code >= Qt::Key_Shift && code <= Qt::Key_ScrollLock;
}

// ###### remove ascii parameter
bool QETWidget::sendKeyEvent( QEvent::Type type, int code, int ascii,
			      int state, bool grab, const QString& text,
			      bool autor )
{
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel events if the keyboard is not grabbed
	QKeyEvent a( type, code, state, text, autor, QMAX(1, int(text.length())) );
	if ( qt_tryAccelEvent( this, &a ) )
	    return TRUE;
    }
    if ( !isEnabled() )
	return FALSE;
    QKeyEvent e( type, code, state, text, autor, QMAX(1, int(text.length())) );
    QApplication::sendSpontaneousEvent( this, &e );
    if ( !isModifierKey(code) && state == Qt::AltButton
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
    QRegion rgn(0,0,1,1); // trigger handle
    int res = GetUpdateRgn(winId(), (HRGN) rgn.handle(), FALSE);
    if ( res == ERROR )
	return TRUE;

    setWState( WState_InPaintEvent );
    PAINTSTRUCT ps;
    hdc = BeginPaint( winId(), &ps );
    if ( res != COMPLEXREGION ) {
	QRect psRect(QPoint(ps.rcPaint.left,ps.rcPaint.top), QPoint(ps.rcPaint.right-1,ps.rcPaint.bottom-1));
	if ( !psRect.isValid() )
	    goto cleanup;
	rgn = psRect;
    }
    {
	QPaintEvent e(rgn);
	QApplication::sendSpontaneousEvent( this, (QEvent*) &e );
    }
cleanup:
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
	    d->createTLExtra();
	    if ( msg.wParam == SIZE_MINIMIZED ) {
		// being "hidden"
		d->extra->topextra->iconic = 1;
		if ( isVisible() ) {
		    QHideEvent e;
		    QApplication::sendSpontaneousEvent( this, &e );
		    hideChildren( TRUE );
		}
	    } else if ( d->extra->topextra->iconic ) {
		// being shown
		d->extra->topextra->iconic = 0;
		showChildren( TRUE );
		QShowEvent e;
		QApplication::sendSpontaneousEvent( this, &e );
	    }
	    QString txt;
#ifndef Q_OS_TEMP
	    if ( IsIconic(winId()) && !!iconText() )
		txt = iconText();
	    else
#endif
		if ( !caption().isNull() )
		txt = caption();

	    if ( !!txt ) {
		QT_WA( {
		    SetWindowText( winId(), (TCHAR*)txt.ucs2() );
		} , {
		    SetWindowTextA( winId(), txt.local8Bit() );
		} );
	    }
	}
	if ( msg.wParam != SIZE_MINIMIZED && oldSize != newSize) {
	    if ( isVisible() ) {
		QResizeEvent e( newSize, oldSize );
		QApplication::sendSpontaneousEvent( this, &e );
		if ( !testAttribute(WA_StaticContents))
		    repaint();
	    } else {
		QResizeEvent *e = new QResizeEvent( newSize, oldSize );
		QApplication::postEvent( this, e );
	    }
	}
    } else if ( msg.message == WM_MOVE ) {	// move event
	int a = (int) (short) LOWORD(msg.lParam);
	int b = (int) (short) HIWORD(msg.lParam);
	QPoint oldPos = geometry().topLeft();
	QPoint newCPos( a, b );
	// Ignore silly Windows move event to wild pos after iconify.
	if ( !isMinimized() && newCPos != oldPos ) {
	    cr.moveTopLeft( newCPos );
	    crect = cr;
	    if ( isVisible() ) {
		QMoveEvent e( newCPos, oldPos );  // cpos (client position)
		QApplication::sendSpontaneousEvent( this, &e );
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
#ifndef Q_OS_TEMP
    SetDoubleClickTime( ms );
#endif
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
    QT_WA( {
	SystemParametersInfo( SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0 );
    } , {
	SystemParametersInfoA( SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0 );
    } );
#else
    wheel_scroll_lines = n;
#endif
}

int QApplication::wheelScrollLines()
{
#ifdef SPI_GETWHEELSCROLLLINES
    uint i = 3;
    QT_WA( {
	SystemParametersInfo( SPI_GETWHEELSCROLLLINES, sizeof( uint ), &i, 0 );
    } , {
	SystemParametersInfoA( SPI_GETWHEELSCROLLLINES, sizeof( uint ), &i, 0 );
    } );
    if ( i > INT_MAX )
	i = INT_MAX;
    return i;
#else
    return wheel_scroll_lines;
#endif
}

static bool effect_override = FALSE;

void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    effect_override = TRUE;
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
    case UI_AnimateToolBox:
	animate_toolbox = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }
    if ( desktopSettingsAware() && !( qt_winver == WV_95 || qt_winver == WV_NT ) ) {
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
	QT_WA( {
	    SystemParametersInfo( api, 0, (void*)onoff, 0 );
	}, {
	    SystemParametersInfoA( api, 0, (void*)onoff, 0 );
	} );
    }
}

bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( QColor::numBitPlanes() < 16 )
	return FALSE;

    if ( !effect_override && desktopSettingsAware() && !( qt_winver == WV_95 || qt_winver == WV_NT ) ) {
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
	QT_WA( {
	    SystemParametersInfo( api, 0, &enabled, 0 );
	} , {
	    SystemParametersInfoA( api, 0, &enabled, 0 );
	} );
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
	case UI_AnimateToolBox:
	    return animate_toolbox;
	default:
	    return animate_ui;
	}
    }
}

void QApplication::flush()
{
}

bool QSessionManager::allowsInteraction()
{
    sm_blockUserInput = FALSE;
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    sm_blockUserInput = FALSE;
    return TRUE;
}

void QSessionManager::release()
{
    if ( sm_smActive )
	sm_blockUserInput = TRUE;
}

void QSessionManager::cancel()
{
    sm_cancel = TRUE;
}
/*!
    \enum Qt::WindowsVersion

    \value WV_32s
    \value WV_95
    \value WV_98
    \value WV_Me
    \value WV_DOS_based

    \value WV_NT
    \value WV_2000
    \value WV_XP
    \value WV_NT_based

    \value WV_CE
    \value WV_CENET
    \value WV_CE_based
*/


