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

#ifndef QGLFRAMEBUFFEROBJECT_H
#define QGLFRAMEBUFFEROBJECT_H

#include <QtOpenGL/qgl.h>
#include <QtGui/qpaintdevice.h>

QT_BEGIN_HEADER

QT_MODULE(OpenGL)

class QGLFramebufferObjectPrivate;

class Q_OPENGL_EXPORT QGLFramebufferObject : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QGLFramebufferObject)
public:
    enum Attachments {
        NoDepthStencil,
        DepthStencil,
        Depth
    };

    QGLFramebufferObject(const QSize &size, GLenum target = GL_TEXTURE_2D);
    QGLFramebufferObject(int width, int height, GLenum target = GL_TEXTURE_2D);
#if !defined(Q_WS_QWS) || defined(Q_QDOC)
    QGLFramebufferObject(const QSize &size, Attachments attachments,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA8);
    QGLFramebufferObject(int width, int height, Attachments attachments,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA8);
#else
    QGLFramebufferObject(const QSize &size, Attachments attachments,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA);
    QGLFramebufferObject(int width, int height, Attachments attachments,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA);
#endif

    virtual ~QGLFramebufferObject();

    bool isValid() const;
    bool bind();
    bool release();
    GLuint texture() const;
    QSize size() const;
    QImage toImage() const;
    Attachments attachments() const;

    QPaintEngine *paintEngine() const;
    GLuint handle() const;

    static bool hasOpenGLFramebufferObjects();

protected:
    int metric(PaintDeviceMetric metric) const;
    int devType() const { return QInternal::FramebufferObject; }

private:
    Q_DISABLE_COPY(QGLFramebufferObject)
    QGLFramebufferObjectPrivate *d_ptr;
    friend class QGLDrawable;
};

QT_END_HEADER
#endif // QGLFRAMEBUFFEROBJECT_H
