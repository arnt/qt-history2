/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture_win.cpp#3 $
**
** Implementation of QPicture class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpicture.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpicture_win.cpp#3 $";
#endif


QPicture::QPicture()
{
    setDevType( PDT_PICTURE | PDF_EXTDEV );	// set device type
}

QPicture::~QPicture()
{
}
