/****************************************************************************
** $Id$
**
** Includes X11 system header files.
**
** Created : 981123
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QT_X11_H
#define QT_X11_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of q*_x11.cpp.  This header file may change from version to version
// without notice, or even be removed.
//
//


#include "qwindowdefs.h"

// the following is necessary to work around breakage in many versions
// of XFree86's Xlib.h still in use
// ### which versions?
#if defined(_XLIB_H_) // crude hack, but...
#error "cannot include <X11/Xlib.h> before this file"
#endif
#define XRegisterIMInstantiateCallback qt_XRegisterIMInstantiateCallback
#define XUnregisterIMInstantiateCallback qt_XUnregisterIMInstantiateCallback
#define XSetIMValues qt_XSetIMValues
#include <X11/Xlib.h>
#undef XRegisterIMInstantiateCallback
#undef XUnregisterIMInstantiateCallback
#undef XSetIMValues

#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>


//#define QT_NO_SHAPE
#ifdef QT_NO_SHAPE
#define XShapeCombineRegion(a,b,c,d,e,f,g)
#define XShapeCombineMask(a,b,c,d,e,f,g)
#else
#include <X11/extensions/shape.h>
#endif // QT_NO_SHAPE


// the wacom tablet (currently just the IRIX version)
#if defined (QT_TABLET_SUPPORT)
#  include <X11/extensions/XInput.h>
#  include <wacom.h>  // wacom driver defines for IRIX (quite handy)
#endif // QT_TABLET_SUPPORT


// #define QT_NO_XINERAMA
#ifndef QT_NO_XINERAMA
#  if defined(Q_OS_SOLARIS)
// Xinerama is only supported in Solaris 7 with patches 107648/108376 and
// Solaris 8 or above which introduce the X11R6.4 Xserver.
// To switch the Xinerama functionality on, you need to add the "+xinerama"
// arguement to the Xsun start line.
// At least Solaris 7 and 8 are missing Xinerama system headers and function
// declarations (bug 4284701).
// The Xinerama API is not documented. In theory it could change but it
// probably won't because Sun are using it in at least dtlogin (bug 4221829).
extern "C" Bool XPanoramiXQueryExtension(
    Display*,
    int*,
    int*
);
extern "C" Status XPanoramiXQueryVersion(
    Display*,
    int*,
    int*
);
extern "C" Status XPanoramiXGetState(
    Display*,
    Drawable,
    XPanoramiXInfo*
);
extern "C" Status XPanoramiXGetScreenCount(
    Display *,
    Drawable,
    XPanoramiXInfo*
);
extern "C" Status XPanoramiXGetScreenSize(
    Display*,
    Drawable,
    int,
    XPanoramiXInfo*
);
#  else
// XFree86 does not C++ify Xinerama (at least up to XFree86 4.0.3).
extern "C" {
#    include <X11/extensions/Xinerama.h>
}
#  endif
#endif // QT_NO_XINERAMA


// #define QT_NO_XRENDER
#ifndef QT_NO_XRENDER
#  include <X11/extensions/Xrender.h>
// #define QT_NO_XFTFREETYPE
#  ifndef QT_NO_XFTFREETYPE
#    include <X11/Xft/XftFreetype.h>
#  endif // QT_NO_XFTFREETYPE
#else
// make sure QT_NO_XFTTREETYPE is defined if QT_NO_XRENDER is defined
#  ifndef QT_NO_XFTFREETYPE
#    define QT_NO_XFTFREETYPE
#  endif
#endif // QT_NO_XRENDER


#ifndef QT_NO_XKB
#  include <X11/XKBlib.h>
#endif // QT_NO_XKB


#if !defined(XlibSpecificationRelease)
#  define X11R4
typedef char *XPointer;
#else
#  undef X11R4
#endif

// #define QT_NO_XIM
#if defined(X11R4)
// X11R4 does not have XIM
#define QT_NO_XIM
#elif defined(Q_OS_OSF) && (XlibSpecificationRelease < 6)
// broken in Xlib up to OSF/1 3.2
#define QT_NO_XIM
#elif defined(Q_OS_AIX)
// broken in Xlib up to what version of AIX?
#define QT_NO_XIM
#elif defined(Q_OS_SOLARIS)
// XRegisterIMInstantiateCallback broken under "C" locale on Solaris
#define QT_NO_XIM
#elif defined(QT_NO_DEBUG) && defined(Q_OS_IRIX) && defined(Q_CC_EDG)
// XCreateIC broken when compiling -64 on IRIX 6.5.2
#define QT_NO_XIM
#elif defined(Q_OS_HPUX) && defined(__LP64__)
// XCreateIC broken when compiling 64-bit ELF on HP-UX 11.0
#define QT_NO_XIM
#endif // QT_NO_XIM


#if !defined(QT_NO_XIM) && (XlibSpecificationRelease >= 6)
#define USE_X11R6_XIM

//######### XFree86 has wrong declarations for XRegisterIMInstantiateCallback
//######### and XUnregisterIMInstantiateCallback in at least version 3.3.2.
//######### Many old X11R6 header files lack XSetIMValues.
//######### Therefore, we have to declare these functions ourselves.

extern "C" Bool XRegisterIMInstantiateCallback(
    Display*,
    struct _XrmHashBucketRec*,
    char*,
    char*,
    XIMProc, //XFree86 has XIDProc, which has to be wrong
    XPointer
);

extern "C" Bool XUnregisterIMInstantiateCallback(
    Display*,
    struct _XrmHashBucketRec*,
    char*,
    char*,
    XIMProc, //XFree86 has XIDProc, which has to be wrong
    XPointer
);

extern "C" char *XSetIMValues( XIM /* im */, ... );

#endif


#ifndef X11R4
#  include <X11/Xlocale.h>
#endif // X11R4


#ifdef QT_MITSHM
#  include <X11/extensions/XShm.h>
#endif // QT_MITSHM


#endif // QT_X11_H
