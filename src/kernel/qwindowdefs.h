/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindowdefs.h#141 $
**
** Definition of general window system dependent functions, types and
** constants
**
** Created : 931029
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWINDOWDEFS_H
#define QWINDOWDEFS_H

#ifndef QT_H
#include "qobjectdefs.h"
#include "qstring.h"
#include "qnamespace.h"
#endif // QT_H


// Class forward definitions

class QPaintDevice;
class QPaintDeviceMetrics;
class QWidget;
class QWidgetMapper;
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
class QMovie;
class QImage;
class QImageIO;
class QPicture;
class QPrinter;
class QAccel;
class QTimer;
class QClipboard;


// Widget list (defined in qwidgetlist.h)

class QWidgetList;
class QWidgetListIt;


// Window system dependent definitions

#if defined(_WS_MAC_)
#endif // _WS_MAC_


#if defined(_WS_WIN_)

#if defined(_CC_BOR_) || defined(_CC_WAT_)
#define NEEDS_QMAIN
#endif

#if !defined(Q_NOWINSTRICT)
#define Q_WINSTRICT
#endif

typedef void *HANDLE;

#if defined(Q_WINSTRICT)

#define STRICT
#define Q_DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name

#else

#undef  STRICT
#define NO_STRICT
#define Q_DECLARE_HANDLE(name) typedef HANDLE name

#endif

Q_DECLARE_HANDLE(HINSTANCE);
Q_DECLARE_HANDLE(HDC);
Q_DECLARE_HANDLE(HWND);
Q_DECLARE_HANDLE(HFONT);
Q_DECLARE_HANDLE(HPEN);
Q_DECLARE_HANDLE(HBRUSH);
Q_DECLARE_HANDLE(HBITMAP);
Q_DECLARE_HANDLE(HICON);
typedef HICON HCURSOR;
Q_DECLARE_HANDLE(HPALETTE);
Q_DECLARE_HANDLE(HRGN);

typedef struct tagMSG MSG;
typedef HWND  WId;


Q_EXPORT HINSTANCE qWinAppInst();
Q_EXPORT HINSTANCE qWinAppPrevInst();
Q_EXPORT int	   qWinAppCmdShow();
Q_EXPORT HDC	   qt_display_dc();

#if defined(QT_DLL) || defined(QT_MAKEDLL)
#define QT_BASEAPP
class QBaseApplication;
#define QApplication QBaseApplication
#endif
// Forward declaration
class QApplication;

#endif // _WS_WIN_


#if defined(_WS_X11_)

typedef unsigned int  WId;
typedef unsigned int  HANDLE;
typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;
typedef struct _XGC *GC;
typedef struct _XRegion *Region;

Q_EXPORT Display *qt_xdisplay();
Q_EXPORT int	 qt_xscreen();
Q_EXPORT WId	 qt_xrootwin();
Q_EXPORT GC	 qt_xget_readonly_gc( bool monochrome=FALSE );
Q_EXPORT GC	 qt_xget_temp_gc( bool monochrome=FALSE );

#endif // _WS_X11_


#if defined(NEEDS_QMAIN)
#define main qMain
#endif


// Global platform-independent types and functions

typedef Q_INT16 QCOORD;				// coordinate type
const int QCOORD_MIN = -32768;
const int QCOORD_MAX =	32767;

typedef unsigned int QRgb;			// RGB triplet

Q_EXPORT char *qAppName();			// get application name


// Misc functions

typedef void (*CleanUpFunction)();
Q_EXPORT void qAddPostRoutine( CleanUpFunction );


Q_EXPORT void *qt_find_obj_child( QObject *, const char *, const char * );
#define Q_CHILD(parent,type,name) \
	((type*)qt_find_obj_child(parent,#type,name))


// QWidget state flags (internal)

const uint QWS_Created		= 0x00000001;
const uint QWS_Disabled		= 0x00000002;
const uint QWS_Visible		= 0x00000004;
const uint QWS_ForceHide	= 0x00000008;
const uint QWS_OwnCursor	= 0x00000010;
const uint QWS_MouseTracking	= 0x00000020;
const uint QWS_CompressKeys	= 0x00000040;
const uint QWS_BlockUpdates	= 0x00000080;
const uint QWS_InPaintEvent	= 0x00000100;
const uint QWS_ClickToFocus	= 0x00000200;
const uint QWS_TabToFocus	= 0x00000400;
const uint QWS_Reparented	= 0x00000800;
const uint QWS_ConfigPending	= 0x00001000;
const uint QWS_Resized		= 0x00002000;
const uint QWS_AutoMask		= 0x00004000;
const uint QWS_Polished		= 0x00008000;
const uint QWS_DND		= 0x00010000;
const uint QWS_USPositionX	= 0x00020000;
const uint QWS_PaletteSet	= 0x00040000;
const uint QWS_PaletteFixed	= 0x00080000;
const uint QWS_FontSet		= 0x0010000;
const uint QWS_FontFixed	= 0x0020000;


// Widget flags

typedef uint WFlags;

const uint WType_TopLevel	= 0x00000000;	// widget type flags
const uint WType_Modal		= 0x00000000;
const uint WType_Popup		= 0x00000000;
const uint WType_Desktop	= 0x00000000;

const uint WStyle_Customize	= 0x00000001;	// window style flags
const uint WStyle_NormalBorder	= 0x00000002;
const uint WStyle_DialogBorder	= 0x00000004;
const uint WStyle_NoBorder	= 0x00000000;
const uint WStyle_Title		= 0x00000008;
const uint WStyle_SysMenu	= 0x00000010;
const uint WStyle_Minimize	= 0x00000020;
const uint WStyle_Maximize	= 0x00000040;
const uint WStyle_MinMax	= WStyle_Minimize | WStyle_Maximize;
const uint WStyle_Tool		= 0x00000080;
const uint WStyle_Reserved1	= 0x00000100;
const uint WStyle_Reserved2	= 0x00000200;
const uint WStyle_Reserved3	= 0x00000400;
const uint WStyle_Reserved4	= 0x00000800;
const uint WStyle_Mask		= 0x00000fff;

const uint WDestructiveClose	= 0x00001000;	// misc flags
const uint WPaintDesktop	= 0x00002000;
const uint WPaintUnclipped	= 0x00004000;
const uint WPaintClever		= 0x00008000;
const uint WResizeNoErase	= 0x00010000;
const uint WMouseNoMask		= 0x00020000;


// Image conversion flags

// The unusual ordering is caused by compatibility and default requirements.

const int ColorMode_Mask	= 0x00000003;
const int AutoColor		= 0x00000000;
const int ColorOnly		= 0x00000003;
const int MonoOnly		= 0x00000002;
//	  Reserved		= 0x00000001;

const int AlphaDither_Mask	= 0x0000000c;
const int ThresholdAlphaDither	= 0x00000000;
const int OrderedAlphaDither	= 0x00000004;
const int DiffuseAlphaDither	= 0x00000008;
//	  ReservedAlphaDither	= 0x0000000c;

const int Dither_Mask		= 0x00000030;
const int DiffuseDither		= 0x00000000;
const int OrderedDither		= 0x00000010;
const int ThresholdDither	= 0x00000020;
//	  ReservedDither	= 0x00000030;

const int DitherMode_Mask	= 0x000000c0;
const int AutoDither		= 0x00000000;
const int PreferDither		= 0x00000040;
const int AvoidDither		= 0x00000080;


#endif // QWINDOWDEFS_H
