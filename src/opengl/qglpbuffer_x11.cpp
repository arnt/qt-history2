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
#include <qlibrary.h>
#include <qdebug.h>
#include <private/qgl_p.h>
#include <private/qt_x11_p.h>
#include <private/qpaintengine_opengl_p.h>

#include <qx11info_x11.h>
#include <GL/glx.h>
#include <qimage.h>

#include "qglpbuffer.h"
#include "qglpbuffer_p.h"

#ifndef GLX_VERSION_1_3
#define GLX_RGBA_BIT            0x00000002
#define GLX_PBUFFER_BIT         0x00000004
#define GLX_DRAWABLE_TYPE       0x8010
#define GLX_RENDER_TYPE         0x8011
#define GLX_RGBA_TYPE           0x8014
#define GLX_PBUFFER_HEIGHT      0x8040
#define GLX_PBUFFER_WIDTH       0x8041
#endif

#ifndef GLX_ARB_multisample
#define GLX_SAMPLE_BUFFERS_ARB  100000
#define GLX_SAMPLES_ARB         100001
#endif

typedef GLXFBConfig* (*_glXChooseFBConfig) (Display *dpy, int screen, const int *attrib_list, int *nelements);
typedef int (*_glXGetFBConfigAttrib) (Display *dpy, GLXFBConfig config, int attribute, int *value);
typedef GLXPbuffer (*_glXCreatePbuffer) (Display *dpy, GLXFBConfig config, const int *attrib_list);
typedef void (*_glXDestroyPbuffer) (Display *dpy, GLXPbuffer pbuf);
typedef GLXContext (*_glXCreateNewContext) (Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef Bool (*_glXMakeContextCurrent) (Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);

static _glXChooseFBConfig qt_glXChooseFBConfig = 0;
static _glXCreateNewContext qt_glXCreateNewContext = 0;
static _glXCreatePbuffer qt_glXCreatePbuffer = 0;
static _glXDestroyPbuffer qt_glXDestroyPbuffer = 0;
static _glXGetFBConfigAttrib qt_glXGetFBConfigAttrib = 0;
static _glXMakeContextCurrent qt_glXMakeContextCurrent = 0;

#define glXChooseFBConfig qt_glXChooseFBConfig
#define glXCreateNewContext qt_glXCreateNewContext
#define glXCreatePbuffer qt_glXCreatePbuffer
#define glXDestroyPbuffer qt_glXDestroyPbuffer
#define glXGetFBConfigAttrib qt_glXGetFBConfigAttrib
#define glXMakeContextCurrent qt_glXMakeContextCurrent

static bool qt_resolve_pbuffer_extensions()
{
    static int resolved = false;
    if (resolved && qt_glXMakeContextCurrent)
        return true;
    else if (resolved)
        return false;

    QLibrary gl("GL");
    qt_glXChooseFBConfig = (_glXChooseFBConfig) gl.resolve("glXChooseFBConfig");
    qt_glXCreateNewContext = (_glXCreateNewContext) gl.resolve("glXCreateNewContext");
    qt_glXCreatePbuffer = (_glXCreatePbuffer) gl.resolve("glXCreatePbuffer");
    qt_glXDestroyPbuffer = (_glXDestroyPbuffer) gl.resolve("glXDestroyPbuffer");
    qt_glXGetFBConfigAttrib = (_glXGetFBConfigAttrib) gl.resolve("glXGetFBConfigAttrib");
    qt_glXMakeContextCurrent = (_glXMakeContextCurrent) gl.resolve("glXMakeContextCurrent");

    resolved = qt_glXMakeContextCurrent ? true : false;
    return resolved;
}

QGLPbuffer::QGLPbuffer(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
    : d_ptr(new QGLPbufferPrivate)
{
    Q_D(QGLPbuffer);

    if (!qt_resolve_pbuffer_extensions()) {
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
    if (f.sampleBuffers()) {
        attribs[i++] = GLX_SAMPLE_BUFFERS_ARB;
        attribs[i++] = 1;
        attribs[i++] = GLX_SAMPLES_ARB;
        attribs[i++] = f.samples() == -1 ? 4 : f.samples();
    }

    attribs[i] = XNone;

    GLXFBConfig *configs = glXChooseFBConfig(X11->display, X11->defaultScreen, attribs, &num_configs);
    if (configs && num_configs) {
        int res;
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_LEVEL, &res);
        d->format.setPlane(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_DOUBLEBUFFER, &res);
        d->format.setDoubleBuffer(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_DEPTH_SIZE, &res);
        d->format.setDepth(res);
        if (d->format.depth())
            d->format.setDepthBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_RGBA, &res);
        d->format.setRgba(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_ALPHA_SIZE, &res);
        d->format.setAlpha(res);
        if (d->format.alpha())
            d->format.setAlphaBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_ACCUM_RED_SIZE, &res);
        d->format.setAccum(res);
        if (d->format.accum())
            d->format.setAccumBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_STENCIL_SIZE, &res);
        d->format.setStencil(res);
        if (d->format.stencil())
            d->format.setStencilBufferSize(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_STEREO, &res);
        d->format.setStereo(res);
        glXGetFBConfigAttrib(X11->display, configs[0], GLX_SAMPLE_BUFFERS_ARB, &res);
        d->format.setSampleBuffers(res);
        if (d->format.sampleBuffers()) {
            glXGetFBConfigAttrib(X11->display, configs[0], GLX_SAMPLES_ARB, &res);
            d->format.setSamples(res);
        }

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
            qWarning("QGLPbuffer: Unable to create a pbuffer/context - giving up.");
        }

        // cleanup
        for (int i = 0; i < num_configs; ++i)
            XFree(reinterpret_cast<void *>(configs[i]));
        XFree(configs);
        d->qctx = new QGLContext(f);
    } else {
        qWarning("QGLPbuffer: Unable to find a context/format match - giving up.");
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


bool QGLPbuffer::hasPbuffers()
{
    return qt_resolve_pbuffer_extensions();
}
