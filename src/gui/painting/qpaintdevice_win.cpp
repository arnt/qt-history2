/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpaintdevice.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include <private/qapplication_p.h>
#include "qt_windows.h"
#include "qprinter.h"

QPaintDevice::QPaintDevice()
{
    painters = 0;
}

QPaintDevice::~QPaintDevice()
{
    if (paintingActive())
        qWarning("QPaintDevice: Cannot destroy paint device that is being "
                  "painted.  Be sure to QPainter::end() painters!");
    extern void qt_painter_removePaintDevice(QPaintDevice *); //qpainter.cpp
    qt_painter_removePaintDevice(this);
}

int QPaintDevice::metric(PaintDeviceMetric) const
{
    qWarning("QPaintDevice::metrics: Device has no metric information");
    return 0;
}


/*! \internal
*/
HDC QPaintDevice::getDC() const
{
    return 0;
}

/*! \internal
*/
void QPaintDevice::releaseDC(HDC) const
{
}
