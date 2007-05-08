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

#include <private/qglpaintdevice_qws_p.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <private/qglwindowsurface_qws_p.h>

class QWSGLPaintDevicePrivate
{
public:
    QWidget *widget;
};

class QMetricAccessor : public QWidget {
public:
    int metric(PaintDeviceMetric m) {
        return QWidget::metric(m);
    }
};

QWSGLPaintDevice::QWSGLPaintDevice(QWidget *widget) :
    d_ptr(new QWSGLPaintDevicePrivate)
{
    Q_D(QWSGLPaintDevice);
    d->widget = widget;
}

QWSGLPaintDevice::~QWSGLPaintDevice()
{
    Q_D(QWSGLPaintDevice);
    delete d;
}

QPaintEngine* QWSGLPaintDevice::paintEngine() const
{
    return qt_qgl_paint_engine();
}

int QWSGLPaintDevice::metric(PaintDeviceMetric m) const
{
    Q_D(const QWSGLPaintDevice);
    Q_ASSERT(d->widget);

    return ((QMetricAccessor *) d->widget)->metric(m);
}

QWSGLWindowSurface* QWSGLPaintDevice::windowSurface() const
{
     Q_D(const QWSGLPaintDevice);
     return static_cast<QWSGLWindowSurface*>(d->widget->windowSurface());
}
