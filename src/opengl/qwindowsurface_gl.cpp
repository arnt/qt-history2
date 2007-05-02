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
#include "private/qwindowsurface_gl_p.h"
#include "private/qpaintdevice_gl_p.h"

#include "private/qpaintengine_opengl_p.h"

class QMetricAccessor : public QWidget {
public:
    int metric(PaintDeviceMetric m) {
        return QWidget::metric(m);
    }
};

// from the qpaintengine_opengl.cpp:
QOpenGLPaintEngine* qt_qgl_paint_engine();

class QGLPaintDevicePrivate
{
public:
    QGLWindowSurface *wsurf;
    QWidget *widget;
};

QGLPaintDevice::QGLPaintDevice(QWidget *w, QGLWindowSurface *surf) :
    d_ptr(new QGLPaintDevicePrivate)
{
    Q_D(QGLPaintDevice);
    d->widget = w;
    d->wsurf = surf;
}

QGLPaintDevice::~QGLPaintDevice()
{
    Q_D(QGLPaintDevice);
    delete d;
}

QPaintEngine *QGLPaintDevice::paintEngine() const
{
    return qt_qgl_paint_engine();
}

int QGLPaintDevice::metric(PaintDeviceMetric m) const
{
    Q_D(const QGLPaintDevice);
    Q_ASSERT(d->widget);

    return ((QMetricAccessor *) d->widget)->metric(m);
}

QGLWindowSurface *QGLPaintDevice::windowSurface() const
{
     Q_D(const QGLPaintDevice);
     return d->wsurf;
}

/*!
    \class QGLWindowSurface
    \since 4.3
    \ingroup qws
    \preliminary

    \brief The QGLWindowSurface class provides the drawing area for top-level
    windows in Qtopia Core on EGL/OpenGLES. It also provides the drawing area for
    QGLWidgets whether they are top-level windows or child widget of another QWidget.

    Note that this class is only available in Qtopia Core and only available if
    Qt is configured with OpenGL support.

*/

class QGLWindowSurfacePrivate
{
public:
    QGLWindowSurfacePrivate() :
        qglContext(0), ownsContext(false) {}

    QGLContext *qglContext;
    bool ownsContext;
};

/*!
    \since 4.3

    Constructs an empty QGLWindowSurface for the given top-level \a window.
    The window surface is later initialized from chooseContext() and resources for it
    is typically allocated in setGeometry().
*/
QGLWindowSurface::QGLWindowSurface(QWidget *window)
    : QWSWindowSurface(window),
      d_ptr(new QGLWindowSurfacePrivate)
{
}

/*!
    \since 4.3

    Constructs an empty QGLWindowSurface.
*/
QGLWindowSurface::QGLWindowSurface()
    : d_ptr(new QGLWindowSurfacePrivate)
{
}

QGLWindowSurface::~QGLWindowSurface()
{
    Q_D(QGLWindowSurface);
    if (d->ownsContext)
        delete d->qglContext;
    delete d;
}

/*!
    \since 4.3

    Returns the QGLContext of the window surface.
*/
QGLContext *QGLWindowSurface::context() const
{
    Q_D(const QGLWindowSurface);
    if (!d->qglContext) {
        QGLWindowSurface *that = const_cast<QGLWindowSurface*>(this);
        that->setContext(new QGLContext(QGLFormat::defaultFormat()));
        that->d_func()->ownsContext = true;
    }
    return d->qglContext;
}

/*!
    \since 4.3

    Sets the QGLContext for this window surface.
*/
void QGLWindowSurface::setContext(QGLContext *context)
{
    Q_D(QGLWindowSurface);
    if (d->ownsContext) {
        delete d->qglContext;
        d->ownsContext = false;
    }
    d->qglContext = context;
}
