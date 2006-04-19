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

bool
QGLPixelBufferPrivate::init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
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
        return false;
    }

    GLint res;
    aglDescribePixelFormat(format, AGL_LEVEL, &res);
    this->format.setPlane(res);
    aglDescribePixelFormat(format, AGL_DOUBLEBUFFER, &res);
    this->format.setDoubleBuffer(res);
    aglDescribePixelFormat(format, AGL_DEPTH_SIZE, &res);
    this->format.setDepth(res);
    if (this->format.depth())
	this->format.setDepthBufferSize(res);
    aglDescribePixelFormat(format, AGL_RGBA, &res);
    this->format.setRgba(res);
    aglDescribePixelFormat(format, AGL_ALPHA_SIZE, &res);
    this->format.setAlpha(res);
    if (this->format.alpha())
	this->format.setAlphaBufferSize(res);
    aglDescribePixelFormat(format, AGL_ACCUM_RED_SIZE, &res);
    this->format.setAccum(res);
    if (this->format.accum())
	this->format.setAccumBufferSize(res);
    aglDescribePixelFormat(format, AGL_STENCIL_SIZE, &res);
    this->format.setStencil(res);
    if (this->format.stencil())
	this->format.setStencilBufferSize(res);
    aglDescribePixelFormat(format, AGL_STEREO, &res);
    this->format.setStereo(res);
    aglDescribePixelFormat(format, AGL_SAMPLE_BUFFERS_ARB, &res);
    this->format.setSampleBuffers(res);
    if (this->format.sampleBuffers()) {
        aglDescribePixelFormat(format, AGL_SAMPLES_ARB, &res);
        this->format.setSamples(res);
    }

    AGLContext share = 0;
    if (shareWidget)
	share = share_ctx = static_cast<AGLContext>(shareWidget->d_func()->glcx->d_func()->cx);
    ctx = aglCreateContext(format, share);
    if (!ctx) {
	qWarning("QGLPixelBuffer: Unable to create a context (AGL error %d).",
		 (int) aglGetError());
	return false;
    }

    if (!aglCreatePBuffer(size.width(), size.height(), GL_TEXTURE_2D, GL_RGBA, 0, &pbuf)) {
	qWarning("QGLPixelBuffer: Unable to create a pbuffer (AGL error %d).",
		 (int) aglGetError());
	return false;
    }

    if (!aglSetPBuffer(ctx, pbuf, 0, 0, 0)) {
	qWarning("QGLPixelBuffer: Unable to set pbuffer (AGL error %d).",
		 (int) aglGetError());
	return false;
    }

    aglDestroyPixelFormat(format);
    return true;
#else
    Q_UNUSED(size);
    Q_UNUSED(f);
    Q_UNUSED(shareWidget);
    return false;
#endif
}

bool
QGLPixelBufferPrivate::cleanup()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    aglSetCurrentContext(0);
    aglDestroyContext(ctx);
    aglDestroyPBuffer(pbuf);
    return true;
#endif
    return false;
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
