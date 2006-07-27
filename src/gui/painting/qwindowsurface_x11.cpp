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

#include <QtGui/QPaintDevice>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>

#include "private/qt_x11_p.h"
#include "qwindowsurface_x11_p.h"

struct QX11WindowSurfacePrivate
{
    QWidget *widget;
    QPixmap device;
};

QX11WindowSurface::QX11WindowSurface(QWidget *widget)
    : d_ptr(new QX11WindowSurfacePrivate)
{
    d_ptr->widget = widget;
}


QX11WindowSurface::~QX11WindowSurface()
{
    delete d_ptr;
}

QPaintDevice *QX11WindowSurface::paintDevice()
{
    return &d_ptr->device;
}


void QX11WindowSurface::flush(QWidget *widget, const QRegion &rgn, const QPoint &offset)
{
    if (d_ptr->device.isNull())
        return;
    extern void *qt_getClipRects(const QRegion &r, int &num); // in qpaintengine_x11.cpp
    extern QWidgetData* qt_widget_data(QWidget *);
    QPoint wOffset = qt_qwidget_data(widget)->wrect.topLeft();
    GC gc = XCreateGC(X11->display, d_ptr->device.handle(), 0, 0);
    QRegion wrgn(rgn);
    QRect br = rgn.boundingRect();
    if (!wOffset.isNull())
        wrgn.translate(-wOffset);
    QRect wbr = wrgn.boundingRect();

    // #### why dirty widget here? should be up to date already
//     if (br.right() + offset.x() >= d_ptr->device.size().width() || br.bottom() + offset.y() >= d_ptr->device.size().height()) {
// //             QRegion dirty = rgn - QRect(-offset, d_ptr->device.size());
// //             qDebug() << dirty;
//         widget->d_func()->dirtyWidget_sys(rgn - QRect(-offset, d_ptr->device.size()));
//     }

    int num;
    XRectangle *rects = (XRectangle *)qt_getClipRects(wrgn, num);
//         qDebug() << "XSetClipRectangles";
//         for  (int i = 0; i < num; ++i)
//             qDebug() << " " << i << rects[i].x << rects[i].x << rects[i].y << rects[i].width << rects[i].height;
    XSetClipRectangles(X11->display, gc, 0, 0, rects, num, YXBanded);
    XSetGraphicsExposures(X11->display, gc, False);
//         XFillRectangle(X11->display, widget->handle(), gc, 0, 0, widget->width(), widget->height());
    XCopyArea(X11->display, d_ptr->device.handle(), widget->handle(), gc,
              br.x() + offset.x(), br.y() + offset.y(), br.width(), br.height(), wbr.x(), wbr.y());
    XFreeGC(X11->display, gc);
}

void QX11WindowSurface::setGeometry(const QRect &rect)
{
    const QSize size = rect.size();
    if (d_ptr->device.size() == size)
        return;
    d_ptr->device = QPixmap(size);
}

void QX11WindowSurface::release()
{
    d_ptr->device = QPixmap();
}


void QX11WindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    QRect rect = area.boundingRect();

    if (d_ptr->device.isNull())
        return;
    GC gc = XCreateGC(X11->display, d_ptr->device.handle(), 0, 0);
    XCopyArea(X11->display, d_ptr->device.handle(), d_ptr->device.handle(), gc,
              rect.x(), rect.y(), rect.width(), rect.height(),
              rect.x()+dx, rect.y()+dy);
    XFreeGC(X11->display, gc);
}

QRect QX11WindowSurface::geometry() const
{
    const QPoint offset = d_ptr->widget->geometry().topLeft();
    const QSize size = d_ptr->device.size();
    return QRect(offset, size);
}
