/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindowdefs.h#83 $
**
** Definition of general window system dependent functions, types and
** constants
**
** Created : 931029
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWINDEFS_H
#define QWINDEFS_H

#include "qobjdefs.h"


// Class forward definitions

class QApplication;
class QPaintDevice;
class QPaintDeviceMetrics;
class QWidget;
class QWidgetMapper;
class QWindow;
class QDialog;
class QColor;
class QColorGroup;
class QPalette;
class QCursor;
class QPoint;
class QSize;
class QRect;
class QPointArray;
class QPainter;
class QRegion;
class QFont;
class QFontMetrics;
class QFontInfo;
class QPen;
class QBrush;
class QWMatrix;
class QPixmap;
class QBitmap;
class QImage;
class QImageIO;
class QPicture;
class QPrinter;
class QAccel;
class QTimer;
class QClipboard;


// Window system setting

#if defined(_OS_MAC_)
#define _WS_MAC_
#elif defined(_OS_MSDOS_)
#define _WS_WIN16_
#error "Qt requires Win32 and does not work with Windows 3.x"
#elif defined(_WIN32_X11_)
#define _WS_X11_
#elif defined(_OS_WIN32_)
#define _WS_WIN32_
#elif defined(_OS_OS2_)
#define _WS_PM_
#elif defined(UNIX)
#define _WS_X11_
#endif

#if defined(_WS_WIN16_) || defined(_WS_WIN32_)
#define _WS_WIN_
#endif


// Window system dependent definitions

#if defined(_WS_MAC_)
#endif // _WS_MAC_


#if defined(_WS_WIN_)

#if defined(_WS_WIN32_)
typedef void *HANDLE;
typedef void *WId;
typedef void *HDC;
#elif defined(_WS_WIN16_)
typedef uint HANDLE;
typedef uint WId;
typedef uint HDC;
#endif
typedef struct tagMSG MSG;

#if defined(_CC_BOR_)
#define NEEDS_QMAIN
#endif

HANDLE qWinAppInst();
HANDLE qWinAppPrevInst();
int    qWinAppCmdShow();
HANDLE qt_display_dc();

enum WindowsVersion { WV_NT, WV_95, WV_32s };

#endif // _WS_WIN16_ or _WS_WIN32_


#if defined(_WS_PM_)

typedef ulong HANDLE;
typedef ulong WId;
typedef ulong HAB;
typedef ulong HPS;
typedef ulong HDC;

typedef struct _QMSG   QMSG;
typedef struct _POINTL POINTL;
typedef struct _RECTL  RECTL;

HAB qPMAppInst();

#endif // _WS_PM_


#if defined(_WS_X11_)

typedef unsigned int  WId;
typedef unsigned int  HANDLE;
typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;
typedef struct _XGC *GC;
typedef struct _XRegion *Region;
struct QXFontStruct;

Display *qt_xdisplay();
int	 qt_xscreen();
WId	 qt_xrootwin();
GC	 qt_xget_readonly_gc( bool monochrome=FALSE );
GC	 qt_xget_temp_gc( bool monochrome=FALSE );

#endif // _WS_X11_


#if defined(NEEDS_QMAIN)
#define main qMain
#endif


// Global platform-independent types and functions

typedef short QCOORD;				// coordinate type
const int QCOORD_MIN = -32768;
const int QCOORD_MAX =	32767;

typedef unsigned int QRgb;			// RGB triplet

char *qAppName();				// get application name


// Misc functions

typedef void (*CleanUpFunction)();
void  qAddPostRoutine( CleanUpFunction );


void *qt_find_obj_child( QObject *, const char *, const char * );
#define CHILD(parent,type,name) \
	((type*)qt_find_obj_child(parent,#type,name))


// GUI styles

enum GUIStyle {
    MacStyle,
    WindowsStyle,
    Win3Style,
    PMStyle,
    MotifStyle
};


// Widget flags

typedef uint WFlags;

const uint WState_Created	= 0x00000001;	// widget state flags
const uint WState_Disabled	= 0x00000002;
const uint WState_Visible	= 0x00000004;
const uint WState_DoHide	= 0x00000008;
const uint WState_AcceptFocus	= 0x00000010;
const uint WState_TrackMouse	= 0x00000020;
const uint WState_BlockUpdates	= 0x00000040;
const uint WState_PaintEvent	= 0x00000080;

const uint WType_TopLevel	= 0x00000100;	// widget type flags
const uint WType_Modal		= 0x00000200;
const uint WType_Popup		= 0x00000400;
const uint WType_Desktop	= 0x00000800;

const uint WStyle_Customize	= 0x00001000;	// window style flags
const uint WStyle_NormalBorder	= 0x00002000;
const uint WStyle_DialogBorder	= 0x00004000;
const uint WStyle_NoBorder	= 0x00000000;
const uint WStyle_Title		= 0x00008000;
const uint WStyle_SysMenu	= 0x00010000;
const uint WStyle_Minimize	= 0x00020000;
const uint WStyle_Maximize	= 0x00040000;
const uint WStyle_MinMax	= WStyle_Minimize | WStyle_Maximize;
const uint WStyle_Tool		= 0x00080000;
const uint WStyle_Mask		= 0x000ff000;

const uint WCursorSet		= 0x00100000;	// misc widget flags
const uint WDestructiveClose	= 0x00200000;
const uint WPaintDesktop	= 0x00400000;
const uint WPaintUnclipped	= 0x00800000;
const uint WPaintClever		= 0x01000000;
const uint WConfigPending	= 0x02000000;
const uint WResizeNoErase	= 0x04000000;
const uint WRecreated		= 0x08000000;
const uint WExportFontMetrics	= 0x10000000;
const uint WExportFontInfo	= 0x20000000;
const uint WFocusSet		= 0x40000000;


// Extra QWidget data
//  - to minimize memory usage for members that are seldom used.

struct QWExtra {
    GUIStyle guistyle;				// GUI Style
    short    minw, minh;			// minimum size
    short    maxw, maxh;			// maximum size
    short    incw, inch;			// size increments
    char    *caption;				// widget caption
    char    *iconText;				// widget icon text
    QPixmap *icon;				// widget icon
    QPixmap *bg_pix;				// background pixmap
};


// Raster operations

enum RasterOp					// raster op/transfer mode
    { CopyROP, OrROP, XorROP, EraseROP,
      NotCopyROP, NotOrROP, NotXorROP, NotEraseROP, NotROP };


// Text formatting flags for QPainter::drawText and QLabel

const int AlignLeft	= 0x0001;		// text alignment
const int AlignRight	= 0x0002;
const int AlignHCenter	= 0x0004;
const int AlignTop	= 0x0008;
const int AlignBottom	= 0x0010;
const int AlignVCenter	= 0x0020;
const int AlignCenter	= AlignVCenter | AlignHCenter;

const int SingleLine	= 0x0040;		// misc. flags
const int DontClip	= 0x0080;
const int ExpandTabs	= 0x0100;
const int ShowPrefix	= 0x0200;
const int WordBreak	= 0x0400;
const int GrayText	= 0x0800;
const int DontPrint	= 0x1000;		// internal


#endif // QWINDEFS_H
