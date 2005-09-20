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

/*!
    \class QGLBuffer
    \brief The QGLBuffer class encapsulates an OpenGL offscreen buffer.

    \ingroup multimedia

    QGLBuffer provides functionality for creating and managing an
    OpenGL offscreen buffer. An OpenGL offscreen buffer can
    be rendered into using full hardware acceleration. This is usually
    much faster than rendering into a system pixmap, since software
    rendering is often used in that case. Under Windows and on the Mac
    it is also possible to bind the buffer directly as a texture
    using the \c render_texture, thus eliminating the need for
    additional copy operations to generate dynamic textures.

    Note that when makeing use of the \c render_texture extension, the
    well known power-of-2 rule applies to the size of the buffer. If
    the size of the buffer is a non-power of 2 size, it can not be
    bound to a texture.
*/

#include <qglpbuffer.h>
#include <private/qglpbuffer_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <qimage.h>

/*! \fn bool QGLBuffer::makeCurrent()

    Makes this buffer the current GL rendering context. Returns true
    on success, false otherwise.
 */

/*! \fn bool QGLBuffer::doneCurrent()

    Makes no context the current GL context. Returns true on success,
    false otherwise.
 */

/*! \fn GLuint QGLBuffer::generateTexture(GLint format)

    This is a convenience function that generates and binds a 2D GL
    texture that is the same size as the buffer, using \a format as
    the internal texture format. The default internal format of the
    generated texture is \c GL_RGBA8. The generated texture id is
    returned.
*/

/*! \fn bool QGLBuffer::bind(GLuint texture_id)

    Binds the texture specified with \a texture_id to this
    buffer. Returns true on success, false otherwise.

    This function uses the \c {render_texture} extension, which is
    currently not supported under X11. Under X11 you can achieve the
    same by copying the buffer contents to a texture after drawing
    into the buffer using copyToTexture().
 */

/*! \fn bool QGLBuffer::release()

    Releases the buffer from any previously bound texture. Returns
    true on success, false otherwise.

    This function uses the \c {render_texture} extension, which is
    currently not supported under X11.
*/

/*! \fn void QGLBuffer::copyToTexture(GLuint texture_id, GLint format)

    This is a convenience function that copies the buffer contents
    (using \c {glCopyTexImage2D()}) into the texture specified with \a
    texture_id, which has the internal format \a format. The default
    format is \c GL_RGBA8.
 */
void QGLBuffer::copyToTexture(GLuint texture_id, GLint format)
{
    Q_D(QGLBuffer);
    if (d->invalid)
        return;
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0, d->size.width(), d->size.height(), 0);
}

/*!
    Returns the size of the buffer.
 */
QSize QGLBuffer::size() const
{
    Q_D(const QGLBuffer);
    return d->size;
}


/*!
    Returns the contents of the buffer as a QImage.
 */
QImage QGLBuffer::toImage() const
{
    Q_D(const QGLBuffer);
    if (d->invalid)
        return QImage();

    const_cast<QGLBuffer *>(this)->makeCurrent();
    QImage img(d->size, QImage::Format_ARGB32);
    int w = d->size.width();
    int h = d->size.height();
    glReadPixels(0, 0, d->size.width(), d->size.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
	// OpenGL gives RGBA; Qt wants ARGB
	uint *p = (uint*)img.bits();
	uint *end = p + w*h;
	if (1) {
	    while (p < end) {
		uint a = *p << 24;
		*p = (*p >> 8) | a;
		p++;
	    }
	} else {
	    while (p < end) {
		*p = 0xFF000000 | (*p>>8);
		++p;
	    }
	}
    } else {
	// OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
	img = img.rgbSwapped();
    }
    return img.mirrored();
}

Qt::HANDLE QGLBuffer::handle() const
{
    Q_D(const QGLBuffer);
    if (d->invalid)
        return 0;
    return 0;
}

/*!
    Returns true if this buffer is valid.
*/
bool QGLBuffer::isValid() const
{
    Q_D(const QGLBuffer);
    return !d->invalid;
}

Q_GLOBAL_STATIC(QOpenGLPaintEngine, qt_buffer_paintengine)
/*! \reimp
*/
QPaintEngine *QGLBuffer::paintEngine() const
{
    return qt_buffer_paintengine();
}

extern int qt_defaultDpi();

/*!
    Returns the size for the specified \a metric on the device.
*/
int QGLBuffer::metric(PaintDeviceMetric metric) const
{
    Q_D(const QGLBuffer);

    float dpmx = qt_defaultDpi()*100./2.54;
    float dpmy = qt_defaultDpi()*100./2.54;
    int w = d->size.width();
    int h = d->size.height();
    switch (metric) {
    case PdmWidth:
        return w;
        break;

    case PdmHeight:
        return h;
        break;

    case PdmWidthMM:
        return qRound(w * 1000 / dpmx);
        break;

    case PdmHeightMM:
        return qRound(h * 1000 / dpmy);
        break;

    case PdmNumColors:
        return 0;
        break;

    case PdmDepth:
        return 32;//d->depth;
        break;

    case PdmDpiX:
        return (int)(dpmx * 0.0254);
        break;

    case PdmDpiY:
        return (int)(dpmy * 0.0254);
        break;

    case PdmPhysicalDpiX:
        return (int)(dpmx * 0.0254);
        break;

    case PdmPhysicalDpiY:
        return (int)(dpmy * 0.0254);
        break;

    default:
        qWarning("QGLBuffer::metric(), Unhandled metric type: %d\n", metric);
        break;
    }
    return 0;
}

/*!
    The same as calling QGLContext::bindTexture().

    \sa deleteTexture()
*/
GLuint QGLBuffer::bindTexture(const QImage &image, GLenum target, GLint format)
{
    Q_D(QGLBuffer);
    return d->qctx->bindTexture(image, target, format);
}


/*! \overload

    Same as calling QGLContext::bindTexture().

    \sa deleteTexture()
*/
GLuint QGLBuffer::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
    Q_D(QGLBuffer);
    return d->qctx->bindTexture(pixmap, target, format);
}

/*! \overload

    The same as calling QGLContext::bindTexture().

    \sa deleteTexture()
*/
GLuint QGLBuffer::bindTexture(const QString &fileName)
{
    Q_D(QGLBuffer);
    return d->qctx->bindTexture(fileName);
}

/*!
    The same as calling QGLContext::deleteTexture().
 */
void QGLBuffer::deleteTexture(GLuint texture_id)
{
    Q_D(QGLBuffer);
    d->qctx->deleteTexture(texture_id);
}
