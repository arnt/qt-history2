/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevicemetrics.h#2 $
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
#include "qpaintd.h"
#include "qpaintdc.h"


class QPaintDeviceMetrics			// paint device metrics
{
public:
    QPaintDeviceMetrics( const QPaintDevice * );

    uint    width()	const	{ return (int)pdev->metric(PDM_WIDTH); }
    uint    height()	const	{ return (int)pdev->metric(PDM_HEIGHT); }
    uint    widthMM()	const	{ return (int)pdev->metric(PDM_WIDTHMM); }
    uint    heightMM()	const	{ return (int)pdev->metric(PDM_HEIGHTMM); }

private:
    QPaintDevice *pdev;
};


#endif // QPDEVMET_H
