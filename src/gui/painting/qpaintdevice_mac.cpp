/****************************************************************************
**
** Implementation of QPaintDevice class for Mac.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREEPROFESSIONAL, ENTERPRISE
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
#include <private/qt_mac_p.h>

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/


/*****************************************************************************
  External functions
 *****************************************************************************/


/*****************************************************************************
  QPaintDevice member functions
 *****************************************************************************/
QPaintDevice::QPaintDevice(uint devflags)
{
    if(!qApp) {
        qFatal("QPaintDevice: Must construct a QApplication before a "
                "QPaintDevice");
        return;
    }
    devFlags = devflags;
    painters = 0;
}

QPaintDevice::~QPaintDevice()
{
    if(paintingActive())
        qWarning("Qt: QPaintDevice: Cannot destroy paint device that is being "
                 "painted.  Be sure to QPainter::end() painters!");
}

int QPaintDevice::metric(int) const
{
    return 0;
}

/*! \internal

    Returns the QuickDraw CGrafPtr of the paint device. 0 is returned if it
    can't be obtained.
*/

GrafPtr qt_macQDHandle(const QPaintDevice *pd)
{
    if (pd->devType() == QInternal::Widget)
        return static_cast<GrafPtr>(static_cast<const QWidget *>(pd)->handle());
    else if (pd->devType() == QInternal::Pixmap)
        return static_cast<GrafPtr>(static_cast<const QPixmap *>(pd)->handle());
    return 0;
}

/*! \internal

    Returns the CoreGraphics CGContextRef of the paint device. 0 is
    returned if it can't be established. It is the caller's
    responsiblity to CGContextRelease the context when finished using
    it.
*/

CGContextRef qt_macCreateCGHandle(const QPaintDevice *pdev)
{
    if(pdev->devType() == QInternal::Pixmap || pdev->devType() == QInternal::Widget) {
        CGContextRef ret = 0;
        GrafPtr port = qt_macQDHandle(pdev);
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

