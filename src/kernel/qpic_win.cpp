/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpic_win.cpp#7 $
**
** Implementation of QPicture class for Windows
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpicture.h"
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qpic_win.cpp#7 $")


QPicture::QPicture()
    : QPaintDevice( PDT_PICTURE | PDF_EXTDEV )	  // set device type
{
}

QPicture::~QPicture()
{
}
