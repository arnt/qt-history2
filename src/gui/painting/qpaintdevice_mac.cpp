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
QPaintDevice *g_cur_paintdev = 0;


/*****************************************************************************
  External functions
 *****************************************************************************/


/*****************************************************************************
  QPaintDevice member functions
 *****************************************************************************/
QPaintDevice::QPaintDevice(uint devflags)
{
    if(!qApp) {                                // global constructor
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

void qt_mac_clip_cg_reset(CGContextRef hd)
{
    QRect qrect = QRect(0, 0, 99999, 999999);
    Rect qdr; SetRect(&qdr, qrect.left(), qrect.top(), qrect.right()+1, qrect.bottom()+1);
    ClipCGContextToRegion(hd, &qdr, QRegion(qrect).handle(true));
}

void qt_mac_clip_cg(CGContextRef hd, const QRegion &rgn, const QPoint *pt)
{
    CGContextBeginPath(hd);
    if(rgn.isEmpty()) {
        CGContextAddRect(hd, CGRectMake(0, 0, 0, 0));
    } else {
        QVector<QRect> rects = rgn.rects();
        const int count = rects.size();
        for(int i = 0; i < count; i++) {
            const QRect &r = rects[i];
            CGRect mac_r = CGRectMake(r.x(), r.y(), r.width(), r.height());
            if(pt) {
                mac_r.origin.x -= pt->x();
                mac_r.origin.y -= pt->y();
            }
            CGContextAddRect(hd, mac_r);
        }
    }
    CGContextClip(hd);
}

/*! \internal

    Returns the CoreGraphics CGContextRef of the paint device. 0 is returned if it
    can't be obtained.
*/

CGContextRef qt_macCGHandle(const QPaintDevice *pd)
{
    if (pd->devType() == QInternal::Widget)
        return static_cast<CGContextRef>(static_cast<const QWidget *>(pd)->macCGHandle());
    else if (pd->devType() == QInternal::Pixmap)
        return static_cast<CGContextRef>(static_cast<const QPixmap *>(pd)->macCGHandle());
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

