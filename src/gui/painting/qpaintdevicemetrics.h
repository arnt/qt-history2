/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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


class Q_GUI_EXPORT QPaintDeviceMetrics                        // paint device metrics
{
public:
    QPaintDeviceMetrics(const QPaintDevice *);

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

    int          width()        const        { return pdev->metric(PdmWidth); }
    int          height()        const        { return pdev->metric(PdmHeight); }
    int          widthMM()        const        { return pdev->metric(PdmWidthMM); }
    int          heightMM()        const        { return pdev->metric(PdmHeightMM); }
    int          logicalDpiX()        const        { return pdev->metric(PdmDpiX); }
    int          logicalDpiY()        const        { return pdev->metric(PdmDpiY); }
    int          physicalDpiX()const        { return pdev->metric(PdmPhysicalDpiX); }
    int          physicalDpiY()const        { return pdev->metric(PdmPhysicalDpiY); }
    int          numColors()        const        { return pdev->metric(PdmNumColors); }
    int          depth()        const        { return pdev->metric(PdmDepth); }

private:
    QPaintDevice *pdev;
};


#endif // QPAINTDEVICEMETRICS_H
