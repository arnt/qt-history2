/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindowdefs.h#60 $
**
** Definition of general window system dependent functions, types and
** constants
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
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


// Window system setting

#if defined(_OS_MAC_)
#define _WS_MAC_
#elif defined(_OS_MSDOS_)
#define _WS_WIN16_
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

HANDLE qWinAppInst();
HANDLE qWinAppPrevInst();
int    qWinAppCmdShow();
HANDLE qt_display_dc();

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

typedef unsigned long WId;
typedef unsigned long HANDLE;
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


// Global platform-independent types and functions

typedef short QCOORD;				// coordinate type
const QCOORD_MIN = -32768;
const QCOORD_MAX =  32767;

char *qAppName();				// get application name


// Misc functions

void  qAddPostRoutine( void (*)() );


// GUI styles

enum GUIStyle {
    MacStyle,
    WindowsStyle,
    Win3Style,
    PMStyle,
    MotifStyle
};


// Widget flags

typedef ulong WFlags;

const ulong WState_Created	= 0x00000001;	// widget state flags
const ulong WState_Disabled	= 0x00000002;
const ulong WState_Visible	= 0x00000004;
const ulong WState_Active	= 0x00000008;
const ulong WState_Paint	= 0x00000010;
const ulong WState_MGrab	= 0x00000020;
const ulong WState_KGrab	= 0x00000040;
const ulong WState_AcceptFocus	= 0x00000080;

const ulong WType_Overlap	= 0x00000100;	// widget type flags
const ulong WType_Modal		= 0x00000200;
const ulong WType_Popup		= 0x00000400;
const ulong WType_Desktop	= 0x00000800;

const ulong WStyle_Title	= 0x00001000;	// widget style flags
const ulong WStyle_Border	= 0x00002000;
const ulong WStyle_Close	= 0x00004000;
const ulong WStyle_Resize	= 0x00008000;
const ulong WStyle_Minimize	= 0x00010000;
const ulong WStyle_Maximize	= 0x00020000;
const ulong WStyle_MinMax	= WStyle_Minimize | WStyle_Maximize;
const ulong WStyle_All		= 0x0003f000;

const ulong WFont_Metrics	= 0x00040000;	// misc widget flags
const ulong WFont_Info		= 0x00080000;
const ulong WMouseTracking	= 0x00100000;
const ulong WHasAccel		= 0x00200000;
const ulong WConfigPending	= 0x00400000;
const ulong WResizeNoErase	= 0x00800000;
const ulong WExplicitHide	= 0x01000000;
const ulong WCursorSet		= 0x02000000;
const ulong WPaintDesktop	= 0x04000000;
const ulong WPaintUnclipped	= 0x08000000;
const ulong WPaintClever	= 0x10000000;
const ulong WNoUpdates		= 0x20000000;
const ulong WRecreated		= 0x40000000;


// Extra QWidget data

struct QWExtra {
    GUIStyle guistyle;				// GUI Style
    short  minw, minh;				// minimum size
    short  maxw, maxh;				// maximum size
    short  incw, inch;				// size increments
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
