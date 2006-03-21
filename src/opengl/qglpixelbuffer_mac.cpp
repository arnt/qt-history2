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

#include "qglpixelbuffer.h"
#include "qglpixelbuffer_p.h"
#include <AGL/agl.h>

#include <qimage.h>
#include <private/qgl_p.h>
#include <qdebug.h>

QGLPixelBuffer::QGLPixelBuffer(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
    : d_ptr(new QGLPixelBufferPrivate)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(QGLPixelBuffer);

    GLint attribs[40], i=0;
    attribs[i++] = AGL_RGBA;
    attribs[i++] = AGL_BUFFER_SIZE;
    attribs[i++] = 32;
    attribs[i++] = AGL_LEVEL;
    attribs[i++] = f.plane();
    if (f.stereo())
        attribs[i++] = AGL_STEREO;
    if (f.alpha()) {
        attribs[i++] = AGL_ALPHA_SIZE;
        attribs[i++] = f.alphaBufferSize() == -1 ? 8 : f.alphaBufferSize();
    }
    if (f.stencil()) {
        attribs[i++] = AGL_STENCIL_SIZE;
        attribs[i++] = f.stencilBufferSize() == -1 ? 8 : f.stencilBufferSize();
    }
    if (f.depth()) {
        attribs[i++] = AGL_DEPTH_SIZE;
        attribs[i++] = f.depthBufferSize() == -1 ? 32 : f.depthBufferSize();
    }
    if (f.accum()) {
        attribs[i++] = AGL_ACCUM_RED_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
        attribs[i++] = AGL_ACCUM_BLUE_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
        attribs[i++] = AGL_ACCUM_GREEN_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
        attribs[i++] = AGL_ACCUM_ALPHA_SIZE;
        attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
    }

    if (f.sampleBuffers()) {
        attribs[i++] = AGL_SAMPLE_BUFFERS_ARB;
        attribs[i++] = 1;
        attribs[i++] = AGL_SAMPLES_ARB;
        attribs[i++] = f.samples() == -1 ? 4 : f.samples();
    }
    attribs[i] = AGL_NONE;

    AGLPixelFormat format = aglChoosePixelFormat(0, 0, attribs);
    if (!format) {
	qWarning("QGLPixelBuffer: Unable to find a pixel format (AGL error %d).",
		 (int) aglGetError());
    }

    GLint res;
    aglDescribePixelFormat(format, AGL_LEVEL, &res);
    d->format.setPlane(res);
    aglDescribePixelFormat(format, AGL_DOUBLEBUFFER, &res);
    d->format.setDoubleBuffer(res);
    aglDescribePixelFormat(format, AGL_DEPTH_SIZE, &res);
    d->format.setDepth(res);
    if (d->format.depth())
	d->format.setDepthBufferSize(res);
    aglDescribePixelFormat(format, AGL_RGBA, &res);
    d->format.setRgba(res);
    aglDescribePixelFormat(format, AGL_ALPHA_SIZE, &res);
    d->format.setAlpha(res);
    if (d->format.alpha())
	d->format.setAlphaBufferSize(res);
    aglDescribePixelFormat(format, AGL_ACCUM_RED_SIZE, &res);
    d->format.setAccum(res);
    if (d->format.accum())
	d->format.setAccumBufferSize(res);
    aglDescribePixelFormat(format, AGL_STENCIL_SIZE, &res);
    d->format.setStencil(res);
    if (d->format.stencil())
	d->format.setStencilBufferSize(res);
    aglDescribePixelFormat(format, AGL_STEREO, &res);
    d->format.setStereo(res);
    aglDescribePixelFormat(format, AGL_SAMPLE_BUFFERS_ARB, &res);
    d->format.setSampleBuffers(res);
    if (d->format.sampleBuffers()) {
        aglDescribePixelFormat(format, AGL_SAMPLES_ARB, &res);
        d->format.setSamples(res);
    }

    AGLContext share = 0;
    if (shareWidget)
	share = d->share_ctx = static_cast<AGLContext>(shareWidget->d_func()->glcx->d_func()->cx);
    d->ctx = aglCreateContext(format, share);
    if (!d->ctx) {
	qWarning("QGLPixelBuffer: Unable to create a context (AGL error %d).",
		 (int) aglGetError());
	return;
    }

    if (!aglCreatePBuffer(size.width(), size.height(), GL_TEXTURE_2D, GL_RGBA, 0, &d->pbuf)) {
	qWarning("QGLPixelBuffer: Unable to create a pbuffer (AGL error %d).",
		 (int) aglGetError());
	return;
    }

    if (!aglSetPBuffer(d->ctx, d->pbuf, 0, 0, 0)) {
	qWarning("QGLPixelBuffer: Unable to set pbuffer (AGL error %d).",
		 (int) aglGetError());
	return;
    }

    aglDestroyPixelFormat(format);
    d->invalid = false;
    d->size = size;
    d->qctx = new QGLContext(f);
    d->qctx->d_func()->sharing = (shareWidget != 0);
    d->qctx->d_func()->paintDevice = this;
#else
    Q_UNUSED(size);
    Q_UNUSED(f);
    Q_UNUSED(shareWidget);
#endif
}

QGLPixelBuffer::~QGLPixelBuffer()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(QGLPixelBuffer);
    aglSetCurrentContext(0);
    aglDestroyContext(d->ctx);
    aglDestroyPBuffer(d->pbuf);
    delete d->qctx;
    delete d_ptr;
#endif
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(QGLPixelBuffer);
    if (d->invalid || !d->share_ctx)
	return false;
    aglSetCurrentContext(d->share_ctx);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    aglTexImagePBuffer(d->share_ctx, d->pbuf, GL_FRONT);
    return true;
#else
    Q_UNUSED(texture_id);
    return false;
#endif
}

void QGLPixelBuffer::releaseFromDynamicTexture()
{
}

bool QGLPixelBuffer::makeCurrent()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(QGLPixelBuffer);
    if (d->invalid)
        return false;

    return aglSetCurrentContext(d->ctx);
#else
    return false;
#endif
}

bool QGLPixelBuffer::doneCurrent()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(QGLPixelBuffer);
    if (d->invalid)
        return false;
    return aglSetCurrentContext(0);
#else
    return false;
#endif
}

GLuint QGLPixelBuffer::generateDynamicTexture() const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    Q_D(const QGLPixelBuffer);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    aglTexImagePBuffer(d->share_ctx, d->pbuf, GL_FRONT);
    glBindTexture(GL_TEXTURE_2D, texture); // updates texture target
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return texture;
#else
    return 0;
#endif
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    return true;
#else
    return false;
#endif
}
