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

#ifndef QWINDOWSURFACE_AHIGL_P_H
#define QWINDOWSURFACE_AHIGL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QAhiGLWindowSurface class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtOpenGL/private/qglwindowsurface_qws_p.h>
#include <GLES/gl.h>
#include <GLES/egl.h>

class QAhiGLWindowSurfacePrivate;

class QAhiGLWindowSurface : public QWSGLWindowSurface
{
public:
    QAhiGLWindowSurface(QWidget *widget, EGLDisplay eglDisplay,
                        EGLSurface eglSurface, EGLContext eglContext);
    QAhiGLWindowSurface(EGLDisplay eglDisplay, EGLSurface eglSurface,
                        EGLContext eglContext);
    ~QAhiGLWindowSurface();

    QString key() const { return QLatin1String("ahigl"); }
    void setGeometry(const QRect &rect);
    QPaintDevice *paintDevice();
    void beginPaint(const QRegion &region);
    bool isValid() const;

    QByteArray permanentState() const;
    void setPermanentState(const QByteArray &);

    QImage image() const { return QImage(); }

    GLuint textureId() const;

private:
    QAhiGLWindowSurfacePrivate *d_ptr;
};

#endif // QWINDOWSURFACE_AHIGL_P_H
