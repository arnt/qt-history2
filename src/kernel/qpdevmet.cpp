/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpdevmet.cpp#4 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpdevmet.cpp#4 $";
#endif

/*! \class QPaintDeviceMetrics qpdevmet.h

  \brief The QPaintDeviceMetrics class provides information about a
  paint device.

  This class is not yet documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */

QPaintDeviceMetrics::QPaintDeviceMetrics( const QPaintDevice *pd )
{
    pdev = (QPaintDevice *)pd;
}
