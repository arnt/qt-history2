/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWINDOWDEFS_H
#define QWINDOWDEFS_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qnamespace.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

// Class forward definitions

class QPaintDevice;
class QWidget;
class QDialog;
class QColor;
class QPalette;
#ifdef QT3_SUPPORT
class QColorGroup;
#endif
class QCursor;
class QPoint;
class QSize;
class QRect;
class QPolygon;
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

QT_END_NAMESPACE
QT_END_HEADER

// Window system dependent definitions

#if defined(Q_WS_MAC) && !defined(Q_WS_QWS)
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
typedef struct OpaqueIconRef *IconRef;
typedef struct OpaqueWindowGroupRef *WindowGroupRef;
typedef struct OpaqueGrafPtr *CGrafPtr;
typedef struct OpaquePMPrintSession *PMPrintSession;
typedef struct OpaquePMPrintSettings *PMPrintSettings;
typedef struct OpaquePMPageFormat *PMPageFormat;
typedef struct OpaqueEventHandlerRef *EventHandlerRef;
typedef struct OpaqueEventHandlerCallRef *EventHandlerCallRef;
typedef struct OpaqueEventRef *EventRef;
#ifdef Q_WS_MAC32
typedef long int OSStatus;
typedef int WId;
#else
typedef int OSStatus;
typedef long WId;
#endif
typedef struct OpaqueScrapRef *ScrapRef;
typedef struct OpaqueRgnHandle *RgnHandle;
typedef struct OpaqueWindowPtr *WindowPtr;
typedef WindowPtr WindowRef;
typedef struct OpaqueGrafPtr *GWorldPtr;
typedef GWorldPtr GrafPtr;
typedef struct GDevice **GDHandle;
typedef void * MSG;
typedef struct AEDesc AppleEvent;
#endif // Q_WS_MAC

#if defined(Q_WS_WIN)
#include <QtGui/qwindowdefs_win.h>
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

#endif // Q_WS_X11

#if defined(Q_WS_QWS)

typedef unsigned long  WId;
QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE
struct QWSEvent;
QT_END_NAMESPACE
QT_END_HEADER

#endif // Q_WS_QWS

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

template<class K, class V> class QHash;
typedef QHash<WId, QWidget *> QWidgetMapper;

template<class V> class QSet;
typedef QSet<QWidget *> QWidgetSet;

QT_END_NAMESPACE
QT_END_HEADER

#if defined(QT_NEEDS_QMAIN)
#define main qMain
#endif

// Global platform-independent types and functions

#endif // QWINDOWDEFS_H
