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

#include "qglpbuffer.h"
#include "qglpbuffer_p.h"
#include <AGL/agl.h>

#include <qimage.h>
#include <private/qgl_p.h>
#include <qdebug.h>


QGLPbuffer::QGLPbuffer(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
    : d_ptr(new QGLPbufferPrivate)
{
    Q_D(QGLPbuffer);
    GLint attribs[40];
    int i = 0;

    attribs[i++] = AGL_RGBA;
    attribs[i++] = AGL_ACCELERATED;
    attribs[i] = AGL_NONE;

    AGLPixelFormat format = aglChoosePixelFormat(0, 0, attribs);
    if (!format) {
	qWarning("Unable to get a pixel format.");
    }

    AGLContext share = 0;
    if (shareWidget)
	share = d->share_ctx = static_cast<AGLContext>(shareWidget->d_func()->glcx->d_func()->cx);
    d->ctx = aglCreateContext(format, share);

    if (!aglCreatePBuffer(size.width(), size.height(), GL_TEXTURE_2D, GL_RGBA, 0, &d->pbuf)) {
	qWarning("Unable to create pbuffer - giving up.");
	return;
    }

    if (!aglSetPBuffer(d->ctx, d->pbuf, 0, 0, 0)) {
	qWarning("Unable to set pbuffer - giving up.");
	return;
    }

    aglDestroyPixelFormat(format);
    d->invalid = false;
    d->size = size;
}

QGLPbuffer::~QGLPbuffer()
{
    Q_D(QGLPbuffer);
    aglSetCurrentContext(0);
    aglDestroyContext(d->ctx);
    aglDestroyPBuffer(d->pbuf);
    delete d_ptr;
}

bool QGLPbuffer::bind(GLuint texture_id, GLenum target)
{
    Q_D(QGLPbuffer);
    if (d->invalid)
	return false;
    glBindTexture(target, texture_id);
    aglTexImagePBuffer(d->share_ctx, d->pbuf, GL_FRONT);
    glBindTexture(target, texture_id); // updates texture target
    return true;
}

bool QGLPbuffer::release()
{
    return false;
}

bool QGLPbuffer::makeCurrent()
{
    Q_D(QGLPbuffer);
    if (d->invalid)
        return false;

    return aglSetCurrentContext(d->ctx);
}

bool QGLPbuffer::doneCurrent()
{
    Q_D(QGLPbuffer);
    if (d->invalid)
        return false;
    return false;
}

void QGLPbuffer::copyToTexture(GLuint texture_id, GLenum target, GLint format)
{
    Q_D(QGLPbuffer);
    if (d->invalid)
        return;
    glBindTexture(target, texture_id);
    glCopyTexImage2D(target, 0, format, 0, 0, d->size.width(), d->size.height(), 0);
}

GLuint QGLPbuffer::generateTexture(GLenum target, GLint format)
{
    Q_D(QGLPbuffer);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    aglTexImagePBuffer(d->share_ctx, d->pbuf, GL_FRONT);
    glBindTexture(target, texture); // updates texture target
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return texture;
}
