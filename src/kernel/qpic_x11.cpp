/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpic_x11.cpp#12 $
**
** Implementation of QPicture class for X11
**
** Created : 940729
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpicture.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qpic_x11.cpp#12 $");


QPicture::QPicture()
    : QPaintDevice( PDT_PICTURE | PDF_EXTDEV )	  // set device type
{
}

QPicture::~QPicture()
{
}
