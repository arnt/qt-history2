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

#include <qglobal.h> // for Q_WS_WIN define (non-PCH)

#include <QtGui/QPaintDevice>
#include <QtGui/QWidget>
#include <QtOpenGL/QGLWidget>
#include "private/qwindowsurface_egl_p.h"

#include "private/qpaintengine_opengl_p.h"

class MetricAccessor : public QWidget {
public:
    int metric(PaintDeviceMetric m) {
        return QWidget::metric(m);
    }
};

class QEGLWindowSurfacePrivate
{
public:
    QEGLWindowSurfacePrivate() :
      qglContext(0) {}

    QPaintDevice *device;
    QGLContext *qglContext;
};

QEGLWindowSurface::QEGLWindowSurface(QWidget *window)
    : QWSWindowSurface(window),
      d_ptr(new QEGLWindowSurfacePrivate)
{
    Q_D(QEGLWindowSurface);
    if (window)
        d->device = window;

    setSurfaceFlags(QWSWindowSurface::Buffered);
}

QEGLWindowSurface::~QEGLWindowSurface()
{
    Q_D(QEGLWindowSurface);
    delete d;
}

QPaintDevice *QEGLWindowSurface::paintDevice()
{
    Q_D(QEGLWindowSurface);
    if (qobject_cast<QGLWidget*>(static_cast<QWidget*>(d->device)) != 0)
        return d->device;
    else
        return this;
}

// from the qpaintengine_opengl.cpp:
QOpenGLPaintEngine* qgl_paint_engine();

QPaintEngine *QEGLWindowSurface::paintEngine() const
{
    return qgl_paint_engine();
}

int QEGLWindowSurface::metric(PaintDeviceMetric m) const
{
    Q_D(const QEGLWindowSurface);
    Q_ASSERT(d->device);

    return ((MetricAccessor *) d->device)->metric(m);
}

void QEGLWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    Q_UNUSED(area);
    Q_UNUSED(dx);
    Q_UNUSED(dy);
}

QGLContext *QEGLWindowSurface::context() const
{
    Q_D(const QEGLWindowSurface);
    return d->qglContext;
}

void QEGLWindowSurface::setContext(QGLContext *context)
{
    Q_D(QEGLWindowSurface);
    d->qglContext = context;
}

