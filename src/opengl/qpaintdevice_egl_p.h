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

#ifndef QEGLPAINTDEVICE_EGL_P_H
#define QEGLPAINTDEVICE_EGL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QEGLWindowSurface class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QPaintDevice>

class QWidget;
class QEGLWindowSurface;
class QEGLPaintDevicePrivate;

class QEGLPaintDevice : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QEGLPaintDevice)
public:
    QEGLPaintDevice(QWidget *w, QEGLWindowSurface *surf);
    ~QEGLPaintDevice();

    QPaintEngine *paintEngine() const;

    int metric(PaintDeviceMetric m) const;

    QEGLWindowSurface *windowSurface() const;

private:
    friend class QEGLWindowSurface;
    QEGLPaintDevicePrivate *d_ptr;
};


#endif // QEGLPAINTDEVICE_EGL_P_H
