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

#include "qpaintdevice.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qwsdisplay_qws.h"

//#### HACK:
#include <private/qpainter_p.h>
#include <private/qpaintengine_qws_p.h>

QPaintDevice::QPaintDevice(uint devflags)
{
    if (!qApp) {                                // global constructor
        qFatal("QPaintDevice: Must construct a QApplication before a "
                "QPaintDevice");
        return;
    }
    devFlags = devflags;
    painters = 0;
}


QPaintDevice::~QPaintDevice()
{
    if (paintingActive())
        qWarning("QPaintDevice: Cannot destroy paint device that is being "
                  "painted");
}


int QPaintDevice::metric(PaintDeviceMetric m) const
{
    qWarning("QPaintDevice::metrics: Device has no metric information");
    if (m == PdmDpiX) {
        return 72;
    } else if (m == PdmDpiY) {
        return 72;
    } else if (m == PdmNumColors) {
        // FIXME: does this need to be a real value?
        return 256;
    } else {
        qDebug("Unrecognised metric %d!",m);
        return 0;
    }
}

/*!
    \internal
*/
QWSDisplay *QPaintDevice::qwsDisplay()
{
    return qt_fbdpy;
}

/*!
    \internal
*/
const uchar *QPaintDevice::scanLine(int) const
{
    return 0;
}

/*!
    \internal
*/
int QPaintDevice::bytesPerLine() const
{
    return 0;
}
