/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_x11.cpp#1 $
**
** Implementation of QPaintDevice class for X11
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptd_x11.cpp#1 $";
#endif


QPaintDevice::QPaintDevice()
{
    devType = PDT_UNDEF;
    dpy = qXDisplay();
    hd = 0;
}

QPaintDevice::~QPaintDevice()
{
}


bool QPaintDevice::cmd( int, QPDevCmdParam * )
{
    return FALSE;
}
