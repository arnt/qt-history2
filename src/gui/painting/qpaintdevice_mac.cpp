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
#include "qprinter.h"
#include <qdebug.h>
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
    painters = 0;
}

QPaintDevice::~QPaintDevice()
{
    if(paintingActive())
        qWarning("QPaintDevice: Cannot destroy paint device that is being "
                 "painted, be sure to QPainter::end() painters");
    extern void qt_painter_removePaintDevice(QPaintDevice *); //qpainter.cpp
    qt_painter_removePaintDevice(this);
}

int QPaintDevice::metric(PaintDeviceMetric) const
{
    return 0;
}

/*! \internal */
float qt_mac_defaultDpi_x()
{
    // Mac OS X currently assumes things to be 72 dpi.
    // (see http://developer.apple.com/releasenotes/GraphicsImaging/RN-ResolutionIndependentUI/)
    // This may need to be re-worked as we go further in the resolution-independence stuff.
    return 72;
}

/*! \internal */
float qt_mac_defaultDpi_y()
{
    // Mac OS X currently assumes things to be 72 dpi.
    // (see http://developer.apple.com/releasenotes/GraphicsImaging/RN-ResolutionIndependentUI/)
    // This may need to be re-worked as we go further in the resolution-independence stuff.
    return 72;
}


/*! \internal

    Returns the QuickDraw CGrafPtr of the paint device. 0 is returned
    if it can't be obtained. Do not hold the pointer around for long
    as it can be relocated.

    \warning This function is only available on Mac OS X.
*/

Q_GUI_EXPORT GrafPtr qt_mac_qd_context(const QPaintDevice *device)
{
    if (device->devType() == QInternal::Pixmap) {
        return static_cast<GrafPtr>(static_cast<const QPixmap *>(device)->macQDHandle());
    } else if(device->devType() == QInternal::Widget) {
        return static_cast<GrafPtr>(static_cast<const QWidget *>(device)->macQDHandle());
    } if(device->devType() == QInternal::Printer) {
        QPaintEngine *engine = static_cast<const QPrinter *>(device)->paintEngine();
        return static_cast<GrafPtr>(static_cast<const QMacPrintEngine *>(engine)->handle());
    }
    return 0;
}

/*! \internal

    Returns the CoreGraphics CGContextRef of the paint device. 0 is
    returned if it can't be obtained. It is the caller's responsiblity to
    CGContextRelease the context when finished using it.

    \warning This function is only available on Mac OS X.
*/

Q_GUI_EXPORT CGContextRef qt_mac_cg_context(const QPaintDevice *pdev)
{
    if(pdev->devType() == QInternal::Pixmap) {
        const QPixmap *pm = static_cast<const QPixmap*>(pdev);
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        CGImageRef img = (CGImageRef)pm->macCGHandle();
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
        uint flags = CGImageGetAlphaInfo(img);
        CGBitmapInfo (*CGImageGetBitmapInfo_ptr)(CGImageRef) = CGImageGetBitmapInfo;
        if(CGImageGetBitmapInfo_ptr)
            flags |= (*CGImageGetBitmapInfo_ptr)(img);
#else
        CGImageAlphaInfo flags = CGImageGetAlphaInfo(img);
#endif
        CGContextRef ret = CGBitmapContextCreate(pm->data->pixels, pm->data->w, pm->data->h,
                                                 8, pm->data->nbytes / pm->data->h, colorspace,
                                                 flags);
        CGColorSpaceRelease(colorspace);
        if(!ret)
            qWarning("QPaintDevice: Unable to create context for pixmap (%d/%d/%d)",
                     pm->data->w, pm->data->h, pm->data->nbytes);
        CGContextTranslateCTM(ret, 0, pm->data->h);
        CGContextScaleCTM(ret, 1, -1);
        CGImageRelease(img);
        return ret;
    } else if(pdev->devType() == QInternal::Widget) {
        CGContextRef ret = static_cast<CGContextRef>(static_cast<const QWidget *>(pdev)->macCGHandle());
        CGContextRetain(ret);
        return ret;
    }
    return 0;
}
