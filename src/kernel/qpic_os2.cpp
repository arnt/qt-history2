/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpic_os2.cpp#11 $
**
** Implementation of QPicture class for OS/2 PM
**
** Created : 940802
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpicture.h"
#define	 INCL_PM
#include <os2.h>

QPicture::QPicture()
{
    setDevType( PDT_PICTURE | PDF_EXTDEV );	// set device type
}

QPicture::~QPicture()
{
}
