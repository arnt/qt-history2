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
#include <QtGui/QWidget>
#include <QtOpenGL/QGLWidget>
#include "private/qwindowsurface_egl_p.h"
#include "private/qpaintdevice_egl_p.h"

#include "private/qpaintengine_opengl_p.h"

class QMetricAccessor : public QWidget {
public:
    int metric(PaintDeviceMetric m) {
        return QWidget::metric(m);
    }
};

// from the qpaintengine_opengl.cpp:
QOpenGLPaintEngine* qt_qgl_paint_engine();

class QEGLPaintDevicePrivate
{
public:
    QEGLWindowSurface *wsurf;
    QWidget *widget;
};

QEGLPaintDevice::QEGLPaintDevice(QWidget *w, QEGLWindowSurface *surf) :
    d_ptr(new QEGLPaintDevicePrivate)
{
    Q_D(QEGLPaintDevice);
    d->widget = w;
    d->wsurf = surf;
}

QEGLPaintDevice::~QEGLPaintDevice()
{
    Q_D(QEGLPaintDevice);
    delete d;
}

QPaintEngine *QEGLPaintDevice::paintEngine() const
{
    return qt_qgl_paint_engine();
}

int QEGLPaintDevice::metric(PaintDeviceMetric m) const
{
    Q_D(const QEGLPaintDevice);
    Q_ASSERT(d->widget);

    return ((QMetricAccessor *) d->widget)->metric(m);
}

QEGLWindowSurface *QEGLPaintDevice::windowSurface() const
{
     Q_D(const QEGLPaintDevice);
     return d->wsurf;
}

class QEGLWindowSurfacePrivate
{
public:
    QEGLWindowSurfacePrivate() :
        qglContext(0), device(0), ownsContext(false) {}

    QGLContext *qglContext;
    QEGLPaintDevice *device;
    bool ownsContext;
};

QEGLWindowSurface::QEGLWindowSurface(QWidget *window)
    : QWSWindowSurface(window),
      d_ptr(new QEGLWindowSurfacePrivate)
{
    Q_D(QEGLWindowSurface);
    if (window)
        d->device = new QEGLPaintDevice(window, this);

    setSurfaceFlags(QWSWindowSurface::Buffered);
}

QEGLWindowSurface::QEGLWindowSurface()
    : d_ptr(new QEGLWindowSurfacePrivate)
{
    setSurfaceFlags(QWSWindowSurface::Buffered);
}

QEGLWindowSurface::~QEGLWindowSurface()
{
    Q_D(QEGLWindowSurface);
    if (d->ownsContext)
        delete d->qglContext;
    delete d->device;
    delete d;
}

QPaintDevice *QEGLWindowSurface::paintDevice()
{
    Q_D(QEGLWindowSurface);
    if (qobject_cast<QGLWidget*>(d->device->d_func()->widget) != 0)
        return d->device->d_func()->widget;
    else
        return d->device;
}

QGLContext *QEGLWindowSurface::context() const
{
    Q_D(const QEGLWindowSurface);
    if (!d->qglContext) {
        QEGLWindowSurface *that = const_cast<QEGLWindowSurface*>(this);
        that->chooseContext(new QGLContext(QGLFormat::defaultFormat()), 0);
        that->d_func()->ownsContext = true;
    }
    return d->qglContext;
}

void QEGLWindowSurface::setContext(QGLContext *context)
{
    Q_D(QEGLWindowSurface);
    if (d->ownsContext) {
        delete d->qglContext;
        d->ownsContext = false;
    }
    d->qglContext = context;
}
