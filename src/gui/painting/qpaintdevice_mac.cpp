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
#include "qprinter.h"
#include <private/qt_mac_p.h>
#include <private/qprintengine_mac_p.h>
#include <private/qpixmap_p.h>

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/


/*****************************************************************************
  External functions
 *****************************************************************************/


/*****************************************************************************
  QPaintDevice member functions
 *****************************************************************************/
QPaintDevice::QPaintDevice()
{
    if(!qApp) {
        qFatal("QPaintDevice: Must construct a QApplication before a "
                "QPaintDevice");
        return;
    }
    painters = 0;
}

QPaintDevice::~QPaintDevice()
{
    if(paintingActive())
        qWarning("Qt: QPaintDevice: Cannot destroy paint device that is being "
                 "painted.  Be sure to QPainter::end() painters!");
}

int QPaintDevice::metric(PaintDeviceMetric) const
{
    return 0;
}

/*! \internal

    Returns the QuickDraw CGrafPtr of the paint device. 0 is returned
    if it can't be obtained. Do not hold the pointer around for long
    as it can be relocated.
*/

GrafPtr qt_mac_qd_context(const QPaintDevice *device)
{
    if (device->devType() == QInternal::Widget) {
        return static_cast<GrafPtr>(static_cast<const QWidget *>(device)->handle());
    } else if (device->devType() == QInternal::Pixmap) {
        return static_cast<GrafPtr>(static_cast<const QPixmap *>(device)->macQDHandle());
    } else if (device->devType() == QInternal::Printer) {
        QPaintEngine *engine = static_cast<const QPrinter *>(device)->paintEngine();
        return static_cast<GrafPtr>(static_cast<const QMacPrintEngine *>(engine)->handle());
    }
    return 0;
}

/*! \internal

    Returns the CoreGraphics CGContextRef of the paint device. 0 is
    returned if it can't be established. It is the caller's
    responsiblity to CGContextRelease the context when finished using
    it.
*/

CGContextRef qt_mac_cg_context(const QPaintDevice *pdev)
{
    if(pdev->devType() == QInternal::Pixmap) {
        const QPixmap *pm = static_cast<const QPixmap*>(pdev);
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        CGContextRef ret = CGBitmapContextCreate(pm->data->pixels, pm->data->w, pm->data->h,
                                                 8, pm->data->nbytes / pm->data->h, colorspace,
                                                 CGImageGetAlphaInfo((CGImageRef)pm->macCGHandle()));
        CGColorSpaceRelease(colorspace);
        if(!ret)
            qWarning("Unable to create context for pixmap (%d/%d/%d)", pm->data->w, pm->data->h, pm->data->nbytes);
        CGContextTranslateCTM(ret, 0, pm->data->h);
        CGContextScaleCTM(ret, 1, -1);
        return ret;
    } else if(pdev->devType() == QInternal::Widget) {
        CGContextRef ret = 0;
        GrafPtr port = qt_mac_qd_context(pdev);
        if(!port)
            return 0;

        if(OSStatus err = CreateCGContextForPort(port, &ret)) {
            qWarning("Unable to create CGContext for port %p [%ld]", port, err);
            return 0;
        }
        SyncCGContextOriginWithPort(ret, port);
        Rect port_rect;
        GetPortBounds(port, &port_rect);
        CGContextTranslateCTM(ret, 0, (port_rect.bottom - port_rect.top));
        CGContextScaleCTM(ret, 1, -1);
        return ret;
    }
    return 0;
}

