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

#ifndef QGLPIXELBUFFER_H
#define QGLPIXELBUFFER_H

#include <QtOpenGL/qgl.h>
#include <QtGui/qpaintdevice.h>

QT_BEGIN_HEADER

QT_MODULE(OpenGL)

class QGLPixelBufferPrivate;

class Q_OPENGL_EXPORT QGLPixelBuffer : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QGLPixelBuffer)
public:
    QGLPixelBuffer(const QSize &size, const QGLFormat &format = QGLFormat::defaultFormat(),
                   QGLWidget *shareWidget = 0);
    virtual ~QGLPixelBuffer();

    bool isValid() const;
    bool makeCurrent();
    bool doneCurrent();

    GLuint generateDynamicTexture() const;
    bool bindToDynamicTexture(GLuint texture);
    void releaseFromDynamicTexture();
    void updateDynamicTexture(GLuint texture_id) const;

    GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D);
    GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D);
    GLuint bindTexture(const QString &fileName);
    void deleteTexture(GLuint texture_id);

    QSize size() const;
    Qt::HANDLE handle() const;
    QImage toImage() const;

    QPaintEngine *paintEngine() const;
    QGLFormat format() const;

    static bool hasOpenGLPbuffers();

protected:
    int metric(PaintDeviceMetric metric) const;
    int devType() const { return QInternal::Pbuffer; }

private:
    Q_DISABLE_COPY(QGLPixelBuffer)
    QGLPixelBufferPrivate *d_ptr;
    friend class QGLDrawable;
};

QT_END_HEADER

#endif // QGLPIXELBUFFER_H
