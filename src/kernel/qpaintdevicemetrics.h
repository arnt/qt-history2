/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevicemetrics.h#15 $
**
** Definition of QPaintDeviceMetrics class
**
** Created : 941109
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPAINTDEVICEMETRICS_H
#define QPAINTDEVICEMETRICS_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qpaintdevice.h"
#include "qpaintdevicedefs.h"
#endif // QT_H


class Q_EXPORT QPaintDeviceMetrics			// paint device metrics
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


#endif // QPAINTDEVICEMETRICS_H
