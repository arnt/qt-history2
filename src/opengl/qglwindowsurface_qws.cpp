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
#include "private/qglwindowsurface_qws_p.h"
#include "private/qglpaintdevice_qws_p.h"
#include "private/qpaintengine_opengl_p.h"

/*!
    \class QWSGLWindowSurface
    \since 4.3
    \ingroup qws
    \preliminary

    \brief The QWSGLWindowSurface class provides the drawing area for top-level
    windows in Qtopia Core on EGL/OpenGLES. It also provides the drawing area for
    QGLWidgets whether they are top-level windows or child widget of another QWidget.

    Note that this class is only available in Qtopia Core and only available if
    Qt is configured with OpenGL support.

*/

class QWSGLWindowSurfacePrivate
{
public:
    QWSGLWindowSurfacePrivate() :
        qglContext(0), ownsContext(false) {}

    QGLContext *qglContext;
    bool ownsContext;
};

/*!
    \since 4.3

    Constructs an empty QWSGLWindowSurface for the given top-level \a window.
    The window surface is later initialized from chooseContext() and resources for it
    is typically allocated in setGeometry().
*/
QWSGLWindowSurface::QWSGLWindowSurface(QWidget *window)
    : QWSWindowSurface(window),
      d_ptr(new QWSGLWindowSurfacePrivate)
{
}

/*!
    \since 4.3

    Constructs an empty QWSGLWindowSurface.
*/
QWSGLWindowSurface::QWSGLWindowSurface()
    : d_ptr(new QWSGLWindowSurfacePrivate)
{
}

QWSGLWindowSurface::~QWSGLWindowSurface()
{
    Q_D(QWSGLWindowSurface);
    if (d->ownsContext)
        delete d->qglContext;
    delete d;
}

/*!
    \since 4.3

    Returns the QGLContext of the window surface.
*/
QGLContext *QWSGLWindowSurface::context() const
{
    Q_D(const QWSGLWindowSurface);
    if (!d->qglContext) {
        QWSGLWindowSurface *that = const_cast<QWSGLWindowSurface*>(this);
        that->setContext(new QGLContext(QGLFormat::defaultFormat()));
        that->d_func()->ownsContext = true;
    }
    return d->qglContext;
}

/*!
    \since 4.3

    Sets the QGLContext for this window surface.
*/
void QWSGLWindowSurface::setContext(QGLContext *context)
{
    Q_D(QWSGLWindowSurface);
    if (d->ownsContext) {
        delete d->qglContext;
        d->ownsContext = false;
    }
    d->qglContext = context;
}
