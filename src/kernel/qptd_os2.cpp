/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qptd_os2.cpp#1 $
**
** Implementation of QPaintDevice class for OS/2 PM
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qptd_os2.cpp#1 $";
#endif


QPaintDevice::QPaintDevice()
{
    devType = PDT_UNDEF;
}

QPaintDevice::~QPaintDevice()
{
}
