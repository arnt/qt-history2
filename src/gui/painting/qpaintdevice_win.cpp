/****************************************************************************
**
** Implementation of QPaintDevice class for Win32.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
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


Q_GUI_EXPORT HDC qt_winHDC(const QPaintDevice *device) {
    if (device->devType() == QInternal::Widget)
        return static_cast<const QWidget *>(device)->winHDC();
    else if (device->devType() == QInternal::Pixmap)
        return static_cast<const QPixmap *>(device)->winHDC();
    else if (device->devType() == QInternal::Printer)
        return static_cast<const QWin32PaintEngine *>(static_cast<const QPrinter *>(device)->paintEngine())->winHDC();
    return 0;
}

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
