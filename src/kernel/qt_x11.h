/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qt_x11.h#6 $
**
** Includes X11 system header files.
**
** Created : 981123
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QT_X11_H
#define QT_X11_H


// this file is not part of the Qt API.  It exists for the convenience
// of q*.cpp.  This header file may change from version to version
// without notice.


#include "qwindowdefs.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

//#define QT_NO_SHAPE
#ifdef QT_NO_SHAPE
#define XShapeCombineRegion(a,b,c,d,e,f,g)
#define XShapeCombineMask(a,b,c,d,e,f,g)
#else
#include <X11/extensions/shape.h>
#endif

#if !defined(XlibSpecificationRelease)
#define X11R4
typedef char *XPointer;
#else
#undef  X11R4
#endif


#if defined(X11R4)
// X11R4 does not have XIM
#define NO_XIM
#elif defined(_OS_OSF_) && (XlibSpecificationRelease < 6)
// broken in Xlib up to OSF/1 3.2
#define NO_XIM
#elif defined(_OS_AIX_)
// broken in Xlib up to what?
#define NO_XIM
#elif defined(NO_DEBUG) && defined(OS_IRIX) && defined(_CC_EDG_)
// XCreateIC broken when compiling -64 on IRIX 6.5.2
#define NO_XIM
#endif


#endif // QT_X11_H
