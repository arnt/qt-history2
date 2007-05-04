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

#ifndef QGLPAINTDEVICE_GL_P_H
#define QGLPAINTDEVICE_GL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QGLWindowSurface class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QPaintDevice>

class QWidget;
class QGLWindowSurface;
class QGLPaintDevicePrivate;

class QGLPaintDevice : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QGLPaintDevice)
public:
    QGLPaintDevice(QWidget *widget);
    ~QGLPaintDevice();

    QPaintEngine *paintEngine() const;

    int metric(PaintDeviceMetric m) const;

    QGLWindowSurface *windowSurface() const;

private:
    friend class QGLWindowSurface;
    QGLPaintDevicePrivate *d_ptr;
};


#endif // QGLPAINTDEVICE_GL_P_H
