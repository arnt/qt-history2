/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice_win.cpp#1 $
**
** Implementation of QPaintDevice class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940801
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qpaintd.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpaintdevice_win.cpp#1 $";
#endif


QPaintDevice::QPaintDevice()
{
    devType = PDT_UNDEF;
}

QPaintDevice::~QPaintDevice()
{
}
