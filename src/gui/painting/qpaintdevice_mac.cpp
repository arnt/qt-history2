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
#include "qt_mac.h"

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
QPaintDevice *g_cur_paintdev = 0;


/*****************************************************************************
  External functions
 *****************************************************************************/
extern WindowPtr qt_mac_window_for(HIViewRef); //qwidget_mac.cpp


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

void bitBlt(QPaintDevice *dst, int dx, int dy,
            const QPaintDevice *src, int sx, int sy, int sw, int sh,
            bool imask)
{
    if(dst->devType() == QInternal::Pixmap && src->devType() == QInternal::Widget) {
        *((QPixmap*)dst) = QPixmap::grabWidget((QWidget*)src, sx, sy, sw, sh);
    } else if(src->devType() == QInternal::Widget &&
              (dst->devType() == QInternal::Widget || dst->devType() == QInternal::Printer)) {
        QPixmap pm = QPixmap::grabWidget((QWidget*)src, sx, sy, sw, sh);
        QPainter p(dst);
        p.drawPixmap(dx, dy, sw, sh, pm);
    } else if(src->devType() == QInternal::Pixmap) {
        QPainter p(dst);
        p.drawPixmap(QRect(dx, dy, sw, sh), *((QPixmap*)src), QRect(sx, sy, sw, sh),
                     imask ? Qt::SourceCopy : Qt::AlphaBlend);
    } else {
        qWarning("bitBlt: Cannot bitBlt from/to device!");
    }
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

/*!
    Returns the window system handle of the paint device for
    CoreGraphics support. Use of this function is not portable. This
    function will return 0 if the handle could not be created.
*/
Qt::HANDLE QPaintDevice::macCGHandle() const
{
    if (devType() == QInternal::Widget)
        return static_cast<const QWidget *>(this)->macCGHandle();
    else if (devType() == QInternal::Pixmap);
        return static_cast<const QPixmap *>(this)->macCGHandle();
    return 0;
}

Qt::HANDLE QPaintDevice::handle() const
{
    if (devType() == QInternal::Widget)
        return static_cast<const QWidget *>(this)->handle();
    else if (devType() == QInternal::Pixmap)
        return static_cast<const QPixmap *>(this)->handle();
    return 0;
}
