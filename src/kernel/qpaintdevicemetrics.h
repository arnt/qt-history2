/****************************************************************************
**
** Definition of QPaintDeviceMetrics class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAINTDEVICEMETRICS_H
#define QPAINTDEVICEMETRICS_H

#ifndef QT_H
#include "qpaintdevice.h"
#endif // QT_H


class Q_GUI_EXPORT QPaintDeviceMetrics			// paint device metrics
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
	PdmDpiY,
	PdmPhysicalDpiX,
	PdmPhysicalDpiY
    };

    int	  width()	const	{ return (int)pdev->metric(PdmWidth); }
    int	  height()	const	{ return (int)pdev->metric(PdmHeight); }
    int	  widthMM()	const	{ return (int)pdev->metric(PdmWidthMM); }
    int	  heightMM()	const	{ return (int)pdev->metric(PdmHeightMM); }
    int	  logicalDpiX()	const	{ return (int)pdev->metric(PdmDpiX); }
    int	  logicalDpiY()	const	{ return (int)pdev->metric(PdmDpiY); }
    int	  physicalDpiX()const	{ return (int)pdev->metric(PdmPhysicalDpiX); }
    int	  physicalDpiY()const	{ return (int)pdev->metric(PdmPhysicalDpiY); }
    int	  numColors()	const	{ return (int)pdev->metric(PdmNumColors); }
    int	  depth()	const	{ return (int)pdev->metric(PdmDepth); }

private:
    QPaintDevice *pdev;
};


#endif // QPAINTDEVICEMETRICS_H
