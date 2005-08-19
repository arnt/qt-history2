/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGLPBUFFER_H
#define QGLPBUFFER_H
#include <QtGui/qpaintdevice.h>
#include <QtOpenGL/qgl.h>

class QGLPbufferPrivate;
class Q_OPENGL_EXPORT QGLPbuffer : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QGLPbuffer)
public:
    QGLPbuffer(const QSize &size,
               const QGLFormat &format = QGLFormat::defaultFormat(),
               QGLWidget *shareWidget = 0);
    virtual ~QGLPbuffer();

    bool isValid() const;
    bool makeCurrent();
    bool doneCurrent();

    GLuint generateTexture(GLint format = GL_RGBA8);
    bool bind(GLuint texture_id);
    bool release();

    void copyToTexture(GLuint texture_id, GLint format = GL_RGBA8);

    QSize size() const;
    Qt::HANDLE handle() const;
    QImage toImage() const;

    QPaintEngine *paintEngine() const;
    int metric(PaintDeviceMetric metric) const;
    QGLFormat format() const { return QGLFormat::defaultFormat(); }
    int devType() const { return QInternal::Pbuffer; }

private:
    QGLPbufferPrivate *d_ptr;
};

#endif // QGLPBUFFER_H
