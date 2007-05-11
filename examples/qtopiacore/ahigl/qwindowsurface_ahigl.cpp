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

#include "qwindowsurface_ahigl_p.h"
#include "qscreenahigl_qws.h"

#include <qwsdisplay_qws.h>
#include <QtGui/QPaintDevice>
#include <QtGui/QWidget>
#include <QtOpenGL/private/qglpaintdevice_qws_p.h>
#include <QtOpenGL/private/qgl_p.h>

class QAhiGLWindowSurfacePrivate
{
public:
    QAhiGLWindowSurfacePrivate(EGLDisplay eglDisplay, EGLSurface eglSurface,
                               EGLContext eglContext);

    QPaintDevice *device;

    int textureWidth;
    int textureHeight;

    GLuint texture;
    GLuint frameBufferObject;
    GLuint depthbuf;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
};

QAhiGLWindowSurfacePrivate::QAhiGLWindowSurfacePrivate(EGLDisplay eglDisplay,
                                                       EGLSurface eglSurface,
                                                       EGLContext eglContext)
    : texture(0), frameBufferObject(0), depthbuf(0), display(eglDisplay),
      surface(eglSurface), context(eglContext)
{
}

inline static int nextPowerOfTwo(uint v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    ++v;
    return v;
}

QAhiGLWindowSurface::QAhiGLWindowSurface(QWidget *widget,
                                         EGLDisplay eglDisplay,
                                         EGLSurface eglSurface,
                                         EGLContext eglContext)
    : QWSGLWindowSurface(widget)
{
    d_ptr = new QAhiGLWindowSurfacePrivate(eglDisplay, eglSurface, eglContext);
    d_ptr->device = new QWSGLPaintDevice(widget);

    setSurfaceFlags(QWSWindowSurface::Buffered);
}

QAhiGLWindowSurface::QAhiGLWindowSurface(EGLDisplay eglDisplay,
                                         EGLSurface eglSurface,
                                         EGLContext eglContext)
{
    d_ptr = new QAhiGLWindowSurfacePrivate(eglDisplay, eglSurface, eglContext);
    setSurfaceFlags(QWSWindowSurface::Buffered);
}

QAhiGLWindowSurface::~QAhiGLWindowSurface()
{
    if (d_ptr->texture)
        glDeleteTextures(1, &d_ptr->texture);
    if (d_ptr->depthbuf)
        glDeleteRenderbuffersOES(1, &d_ptr->depthbuf);
    if (d_ptr->frameBufferObject)
        glDeleteFramebuffersOES(1, &d_ptr->frameBufferObject);

    delete d_ptr;
}

void QAhiGLWindowSurface::setGeometry(const QRect &rect)
{
    QSize size = rect.size();

    const QWidget *w = window();
    if (w && !w->mask().isEmpty()) {
        const QRegion region = w->mask()
                               & rect.translated(-w->geometry().topLeft());
        size = region.boundingRect().size();
    }

    if (geometry().size() != size) {

        // Driver specific limitations:
        //   FBO maximimum size of 256x256
        //   Depth buffer required

        d_ptr->textureWidth = qMin(256, nextPowerOfTwo(size.width()));
        d_ptr->textureHeight = qMin(256, nextPowerOfTwo(size.height()));

        glGenTextures(1, &d_ptr->texture);
        glBindTexture(GL_TEXTURE_2D, d_ptr->texture);

        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        const int bufSize = d_ptr->textureWidth * d_ptr->textureHeight * 2;
        GLshort buf[bufSize];
        memset(buf, 0, sizeof(GLshort) * bufSize);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d_ptr->textureWidth,
                     d_ptr->textureHeight, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, buf);

        glGenRenderbuffersOES(1, &d_ptr->depthbuf);
        glBindRenderbufferOES(GL_RENDERBUFFER_EXT, d_ptr->depthbuf);
        glRenderbufferStorageOES(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16,
                                 d_ptr->textureWidth, d_ptr->textureHeight);

        glGenFramebuffersOES(1, &d_ptr->frameBufferObject);
        glBindFramebufferOES(GL_FRAMEBUFFER_EXT, d_ptr->frameBufferObject);

        glFramebufferTexture2DOES(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                  GL_TEXTURE_2D, d_ptr->texture, 0);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_EXT,
                                     GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT, d_ptr->depthbuf);
        glBindFramebufferOES(GL_FRAMEBUFFER_EXT, 0);
    }

    QWSGLWindowSurface::setGeometry(rect);
}

QByteArray QAhiGLWindowSurface::permanentState() const
{
    QByteArray array;
    array.resize(sizeof(GLuint));

    char *ptr = array.data();
    reinterpret_cast<GLuint*>(ptr)[0] = textureId();
    return array;
}

void QAhiGLWindowSurface::setPermanentState(const QByteArray &data)
{
    const char *ptr = data.constData();
    d_ptr->texture = reinterpret_cast<const GLuint*>(ptr)[0];
}

QPaintDevice *QAhiGLWindowSurface::paintDevice()
{
    return d_ptr->device;
}

GLuint QAhiGLWindowSurface::textureId() const
{
    return d_ptr->texture;
}

void QAhiGLWindowSurface::beginPaint(const QRegion &region)
{
    QWSGLWindowSurface::beginPaint(region);

    if (d_ptr->frameBufferObject)
        glBindFramebufferOES(GL_FRAMEBUFFER_EXT, d_ptr->frameBufferObject);
}

bool QAhiGLWindowSurface::isValid() const
{
    if (!qobject_cast<QGLWidget*>(window())) {
        const QRect r = window()->frameGeometry();
        if (r.width() > 256 || r.height() > 256)
            return false;
    }
    return true;
}
