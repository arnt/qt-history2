/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpic_win.cpp#5 $
**
** Implementation of QPicture class for Windows
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpicture.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpic_win.cpp#5 $";
#endif


QPicture::QPicture()
    : QPaintDevice( PDT_PICTURE | PDF_EXTDEV )	  // set device type
{
}

QPicture::~QPicture()
{
}
