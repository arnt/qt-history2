/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevicemetrics.h#18 $
**
** Definition of QPaintDeviceMetrics class
**
** Created : 941109
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPAINTDEVICEMETRICS_H
#define QPAINTDEVICEMETRICS_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qpaintdevice.h"
#endif // QT_H


class Q_EXPORT QPaintDeviceMetrics			// paint device metrics
{
public:
    QPaintDeviceMetrics( const QPaintDevice * );

    enum {
	PdmWidth = 1,
	PdmHeight,
	PdmWidthMM,
	PdmHeightMM,
	PdmNumColors,
	PdmDepth,
	PdmDpiX,
	PdmDpiY
    };

    int	  width()	const	{ return (int)pdev->metric(PdmWidth); }
    int	  height()	const	{ return (int)pdev->metric(PdmHeight); }
    int	  widthMM()	const	{ return (int)pdev->metric(PdmWidthMM); }
    int	  heightMM()	const	{ return (int)pdev->metric(PdmHeightMM); }
    int	  logicalDpiX()	const	{ return (int)pdev->metric(PdmDpiX); }
    int	  logicalDpiY()	const	{ return (int)pdev->metric(PdmDpiY); }
    int	  numColors()	const	{ return (int)pdev->metric(PdmNumColors); }
    int	  depth()	const	{ return (int)pdev->metric(PdmDepth); }

private:
    QPaintDevice *pdev;
};


#endif // QPAINTDEVICEMETRICS_H
