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
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include <private/qapplication_p.h>
#include "qt_windows.h"
#include "qprinter.h"
#include "qpaintengine_win.h"


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
                  "painted.  Be sure to QPainter::end() painters!");
}

int QPaintDevice::metric(int) const
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
