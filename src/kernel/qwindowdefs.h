/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindowdefs.h#151 $
**
** Definition of general window system dependent functions, types and
** constants
**
** Created : 931029
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWINDOWDEFS_H
#define QWINDOWDEFS_H

#ifndef QT_H
#include "qobjectdefs.h"
#include "qstring.h"
#include "qnamespace.h"
#endif // QT_H

#include <limits.h>

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
class QTime;
class QClipboard;


// Widget list (defined in qwidgetlist.h)

class QWidgetList;
class QWidgetListIt;


// Window system dependent definitions

#if defined(_WS_MAC_)

typedef void * HANDLE;
typedef int WId;
typedef void * MSG;

#endif


#if defined(_WS_WIN_)
#include "qwindowdefs_win.h"
#endif // _WS_WIN_


#if defined(_WS_X11_)

#if QT_VERSION > 290
#error "define WId and friends to ulong always"
#endif

#if defined(_OS_OSF_) && defined(UINT_MAX) && defined(ULONG_MAX) && (ULONG_MAX > UINT_MAX)
typedef unsigned long  WId;
typedef unsigned long  HANDLE;
#else
typedef unsigned int  WId;
typedef unsigned int  HANDLE;
#endif

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

#if defined(_WS_QWS_)

struct QWSEvent;
typedef unsigned int WId;
typedef void* HANDLE;
class QGfx;

#endif // _WS_QWS_

class QApplication;

#if defined(NEEDS_QMAIN)
#define main qMain
#endif

// Global platform-independent types and functions


typedef Q_INT32 QCOORD;				// coordinate type
const QCOORD QCOORD_MAX =  2147483647;
const QCOORD QCOORD_MIN = -QCOORD_MAX - 1;

typedef unsigned int QRgb;			// RGB triplet

Q_EXPORT char *qAppName();			// get application name


// Misc functions

typedef void (*Q_CleanUpFunction)();
Q_EXPORT void qAddPostRoutine( Q_CleanUpFunction );
Q_EXPORT void qRemovePostRoutine( Q_CleanUpFunction );

// ### remove 3.0
Q_EXPORT void *qt_find_obj_child( QObject *, const char *, const char * );
#define Q_CHILD(parent,type,name) \
	((type*)qt_find_obj_child(parent,#type,name))


#endif // QWINDOWDEFS_H
