/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpdevmet.h#1 $
**
** Definition of QPaintDeviceMetrics class
**
** Author  : Haavard Nord
** Created : 941109
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPDEVMET_H
#define QPDEVMET_H

#include "qwindefs.h"


class QPaintDeviceMetrics			// paint device metrics
{
public:
    QPaintDeviceMetrics();

private:
    QPaintDevice *pdev;
};


#endif // QPDEVMET_H
