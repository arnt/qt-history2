#ifndef __QKERNEL_MAC_H__
#define __QKERNEL_MAC_H__
/****************************************************************************
**
** Definition of ???.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#undef OLD_DEBUG
#ifdef DEBUG
#define OLD_DEBUG DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#ifndef __IMAGECAPTURE__
#define __IMAGECAPTURE__
#endif
#ifdef qDebug
#    undef qDebug
#    include <Carbon/Carbon.h>
#    ifdef QT_NO_DEBUG 
#        define qDebug qt_noop(),1?(void)0:qDebug
#    endif
#else
#    include <Carbon/Carbon.h>
#endif

#include <QuickTime/Movies.h>
#undef QT_BUILD_KEY

/* We don't use the ApplicationEventLoop because it can causes bad behaviour in
   multithreaded applications. I've left the code in however because using the
   ApplicationEventLoop solved other problems (ages ago) - for example the gumdrop
   "hover" effects. */
//#define QMAC_USE_APPLICATION_EVENT_LOOP

#undef DEBUG
#ifdef OLD_DEBUG
#define DEBUG OLD_DEBUG
#endif
#undef OLD_DEBUG


#endif /* __QKERNEL_MAC_H__ */
