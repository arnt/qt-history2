/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpdevmet.cpp#1 $
**
** Implementation of QPaintDeviceMetrics class
**
** Author  : Haavard Nord
** Created : 941109
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpdevmet.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpdevmet.cpp#1 $";
#endif


QPaintDeviceMetrics::QPaintDeviceMetrics( QPaintDevice *pd )
{
    pdev = pd;
}
