/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpdevmet.h#11 $
**
** Definition of QPaintDeviceMetrics class
**
** Created : 941109
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPDEVMET_H
#define QPDEVMET_H

#ifndef QT_H
#include "qwindefs.h"
#include "qpaintd.h"
#include "qpaintdc.h"
#endif // QT_H


class QPaintDeviceMetrics			// paint device metrics
{
public:
    QPaintDeviceMetrics( const QPaintDevice * );

    int	  width()	const	{ return (int)pdev->metric(PDM_WIDTH); }
    int	  height()	const	{ return (int)pdev->metric(PDM_HEIGHT); }
    int	  widthMM()	const	{ return (int)pdev->metric(PDM_WIDTHMM); }
    int	  heightMM()	const	{ return (int)pdev->metric(PDM_HEIGHTMM); }
    int	  numColors()	const	{ return (int)pdev->metric(PDM_NUMCOLORS); }
    int	  depth()	const	{ return (int)pdev->metric(PDM_DEPTH); }

private:
    QPaintDevice *pdev;
};


#endif // QPDEVMET_H
