/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture_win.cpp#11 $
**
** Implementation of QPicture class for Win32
**
** Created : 940802
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpicture.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qpicture_win.cpp#11 $");


QPicture::QPicture()
    : QPaintDevice( PDT_PICTURE | PDF_EXTDEV )	  // set device type
{
}

QPicture::~QPicture()
{
}
