/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpic_os2.cpp#3 $
**
** Implementation of QPicture class for OS/2 PM
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpicture.h"
#define	 INCL_PM
#include <os2.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpic_os2.cpp#3 $";
#endif


QPicture::QPicture()
{
    setDevType( PDT_PICTURE | PDF_EXTDEV );	// set device type
}

QPicture::~QPicture()
{
}
