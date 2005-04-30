/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include <private/qeventdispatcher_win_p.h>
#include "qeventloop.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qpointer.h"
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
#include "qcolormap.h"
#include "qt_windows.h"
#if defined(QT_NON_COMMERCIAL)
#include "qnc_win.h"
#endif

#include <private/qwininputcontext_p.h>

#include <private/qpaintengine_win_p.h>
#include <private/qcursor_p.h>
#include <private/qmath_p.h>

#ifdef QT_THREAD_SUPPORT
#include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#include <winable.h>
#include <oleacc.h>
#ifndef WM_GETOBJECT
#define WM_GETOBJECT                    0x003D
#endif

extern IAccessible *qt_createWindowsAccessible(QAccessibleInterface *object);
#endif // QT_NO_ACCESSIBILITY

#include "private/qapplication_p.h"

#include "private/qinternal_p.h"

#include <windowsx.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef Q_OS_TEMP
#include <sipapi.h>
#endif

#define PACKETDATA  (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE \
                     | PK_ORIENTATION | PK_CURSOR | PK_Z)
#define PACKETMODE  0

extern bool qt_tabletChokeMouse;

#include <wintab.h>
#ifndef CSR_TYPE
#define CSR_TYPE 20 // Some old Wacom wintab.h may not provide this constant.
#endif
#include <pktdef.h>
#include <math.h>

typedef HCTX (API *PtrWTOpen)(HWND, LPLOGCONTEXT, BOOL);
typedef BOOL (API *PtrWTClose)(HCTX);
typedef UINT (API *PtrWTInfo)(UINT, UINT, LPVOID);
typedef BOOL (API *PtrWTEnable)(HCTX, BOOL);
typedef BOOL (API *PtrWTOverlap)(HCTX, BOOL);
typedef int  (API *PtrWTPacketsGet)(HCTX, int, LPVOID);
typedef BOOL (API *PtrWTGet)(HCTX, LPLOGCONTEXT);
typedef int  (API *PtrWTQueueSizeGet)(HCTX);
typedef BOOL (API *PtrWTQueueSizeSet)(HCTX, int);

static PtrWTInfo ptrWTInfo = 0;
static PtrWTEnable ptrWTEnable = 0;
static PtrWTOverlap ptrWTOverlap = 0;
static PtrWTPacketsGet ptrWTPacketsGet = 0;
static PtrWTGet ptrWTGet = 0;

static PACKET localPacketBuf[QT_TABLET_NPACKETQSIZE];  // our own tablet packet queue.
HCTX qt_tablet_context;  // the hardware context for the tablet (like a window handle)
bool qt_tablet_tilt_support;
static void tabletInit(UINT wActiveCsr, HCTX hTab);
static void initWinTabFunctions();        // resolve the WINTAB api functions

Q_CORE_EXPORT bool winPeekMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                            UINT wMsgFilterMax, UINT wRemoveMsg);
Q_CORE_EXPORT bool winPostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#if defined(__CYGWIN32__)
#define __INSIDE_CYGWIN32__
#include <mywinsock.h>
#endif

// support for on-the-fly changes of the XP theme engine
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif
#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT                29
#define COLOR_MENUBAR                        30
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

static int translateButtonState(int s, int type, int button);

/*
  Internal functions.
*/

void qt_draw_tiled_pixmap(HDC, int, int, int, int,
                           const QPixmap *, int, int);

void qt_erase_background(HDC hdc, int x, int y, int w, int h,
                         const QBrush &brush, int off_x, int off_y,
                         QWidget *widget)
{
    if (brush.style() == Qt::TexturePattern && brush.texture().isNull())        // empty background
        return;
    HPALETTE oldPal = 0;
    HPALETTE hpal = QColormap::hPal();
    if (hpal) {
        oldPal = SelectPalette(hdc, hpal, false);
        RealizePalette(hdc);
    }
    if (brush.style() == Qt::LinearGradientPattern) {
        QPainter p(widget);
        p.fillRect(x, y, w, h, brush);
        return;
    } else if (brush.style() == Qt::TexturePattern) {
        QPixmap texture = brush.texture();
        qt_draw_tiled_pixmap(hdc, x, y, w, h, &texture, off_x, off_y);
    } else {
        QColor c = brush.color();
        HBRUSH hbrush = CreateSolidBrush(RGB(c.red(), c.green(), c.blue()));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hbrush);
        PatBlt(hdc, x, y, w, h, PATCOPY);
        SelectObject(hdc, oldBrush);
        DeleteObject(hbrush);
    }
    if (hpal) {
        SelectPalette(hdc, oldPal, true);
        RealizePalette(hdc);
    }
}

// ##### get rid of this!
QRgb qt_colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

extern Q_CORE_EXPORT char      appName[];
extern Q_CORE_EXPORT char      appFileName[];
extern Q_CORE_EXPORT HINSTANCE appInst;                        // handle to app instance
extern Q_CORE_EXPORT HINSTANCE appPrevInst;                        // handle to prev app instance
extern Q_CORE_EXPORT int appCmdShow;                                // main window show command
static HWND         curWin                = 0;                // current window
static HDC         displayDC        = 0;                // display device context
#ifdef Q_OS_TEMP
static UINT         appUniqueID        = 0;                // application id
#endif

// Session management
static bool        sm_blockUserInput    = false;
static bool        sm_smActive             = false;
extern QSessionManager* qt_session_manager_self;
static bool        sm_cancel;

static bool replayPopupMouseEvent = false; // replay handling when popups close

// ignore the next release event if return from a modal widget
Q_GUI_EXPORT bool qt_win_ignoreNextMouseReleaseEvent = false;

#if defined(QT_DEBUG)
static bool        appNoGrab        = false;        // mouse/keyboard grabbing
#endif

static bool        app_do_modal           = false;        // modal mode
extern QWidgetList *qt_modal_stack;
extern QDesktopWidget *qt_desktopWidget;
static QWidget *popupButtonFocus   = 0;
static bool        qt_try_modal(QWidget *, MSG *, int& ret);

QWidget               *qt_button_down = 0;                // widget got last button-down

static HWND        autoCaptureWnd = 0;
static void        setAutoCapture(HWND);                // automatic capture
static void        releaseAutoCapture();

static void     unregWinClasses();

static int        translateKeyCode(int);

extern QCursor *qt_grab_cursor();

#if defined(Q_WS_WIN)
#define __export
#endif

extern "C" LRESULT CALLBACK QtWndProc(HWND, UINT, WPARAM, LPARAM);

class QETWidget : public QWidget                // event translator widget
{
public:
    QWExtra    *xtra() { return d_func()->extraData(); }
    QTLWExtra  *topData() { return d_func()->topData(); }
    QWidgetData *dataPtr() { return data; }
    bool        winEvent(MSG *m, long *r)        { return QWidget::winEvent(m, r); }
    void        markFrameStrutDirty()        { data->fstrut_dirty = 1; }
    bool        translateMouseEvent(const MSG &msg);
    bool        translateKeyEvent(const MSG &msg, bool grab);
    bool        translateWheelEvent(const MSG &msg);
    bool        sendKeyEvent(QEvent::Type type, int code,
                              int state, bool grab, const QString& text,
                              bool autor=false);
    bool        translatePaintEvent(const MSG &msg);
    bool        translateConfigEvent(const MSG &msg);
    bool        translateCloseEvent(const MSG &msg);
    bool        translateTabletEvent(const MSG &msg, PACKET *localPacketBuf, int numPackets);
    void        repolishStyle(QStyle &style) { setStyle(&style); }
    void eraseWindowBackground(HDC);
    inline void showChildren(bool spontaneous) { d_func()->showChildren(spontaneous); }
    inline void hideChildren(bool spontaneous) { d_func()->hideChildren(spontaneous); }
};

static void qt_show_system_menu(QWidget* tlw)
{
    HMENU menu = GetSystemMenu(tlw->winId(), false);
    if (!menu)
        return; // no menu for this window

#define enabled (MF_BYCOMMAND | MF_ENABLED)
#define disabled (MF_BYCOMMAND | MF_GRAYED)

#ifndef Q_OS_TEMP
    EnableMenuItem(menu, SC_MINIMIZE, (tlw->windowFlags() & Qt::WindowMinimizeButtonHint)?enabled:disabled);
    bool maximized = IsZoomed(tlw->winId());

    EnableMenuItem(menu, SC_MAXIMIZE, ! (tlw->windowFlags() & Qt::WindowMaximizeButtonHint) || maximized?disabled:enabled);
    EnableMenuItem(menu, SC_RESTORE, maximized?enabled:disabled);

    EnableMenuItem(menu, SC_SIZE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_MOVE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_CLOSE, enabled);
#endif

#undef enabled
#undef disabled

    int ret = TrackPopupMenuEx(menu,
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

static void qt_set_windows_resources()
{
#ifndef Q_OS_TEMP
    QFont menuFont;
    QFont messageFont;
    QFont statusFont;
    QFont titleFont;
    QFont smallTitleFont;

    QT_WA({
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        menuFont = qt_LOGFONTtoQFont(ncm.lfMenuFont,true);
        messageFont = qt_LOGFONTtoQFont(ncm.lfMessageFont,true);
        statusFont = qt_LOGFONTtoQFont(ncm.lfStatusFont,true);
        titleFont = qt_LOGFONTtoQFont(ncm.lfCaptionFont,true);
        smallTitleFont = qt_LOGFONTtoQFont(ncm.lfSmCaptionFont,true);
    } , {
        // A version
        NONCLIENTMETRICSA ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        menuFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMenuFont,true);
        messageFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMessageFont,true);
        statusFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfStatusFont,true);
        titleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfCaptionFont,true);
        smallTitleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfSmCaptionFont,true);
    });

    QApplication::setFont(menuFont, "QMenu");
    QApplication::setFont(menuFont, "QMenuBar");
    QApplication::setFont(messageFont, "QMessageBox");
    QApplication::setFont(statusFont, "QTipLabel");
    QApplication::setFont(statusFont, "QStatusBar");
    QApplication::setFont(titleFont, "QTitleBar");
    QApplication::setFont(smallTitleFont, "QDockWidgetTitle");
#else
    LOGFONT lf;
    HGDIOBJ stockFont = GetStockObject(SYSTEM_FONT);
    GetObject(stockFont, sizeof(lf), &lf);
    QApplication::setFont(qt_LOGFONTtoQFont(lf, true));
#endif// Q_OS_TEMP

    // Do the color settings
    QPalette pal;
    pal.setColor(QPalette::Foreground,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))));
    pal.setColor(QPalette::Button,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))));
    pal.setColor(QPalette::Light,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))));
    pal.setColor(QPalette::Dark,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNSHADOW))));
    pal.setColor(QPalette::Mid, pal.button().color().dark(150));
    pal.setColor(QPalette::Text,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))));
    pal.setColor(QPalette::BrightText,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))));
    pal.setColor(QPalette::Base,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOW))));
    pal.setColor(QPalette::Background,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))));
    pal.setColor(QPalette::ButtonText,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNTEXT))));
    pal.setColor(QPalette::Midlight,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DLIGHT))));
    pal.setColor(QPalette::Shadow,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DDKSHADOW))));
    pal.setColor(QPalette::Highlight,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))));
    pal.setColor(QPalette::HighlightedText,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))));
    // ### hardcoded until I find out how to get it from the system settings.
    pal.setColor(QPalette::Link, Qt::blue);
    pal.setColor(QPalette::LinkVisited, Qt::magenta);

    if (QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95) {
        if (pal.midlight() == pal.button())
            pal.setColor(QPalette::Midlight, pal.button().color().light(110));
        if (pal.background() != pal.base()) {
            pal.setColor(QPalette::Inactive, QPalette::Highlight, pal.color(QPalette::Inactive, QPalette::Background));
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText, pal.color(QPalette::Inactive, QPalette::Text));
        }
    }

    const QColor fg = pal.foreground().color(), btn = pal.button().color();
    QColor disabled((fg.red()+btn.red())/2,(fg.green()+btn.green())/2,
                     (fg.blue()+btn.blue())/2);
    pal.setColor(QPalette::Disabled, QPalette::Foreground, disabled);
    pal.setColor(QPalette::Disabled, QPalette::Text, disabled);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    pal.setColor(QPalette::Disabled, QPalette::Highlight,
                  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText,
                  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))));

    QApplicationPrivate::setSystemPalette(pal);

    QColor menuCol(qt_colorref2qrgb(GetSysColor(COLOR_MENU)));
    QColor menuText(qt_colorref2qrgb(GetSysColor(COLOR_MENUTEXT)));
    {
        BOOL isFlat = 0;
        if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
            SystemParametersInfo(0x1022 /*SPI_GETFLATMENU*/, 0, &isFlat, 0);
        QPalette menu(pal);
        // we might need a special color group for the menu.
        menu.setColor(QPalette::Active, QPalette::Button, menuCol);
        menu.setColor(QPalette::Active, QPalette::Text, menuText);
        menu.setColor(QPalette::Active, QPalette::Foreground, menuText);
        menu.setColor(QPalette::Active, QPalette::ButtonText, menuText);
        const QColor fg = menu.foreground().color(), btn = menu.button().color();
        QColor disabled(qt_colorref2qrgb(GetSysColor(COLOR_GRAYTEXT)));
        menu.setColor(QPalette::Disabled, QPalette::Foreground, disabled);
        menu.setColor(QPalette::Disabled, QPalette::Text, disabled);
        menu.setColor(QPalette::Disabled, QPalette::Highlight,
                       QColor(qt_colorref2qrgb(GetSysColor(
                                               QSysInfo::WindowsVersion == QSysInfo::WV_XP
                                               && isFlat ? COLOR_MENUHILIGHT
                                                         : COLOR_HIGHLIGHT))));
        menu.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);
        menu.setColor(QPalette::Disabled, QPalette::Button,
                      menu.color(QPalette::Active, QPalette::Button));
        menu.setColor(QPalette::Inactive, QPalette::Button,
                      menu.color(QPalette::Active, QPalette::Button));
        menu.setColor(QPalette::Inactive, QPalette::Text,
                      menu.color(QPalette::Active, QPalette::Text));
        menu.setColor(QPalette::Inactive, QPalette::Foreground,
                      menu.color(QPalette::Active, QPalette::Foreground));
        menu.setColor(QPalette::Inactive, QPalette::ButtonText,
                      menu.color(QPalette::Active, QPalette::ButtonText));
        if (QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95)
            menu.setColor(QPalette::Inactive, QPalette::ButtonText,
                          pal.color(QPalette::Inactive, QPalette::Dark));
        QApplication::setPalette(menu, "QMenu");

        if (QSysInfo::WindowsVersion == QSysInfo::WV_XP && isFlat) {
            QColor menubar(qt_colorref2qrgb(GetSysColor(COLOR_MENUBAR)));
            menu.setColor(QPalette::Active, QPalette::Button, menubar);
            menu.setColor(QPalette::Disabled, QPalette::Button, menubar);
            menu.setColor(QPalette::Inactive, QPalette::Button, menubar);
        }
        QApplication::setPalette(menu, "QMenuBar");
    }

    QColor ttip(qt_colorref2qrgb(GetSysColor(COLOR_INFOBK)));

    QColor ttipText(qt_colorref2qrgb(GetSysColor(COLOR_INFOTEXT)));
    {
        QPalette tiplabel(pal);
        tiplabel.setColor(QPalette::All, QPalette::Button, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Background, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Text, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::Foreground, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::ButtonText, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::Button, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Background, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Text, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::Foreground, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::ButtonText, ttipText);
        const QColor fg = tiplabel.foreground().color(), btn = tiplabel.button().color();
        QColor disabled((fg.red()+btn.red())/2,(fg.green()+btn.green())/2,
                         (fg.blue()+btn.blue())/2);
        tiplabel.setColor(QPalette::Disabled, QPalette::Foreground, disabled);
        tiplabel.setColor(QPalette::Disabled, QPalette::Text, disabled);
        tiplabel.setColor(QPalette::Disabled, QPalette::Base, Qt::white);
        tiplabel.setColor(QPalette::Disabled, QPalette::BrightText, Qt::white);
        QApplication::setPalette(tiplabel, "QTipLabel");
    }
}

/*****************************************************************************
  qt_init() - initializes Qt for Windows
 *****************************************************************************/

// need to get default font?
extern bool qt_app_has_font;

void qt_init(QApplicationPrivate *priv, int)
{

#if defined(QT_DEBUG)
    int argc = priv->argc;
    char **argv = priv->argv;
    int i, j;

  // Get command line params

    j = argc ? 1 : 0;
    for (i=1; i<argc; i++) {
        if (argv[i] && *argv[i] != '-') {
            argv[j++] = argv[i];
            continue;
        }
        if (qstrcmp(argv[i], "-nograb") == 0)
            appNoGrab = !appNoGrab;
        else
            argv[j++] = argv[i];
    }
    priv->argc = j;
#else
    Q_UNUSED(priv);
#endif // QT_DEBUG

    // Get the application name/instance if qWinMain() was not invoked
#ifndef Q_OS_TEMP
    // No message boxes but important ones
    SetErrorMode(SetErrorMode(0) | SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
#endif

    if (appInst == 0) {
        QT_WA({
            appInst = GetModuleHandle(0);
        }, {
            appInst = GetModuleHandleA(0);
        });
    }

#ifndef Q_OS_TEMP
    // Initialize OLE/COM
    //         S_OK means success and S_FALSE means that it has already
    //         been initialized
    HRESULT r;
    r = OleInitialize(0);
    if (r != S_OK && r != S_FALSE) {
        qWarning("Qt: Could not initialize OLE (error %x)", (unsigned int)r);
    }
#endif

    // Misc. initialization
#if defined(QT_DEBUG)
    GdiSetBatchLimit(1);
#endif

    QColormap::initialize();
    QFont::initialize();
    QCursorData::initialize();
    QWin32PaintEngine::initialize();
    qApp->setObjectName(appName);

    // default font
    if (!qt_app_has_font) {
        HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        QFont f("MS Sans Serif",8);
        QT_WA({
            LOGFONT lf;
            if (GetObject(hfont, sizeof(lf), &lf))
                f = qt_LOGFONTtoQFont((LOGFONT&)lf,true);
        } , {
            LOGFONTA lf;
            if (GetObjectA(hfont, sizeof(lf), &lf))
                f = qt_LOGFONTtoQFont((LOGFONT&)lf,true);
        });
        QApplication::setFont(f);
    }

    // QFont::locale_init();  ### Uncomment when it does something on Windows

    if (QApplication::desktopSettingsAware())
        qt_set_windows_resources();

    QT_WA({
        WM95_MOUSEWHEEL = RegisterWindowMessage(L"MSWHEEL_ROLLMSG");
    } , {
        WM95_MOUSEWHEEL = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
    });
    initWinTabFunctions();
    QApplicationPrivate::inputContext = new QWinInputContext(0);
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    unregWinClasses();
    QPixmapCache::clear();
    QWin32PaintEngine::cleanup();

    QCursorData::cleanup();
    QFont::cleanup();
    QColormap::cleanup();
    if (displayDC) {
        ReleaseDC(0, displayDC);
        displayDC = 0;
    }

    delete QApplicationPrivate::inputContext;
    QApplicationPrivate::inputContext = 0;

#ifndef Q_OS_TEMP
  // Deinitialize OLE/COM
    OleUninitialize();
#endif
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

Q_GUI_EXPORT int qWinAppCmdShow()                        // get main window show command
{
    return appCmdShow;
}


Q_GUI_EXPORT HDC qt_win_display_dc()                        // get display DC
{
    if (!displayDC)
        displayDC = GetDC(0);
    return displayDC;
}

bool qt_nograb()                                // application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return false;
#endif
}

typedef QHash<QString, int> WinClassNameHash;
Q_GLOBAL_STATIC(WinClassNameHash, winclassNames)

const QString qt_reg_winclass(Qt::WFlags flags)        // register window class
{
    int type = flags & Qt::WindowType_Mask;

    uint style;
    bool icon;
    QString cname;
    if (flags & Qt::MSWindowsOwnDC) {
        cname = "QWidgetOwnDC";
        style = CS_DBLCLKS;
#ifndef Q_OS_TEMP
        style |= CS_OWNDC;
#endif
        icon  = true;
    } else if (type == Qt::Tool || type == Qt::ToolTip){
	cname = "QTool";
	style = CS_DBLCLKS;
#ifndef Q_OS_TEMP
	style |= CS_SAVEBITS;
#endif
	icon = false;
    } else if (type == Qt::Popup) {
        cname = "QPopup";
        style = CS_DBLCLKS;
#ifndef Q_OS_TEMP
        style |= CS_SAVEBITS;
#endif
        if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
            style |= 0x00020000;                // CS_DROPSHADOW
        icon = false;
    } else {
        cname = "QWidget";
        style = CS_DBLCLKS;
        icon  = true;
    }

#ifdef Q_OS_TEMP
    // We need to register the classes with the
    // unique ID on WinCE to make sure we can
    // move the windows to the front when starting
    // a second instance.
    cname = QString::number(appUniqueID);
#endif

    // since multiple Qt versions can be used in one process
    // each one has to have window class names with a unique name
    // The first instance gets the unmodified name; if the class
    // has already been registered by another instance of Qt then
    // add an instance-specific ID, the address of the window proc.
    static int classExists = -1;

    if (classExists == -1) {
        QT_WA({
            WNDCLASS wcinfo;
            classExists = GetClassInfo((HINSTANCE)qWinAppInst(), (TCHAR*)cname.utf16(), &wcinfo);
            classExists = classExists && wcinfo.lpfnWndProc != QtWndProc;
        }, {
            WNDCLASSA wcinfo;
            classExists = GetClassInfoA((HINSTANCE)qWinAppInst(), cname.toLatin1(), &wcinfo);
            classExists = classExists && wcinfo.lpfnWndProc != QtWndProc;
        });
    }

    if (classExists)
        cname += QString::number((uint)QtWndProc);

    if (winclassNames()->contains(cname))        // already registered in our list
        return cname;

    ATOM atom;
#ifndef Q_OS_TEMP
    QT_WA({
        WNDCLASS wc;
        wc.style        = style;
        wc.lpfnWndProc        = (WNDPROC)QtWndProc;
        wc.cbClsExtra        = 0;
        wc.cbWndExtra        = 0;
        wc.hInstance        = (HINSTANCE)qWinAppInst();
        if (icon) {
            wc.hIcon = LoadIcon(appInst, L"IDI_ICON1");
            if (!wc.hIcon)
                wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        } else {
            wc.hIcon = 0;
        }
        wc.hCursor        = 0;
        wc.hbrBackground= 0;
        wc.lpszMenuName        = 0;
        wc.lpszClassName= (TCHAR*)cname.utf16();
        atom = RegisterClass(&wc);
    } , {
        WNDCLASSA wc;
        wc.style        = style;
        wc.lpfnWndProc        = (WNDPROC)QtWndProc;
        wc.cbClsExtra        = 0;
        wc.cbWndExtra        = 0;
        wc.hInstance        = (HINSTANCE)qWinAppInst();
        if (icon) {
            wc.hIcon = LoadIconA(appInst, (char*)"IDI_ICON1");
            if (!wc.hIcon)
                wc.hIcon = LoadIconA(0, (char*)IDI_APPLICATION);
        } else {
            wc.hIcon = 0;
        }
        wc.hCursor        = 0;
        wc.hbrBackground= 0;
        wc.lpszMenuName        = 0;
	  QByteArray tempArray = cname.toLatin1();
        wc.lpszClassName= tempArray;
        atom = RegisterClassA(&wc);
    });
#else
        WNDCLASS wc;
        wc.style        = style;
        wc.lpfnWndProc        = (WNDPROC)QtWndProc;
        wc.cbClsExtra        = 0;
        wc.cbWndExtra        = 0;
        wc.hInstance        = (HINSTANCE)qWinAppInst();
        if (icon) {
            wc.hIcon = LoadIcon(appInst, L"IDI_ICON1");
//            if (!wc.hIcon)
//                wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        } else {
            wc.hIcon = 0;
        }
        wc.hCursor        = 0;
        wc.hbrBackground= 0;
        wc.lpszMenuName        = 0;
        wc.lpszClassName= (TCHAR*)cname.utf16();
        atom = RegisterClass(&wc);
#endif

#ifndef QT_NO_DEBUG
    if (!atom)
        qErrnoWarning("QApplication::regClass: Registering window class failed.");
#endif

    winclassNames()->insert(cname, 1);
    return cname;
}

static void unregWinClasses()
{
    WinClassNameHash *hash = winclassNames();
    QHash<QString, int>::ConstIterator it = hash->constBegin();
    while (it != hash->constEnd()) {
        QT_WA({
            UnregisterClass((TCHAR*)it.key().utf16(), (HINSTANCE)qWinAppInst());
        } , {
            UnregisterClassA(it.key().toLatin1(), (HINSTANCE)qWinAppInst());
        });
        ++it;
    }
    hash->clear();
}


/*****************************************************************************
  Safe configuration (move,resize,setGeometry) mechanism to avoid
  recursion when processing messages.
 *****************************************************************************/

struct QWinConfigRequest {
    WId         id;                                        // widget to be configured
    int         req;                                        // 0=move, 1=resize, 2=setGeo
    int         x, y, w, h;                                // request parameters
};

static QList<QWinConfigRequest*> *configRequests = 0;

void qWinRequestConfig(WId id, int req, int x, int y, int w, int h)
{
    if (!configRequests)                        // create queue
        configRequests = new QList<QWinConfigRequest*>;
    QWinConfigRequest *r = new QWinConfigRequest;
    r->id = id;                                        // create new request
    r->req = req;
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    configRequests->append(r);                // store request in queue
}

Q_GUI_EXPORT void qWinProcessConfigRequests()                // perform requests in queue
{
    if (!configRequests)
        return;
    QWinConfigRequest *r;
    for (;;) {
        if (configRequests->isEmpty())
            break;
        r = configRequests->takeLast();
        QWidget *w = QWidget::find(r->id);
        QRect rect(r->x, r->y, r->w, r->h);
        int req = r->req;
        delete r;

	if ( w ) {				// widget exists
	    if (w->testAttribute(Qt::WA_WState_ConfigPending))
		return;				// biting our tail
	    if (req == 0)
		w->move(rect.topLeft());
	    else if (req == 1)
		w->resize(rect.size());
	    else
		w->setGeometry(rect);
	}
    }
    delete configRequests;
    configRequests = 0;
}


/*****************************************************************************
    GUI event dispatcher
 *****************************************************************************/

class QGuiEventDispatcherWin32 : public QEventDispatcherWin32
{
    Q_DECLARE_PRIVATE(QEventDispatcherWin32)
public:
    QGuiEventDispatcherWin32(QObject *parent = 0);
    bool processEvents(QEventLoop::ProcessEventsFlags flags);
};

QGuiEventDispatcherWin32::QGuiEventDispatcherWin32(QObject *parent)
    : QEventDispatcherWin32(parent)
{ }

bool QGuiEventDispatcherWin32::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    if (!QEventDispatcherWin32::processEvents(flags))
        return false;

    if (configRequests)                        // any pending configs?
        qWinProcessConfigRequests();
    QCoreApplication::sendPostedEvents();

    return true;
}

void QApplicationPrivate::createEventDispatcher()
{
    Q_Q(QApplication);
    if (q->type() != QApplication::Tty)
        eventDispatcher = new QGuiEventDispatcherWin32(q);
    else
        eventDispatcher = new QEventDispatcherWin32(q);
}

/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

#ifdef QT3_SUPPORT
void QApplication::setMainWidget(QWidget *mainWidget)
{
    QApplicationPrivate::main_widget = mainWidget;
    if (QApplicationPrivate::main_widget && windowIcon().isNull()
	&& QApplicationPrivate::main_widget->testAttribute(Qt::WA_SetWindowIcon))
        setWindowIcon(QApplicationPrivate::main_widget->windowIcon());
}
#endif

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

void QApplication::setOverrideCursor(const QCursor &cursor)
{
    qApp->d_func()->cursor_list.prepend(cursor);
    SetCursor(qApp->d_func()->cursor_list.first().handle());
}

void QApplication::restoreOverrideCursor()
{
    if (qApp->d_func()->cursor_list.isEmpty())
        return;
    qApp->d_func()->cursor_list.removeFirst();

    if (!qApp->d_func()->cursor_list.isEmpty()) {
        SetCursor(qApp->d_func()->cursor_list.first().handle());
    } else {
        QWidget *w = QWidget::find(curWin);
        if (w)
            SetCursor(w->cursor().handle());
        else
            SetCursor(QCursor(Qt::ArrowCursor).handle());
    }
}

#endif

/*
  Internal function called from QWidget::setCursor()
*/

void qt_win_set_cursor(QWidget *w, const QCursor& /* c */)
{
    if (!curWin)
        return;
    QWidget* cW = QWidget::find(curWin);
    if (!cW || cW->window() != w->window() ||
         !cW->isVisible() || !cW->underMouse())
        return;

    SetCursor(cW->cursor().handle());
}



/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget(const QWidget *p, const QPoint &pos)
{
    QObjectList children = p->children();
    for(int i = children.size(); i > 0 ;) {
        --i;
        QObject *o = children.at(i);
        if (o->isWidgetType()) {
            QWidget *w = (QWidget*)o;
            if (w->isVisible() && w->geometry().contains(pos)) {
                QWidget *c = findChildWidget(w, w->mapFromParent(pos));
                return c ? c : w;
            }
        }
    }
    return 0;
}

QWidget *QApplication::topLevelAt(const QPoint &p)
{
    QWidget *c = QApplicationPrivate::widgetAt_sys(p.x(), p.y());
    return c ? c->window() : 0;
}

QWidget *QApplicationPrivate::widgetAt_sys(int x, int y)
{
    POINT p;
    HWND  win;
    QWidget *w;
    p.x = x;
    p.y = y;
    win = WindowFromPoint(p);
    if (!win)
        return 0;

    w = QWidget::find(win);
    while (!w && win) {
        win = GetParent(win);
        w = QWidget::find(win);
    }
    return w;
}

void QApplication::beep()
{
    MessageBeep(MB_OK);
}

/*****************************************************************************
  Windows-specific drawing used here
 *****************************************************************************/

static void drawTile(HDC hdc, int x, int y, int w, int h,
                      const QPixmap *pixmap, int xOffset, int yOffset)
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    HDC tmp_hdc = pixmap->getDC();
    while(yPos < y + h) {
        drawH = pixmap->height() - yOff;        // Cropping first row
        if (yPos + drawH > y + h)                // Cropping last row
            drawH = y + h - yPos;
        xPos = x;
        xOff = xOffset;
        while(xPos < x + w) {
            drawW = pixmap->width() - xOff;        // Cropping first column
            if (xPos + drawW > x + w)                // Cropping last column
                drawW = x + w - xPos;
            BitBlt(hdc, xPos, yPos, drawW, drawH, tmp_hdc, xOff, yOff, SRCCOPY);
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
    pixmap->releaseDC(tmp_hdc);
}

extern void qt_fill_tile(QPixmap *tile, const QPixmap &pixmap);

void qt_draw_tiled_pixmap(HDC hdc, int x, int y, int w, int h,
                           const QPixmap *bg_pixmap,
                           int off_x, int off_y)
{
    QPixmap *tile = 0;
    QPixmap *pm;
    int  sw = bg_pixmap->width(), sh = bg_pixmap->height();
    if (sw*sh < 8192 && sw*sh < 16*w*h) {
        int tw = sw, th = sh;
        while (tw*th < 32678 && tw < w/2)
            tw *= 2;
        while (tw*th < 32678 && th < h/2)
            th *= 2;
        tile = new QPixmap(tw, th);
        qt_fill_tile(tile, *bg_pixmap);
        pm = tile;
    } else {
        pm = (QPixmap*)bg_pixmap;
    }
    drawTile(hdc, x, y, w, h, pm, off_x, off_y);
    if (tile)
        delete tile;
}



/*****************************************************************************
  Main event loop
 *****************************************************************************/

extern uint qGlobalPostedEventsCount();

/*!
    If \a gotFocus is true, \a widget will become the active window.
    Otherwise the active window is reset to NULL.
*/
void QApplication::winFocus(QWidget *widget, bool gotFocus)
{
    if (d_func()->inPopupMode()) // some delayed focus event to ignore
        return;
    if (gotFocus) {
        setActiveWindow(widget);
        if (QApplicationPrivate::active_window
	    && (QApplicationPrivate::active_window->windowType() == Qt::Dialog)) {
            // raise the entire application, not just the dialog
            QWidget* mw = QApplicationPrivate::active_window;
            while(mw->parentWidget() && (mw->windowType() == Qt::Dialog))
                mw = mw->parentWidget()->window();
            if (mw != QApplicationPrivate::active_window)
                SetWindowPos(mw->winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        }
    } else {
        setActiveWindow(0);
    }
}

struct KeyRec {
    KeyRec(int c, int a, int s, const QString& t) : code(c), ascii(a), state(s), text(t) { }
    KeyRec() { }
    int code, ascii, state;
    QString text;
};

static const int maxrecs=64; // User has LOTS of fingers...
static KeyRec key_rec[maxrecs];
static int nrecs=0;

static KeyRec* find_key_rec(int code, bool remove)
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

static void store_key_rec(int code, int ascii, int state, const QString& text)
{
    if (nrecs == maxrecs) {
        qWarning("Qt: Internal keyboard buffer overflow");
        return;
    }

    key_rec[nrecs++] = KeyRec(code,ascii,state,text);
}

static void clear_key_rec()
{
    nrecs = 0;
}

//
// QtWndProc() receives all messages from the main event loop
//

static bool inLoop = false;
static int inputcharset = CP_ACP;

#define RETURN(x) { inLoop=false;return x; }

bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event)
{
    return QCoreApplication::sendSpontaneousEvent(receiver, event);
}

extern "C"
LRESULT CALLBACK QtWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool result = true;
    QEvent::Type evt_type = QEvent::None;
    QETWidget *widget = 0;

        // there is no need to process pakcets from tablet unless
        // it is actually on the tablet, a flag to let us know...
        int nPackets;        // the number of packets we get from the queue

    long res = 0;
    if (!qApp)                                // unstable app state
        goto do_default;

#if 0
    // make sure we update widgets also when the user resizes
    if (inLoop && qApp->loopLevel())
        qApp->sendPostedEvents(0, QEvent::Paint);
#endif

    inLoop = true;

    MSG msg;
    msg.hwnd = hwnd;                                // create MSG structure
    msg.message = message;                        // time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.pt.x = GET_X_LPARAM(lParam);
    msg.pt.y = GET_Y_LPARAM(lParam);
    ClientToScreen(msg.hwnd, &msg.pt);         // the coords we get are client coords

    /*
    // sometimes the autograb is not released, so the clickevent is sent
    // to the wrong window. We ignore this for now, because it doesn't
    // cause any problems.
    if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_MBUTTONDOWN) {
        HWND handle = WindowFromPoint(msg.pt);
        if (msg.hwnd != handle) {
            msg.hwnd = handle;
            hwnd = handle;
        }
    }
    */

#if defined(QT_NON_COMMERCIAL)
    QT_NC_WNDPROC
#endif

    // send through app filter
    if (qApp->filterEvent(&msg, &res))
        return res;

    switch (message) {
#ifndef Q_OS_TEMP
    case WM_QUERYENDSESSION: {
        if (sm_smActive) // bogus message from windows
            RETURN(true);

        sm_smActive = true;
        sm_blockUserInput = true; // prevent user-interaction outside interaction windows
        sm_cancel = false;
        if (qt_session_manager_self)
            qApp->commitData(*qt_session_manager_self);
        if (lParam == (LPARAM)ENDSESSION_LOGOFF) {
            _flushall();
        }
        RETURN(!sm_cancel);
    }
    case WM_ENDSESSION: {
        sm_smActive = false;
        sm_blockUserInput = false;
        bool endsession = (bool) wParam;

        if (endsession) {
            // since the process will be killed immediately quit() has no real effect
            int index = QApplication::staticMetaObject.indexOfSignal("aboutToQuit()");
            qApp->qt_metacall(QMetaObject::InvokeMetaMember, index,0);
            qApp->quit();
        }

        RETURN(0);
    }
    case WM_DISPLAYCHANGE:
        if (qApp->type() == QApplication::Tty)
            break;
        if (qt_desktopWidget) {
            int x = GetSystemMetrics(76);
            int y = GetSystemMetrics(77);
            QMoveEvent mv(QPoint(x, y), qt_desktopWidget->pos());
            QApplication::sendEvent(qt_desktopWidget, &mv);
            x = GetSystemMetrics(78);
            y = GetSystemMetrics(79);
            if (QSize(x, y) == qt_desktopWidget->size()) {
                 // a screen resized without changing size of the virtual desktop
                QResizeEvent rs(QSize(x, y), qt_desktopWidget->size());
                QApplication::sendEvent(qt_desktopWidget, &rs);
            } else {
                qt_desktopWidget->resize(x, y);
            }
        }
        break;
#endif

    case WM_SETTINGCHANGE:
#ifdef Q_OS_TEMP
        // CE SIP hide/show
        if (wParam == SPI_SETSIPINFO) {
            QResizeEvent re(QSize(0, 0), QSize(0, 0)); // Calculated by QDesktopWidget
            QApplication::sendEvent(qt_desktopWidget, &re);
            break;
        }
#endif
        // ignore spurious XP message when user logs in again after locking
        if (qApp->type() == QApplication::Tty)
            break;
        if (QApplication::desktopSettingsAware() && wParam != SPI_SETWORKAREA) {
            widget = (QETWidget*)QWidget::find(hwnd);
            if (widget) {
                widget->markFrameStrutDirty();
                if (!widget->parentWidget())
                    qt_set_windows_resources();
            }
        }
        break;
    case WM_SYSCOLORCHANGE:
        if (qApp->type() == QApplication::Tty)
            break;
        if (QApplication::desktopSettingsAware()) {
            widget = (QETWidget*)QWidget::find(hwnd);
            if (widget && !widget->parentWidget())
                qt_set_windows_resources();
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
        if (qt_win_ignoreNextMouseReleaseEvent)
            qt_win_ignoreNextMouseReleaseEvent = false;
        break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
	if (qt_win_ignoreNextMouseReleaseEvent) {
	    qt_win_ignoreNextMouseReleaseEvent = false;
	    if (qt_button_down && qt_button_down->winId() == autoCaptureWnd) {
		releaseAutoCapture();
		qt_button_down = 0;
	    }

            RETURN(0);
        }
        break;

    default:
        break;
    }

    if (!widget)
        widget = (QETWidget*)QWidget::find(hwnd);
    if (!widget)                                // don't know this widget
        goto do_default;

    if (app_do_modal)        {                        // modal event handling
        int ret = 0;
        if (!qt_try_modal(widget, &msg, ret))
            RETURN(ret);
    }

    res = 0;
    if (widget->winEvent(&msg, &res))                // send through widget filter
        RETURN(res);

    if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST ||
           message >= WM_XBUTTONDOWN && message <= WM_XBUTTONDBLCLK)
         && message != WM_MOUSEWHEEL) {
        if (qApp->activePopupWidget() != 0) { // in popup mode
            POINT curPos = msg.pt;
            QWidget* w = QApplication::widgetAt(curPos.x, curPos.y);
            if (w)
                widget = (QETWidget*)w;
        }

        if (!qt_tabletChokeMouse) {
            result = widget->translateMouseEvent(msg);        // mouse event
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
            bool next_is_button = false;
            bool is_mouse_move = (message == WM_MOUSEMOVE);
            if (is_mouse_move) {
                MSG msg1;
                if (winPeekMessage(&msg1, msg.hwnd, WM_MOUSEFIRST,
                                    WM_MOUSELAST, PM_NOREMOVE))
                    next_is_button = (msg1.message == WM_LBUTTONUP
                                       || msg1.message == WM_LBUTTONDOWN);
            }
            if (!is_mouse_move || (is_mouse_move && !next_is_button))
                qt_tabletChokeMouse = false;
        }
    } else if (message == WM95_MOUSEWHEEL) {
        result = widget->translateWheelEvent(msg);
    } else {
        switch (message) {
        case WM_KEYDOWN:                        // keyboard event
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_IME_CHAR:
        case WM_IME_KEYDOWN:
        case WM_CHAR: {
            MSG msg1;
            bool anyMsg = winPeekMessage(&msg1, msg.hwnd, 0, 0, PM_NOREMOVE);
            if (anyMsg && msg1.message == WM_DEADCHAR) {
                result = true; // consume event since there is a dead char next
                break;
            }
            QWidget *g = QWidget::keyboardGrabber();
            if (g)
                widget = (QETWidget*)g;
            else if (QApplication::activePopupWidget())
                widget = (QETWidget*)QApplication::activePopupWidget();
            else if (qApp->focusWidget())
                widget = (QETWidget*)QApplication::focusWidget();
            else if (!widget || widget->winId() == GetFocus()) // We faked the message to go to exactly that widget.
                widget = (QETWidget*)widget->window();
            if (widget->isEnabled())
                result = widget->translateKeyEvent(msg, g != 0);
            break;
        }
        case WM_SYSCHAR:
            result = true;                        // consume event
            break;

        case WM_MOUSEWHEEL:
            result = widget->translateWheelEvent(msg);
            break;

        case WM_APPCOMMAND:
            {
                uint cmd = GET_APPCOMMAND_LPARAM(lParam);
                uint uDevice = GET_DEVICE_LPARAM(lParam);
                uint dwKeys = GET_KEYSTATE_LPARAM(lParam);

                int state = translateButtonState(dwKeys, QEvent::KeyPress, 0);

                switch (uDevice) {
                case FAPPCOMMAND_KEY:
                    {
                        int key = 0;

                        switch(cmd) {
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
                            key = Qt::Key_MediaPrevious;
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
                        if (key) {
                            bool res = false;
                            QWidget *g = QWidget::keyboardGrabber();
                            if (g)
                                widget = (QETWidget*)g;
                            else if (qApp->focusWidget())
                                widget = (QETWidget*)qApp->focusWidget();
                            else
                                widget = (QETWidget*)widget->window();
                            if (widget->isEnabled())
                                res = ((QETWidget*)widget)->sendKeyEvent(QEvent::KeyPress, key, state, false, QString(), g != 0);
                            if (res)
                                return true;
                        }
                    }
                    break;

                default:
                    break;
                }

                result = false;
            }
            break;

#ifndef Q_OS_TEMP
        case WM_NCHITTEST:
            if (widget->isWindow()) {
                QPoint pos = widget->mapFromGlobal(QPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                // don't show resize-cursors for fixed-size widgets
                int fleft = widget->topData()->fleft;
                int ftop = widget->topData()->ftop;

                if (widget->minimumWidth() == widget->maximumWidth() && (pos.x() < 0 || pos.x() >= widget->width()))
                    break;
                if (widget->minimumHeight() == widget->maximumHeight() && (pos.y() < -(ftop - fleft) || pos.y() >= widget->height()))
                    break;
            }

            result = false;
            break;
        case WM_NCMOUSEMOVE:
            {
                // span the application wide cursor over the
                // non-client area.
                QCursor *c = qt_grab_cursor();
                if (!c)
                    c = QApplication::overrideCursor();
                if (c)        // application cursor defined
                    SetCursor(c->handle());
                else
                    result = false;
                // generate leave event also when the caret enters
                // the non-client area.
                QApplicationPrivate::dispatchEnterLeave(0, QWidget::find(curWin));
                curWin = 0;
            }
            break;
#endif

        case WM_SYSCOMMAND: {
#ifndef Q_OS_TEMP
            bool window_state_change = false;
            Qt::WindowStates oldstate = Qt::WindowStates(widget->dataPtr()->window_state);
            switch(wParam) {
            case SC_CONTEXTHELP:
#ifndef QT_NO_WHATSTHIS
                QWhatsThis::enterWhatsThisMode();
#endif
                QT_WA({
                    DefWindowProc(hwnd, WM_NCPAINT, 1, 0);
                } , {
                    DefWindowProcA(hwnd, WM_NCPAINT, 1, 0);
                });
                break;
#if defined(QT_NON_COMMERCIAL)
                QT_NC_SYSCOMMAND
#endif
            case SC_MAXIMIZE:
                window_state_change = true;
                widget->dataPtr()->window_state &= ~Qt::WindowMinimized;
                widget->dataPtr()->window_state |= Qt::WindowMaximized;
                result = false;
                break;
            case SC_MINIMIZE:
                window_state_change = true;
                widget->dataPtr()->window_state |= Qt::WindowMinimized;
                if (widget->isVisible()) {
                    QHideEvent e;
                    qt_sendSpontaneousEvent(widget, &e);
                    widget->hideChildren(true);
                }
                result = false;
                break;
            case SC_RESTORE:
                window_state_change = true;
                if (widget->isMinimized()) {
                    widget->dataPtr()->window_state &= ~Qt::WindowMinimized;
                    widget->showChildren(true);
                    QShowEvent e;
                    qt_sendSpontaneousEvent(widget, &e);
                } else {
                    widget->dataPtr()->window_state &= ~Qt::WindowMaximized;
                }
                result = false;
                break;
            default:
                result = false;
                break;
            }

            if (window_state_change) {
                QWindowStateChangeEvent e(oldstate);
                qt_sendSpontaneousEvent(widget, &e);
            }
#endif

            break;
        }

        case WM_SETTINGCHANGE:
            if ( qApp->type() == QApplication::Tty )
	        break;

            if (!msg.wParam) {
                QString area = QT_WA_INLINE(QString::fromUtf16((unsigned short *)msg.lParam),
                                             QString::fromLocal8Bit((char*)msg.lParam));
                if (area == "intl")
                    QApplication::postEvent(widget, new QEvent(QEvent::LocaleChange));
            }
            break;

#ifndef Q_OS_TEMP
        case WM_NCLBUTTONDBLCLK:
            if (wParam == HTCAPTION) {
                bool window_state_changed = false;
                Qt::WindowStates oldstate = widget->windowState();
                if (widget->isMaximized()) {
                    window_state_changed = true;
                    widget->setWindowState(widget->windowState() & ~Qt::WindowMaximized);
                } else if (widget->isMaximized()){
                    window_state_changed = true;
                    widget->setWindowState(widget->windowState() | Qt::WindowMaximized);
                }

                if (window_state_changed) {
                    QWindowStateChangeEvent e(oldstate);
                    qt_sendSpontaneousEvent(widget, &e);
                }
            }
            result = false;
            break;
#endif
        case WM_PAINT:                                // paint event
            result = widget->translatePaintEvent(msg);
            break;

        case WM_ERASEBKGND:                        // erase window background
            if (!widget->testAttribute(Qt::WA_PendingUpdate)
                && !widget->testAttribute(Qt::WA_NoBackground)
                && widget->windowType() != Qt::Popup
                && widget->windowType() != Qt::ToolTip)
                widget->eraseWindowBackground((HDC)wParam);
            RETURN(true);
            break;

        case WM_MOVE:                                // move window
        case WM_SIZE:                                // resize window
            result = widget->translateConfigEvent(msg);
            break;

        case WM_ACTIVATE:
	    if ( qApp->type() == QApplication::Tty )
	        break;

            if (ptrWTOverlap && ptrWTEnable) {
                // cooperate with other tablet applications, but when
                // we get focus, I want to use the tablet...
                if (qt_tablet_context && GET_WM_ACTIVATE_STATE(wParam, lParam)) {
                    if (ptrWTEnable(qt_tablet_context, true))
                        ptrWTOverlap(qt_tablet_context, true);
                }
            }
            if (QApplication::activePopupWidget() && LOWORD(wParam) == WA_INACTIVE &&
                QWidget::find((HWND)lParam) == 0) {
                // Another application was activated while our popups are open,
                // then close all popups.  In case some popup refuses to close,
                // we give up after 1024 attempts (to avoid an infinite loop).
                int maxiter = 1024;
                QWidget *popup;
                while ((popup=QApplication::activePopupWidget()) && maxiter--)
                    popup->close();
            }

            qApp->winFocus(widget, LOWORD(wParam) != WA_INACTIVE);
            // Windows tries to activate a modally blocked window.
            // This happens when restoring an application after "Show Desktop"
            if (app_do_modal && LOWORD(wParam) == WA_ACTIVE) {
                QWidget *top = 0;
                if (!QApplicationPrivate::tryModalHelper(widget, &top) && top && widget != top)
                    top->activateWindow();
            }
            if (LOWORD(wParam) == WA_INACTIVE)
                clear_key_rec(); // Ensure nothing gets consider an auto-repeat press later
	    break;

#ifndef Q_OS_TEMP
            case WM_MOUSEACTIVATE:
                {
                    const QWidget *tlw = widget->window();
                    // Do not change activation if the clicked widget is inside a floating dock window
                    if (tlw->inherits("QDockWidget") && qApp->activeWindow()
                         && !qApp->activeWindow()->inherits("QDockWidget"))
                        RETURN(MA_NOACTIVATE);
                }
                result = false;
                break;
#endif
            case WM_SHOWWINDOW:
#ifndef Q_OS_TEMP
                if (lParam == SW_PARENTOPENING) {
                    if (widget->testAttribute(Qt::WA_WState_Hidden))
                        RETURN(0);
                }
#endif
                if  (!wParam && autoCaptureWnd == widget->winId())
                    releaseAutoCapture();
                result = false;
                break;

        case WM_PALETTECHANGED:                        // our window changed palette
            if (QColormap::hPal() && (WId)wParam == widget->winId())
                RETURN(0);                        // otherwise: FALL THROUGH!
            // FALL THROUGH
        case WM_QUERYNEWPALETTE:                // realize own palette
            if (QColormap::hPal()) {
                HDC hdc = GetDC(widget->winId());
                HPALETTE hpalOld = SelectPalette(hdc, QColormap::hPal(), false);
                uint n = RealizePalette(hdc);
                if (n)
                    InvalidateRect(widget->winId(), 0, true);
                SelectPalette(hdc, hpalOld, true);
                RealizePalette(hdc);
                ReleaseDC(widget->winId(), hdc);
                RETURN(n);
            }
            break;
        case WM_CLOSE:                                // close window
            widget->translateCloseEvent(msg);
            RETURN(0);                                // always handled

        case WM_DESTROY:                        // destroy window
            if (hwnd == curWin) {
                QEvent leave(QEvent::Leave);
                QApplication::sendEvent(widget, &leave);
                curWin = 0;
            }
            if (widget == popupButtonFocus)
                popupButtonFocus = 0;
            result = false;
            break;

#ifndef Q_OS_TEMP
        case WM_GETMINMAXINFO:
            if (widget->xtra()) {
                MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                QWExtra           *x = widget->xtra();
		if ( x->minw > 0 )
		    mmi->ptMinTrackSize.x = x->minw + x->topextra->fright + x->topextra->fleft;
		if ( x->minh > 0 )
		    mmi->ptMinTrackSize.y = x->minh + x->topextra->ftop + x->topextra->fbottom;
                if ( x->maxw < QWIDGETSIZE_MAX ) {
		    mmi->ptMaxTrackSize.x = x->maxw + x->topextra->fright + x->topextra->fleft;
                    // windows with titlebar have an implicit sizelimit of 112 pixels
                    if (widget->windowFlags() & Qt::WindowTitleHint)
                        mmi->ptMaxTrackSize.x = qMax<long>(mmi->ptMaxTrackSize.x, 112);
                }
		if ( x->maxh < QWIDGETSIZE_MAX )
		    mmi->ptMaxTrackSize.y = x->maxh + x->topextra->ftop + x->topextra->fbottom;
                RETURN(0);
            }
            break;

            case WM_CONTEXTMENU:
            {
                // it's not VK_APPS or Shift+F10, but a click in the NC area
                if (lParam != (int)0xffffffff) {
                    result = false;
                    break;
                }
                QWidget *fw = qApp->focusWidget();
                if (fw) {
                    QPoint pos = fw->inputMethodQuery(Qt::ImMicroFocus).toRect().center();
                    QContextMenuEvent e(QContextMenuEvent::Keyboard, pos, fw->mapToGlobal(pos));
                    result = qt_sendSpontaneousEvent(fw, &e);
                }
            }
            break;
#endif

        case WM_IME_STARTCOMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_COMPOSITION: {
            QWidget *fw = qApp->focusWidget();
            QWinInputContext *im = fw ? qobject_cast<QWinInputContext *>(fw->inputContext()) : 0;
            if (fw && im) {
                if(message == WM_IME_STARTCOMPOSITION)
                    result = im->startComposition();
                else if (message == WM_IME_ENDCOMPOSITION)
                    result = im->endComposition();
                else if (message == WM_IME_COMPOSITION)
                    result = im->composition(lParam);
            }
            break;
        }

#ifndef Q_OS_TEMP
        case WM_CHANGECBCHAIN:
        case WM_DRAWCLIPBOARD:
        case WM_RENDERFORMAT:
        case WM_RENDERALLFORMATS:
        case WM_DESTROYCLIPBOARD:
            if (qt_clipboard) {
                QClipboardEvent e(reinterpret_cast<QEventPrivate *>(&msg));
                qt_sendSpontaneousEvent(qt_clipboard, &e);
                RETURN(0);
            }
            result = false;
            break;
#endif
#ifndef QT_NO_ACCESSIBILITY
        case WM_GETOBJECT:
            {
                // Ignoring all requests while starting up
                if (qApp->startingUp() || qApp->closingDown() || (DWORD)lParam != OBJID_CLIENT) {
                    result = false;
                    break;
                }

                typedef LRESULT (WINAPI *PtrLresultFromObject)(REFIID, WPARAM, LPUNKNOWN);
                static PtrLresultFromObject ptrLresultFromObject = 0;
                static bool oleaccChecked = false;

                if (!oleaccChecked) {
                    oleaccChecked = true;
                    ptrLresultFromObject = (PtrLresultFromObject)QLibrary::resolve("oleacc.dll", "LresultFromObject");
                }
                if (ptrLresultFromObject) {
                    QAccessibleInterface *acc = QAccessible::queryAccessibleInterface(widget);
                    if (!acc) {
                        result = false;
                        break;
                    }

                    // and get an instance of the IAccessibile implementation
                    IAccessible *iface = qt_createWindowsAccessible(acc);
                    res = ptrLresultFromObject(IID_IAccessible, wParam, iface);  // ref == 2
                    iface->Release(); // the client will release the object again, and then it will destroy itself

                    if (res > 0)
                        RETURN(res);
                }
            }
            result = false;
            break;
#endif
        case WT_PACKET:
            if (ptrWTPacketsGet) {
                if ((nPackets = ptrWTPacketsGet(qt_tablet_context, QT_TABLET_NPACKETQSIZE, &localPacketBuf))) {
                    result = widget->translateTabletEvent(msg, localPacketBuf, nPackets);
                }
            }
            break;
        case WT_PROXIMITY:
            // flush the Queue
            if (ptrWTPacketsGet)
                ptrWTPacketsGet(qt_tablet_context, QT_TABLET_NPACKETQSIZE + 1, NULL);
            if (qt_tabletChokeMouse)
                qt_tabletChokeMouse = false;
            break;
        case WM_KILLFOCUS:
            if (!QWidget::find((HWND)wParam)) { // we don't get focus, so unset it now
                if (!widget->hasFocus()) // work around Windows bug after minimizing/restoring
                    widget = (QETWidget*)qApp->focusWidget();
                HWND focus = ::GetFocus();
                if (!widget || (focus && ::IsChild(widget->winId(), focus))) {
                    result = false;
                } else {
                    widget->clearFocus();
                    result = true;
                }
            } else {
                result = false;
            }
            break;

        case WM_THEMECHANGED:
            if ((widget->windowType() == Qt::Desktop) || !qApp || qApp->closingDown()
                                                         || qApp->type() == QApplication::Tty)
                break;

            if (widget->testAttribute(Qt::WA_WState_Polished))
                qApp->style()->unpolish(widget);

            if (widget->testAttribute(Qt::WA_WState_Polished))
                qApp->style()->polish(widget);
            widget->repolishStyle(*qApp->style());
            if (widget->isVisible())
                widget->update();
            break;

#ifndef Q_OS_TEMP
        case WM_INPUTLANGCHANGE: {
            char info[7];
            if (!GetLocaleInfoA(MAKELCID(lParam, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, info, 6)) {
                inputcharset = CP_ACP;
            } else {
                inputcharset = QString(info).toInt();
            }
            break;
        }
#else
        case WM_COMMAND:
            result = (wParam == 0x1);
            if (result)
                QApplication::postEvent(widget, new QEvent(QEvent::OkRequest));
            break;
        case WM_HELP:
            QApplication::postEvent(widget, new QEvent(QEvent::HelpRequest));
            result = true;
            break;
#endif

        case WM_MOUSELEAVE:
            // We receive a mouse leave for curWin, meaning
            // the mouse was moved outside our widgets
            if (widget->winId() == curWin) {
                bool dispatch = !widget->underMouse();
                // hasMouse is updated when dispatching enter/leave,
                // so test if it is actually up-to-date
                if (!dispatch) {
                    QRect geom = widget->geometry();
                    if (widget->parentWidget() && !widget->isWindow()) {
                        QPoint gp = widget->parentWidget()->mapToGlobal(widget->pos());
                        geom.setX(gp.x());
                        geom.setY(gp.y());
                    }
                    QPoint cpos = QCursor::pos();
                    dispatch = !geom.contains(cpos);
                    if ( !dispatch) {
                        QWidget *hittest = QApplication::widgetAt(cpos);
                        dispatch = !hittest || hittest->winId() != curWin;
                    }
                    if (!dispatch) {
                        HRGN hrgn = CreateRectRgn(0,0,0,0);
                        if (GetWindowRgn(curWin, hrgn) != ERROR) {
                            QPoint lcpos = widget->mapFromGlobal(cpos);
                            dispatch = !PtInRegion(hrgn, lcpos.x(), lcpos.y());
                        }
                        DeleteObject(hrgn);
                    }
                }
                if (dispatch) {
                    QApplicationPrivate::dispatchEnterLeave(0, QWidget::find((WId)curWin));
                    curWin = 0;
                }
            }
            break;

        case WM_CANCELMODE:
            {
                // this goes through QMenuBar's event filter
                QEvent e(QEvent::ActivationChange);
                QApplication::sendEvent(qApp, &e);
            }
            break;

        default:
            result = false;                        // event was not processed
            break;
        }
    }

    if (evt_type != QEvent::None) {                // simple event
        QEvent e(evt_type);
        result = qt_sendSpontaneousEvent(widget, &e);
    }
    if (result)
        RETURN(false);

do_default:
    RETURN(QWinInputContext::DefWindowProc(hwnd,message,wParam,lParam))
}


/*****************************************************************************
  Modal widgets; We have implemented our own modal widget mechanism
  to get total control.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  QApplicationPrivate::enterModal()
        Enters modal state
        Arguments:
            QWidget *widget        A modal widget

  QApplicationPrivate::leaveModal()
        Leaves modal state for a widget
        Arguments:
            QWidget *widget        A modal widget
 *****************************************************************************/

bool QApplicationPrivate::modalState()
{
    return app_do_modal;
}


void QApplicationPrivate::enterModal(QWidget *widget)
{
    if (!qt_modal_stack) {                        // create modal stack
        qt_modal_stack = new QWidgetList;
    }
    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }

    releaseAutoCapture();
    QApplicationPrivate::dispatchEnterLeave(0, QWidget::find((WId)curWin));
    qt_modal_stack->insert(0, widget);
    app_do_modal = true;
    curWin = 0;
    qt_button_down = 0;
    qt_win_ignoreNextMouseReleaseEvent = false;
}


void QApplicationPrivate::leaveModal(QWidget *widget)
{
    if (qt_modal_stack && qt_modal_stack->removeAll(widget)) {
        if (qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
            QPoint p(QCursor::pos());
            app_do_modal = false; // necessary, we may get recursively into qt_try_modal below
            QWidget* w = QApplication::widgetAt(p.x(), p.y());
            QApplicationPrivate::dispatchEnterLeave(w, QWidget::find(curWin)); // send synthetic enter event
            curWin = w? w->winId() : 0;
        }
        qt_win_ignoreNextMouseReleaseEvent = true;
    }
    app_do_modal = qt_modal_stack != 0;

    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }
}

static bool qt_blocked_modal(QWidget *widget)
{
    if (!app_do_modal)
        return false;
    if (qApp->activePopupWidget())
        return false;
    if ((widget->windowType() == Qt::Tool))        // allow tool windows
        return false;

    QWidget *modal=0, *top=qt_modal_stack->first();

    widget = widget->window();
    if (widget->testAttribute(Qt::WA_ShowModal))        // widget is modal
        modal = widget;
    if (!top || modal == top)                                // don't block event
        return false;
    return true;
}

bool qt_try_modal(QWidget *widget, MSG *msg, int& ret)
{
    QWidget * top = 0;

    if (QApplicationPrivate::tryModalHelper(widget, &top))
        return true;

    int type = msg->message;

    bool block_event = false;
#ifndef Q_OS_TEMP
    if (type == WM_NCHITTEST) {
        block_event = true;
    } else
#endif
        if ((type >= WM_MOUSEFIRST && type <= WM_MOUSELAST) ||
             type == WM_MOUSEWHEEL || type == (int)WM95_MOUSEWHEEL ||
             type == WM_MOUSELEAVE ||
             (type >= WM_KEYFIRST && type <= WM_KEYLAST)
#ifndef Q_OS_TEMP
            || type == WM_NCMOUSEMOVE
#endif
               ) {
      if (type == WM_MOUSEMOVE
#ifndef Q_OS_TEMP
          || type == WM_NCMOUSEMOVE
#endif
                       ) {
        QCursor *c = qt_grab_cursor();
        if (!c)
            c = QApplication::overrideCursor();
        if (c)                                // application cursor defined
            SetCursor(c->handle());
        else
            SetCursor(QCursor(Qt::ArrowCursor).handle());
      }
      block_event = true;
    } else if (type == WM_CLOSE) {
        block_event = true;
    }
#ifndef Q_OS_TEMP
    else if (type == WM_MOUSEACTIVATE || type == WM_NCLBUTTONDOWN){
        if (!top->isActiveWindow()) {
            top->activateWindow();
        } else {
            QApplication::beep();
        }
        block_event = true;
        ret = MA_NOACTIVATEANDEAT;
    } else if (type == WM_SYSCOMMAND) {
        if (!(msg->wParam == SC_RESTORE && widget->isMinimized()))
            block_event = true;
    }
#endif

    return !block_event;
}


/*****************************************************************************
  Popup widget mechanism

  openPopup()
        Adds a widget to the list of popup widgets
        Arguments:
            QWidget *widget        The popup widget to be added

  closePopup()
        Removes a widget from the list of popup widgets
        Arguments:
            QWidget *widget        The popup widget to be removed
 *****************************************************************************/

void QApplicationPrivate::openPopup(QWidget *popup)
{
    if (!QApplicationPrivate::popupWidgets)
        QApplicationPrivate::popupWidgets = new QWidgetList;
    QApplicationPrivate::popupWidgets->append(popup);
    if (!popup->isEnabled())
        return;

    if (QApplicationPrivate::popupWidgets->count() == 1 && !qt_nograb())
        setAutoCapture(popup->winId());        // grab mouse/keyboard
    // Popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    if (popup->focusWidget()) {
        popup->focusWidget()->setFocus(Qt::PopupFocusReason);
    } else if (QApplicationPrivate::popupWidgets->count() == 1) { // this was the first popup
        if (QWidget *fw = q_func()->focusWidget()) {
            QFocusEvent e(QEvent::FocusOut, Qt::PopupFocusReason);
            q_func()->sendEvent(fw, &e);
        }
    }
}

void QApplicationPrivate::closePopup(QWidget *popup)
{
    if (!QApplicationPrivate::popupWidgets)
        return;
    QApplicationPrivate::popupWidgets->removeAll(popup);
    POINT curPos;
    GetCursorPos(&curPos);
    replayPopupMouseEvent = (!popup->geometry().contains(QPoint(curPos.x, curPos.y))
                             && !popup->testAttribute(Qt::WA_NoMouseReplay));

    if (QApplicationPrivate::popupWidgets->isEmpty()) { // this was the last popup
        delete QApplicationPrivate::popupWidgets;
        QApplicationPrivate::popupWidgets = 0;
        if (!popup->isEnabled())
            return;
        if (!qt_nograb())                        // grabbing not disabled
            releaseAutoCapture();
        if (QApplicationPrivate::active_window) {
            if (QWidget *fw = QApplicationPrivate::active_window->focusWidget()) {
                if (fw != q_func()->focusWidget()) {
                    fw->setFocus(Qt::PopupFocusReason);
                } else {
                    QFocusEvent e(QEvent::FocusIn, Qt::PopupFocusReason);
                    q_func()->sendEvent(fw, &e);
                }
            }
        }
    } else {
        // Popups are not focus-handled by the window system (the
        // first popup grabbed the keyboard), so we have to do that
        // manually: A popup was closed, so the previous popup gets
        // the focus.
        QWidget* aw = QApplicationPrivate::popupWidgets->last();
        if (QApplicationPrivate::popupWidgets->count() == 1)
            setAutoCapture(aw->winId());
        if (QWidget *fw = aw->focusWidget())
            fw->setFocus(Qt::PopupFocusReason);
    }
}




/*****************************************************************************
  Event translation; translates Windows events to Qt events
 *****************************************************************************/

//
// Auto-capturing for mouse press and mouse release
//

static void setAutoCapture(HWND h)
{
    if (autoCaptureWnd)
        releaseAutoCapture();
    autoCaptureWnd = h;
    SetCapture(h);
}

static void releaseAutoCapture()
{
    if (autoCaptureWnd) {
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
    WM_MOUSEMOVE,        QEvent::MouseMove,                0,
    WM_LBUTTONDOWN,      QEvent::MouseButtonPress,        Qt::LeftButton,
    WM_LBUTTONUP,        QEvent::MouseButtonRelease,      Qt::LeftButton,
    WM_LBUTTONDBLCLK,    QEvent::MouseButtonDblClick,     Qt::LeftButton,
    WM_RBUTTONDOWN,      QEvent::MouseButtonPress,        Qt::RightButton,
    WM_RBUTTONUP,        QEvent::MouseButtonRelease,      Qt::RightButton,
    WM_RBUTTONDBLCLK,    QEvent::MouseButtonDblClick,     Qt::RightButton,
    WM_MBUTTONDOWN,      QEvent::MouseButtonPress,        Qt::MidButton,
    WM_MBUTTONUP,        QEvent::MouseButtonRelease,      Qt::MidButton,
    WM_MBUTTONDBLCLK,    QEvent::MouseButtonDblClick,     Qt::MidButton,
    // use XButton1 for now, the real X button is decided later
    WM_XBUTTONDOWN,      QEvent::MouseButtonPress,        Qt::XButton1,
    WM_XBUTTONUP,        QEvent::MouseButtonRelease,      Qt::XButton1,
    WM_XBUTTONDBLCLK,    QEvent::MouseButtonDblClick,     Qt::XButton1,
    0,                        0,                                0
};

static int translateButtonState(int s, int type, int button)
{
    Q_UNUSED(type);
    Q_UNUSED(button);
    int bst = 0;
    if (s & MK_LBUTTON)
        bst |= Qt::LeftButton;
    if (s & MK_MBUTTON)
        bst |= Qt::MidButton;
    if (s & MK_RBUTTON)
        bst |= Qt::RightButton;
    if (s & MK_SHIFT)
        bst |= Qt::ShiftModifier;
    if (s & MK_CONTROL)
        bst |= Qt::ControlModifier;

    if (s & MK_XBUTTON1)
        bst |= Qt::XButton1;
    if (s & MK_XBUTTON2)
        bst |= Qt::XButton2;

    if (GetKeyState(VK_MENU) < 0)
        bst |= Qt::AltModifier;

    if ((GetKeyState(VK_LWIN) < 0) ||
         (GetKeyState(VK_RWIN) < 0))
        bst |= Qt::MetaModifier;

    return bst;
}

void qt_win_eatMouseMove()
{
    // after closing a windows dialog with a double click (i.e. open a file)
    // the message queue still contains a dubious WM_MOUSEMOVE message where
    // the left button is reported to be down (wParam != 0).
    // remove all those messages (usually 1) and post the last one with a
    // reset button state

    MSG msg = {0, 0, 0, 0, 0, 0, 0};
    QT_WA( {
        while (PeekMessage(&msg, 0, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
            ;
        if (msg.message == WM_MOUSEMOVE)
            PostMessage(msg.hwnd, msg.message, 0, msg.lParam);
    }, {
        MSG msg;
        msg.message = 0;
        while (PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
            ;
        if (msg.message == WM_MOUSEMOVE)
            PostMessageA(msg.hwnd, msg.message, 0, msg.lParam);
    } );
}

// In DnD, the mouse release event never appears, so the
// mouse button state machine must be manually reset
/*! \internal */
void QApplication::winMouseButtonUp()
{
    qt_button_down = 0;
    releaseAutoCapture();
}

bool QETWidget::translateMouseEvent(const MSG &msg)
{
    static QPoint pos;
    static POINT gpos={-1,-1};
    QEvent::Type type;                                // event parameters
    int           button;
    int           state;
    int           i;

    if (sm_blockUserInput) //block user interaction during session management
        return true;

    // Compress mouse move events
    if (msg.message == WM_MOUSEMOVE) {
        MSG mouseMsg;
        while (winPeekMessage(&mouseMsg, msg.hwnd, WM_MOUSEFIRST,
                WM_MOUSELAST, PM_NOREMOVE)) {
            if (mouseMsg.message == WM_MOUSEMOVE) {
#define PEEKMESSAGE_IS_BROKEN 1
#ifdef PEEKMESSAGE_IS_BROKEN
                // Since the Windows PeekMessage() function doesn't
                // correctly return the wParam for WM_MOUSEMOVE events
                // if there is a key release event in the queue
                // _before_ the mouse event, we have to also consider
                // key release events (kls 2003-05-13):
                MSG keyMsg;
                bool done = false;
                while (winPeekMessage(&keyMsg, 0, WM_KEYFIRST, WM_KEYLAST,
                        PM_NOREMOVE)) {
                    if (keyMsg.time < mouseMsg.time) {
                        if ((keyMsg.lParam & 0xC0000000) == 0x40000000) {
                            winPeekMessage(&keyMsg, 0, keyMsg.message,
                                            keyMsg.message, PM_REMOVE);
                        } else {
                            done = true;
                            break;
                        }
                    } else {
                        break; // no key event before the WM_MOUSEMOVE event
                    }
                }
                if (done)
                    break;
#else
                // Actually the following 'if' should work instead of
                // the above key event checking, but apparently
                // PeekMessage() is broken :-(
                if (mouseMsg.wParam != msg.wParam)
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
                winPeekMessage(&mouseMsg, msg.hwnd, WM_MOUSEMOVE,
                                WM_MOUSEMOVE, PM_REMOVE);
            } else {
                break; // there was no more WM_MOUSEMOVE event
            }
        }
    }


    for (i=0; (UINT)mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3)
        ;
    if (!mouseTbl[i])
        return false;
    type   = (QEvent::Type)mouseTbl[++i];        // event type
    button = mouseTbl[++i];                        // which button
    if (button == Qt::XButton1) {
        switch(GET_XBUTTON_WPARAM(msg.wParam)) {
        case XBUTTON1:
            button = Qt::XButton1;
            break;
        case XBUTTON2:
            button = Qt::XButton2;
            break;
        }
    }
    state  = translateButtonState(msg.wParam, type, button); // button state
    if (type == QEvent::MouseMove) {
        if (!(state & Qt::MouseButtonMask))
            qt_button_down = 0;
        QCursor *c = qt_grab_cursor();
        if (!c)
            c = QApplication::overrideCursor();
        if (c)                                // application cursor defined
            SetCursor(c->handle());
        else {
            QWidget *w = this; // use  widget cursor if widget is enabled
            while (!w->isWindow() && !w->isEnabled())
                w = w->parentWidget();
            SetCursor(w->cursor().handle());
        }
        if (curWin != winId()) {                // new current window
            QApplicationPrivate::dispatchEnterLeave(this, QWidget::find(curWin));
            curWin = winId();
#ifndef Q_OS_TEMP
            static bool trackMouseEventLookup = false;
            typedef BOOL (WINAPI *PtrTrackMouseEvent)(LPTRACKMOUSEEVENT);
            static PtrTrackMouseEvent ptrTrackMouseEvent = 0;
            if (!trackMouseEventLookup) {
                trackMouseEventLookup = true;
                ptrTrackMouseEvent = (PtrTrackMouseEvent)QLibrary::resolve("comctl32", "_TrackMouseEvent");
            }
            if (ptrTrackMouseEvent && !qApp->d_func()->inPopupMode()) {
                // We always have to set the tracking, since
                // Windows detects more leaves than we do..
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = 0x00000002;    // TME_LEAVE
                tme.hwndTrack = curWin;      // Track on window receiving msgs
                tme.dwHoverTime = (DWORD)-1; // HOVER_DEFAULT
                ptrTrackMouseEvent(&tme);
            }
#endif // Q_OS_TEMP
        }

        POINT curPos = msg.pt;
        if (curPos.x == gpos.x && curPos.y == gpos.y)
            return true;                        // same global position
        gpos = curPos;

        ScreenToClient(winId(), &curPos);

        pos.rx() = curPos.x;
        pos.ry() = curPos.y;
        pos = d_func()->mapFromWS(pos);
    } else {
        gpos = msg.pt;
        pos = mapFromGlobal(QPoint(gpos.x, gpos.y));

        if (type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick) {        // mouse button pressed
            // Magic for masked widgets
            qt_button_down = findChildWidget(this, pos);
            if (!qt_button_down || !qt_button_down->testAttribute(Qt::WA_MouseNoMask))
                qt_button_down = this;
        }
    }

    bool res = false;

    if (qApp->d_func()->inPopupMode()) {                        // in popup mode
        replayPopupMouseEvent = false;
        QWidget* activePopupWidget = qApp->activePopupWidget();
        QWidget *popup = activePopupWidget;

        if (popup != this) {
            if ((windowType() == Qt::Popup) && rect().contains(pos))
                popup = this;
            else                                // send to last popup
                pos = popup->mapFromGlobal(QPoint(gpos.x, gpos.y));
        }
        QWidget *popupChild = findChildWidget(popup, pos);
        bool releaseAfter = false;
        switch (type) {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonDblClick:
                popupButtonFocus = popupChild;
                break;
            case QEvent::MouseButtonRelease:
                releaseAfter = true;
                break;
            default:
                break;                                // nothing for mouse move
        }

        if (popupButtonFocus)
            popup = popupButtonFocus;
        else if (popupChild)
            popup = popupChild;

        QPoint globalPos(gpos.x, gpos.y);
        pos = popup->mapFromGlobal(globalPos);
	QMouseEvent e(type, pos, globalPos,
                      Qt::MouseButton(button),
                      Qt::MouseButtons(state & Qt::MouseButtonMask),
                      Qt::KeyboardModifiers(state & Qt::KeyboardModifierMask));
	res = QApplication::sendSpontaneousEvent(popup, &e);
        res = res && e.isAccepted();

        if (releaseAfter) {
            popupButtonFocus = 0;
            qt_button_down = 0;
        }

        if (type == QEvent::MouseButtonPress
             && qApp->activePopupWidget() != activePopupWidget
             && replayPopupMouseEvent) {
            // the popup dissappeared. Replay the event
            QWidget* w = QApplication::widgetAt(gpos.x, gpos.y);
            if (w && !qt_blocked_modal(w)) {
                if (QWidget::mouseGrabber() == 0)
                    setAutoCapture(w->winId());
                POINT widgetpt = gpos;
                ScreenToClient(w->winId(), &widgetpt);
                LPARAM lParam = MAKELPARAM(widgetpt.x, widgetpt.y);
                winPostMessage(w->winId(), msg.message, msg.wParam, lParam);
            }
         } else if (type == QEvent::MouseButtonRelease && button == Qt::RightButton
                   && qApp->activePopupWidget() == activePopupWidget) {
            // popup still alive and received right-button-release
	    QContextMenuEvent e2(QContextMenuEvent::Mouse, pos, globalPos);
	    bool res2 = QApplication::sendSpontaneousEvent( popup, &e2 );
            if (!res) // RMB not accepted
                res = res2 && e2.isAccepted();
        }
    } else {                                        // not popup mode
        int bs = state & Qt::MouseButtonMask;
        if ((type == QEvent::MouseButtonPress ||
              type == QEvent::MouseButtonDblClick) && bs == button) {
            if (QWidget::mouseGrabber() == 0)
                setAutoCapture(winId());
        } else if (type == QEvent::MouseButtonRelease && bs == 0) {
            if (QWidget::mouseGrabber() == 0)
                releaseAutoCapture();
        }

        QWidget *widget = this;
        QWidget *w = QWidget::mouseGrabber();
        if (!w)
            w = qt_button_down;
        if (w && w != this) {
            widget = w;
            pos = w->mapFromGlobal(QPoint(gpos.x, gpos.y));
        }

        if (type == QEvent::MouseButtonRelease &&
             (state & Qt::MouseButtonMask) == 0) {
            qt_button_down = 0;
        }

        QMouseEvent e(type, pos, QPoint(gpos.x,gpos.y),
                      Qt::MouseButton(button),
                      Qt::MouseButtons(state & Qt::MouseButtonMask),
                      Qt::KeyboardModifiers(state & Qt::KeyboardModifierMask));
        res = QApplication::sendSpontaneousEvent(widget, &e);
        res = res && e.isAccepted();
        if (type == QEvent::MouseButtonRelease && button == Qt::RightButton) {
            QContextMenuEvent e2(QContextMenuEvent::Mouse, pos, QPoint(gpos.x,gpos.y));
            bool res2 = QApplication::sendSpontaneousEvent(widget, &e2);
            if (!res)
                res = res2 && e2.isAccepted();
        }

        if (type != QEvent::MouseMove)
            pos.rx() = pos.ry() = -9999;        // init for move compression
    }
    return res;
}


//
// Keyboard event translation
//

static const uint KeyTbl[] = {                // keyboard mapping table
    VK_ESCAPE,                Qt::Key_Escape,                // misc keys
    VK_TAB,                Qt::Key_Tab,
    VK_BACK,                Qt::Key_Backspace,
    VK_RETURN,                Qt::Key_Return,
    VK_INSERT,                Qt::Key_Insert,
    VK_DELETE,                Qt::Key_Delete,
    VK_CLEAR,                Qt::Key_Clear,
    VK_PAUSE,                Qt::Key_Pause,
    VK_SNAPSHOT,        Qt::Key_Print,
    VK_HOME,                Qt::Key_Home,                // cursor movement
    VK_END,                Qt::Key_End,
    VK_LEFT,                Qt::Key_Left,
    VK_UP,                Qt::Key_Up,
    VK_RIGHT,                Qt::Key_Right,
    VK_DOWN,                Qt::Key_Down,
    VK_PRIOR,                Qt::Key_PageUp,
    VK_NEXT,                Qt::Key_PageDown,
    VK_SHIFT,                Qt::Key_Shift,                // modifiers
    VK_CONTROL,                Qt::Key_Control,
    VK_LWIN,                Qt::Key_Meta,
    VK_RWIN,                Qt::Key_Meta,
    VK_MENU,                Qt::Key_Alt,
    VK_CAPITAL,                Qt::Key_CapsLock,
    VK_NUMLOCK,                Qt::Key_NumLock,
    VK_SCROLL,                Qt::Key_ScrollLock,
    VK_NUMPAD0,                Qt::Key_0,                        // numeric Keypad
    VK_NUMPAD1,                Qt::Key_1,
    VK_NUMPAD2,                Qt::Key_2,
    VK_NUMPAD3,                Qt::Key_3,
    VK_NUMPAD4,                Qt::Key_4,
    VK_NUMPAD5,                Qt::Key_5,
    VK_NUMPAD6,                Qt::Key_6,
    VK_NUMPAD7,                Qt::Key_7,
    VK_NUMPAD8,                Qt::Key_8,
    VK_NUMPAD9,                Qt::Key_9,
    VK_MULTIPLY,        Qt::Key_Asterisk,
    VK_ADD,                Qt::Key_Plus,
    VK_SEPARATOR,        Qt::Key_Comma,
    VK_SUBTRACT,        Qt::Key_Minus,
    VK_DECIMAL,                Qt::Key_Period,
    VK_DIVIDE,                Qt::Key_Slash,
    VK_APPS,                Qt::Key_Menu,
    0,                        0
};

static int translateKeyCode(int key)                // get Qt::Key_... code
{
    int code;
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
        code = 0;
    } else if (key >= VK_F1 && key <= VK_F24) {
        code = Qt::Key_F1 + (key - VK_F1);                // function keys
    } else {
        int i = 0;                                // any other keys
        code = 0;
        while (KeyTbl[i]) {
            if (key == (int)KeyTbl[i]) {
                code = KeyTbl[i+1];
                break;
            }
            i += 2;
        }
    }
    return code;
}

Q_GUI_EXPORT int qt_translateKeyCode(int key)
{
    return translateKeyCode(key);
}

static int asciiToKeycode(char a, int state)
{
    if (a >= 'a' && a <= 'z')
        a = toupper(a);
    if ((state & Qt::ControlModifier) != 0) {
        if ( a >= 0 && a <= 31 )      // Ctrl+@..Ctrl+A..CTRL+Z..Ctrl+_
            a += '@';                 // to @..A..Z.._
    }

    return a & 0xff;
}

static
QChar wmchar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA({
        return QChar((ushort)c);
    } , {
        char mb[2];
        mb[0] = c&0xff;
        mb[1] = 0;
        WCHAR wc[1];
        MultiByteToWideChar(inputcharset, MB_PRECOMPOSED, mb, -1, wc, 1);
        return QChar(wc[0]);
    });
}

static
QChar imechar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA({
        return QChar((ushort)c);
    } , {
        char mb[3];
        mb[0] = (c>>8)&0xff;
        mb[1] = c&0xff;
        mb[2] = 0;
        WCHAR wc[1];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
            mb, -1, wc, 1);
        return QChar(wc[0]);
    });
}

bool QETWidget::translateKeyEvent(const MSG &msg, bool grab)
{
    bool k0=false, k1=false;
    int  state = 0;

    if (sm_blockUserInput) // block user interaction during session management
        return true;

    if (GetKeyState(VK_SHIFT) < 0)
        state |= Qt::ShiftModifier;
    if (GetKeyState(VK_CONTROL) < 0)
        state |= Qt::ControlModifier;
    if (GetKeyState(VK_MENU) < 0)
        state |= Qt::AltModifier;
    if ((GetKeyState(VK_LWIN) < 0) ||
         (GetKeyState(VK_RWIN) < 0))
        state |= Qt::MetaModifier;

    if (msg.message == WM_CHAR) {
        // a multi-character key not found by our look-ahead
        QString s;
        QChar ch = wmchar_to_unicode(msg.wParam);
        if (!ch.isNull())
            s += ch;
        k0 = sendKeyEvent(QEvent::KeyPress, 0, state, grab, s);
        k1 = sendKeyEvent(QEvent::KeyRelease, 0, state, grab, s);
    }
    else if (msg.message == WM_IME_CHAR) {
        // input method characters not found by our look-ahead
        QString s;
        QChar ch = imechar_to_unicode(msg.wParam);
        if (!ch.isNull())
            s += ch;
        k0 = sendKeyEvent(QEvent::KeyPress, 0, state, grab, s);
        k1 = sendKeyEvent(QEvent::KeyRelease, 0, state, grab, s);
    } else {
        extern bool qt_use_rtl_extensions;
        if (qt_use_rtl_extensions) {
            // for Directionality changes (BiDi)
            static int dirStatus = 0;
            if (!dirStatus && state == Qt::ControlModifier && msg.wParam == VK_CONTROL && msg.message == WM_KEYDOWN) {
                if (GetKeyState(VK_LCONTROL) < 0) {
                    dirStatus = VK_LCONTROL;
                } else if (GetKeyState(VK_RCONTROL) < 0) {
                    dirStatus = VK_RCONTROL;
                }
            } else if (dirStatus) {
                if (msg.message == WM_KEYDOWN) {
                    if (msg.wParam == VK_SHIFT) {
                        if (dirStatus == VK_LCONTROL && GetKeyState(VK_LSHIFT) < 0) {
                                dirStatus = VK_LSHIFT;
                        } else if (dirStatus == VK_RCONTROL && GetKeyState(VK_RSHIFT) < 0) {
                            dirStatus = VK_RSHIFT;
                        }
                    } else {
                        dirStatus = 0;
                    }
                } else if (msg.message == WM_KEYUP) {
                    if (dirStatus == VK_LSHIFT &&
                        (msg.wParam == VK_SHIFT && GetKeyState(VK_LCONTROL)  ||
                          msg.wParam == VK_CONTROL && GetKeyState(VK_LSHIFT))) {
                        k0 = sendKeyEvent(QEvent::KeyPress, Qt::Key_Direction_L, 0, grab, QString());
                        k1 = sendKeyEvent(QEvent::KeyRelease, Qt::Key_Direction_L, 0, grab, QString());
                        dirStatus = 0;
                    } else if (dirStatus == VK_RSHIFT &&
                        (msg.wParam == VK_SHIFT && GetKeyState(VK_RCONTROL) ||
                          msg.wParam == VK_CONTROL && GetKeyState(VK_RSHIFT))) {
                        k0 = sendKeyEvent(QEvent::KeyPress, Qt::Key_Direction_R, 0, grab, QString());
                        k1 = sendKeyEvent(QEvent::KeyRelease, Qt::Key_Direction_R, 0, grab, QString());
                        dirStatus = 0;
                    } else {
                        dirStatus = 0;
                    }
                } else {
                    dirStatus = 0;
                }
            }
        }

        if(msg.wParam == VK_PROCESSKEY)
            // the IME will process these
            return true;

        int code = translateKeyCode(msg.wParam);
        // Invert state logic
        if (code == Qt::Key_Alt)
            state = state^Qt::AltModifier;
        else if (code == Qt::Key_Control)
            state = state^Qt::ControlModifier;
        else if (code == Qt::Key_Shift)
            state = state^Qt::ShiftModifier;

        // If the bit 24 of lParm is set you received a enter,
        // otherwise a Return. (This is the extended key bit)
        if ((code == Qt::Key_Return) && (msg.lParam & 0x1000000)) {
            code = Qt::Key_Enter;
        }

        if (!(msg.lParam & 0x1000000)) {        // All cursor keys without extended bit
            switch (code) {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_Insert:
            case Qt::Key_Delete:
            case Qt::Key_Asterisk:
            case Qt::Key_Plus:
            case Qt::Key_Minus:
            case Qt::Key_Period:
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
                state |= Qt::KeypadModifier;
            default:
                if ((uint)msg.lParam == 0x004c0001 ||
                     (uint)msg.lParam == 0xc04c0001)
                    state |= Qt::KeypadModifier;
                break;
            }
        } else {                                // And some with extended bit
            switch (code) {
            case Qt::Key_Enter:
            case Qt::Key_Slash:
            case Qt::Key_NumLock:
                state |= Qt::KeypadModifier;
            default:
                break;
            }
        }

        int t = msg.message;
        if (t == WM_KEYDOWN || t == WM_IME_KEYDOWN || t == WM_SYSKEYDOWN) {
            // KEYDOWN
            KeyRec* rec = find_key_rec(msg.wParam, false);
            // If rec's state doesn't match the current state, something
            // has changed without us knowning about it. (Consumed by
            // modal widget is one posibility) So, remove rec from list.
            if ( rec && rec->state != state ) {
                find_key_rec( msg.wParam, TRUE );
                rec = 0;
            }
            // Find uch
            QChar uch;
            MSG wm_char;
            UINT charType = (t == WM_KEYDOWN ? WM_CHAR :
                              t == WM_IME_KEYDOWN ? WM_IME_CHAR : WM_SYSCHAR);
            if (winPeekMessage(&wm_char, 0, charType, charType, PM_REMOVE)) {
                // Found a XXX_CHAR
                uch = charType == WM_IME_CHAR
                        ? imechar_to_unicode(wm_char.wParam)
                        : wmchar_to_unicode(wm_char.wParam);
                if (t == WM_SYSKEYDOWN &&
                     uch.isLetter() && (msg.lParam & KF_ALTDOWN)) {
                    // (See doc of WM_SYSCHAR)
                    uch = uch.toLower(); //Alt-letter
                }
                if (!code && !uch.row())
                    code = asciiToKeycode(uch.cell(), state);
            }
            if (uch.isNull()) {
                // No XXX_CHAR; deduce uch from XXX_KEYDOWN params
                if (msg.wParam == VK_DELETE)
                    uch = QChar((char)0x7f); // Windows doesn't know this one.
                else {
                    if (t != WM_SYSKEYDOWN || !code) {
                        UINT map;
                        QT_WA({
                            map = MapVirtualKey(msg.wParam, 2);
                        } , {
                            map = MapVirtualKeyA(msg.wParam, 2);
                            // High-order bit is 0x8000 on '95
                            if (map & 0x8000)
                                map = (map^0x8000)|0x80000000;
                        });
                        // If the high bit of the return value of
                        // MapVirtualKey is set, the key is a deadkey.
                        if (!(map & 0x80000000)) {
                            uch = wmchar_to_unicode((DWORD)map);
                        }
                    }
                }
                if (!code && !uch.row())
                    code = asciiToKeycode(uch.cell(), state);
            }

            if (state == Qt::AltModifier) {
                // Special handling of global Windows hotkeys
                switch (code) {
                case Qt::Key_Escape:
                case Qt::Key_Tab:
                case Qt::Key_Enter:
                case Qt::Key_F4:
                    return false;                // Send the event on to Windows
                case Qt::Key_Space:
                    // do not pass this key to windows, we will process it ourselves
                    qt_show_system_menu(window());
                    return true;
                default:
                    break;
                }
            }

            // map shift+tab to shift+backtab, QShortcutMap knows about it
            // and will handle it
            if (code == Qt::Key_Tab && (state & Qt::ShiftModifier) == Qt::ShiftModifier)
                code = Qt::Key_Backtab;

            if (rec) {
                // it is already down (so it is auto-repeating)
                if (code < Qt::Key_Shift || code > Qt::Key_ScrollLock) {
                    k0 = sendKeyEvent(QEvent::KeyRelease, code, state, grab, rec->text, true);
                    k1 = sendKeyEvent(QEvent::KeyPress, code, state, grab, rec->text, true);
                }
            } else {
                QString text;
                if (!uch.isNull())
                    text += uch;
                char a = uch.row() ? 0 : uch.cell();
                k0 = sendKeyEvent(QEvent::KeyPress, code, state, grab, text);

                bool store = true;
                // Alt+<alphanumerical> go to the Win32 menu system if unhandled by Qt
                if (msg.message == WM_SYSKEYDOWN && !k0 && a) {
                    HWND parent = GetParent(winId());
                    while (parent) {
                        if (GetMenu(parent)) {
                            SendMessage(parent, WM_SYSCOMMAND, SC_KEYMENU, a);
                            store = false;
                            k0 = true;
                            break;
                        }
                        parent = GetParent(parent);
                    }
                }
                if (store)
                    store_key_rec( msg.wParam, a, state, text );
            }
        } else {
            // Must be KEYUP
            KeyRec* rec = find_key_rec(msg.wParam, true);
            if (!rec) {
                // Someone ate the key down event
            } else {
                if (!code)
                    code = asciiToKeycode(rec->ascii ? rec->ascii : msg.wParam, state);
                // see comment above
                if (code == Qt::Key_Tab && (state & Qt::ShiftModifier) == Qt::ShiftModifier)
                    code = Qt::Key_Backtab;

                k0 = sendKeyEvent(QEvent::KeyRelease, code, state, grab, rec->text);

                // don't pass Alt to Windows unless we are embedded in a non-Qt window
                if ( code == Qt::Key_Alt ) {
                    k0 = true;
                    HWND parent = GetParent(winId());
                    while (parent) {
                        if (!QWidget::find(parent) && GetMenu(parent)) {
                            k0 = false;
                            break;
                        }
                        parent = GetParent(parent);
                    }
                }
            }
        }
    }

    return k0 || k1;
}


bool QETWidget::translateWheelEvent(const MSG &msg)
{
    int  state = 0;

    if (sm_blockUserInput) // block user interaction during session management
        return true;

    if (GetKeyState(VK_SHIFT) < 0)
        state |= Qt::ShiftModifier;
    if (GetKeyState(VK_CONTROL) < 0)
        state |= Qt::ControlModifier;
    if (GetKeyState(VK_MENU) < 0)
        state |= Qt::AltModifier;
    if ((GetKeyState(VK_LWIN) < 0) ||
         (GetKeyState(VK_RWIN) < 0))
        state |= Qt::MetaModifier;

    int delta;
    if (msg.message == WM_MOUSEWHEEL)
        delta = (short) HIWORD (msg.wParam);
    else
        delta = (int) msg.wParam;

    Qt::Orientation orient = (state&Qt::AltModifier
#if 0 // disabled for now - Trenton's one-wheel mouse makes trouble...
    // "delta" for usual wheels is +-120. +-240 seems to indicate the second wheel
    // see more recent MSDN for WM_MOUSEWHEEL

         || delta == 240 || delta == -240)?Qt::Horizontal:Vertical;
    if (delta == 240 || delta == -240)
        delta /= 2;
#endif
       ) ? Qt::Horizontal : Qt::Vertical;

    QPoint globalPos;

    globalPos.rx() = (short)LOWORD (msg.lParam);
    globalPos.ry() = (short)HIWORD (msg.lParam);


    // if there is a widget under the mouse and it is not shadowed
    // by modality, we send the event to it first
    int ret = 0;
    QWidget* w = QApplication::widgetAt(globalPos);
    if (!w || !qt_try_modal(w, (MSG*)&msg, ret))
        w = this;

    // send the event to the widget or its ancestors
    {
        QWidget* popup = qApp->activePopupWidget();
        if (popup && w->window() != popup)
            popup->close();
        QWheelEvent e(w->mapFromGlobal(globalPos), globalPos, delta,
                      Qt::MouseButtons(state & Qt::MouseButtonMask),
                      Qt::KeyboardModifier(state & Qt::KeyboardModifierMask), orient);
        if (QApplication::sendSpontaneousEvent(w, &e))
            return true;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    if (w != qApp->focusWidget() && (w = qApp->focusWidget())) {
        QWidget* popup = qApp->activePopupWidget();
        if (popup && w->window() != popup)
            popup->close();
        QWheelEvent e(w->mapFromGlobal(globalPos), globalPos, delta,
                      Qt::MouseButtons(state & Qt::MouseButtonMask),
                      Qt::KeyboardModifier(state & Qt::KeyboardModifierMask), orient);
        if (QApplication::sendSpontaneousEvent(w, &e))
            return true;
    }
    return false;
}


//
// Windows Wintab to QTabletEvent translation
//
typedef QHash<UINT, TabletDeviceData> TabletCursorInfo;
Q_GLOBAL_STATIC(TabletCursorInfo, tCursorInfo)

// the following is adapted from the wintab syspress example (public domain)
/* -------------------------------------------------------------------------- */
static void tabletInit(UINT wActiveCsr, HCTX hTab)
{
    /* browse WinTab's many info items to discover pressure handling. */
    if (ptrWTInfo && ptrWTGet) {
        AXIS np;
        LOGCONTEXT lc;
        BYTE wPrsBtn;
        BYTE logBtns[32];
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
        TabletDeviceData tdd;
        ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_NPRESSURE, &np);
        tdd.minPressure = int(np.axMin);
        tdd.maxPressure = int(np.axMax);

        ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_TPRESSURE, &np);
        tdd.minTanPressure = int(np.axMin);
        tdd.maxTanPressure = int(np.axMax);

        ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_X, &np);
        tdd.minX = int(np.axMin);
        tdd.maxX = int(np.axMax);

        ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_Y, &np);
        tdd.minY = int(np.axMin);
        tdd.maxY = int(np.axMax);

        ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_Z, &np);
        tdd.minZ = int(np.axMin);
        tdd.maxZ = int(np.axMax);

        tCursorInfo()->insert(wActiveCsr, tdd);
    }
}

bool QETWidget::translateTabletEvent(const MSG &msg, PACKET *localPacketBuf,
                                      int numPackets)
{
    Q_UNUSED(msg);
    POINT ptNew;
    static DWORD btnNew, btnOld, btnChange;
    qreal prsNew;
    ORIENTATION ort;
    static bool button_pressed = false;
    int i,
        dev,
        pointerType,
        tiltX,
        tiltY;
    bool sendEvent = false;
    QEvent::Type t;
    int z = 0;
    qreal rotation = 0.0;
    qreal tangentialPressure;

    // the most common event that we get...
    t = QEvent::TabletMove;
    for (i = 0; i < numPackets; i++) {
        // get the unique ID of the device...
        int csr_type,
            csr_physid;
        ptrWTInfo(WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_TYPE, &csr_type);
        ptrWTInfo(WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_PHYSID, &csr_physid);
        qint64 llId = csr_type;
        llId = (llId << 24) | csr_physid;
        switch (csr_type & 0x0F06) {
        case 0x0802:
            dev = QTabletEvent::Stylus;
            break;
        case 0x0902:
            dev = QTabletEvent::Airbrush;
            break;
        case 0x0004:
            dev = QTabletEvent::FourDMouse;
            break;
        case 0x0006:
            dev = QTabletEvent::Puck;
            break;
        default:
            dev = QTabletEvent::NoDevice;
        }

        switch (localPacketBuf[i].pkCursor) {
        case 2:
            pointerType = QTabletEvent::Eraser;
            break;
        case 1:
            pointerType = QTabletEvent::Pen;
            break;
        case 0:
            pointerType = QTabletEvent::Cursor;
            break;
        default:
            pointerType = QTabletEvent::UnknownPointer;
        }
        btnOld = btnNew;
        btnNew = localPacketBuf[i].pkButtons;
        btnChange = btnOld ^ btnNew;

        if (btnNew & btnChange) {
            button_pressed = true;
            t = QEvent::TabletPress;
        }
        ptNew.x = UINT(localPacketBuf[i].pkX);
        ptNew.y = UINT(localPacketBuf[i].pkY);

        z = (dev == QTabletEvent::FourDMouse) ? UINT(localPacketBuf[i].pkZ) : 0;

        prsNew = 0.0;
        if (!tCursorInfo()->contains(localPacketBuf[i].pkCursor))
            tabletInit(localPacketBuf[i].pkCursor, qt_tablet_context);
        TabletDeviceData tdd = tCursorInfo()->value(localPacketBuf[i].pkCursor);
        QSize desktopSize = qt_desktopWidget->screenGeometry().size();
        QPointF hiResGlobal = tdd.scaleCoord(ptNew.x, ptNew.y, 0, desktopSize.width(),
                                             0, desktopSize.height());
        if (btnNew) {
            // I'm not sure what is going on here. It could be the driver or it could be something else
            // But as near as I can figure, the pressure is being reported in the Z_AXIS now instead of in
            // the normal pressure area. So that that into account...
            if (pointerType == QTabletEvent::Pen || pointerType == QTabletEvent::Eraser)
                prsNew = localPacketBuf[i].pkZ / qreal(tdd.maxPressure - tdd.minPressure);
            else
                prsNew = 0;
        } else if (button_pressed) {
            // One button press, should only give one button release
            t = QEvent::TabletRelease;
            button_pressed = false;
        }
        // Truncate the stuff here as that what wintab does.
        QPoint globalPos(hiResGlobal.x(), hiResGlobal.y());

        // make sure the tablet event get's sent to the proper widget...
        QWidget *w = QApplication::widgetAt(globalPos);
        if (!w)
            w = this;
        QPoint localPos = w->mapFromGlobal(globalPos);
        if (dev == QTabletEvent::Airbrush) {
            tangentialPressure = localPacketBuf[i].pkTangentPressure
                                / qreal(tdd.maxTanPressure - tdd.minTanPressure);
        } else {
            tangentialPressure = 0.0;
        }

        if (!qt_tablet_tilt_support) {
            tiltX = tiltY = 0;
            rotation = 0.0;
        } else {
            ort = localPacketBuf[i].pkOrientation;
            // convert from azimuth and altitude to x tilt and y tilt
            // what follows is the optimized version.  Here are the equations
            // I used to get to this point (in case things change :)
            // X = sin(azimuth) * cos(altitude)
            // Y = cos(azimuth) * cos(altitude)
            // Z = sin(altitude)
            // X Tilt = arctan(X / Z)
            // Y Tilt = arctan(Y / Z)
            double radAzim = (ort.orAzimuth / 10) * (Q_PI / 180);
            //double radAlt = abs(ort.orAltitude / 10) * (Q_PI / 180);
            double tanAlt = tan((abs(ort.orAltitude / 10)) * (Q_PI / 180));

            double degX = atan(sin(radAzim) / tanAlt);
            double degY = atan(cos(radAzim) / tanAlt);
            tiltX = int(degX * (180 / Q_PI));
            tiltY = int(-degY * (180 / Q_PI));
            rotation = ort.orTwist;
        }

        QTabletEvent e(t, localPos, globalPos, hiResGlobal, dev, pointerType,
                       prsNew, tiltX, tiltY, tangentialPressure,
                       rotation, z, 0, llId);
        sendEvent = QApplication::sendSpontaneousEvent(w, &e);
    }
    return sendEvent;
}

extern bool qt_is_gui_used;
static void initWinTabFunctions()
{
    if (!qt_is_gui_used)
        return;

    QLibrary library("wintab32");
    if (library.load()) {
        QT_WA({
            ptrWTInfo = (PtrWTInfo)library.resolve("WTInfoW");
            ptrWTGet = (PtrWTGet)library.resolve("WTGetW");
        } , {
            ptrWTInfo = (PtrWTInfo)library.resolve("WTInfoA");
            ptrWTGet = (PtrWTGet)library.resolve("WTGetA");
        });

        ptrWTEnable = (PtrWTEnable)library.resolve("WTEnable");
        ptrWTOverlap = (PtrWTEnable)library.resolve("WTOverlap");
        ptrWTPacketsGet = (PtrWTPacketsGet)library.resolve("WTPacketsGet");
    }
}

static bool isModifierKey(int code)
{
    return code >= Qt::Key_Shift && code <= Qt::Key_ScrollLock;
}

bool QETWidget::sendKeyEvent(QEvent::Type type, int code,
                              int state, bool grab, const QString& text,
                              bool autor)
{
#if defined QT3_SUPPORT && !defined(QT_NO_ACCEL)
    if (type == QEvent::KeyPress && !grab
        && static_cast<QApplicationPrivate*>(qApp->d_ptr)->use_compat()) {
        // send accel events if the keyboard is not grabbed
        QKeyEvent a(type, code, 0, Qt::KeyboardModifierMask & state, text, autor,
                    qMax(1, int(text.length())));
        if (static_cast<QApplicationPrivate*>(qApp->d_ptr)->qt_tryAccelEvent(this, &a))
            return true;
    }
#endif
    if (!isEnabled())
        return false;
    QKeyEvent e(type, code, Qt::KeyboardModifiers(state & Qt::KeyboardModifierMask), text,
                autor, qMax(1, int(text.length())));
    QApplication::sendSpontaneousEvent(this, &e);
    if (!isModifierKey(code) && state == Qt::AltModifier
         && ((code>=Qt::Key_A && code<=Qt::Key_Z) || (code>=Qt::Key_0 && code<=Qt::Key_9))
         && type == QEvent::KeyPress && !e.isAccepted())
        QApplication::beep();  // emulate windows behaviour
    return e.isAccepted();
}

//
// Paint event translation
//
bool QETWidget::translatePaintEvent(const MSG &)
{
    QRegion rgn(0, 0, 1, 1);
    int res = GetUpdateRgn(winId(), rgn.handle(), false);
    if (!GetUpdateRect(winId(), 0, false)  // The update bounding rect is invalid
         || (res == ERROR)
         || (res == NULLREGION)) {
        d_func()->hd = 0;
        return false;
    }

    PAINTSTRUCT ps;
    d_func()->hd = BeginPaint(winId(), &ps);

    // Mapping region from system to qt (32 bit) coordinate system.
    rgn.translate(data->wrect.topLeft());
    repaint(rgn);

    d_func()->hd = 0;
    EndPaint(winId(), &ps);
    return true;
}

//
// Window move and resize (configure) events
//

bool QETWidget::translateConfigEvent(const MSG &msg)
{
    if (!testAttribute(Qt::WA_WState_Created))                // in QWidget::create()
        return true;
    if (testAttribute(Qt::WA_WState_ConfigPending))
        return true;
    if (!isWindow())
        return true;
    setAttribute(Qt::WA_WState_ConfigPending);                // set config flag
    QRect cr = geometry();
    if (msg.message == WM_SIZE) {                // resize event
        WORD a = LOWORD(msg.lParam);
        WORD b = HIWORD(msg.lParam);
        QSize oldSize = size();
        QSize newSize(a, b);
        cr.setSize(newSize);
        if (msg.wParam != SIZE_MINIMIZED)
            data->crect = cr;
        if (isWindow()) {                        // update title/icon text
            d_func()->createTLExtra();
            // Capture SIZE_MINIMIZED without preceding WM_SYSCOMMAND
            // (like Windows+M)
            if (msg.wParam == SIZE_MINIMIZED && !isMinimized()) {
            data->window_state |= Qt::WindowMinimized;
            if (isVisible()) {
                QHideEvent e;
                QApplication::sendSpontaneousEvent(this, &e);
                hideChildren(true);
            }
        } else if (msg.wParam != SIZE_MINIMIZED && isMinimized()) {
            data->window_state &= ~Qt::WindowMinimized;
            showChildren(true);
            QShowEvent e;
            QApplication::sendSpontaneousEvent(this, &e);
        }
        QString txt;
#ifndef Q_OS_TEMP
        if (IsIconic(winId()) && windowIconText().size())
            txt = windowIconText();
        else
#endif
        txt = windowTitle();
        if (!txt.isEmpty())
            d_func()->setWindowTitle_helper(txt);
    }
    if (msg.wParam != SIZE_MINIMIZED && oldSize != newSize) {
        if (isVisible()) {
            QResizeEvent e(newSize, oldSize);
            QApplication::sendSpontaneousEvent(this, &e);
            if (!testAttribute(Qt::WA_StaticContents))
                testAttribute(Qt::WA_WState_InPaintEvent)?update():repaint();
        } else {
            QResizeEvent *e = new QResizeEvent(newSize, oldSize);
            QApplication::postEvent(this, e);
        }
    }
} else if (msg.message == WM_MOVE) {        // move event
        int a = (int) (short) LOWORD(msg.lParam);
        int b = (int) (short) HIWORD(msg.lParam);
        QPoint oldPos = geometry().topLeft();
        QPoint newCPos(a, b);
        // Ignore silly Windows move event to wild pos after iconify.
        if (!IsIconic(winId()) && newCPos != oldPos) {
            cr.moveTopLeft(newCPos);
            data->crect = cr;
            if (isVisible()) {
                QMoveEvent e(newCPos, oldPos);  // cpos (client position)
                QApplication::sendSpontaneousEvent(this, &e);
            } else {
                QMoveEvent * e = new QMoveEvent(newCPos, oldPos);
                QApplication::postEvent(this, e);
            }
        }
    }
    setAttribute(Qt::WA_WState_ConfigPending, false);                // clear config flag
    return true;
}


//
// Close window event translation.
//
// This class is a friend of QApplication because it needs to emit the
// lastWindowClosed() signal when the last top level widget is closed.
//

bool QETWidget::translateCloseEvent(const MSG &)
{
    return d_func()->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
}


void QETWidget::eraseWindowBackground(HDC hdc)
{
    if (testAttribute(Qt::WA_NoSystemBackground) || testAttribute(Qt::WA_UpdatesDisabled))
        return;

    const QWidget *w = this;
    QPoint offset;
    while (w->d_func()->isBackgroundInherited()) {
        offset += w->pos();
        w = w->parentWidget();
    }

    RECT r;
    GetClientRect(data->winid, &r);

    QWidget *that = const_cast<QWidget*>(w);
    that->setAttribute(Qt::WA_WState_InPaintEvent);

    qt_erase_background
        (hdc, r.left, r.top,
          r.right-r.left, r.bottom-r.top,
          data->pal.brush(w->backgroundRole()),
          offset.x(), offset.y(), const_cast<QWidget*>(w));
    that->setAttribute(Qt::WA_WState_InPaintEvent, false);
}


void  QApplication::setCursorFlashTime(int msecs)
{
    SetCaretBlinkTime(msecs / 2);
    QApplicationPrivate::cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    int blink = (int)GetCaretBlinkTime();
    if (!blink)
        return QApplicationPrivate::cursor_flash_time;
    if (blink > 0)
        return 2*blink;
    return 0;
}


void QApplication::setDoubleClickInterval(int ms)
{
#ifndef Q_OS_TEMP
    SetDoubleClickTime(ms);
#endif
    QApplicationPrivate::mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    int ms = GetDoubleClickTime();
    if (ms != 0)
        return ms;
    return QApplicationPrivate::mouse_double_click_time;
}


void QApplication::setKeyboardInputInterval(int ms)
{
    QApplicationPrivate::keyboard_input_time = ms;
}

int QApplication::keyboardInputInterval()
{
    // FIXME: get from the system
    return QApplicationPrivate::keyboard_input_time;
}


void QApplication::setWheelScrollLines(int n)
{
#ifdef SPI_SETWHEELSCROLLLINES
    if (n < 0)
        n = 0;
    QT_WA({
        SystemParametersInfo(SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0);
    } , {
        SystemParametersInfoA(SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0);
    });
#else
    QApplicationPrivate::wheel_scroll_lines = n;
#endif
}

int QApplication::wheelScrollLines()
{
#ifdef SPI_GETWHEELSCROLLLINES
    uint i = 3;
    QT_WA({
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, sizeof(uint), &i, 0);
    } , {
        SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, sizeof(uint), &i, 0);
    });
    if (i > INT_MAX)
        i = INT_MAX;
    return i;
#else
    return QApplicationPrivate::wheel_scroll_lines;
#endif
}

static bool effect_override = false;

void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
    effect_override = true;
    switch (effect) {
    case Qt::UI_AnimateMenu:
        QApplicationPrivate::animate_menu = enable;
        break;
    case Qt::UI_FadeMenu:
        QApplicationPrivate::fade_menu = enable;
        break;
    case Qt::UI_AnimateCombo:
        QApplicationPrivate::animate_combo = enable;
        break;
    case Qt::UI_AnimateTooltip:
        QApplicationPrivate::animate_tooltip = enable;
        break;
    case Qt::UI_FadeTooltip:
        QApplicationPrivate::fade_tooltip = enable;
        break;
    case Qt::UI_AnimateToolBox:
        QApplicationPrivate::animate_toolbox = enable;
        break;
    default:
        QApplicationPrivate::animate_ui = enable;
        break;
    }
}

bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
    if (QColormap::instance().depth() < 16)
        return false;

    if (!effect_override && desktopSettingsAware()
        && !(QSysInfo::WindowsVersion == QSysInfo::WV_95 || QSysInfo::WindowsVersion == QSysInfo::WV_NT)) {
        // we know that they can be used when we are here
        BOOL enabled = false;
        UINT api;
        switch (effect) {
        case Qt::UI_AnimateMenu:
            api = SPI_GETMENUANIMATION;
            break;
        case Qt::UI_FadeMenu:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                return false;
            api = SPI_GETMENUFADE;
            break;
        case Qt::UI_AnimateCombo:
            api = SPI_GETCOMBOBOXANIMATION;
            break;
        case Qt::UI_AnimateTooltip:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                api = SPI_GETMENUANIMATION;
            else
                api = SPI_GETTOOLTIPANIMATION;
            break;
        case Qt::UI_FadeTooltip:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                return false;
            api = SPI_GETTOOLTIPFADE;
            break;
        default:
            api = SPI_GETUIEFFECTS;
            break;
        }
        QT_WA({
            SystemParametersInfo(api, 0, &enabled, 0);
        } , {
            SystemParametersInfoA(api, 0, &enabled, 0);
        });
        return enabled;
    }

    switch(effect) {
    case Qt::UI_AnimateMenu:
        return QApplicationPrivate::animate_menu;
    case Qt::UI_FadeMenu:
        return QApplicationPrivate::fade_menu;
    case Qt::UI_AnimateCombo:
        return QApplicationPrivate::animate_combo;
    case Qt::UI_AnimateTooltip:
        return QApplicationPrivate::animate_tooltip;
    case Qt::UI_FadeTooltip:
        return QApplicationPrivate::fade_tooltip;
    case Qt::UI_AnimateToolBox:
        return QApplicationPrivate::animate_toolbox;
    default:
        return QApplicationPrivate::animate_ui;
    }
}

bool QSessionManager::allowsInteraction()
{
    sm_blockUserInput = false;
    return true;
}

bool QSessionManager::allowsErrorInteraction()
{
    sm_blockUserInput = false;
    return true;
}

void QSessionManager::release()
{
    if (sm_smActive)
        sm_blockUserInput = true;
}

void QSessionManager::cancel()
{
    sm_cancel = true;
}
