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

#ifndef QWINDOWDEFS_H
#define QWINDOWDEFS_H

#include "qobjectdefs.h"
#include "qnamespace.h"

// Class forward definitions

class QPaintDevice;
class QWidget;
class QDialog;
class QColor;
class QPalette;
#ifdef QT_COMPAT
class QColorGroup;
#endif
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
class QMatrix;
class QPixmap;
class QBitmap;
class QMovie;
class QImage;
class QImageIO;
class QPicture;
class QPrinter;
class QTimer;
class QTime;
class QClipboard;
class QString;
class QByteArray;
class QApplication;

template<typename T> class QList;
typedef QList<QWidget *> QWidgetList;

// Window system dependent definitions

#if defined(Q_WS_MAC)
#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_2)
typedef struct OpaqueEventLoopTimerRef* EventLoopTimerRef;
typedef struct OpaqueMenuHandle *MenuRef;
#else
typedef struct __EventLoopTimer *EventLoopTimerRef;
typedef struct OpaqueMenuRef *MenuRef;
#endif
typedef char **MenuBarHandle;
typedef struct OpaqueDragRef *DragRef;
typedef struct OpaqueControlRef* ControlRef;
typedef ControlRef HIViewRef;
typedef struct CGImage *CGImageRef;
typedef struct CGContext *CGContextRef;
typedef struct OpaqueWindowGroupRef *WindowGroupRef;
typedef struct OpaqueGrafPtr *CGrafPtr;
typedef struct OpaquePMPrintSession *PMPrintSession;
typedef struct OpaquePMPrintSettings *PMPrintSettings;
typedef struct OpaquePMPageFormat *PMPageFormat;
typedef struct Point Point;
typedef struct OpaqueEventHandlerRef *EventHandlerRef;
typedef struct OpaqueEventHandlerCallRef *EventHandlerCallRef;
typedef struct OpaqueEventRef *EventRef;
typedef long int OSStatus;
typedef struct OpaqueScrapRef *ScrapRef;
typedef struct OpaqueRgnHandle *RgnHandle;
typedef struct OpaqueWindowPtr *WindowPtr;
typedef WindowPtr WindowRef;
typedef struct OpaqueGrafPtr *GWorldPtr;
typedef GWorldPtr GrafPtr;
typedef struct GDevice **GDHandle;
typedef struct ColorTable ColorTable;
typedef struct BitMap BitMap;
typedef struct EventRecord EventRecord;
typedef void * MSG;
typedef int WId;
typedef struct AEDesc AppleEvent;
#endif // Q_WS_MAC

#if defined(Q_WS_WIN)
#include "qwindowdefs_win.h"
#endif // Q_WS_WIN


#if defined(Q_OS_TEMP)
#include "qwinfunctions_wce.h"
#endif // Q_OS_TEMP

#if defined(Q_WS_X11)

typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;
typedef struct _XGC *GC;
typedef struct _XRegion *Region;
typedef unsigned long  WId;

Q_GUI_EXPORT Display *qt_xdisplay();
Q_GUI_EXPORT int         qt_xscreen();
Q_GUI_EXPORT WId         qt_xrootwin(int scrn = -1); // ### 4.0 add default arg of -1
Q_GUI_EXPORT GC         qt_xget_readonly_gc(int scrn, bool monochrome);
Q_GUI_EXPORT GC         qt_xget_temp_gc(int scrn, bool monochrome);

Q_GUI_EXPORT const char *qAppClass();                // get application class

#endif // Q_WS_X11

#if defined(Q_WS_QWS)

typedef unsigned long  WId;
struct QWSEvent;

#endif // Q_WS_QWS

template<class K, class V> class QHash;
typedef QHash<WId, QWidget *> QWidgetMapper;

#if defined(QT_NEEDS_QMAIN)
#define main qMain
#endif

// Global platform-independent types and functions

//typedef Q_INT32 QCOORD;                                // coordinate type
//enum {
//    QCOORD_MAX =  2147483647,
//    QCOORD_MIN = -QCOORD_MAX - 1
//};


#endif // QWINDOWDEFS_H
