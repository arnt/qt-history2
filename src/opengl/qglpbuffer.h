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

#include <QtOpenGL/qgl.h>
#include <QtGui/qpaintdevice.h>

QT_MODULE(OpenGL)

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

    GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D,
		       GLint format = GL_RGBA8);
    GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D,
		       GLint format = GL_RGBA8);
    GLuint bindTexture(const QString &fileName);
    void deleteTexture(GLuint texture_id);
    void copyToTexture(GLuint texture_id, GLint format = GL_RGBA8);

    QSize size() const;
    Qt::HANDLE handle() const;
    QImage toImage() const;

    QPaintEngine *paintEngine() const;
    QGLFormat format() const { return QGLFormat::defaultFormat(); }

    static bool hasPbuffers();
protected:
    int metric(PaintDeviceMetric metric) const;
    int devType() const { return QInternal::Pbuffer; }

private:
    Q_DISABLE_COPY(QGLPbuffer)
    QGLPbufferPrivate *d_ptr;
    friend class QGLDrawable;
};

#endif // QGLPBUFFER_H
