/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindefs.h#47 $
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
class QView;
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
class Q2DMatrix;
class QPixmap;
class QBitmap;
class QPicture;
class QPrinter;
class QAccel;
class QTimer;


// Window system setting

#if defined(_OS_MAC_)
#define _WS_MAC_
#elif defined(_OS_MSDOS_)
#define _WS_WIN16_
#elif defined(_OS_WINNT_)
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
HANDLE qWinPrevAppInst();
int    qWinAppCmdShow();

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
typedef unsigned long Atom;
typedef unsigned long Window;
typedef unsigned long Pixmap;
typedef unsigned long Cursor;
typedef unsigned long Font;
typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;
typedef struct _XGC *GC;
typedef struct _XRegion *Region;
struct QXFontStruct;

Display *qt_xdisplay();
int	 qt_xscreen();
Window	 qt_xrootwin();
GC	 qt_xget_readonly_gc( bool monochrome=FALSE );
GC	 qt_xget_temp_gc( bool monochrome=FALSE );

#endif // _WS_X11_


// Useful macros etc.

class QListM_QPainter;				// internal class for QPainter
#define QPnList QListM_QPainter


// Global platform-independent types and functions

typedef short QCOORD;				// coordinate type
const QCOORD_MIN = -32768;
const QCOORD_MAX =  32767;

char *qAppName();				// get application name


// Misc functions

void  qAddPreRoutine( void (*)() );
void  qAddPostRoutine( void (*)() );

class QAddPreRoutine {				// class for registering pre-
public:						//    routines
    typedef void (*vf)();
    QAddPreRoutine( void (*p)( int, char ** ) ) { qAddPreRoutine((vf)p); }
    QAddPreRoutine( void (*p)() ) { qAddPreRoutine(p); }
};


// GUI styles

enum GUIStyle {
    MacStyle,
    WindowsStyle,
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
const ulong WState_Focus	= 0x00000080;

const ulong WType_Overlap	= 0x00000100;	// widget type flags
const ulong WType_Modal		= 0x00000200;
const ulong WType_Popup		= 0x00000400;
const ulong WType_Desktop	= 0x00000800;

const ulong WStyle_Title	= 0x00001000;	// widget style flags
const ulong WStyle_Border	= 0x00002000;
const ulong WStyle_Close	= 0x00004000;
const ulong WStyle_Resize	= 0x00008000;
const ulong WStyle_Minimize 	= 0x00010000;
const ulong WStyle_Maximize 	= 0x00020000;
const ulong WStyle_MinMax	= WStyle_Minimize | WStyle_Maximize;
const ulong WStyle_All		= 0x000ff000;

const ulong WMouseTracking	= 0x00100000;	// misc widget flags
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


// Extra Widget data

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

const AlignLeft		= 0x000;		// text alignment
const AlignRight	= 0x001;
const AlignHCenter	= 0x002;
const AlignTop		= 0x000;
const AlignBottom	= 0x004;
const AlignVCenter	= 0x008;
const AlignCenter	= AlignVCenter | AlignHCenter;

const SingleLine	= 0x010;		// misc. flags
const DontClip		= 0x020;
const ExpandTabs	= 0x040;
const ShowPrefix	= 0x080;
const WordBreak		= 0x100;
const GrayText		= 0x200;
const DontPrint		= 0x400;


#endif // QWINDEFS_H
