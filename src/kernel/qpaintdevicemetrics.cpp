/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevicemetrics.cpp#3 $
**
** Implementation of QPaintDeviceMetrics class
**
** Author  : Haavard Nord
** Created : 941109
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpdevmet.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpaintdevicemetrics.cpp#3 $";
#endif


QPaintDeviceMetrics::QPaintDeviceMetrics( const QPaintDevice *pd )
{
    pdev = (QPaintDevice *)pd;
}
