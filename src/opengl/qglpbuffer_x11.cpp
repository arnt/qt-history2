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
#include <qdebug.h>
#include <private/qgl_p.h>
#include <private/qt_x11_p.h>
#include <private/qpaintengine_opengl_p.h>

#include <qx11info_x11.h>
#include <GL/glx.h>
#include <qimage.h>

#include "qglpbuffer.h"
#include "qglpbuffer_p.h"
#include "qglsymbols_x11_p.h"

#ifndef GLX_VERSION_1_3
#define GLX_RGBA_BIT            0x00000002
#define GLX_PBUFFER_BIT         0x00000004
#define GLX_DRAWABLE_TYPE       0x8010
#define GLX_RENDER_TYPE         0x8011
#define GLX_RGBA_TYPE           0x8014
#define GLX_PBUFFER_HEIGHT      0x8040
#define GLX_PBUFFER_WIDTH       0x8041
#endif

QGLPbuffer::QGLPbuffer(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
    : d_ptr(new QGLPbufferPrivate)
{
    Q_D(QGLPbuffer);

    if (qt_resolve_gl_symbols() < QGL_GLX_1_3) {
        qWarning("QGLPbuffer: pbuffers are not supported on this system.");
        return;
    }

    int i = 0;
    int attribs[40];
    int num_configs = 0;

    attribs[i++] = GLX_RENDER_TYPE;
    attribs[i++] = GLX_RGBA_BIT;
    attribs[i++] = GLX_DRAWABLE_TYPE;
    attribs[i++] = GLX_PBUFFER_BIT;

    if (f.doubleBuffer()) {
        attribs[i++] = GLX_DOUBLEBUFFER;
        attribs[i++] = true;
    }
    if (f.depth()) {
        attribs[i++] = GLX_DEPTH_SIZE;
        attribs[i++] = f.depthBufferSize() == -1 ? 1 : f.depthBufferSize();
    }
    if (f.stereo()) {
        attribs[i++] = GLX_STEREO;
        attribs[i++] = true;
    }
    if (f.stencil()) {
        attribs[i++] = GLX_STENCIL_SIZE;
        attribs[i++] = f.stencilBufferSize() == -1 ? 1 : f.stencilBufferSize();
    }
    if (f.alpha()) {
        attribs[i++] = GLX_ALPHA_SIZE;
        attribs[i++] = f.alphaBufferSize() == -1 ? 1 : f.alphaBufferSize();
    }
    if (f.accum()) {
        attribs[i++] = GLX_ACCUM_RED_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        attribs[i++] = GLX_ACCUM_GREEN_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        attribs[i++] = GLX_ACCUM_BLUE_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        if (f.alpha()) {
            attribs[i++] = GLX_ACCUM_ALPHA_SIZE;
            attribs[i++] = f.accumBufferSize() == -1 ? 1 : f.accumBufferSize();
        }
    }
    attribs[i] = XNone;

    GLXFBConfig *configs = glXChooseFBConfig(QX11Info::display(), QX11Info::appScreen(), attribs, &num_configs);
    if (configs && num_configs) {
        int pb_attribs[] = {GLX_PBUFFER_WIDTH, size.width(), GLX_PBUFFER_HEIGHT, size.height(), XNone};
        GLXContext shareContext = 0;
        if (shareWidget && shareWidget->d_func()->glcx)
            shareContext = (GLXContext) shareWidget->d_func()->glcx->d_func()->cx;

        d->pbuf = glXCreatePbuffer(QX11Info::display(), configs[0], pb_attribs);
        d->ctx = glXCreateNewContext(QX11Info::display(), configs[0], GLX_RGBA_TYPE, shareContext, true);

        if (d->pbuf && d->ctx) {
            d->size = size;
            d->invalid = false;
        } else {
            qWarning("Unable to create a pbuffer/context - giving up.");
        }

        // cleanup
        for (int i = 0; i < num_configs; ++i)
            XFree(reinterpret_cast<void *>(configs[i]));
        XFree(configs);
        d->qctx = new QGLContext(f);
    } else {
        qWarning("Unable to find a context/format match - giving up");
        return;
    }
}

QGLPbuffer::~QGLPbuffer()
{
    Q_D(QGLPbuffer);
    glXDestroyContext(QX11Info::display(), d->ctx);
    glXDestroyPbuffer(QX11Info::display(), d->pbuf);
    delete d->qctx;
    delete d_ptr;
}

bool QGLPbuffer::makeCurrent()
{
    Q_D(QGLPbuffer);
    if (d->invalid)
        return false;
    return glXMakeContextCurrent(QX11Info::display(), d->pbuf, d->pbuf, d->ctx);
}

bool QGLPbuffer::doneCurrent()
{
    Q_D(QGLPbuffer);
    if (d->invalid)
        return false;
    return glXMakeContextCurrent(QX11Info::display(), 0, 0, 0);
}

bool QGLPbuffer::bind(GLuint)
{
    return false;
}

bool QGLPbuffer::release()
{
    return false;
}

GLuint QGLPbuffer::generateTexture(GLint format)
{
    Q_D(QGLPbuffer);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, d->size.width(), d->size.height(), 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return texture;
}
