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
    \class QGLPbuffer
    \brief The QGLPbuffer class encapsulates an OpenGL pbuffer.

    \ingroup multimedia

    QGLPbuffer provides functionality for creating and managing an
    OpenGL pbuffer. An OpenGL pbuffer is an offscreen buffer that can
    be rendered into using full hardware acceleration. This is usually
    much faster than rendering into a system pixmap, since software
    rendering is often used in that case. Under Windows and on the Mac
    it is also possible to bind the pbuffer directly as a texture,
    thus eliminating the need for additional copy operations to
    generate dynamic textures.
*/

#include "qglpbuffer.h"
#include "qglpbuffer_p.h"
#include <private/qpaintengine_opengl_p.h>
#include <qimage.h>

/*! \fn bool QGLPbuffer::makeCurrent()

    Makes this pbuffer the current GL rendering context. Returns true
    on success, false otherwise.
 */

/*! \fn bool QGLPbuffer::doneCurrent()

    Makes no context the current GL context. Returns true on success, false
    otherwise.
 */

/*! \fn GLuint QGLPbuffer::generateTexture(GLenum target, GLint format)

    This is a convenience function that generates and binds a GL
    texture that is the same size as the pbuffer, using target as
    texture target and format as texture format. The generated texture
    id is returned.
*/

/*! \fn void QGLPbuffer::copyToTexture(GLuint texture_id, GLenum target, GLint format)

    This is a convenience function that copies the pbuffer contents
    (using \c {glCopyTexImage2D()}) into the texture specified with \c
    {texture_id}.
 */

/*! \fn bool QGLPbuffer::bind(GLuint texture_id, GLenum target)

    Binds the texture specified with \c {texture_id} to the
    pbuffer. Returns true on success, false otherwise.

    This function uses the \c {render_texture} extension, which is
    currently not supported under X11. Under X11 you can achieve the
    same by copying the pbuffer contents to a texture after drawing
    into the pbuffer using copyToTexture().
 */

/*! \fn bool QGLPbuffer::release()

    Releases the pbuffer from any previously bound texture. Returns
    true on success, false otherwise.

    This function uses the \c {render_texture} extension, which is
    currently not supported under X11.
*/

/*!
    Returns the size of the pbuffer.
 */
QSize QGLPbuffer::size() const
{
    Q_D(const QGLPbuffer);
    return d->size;
}


/*!
    Returns the contents of the pbuffer as a QImage.
 */
QImage QGLPbuffer::toImage() const
{
    Q_D(const QGLPbuffer);
    if (d->invalid)
        return QImage();

    const_cast<QGLPbuffer *>(this)->makeCurrent();
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

//     Q_D(const QGLPbuffer);
//     if (d->invalid)
//         return QImage();

//     const_cast<QGLPbuffer *>(this)->makeCurrent();
//     QImage img(d->size, QImage::Format_ARGB32);
//     glReadPixels(0, 0, d->size.width(), d->size.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
//     return img.rgbSwapped();
}

Qt::HANDLE QGLPbuffer::handle() const
{
    Q_D(const QGLPbuffer);
    if (d->invalid)
        return 0;
    return 0;
}

/*!
    Returns true if this pbuffer is valid.
 */
bool QGLPbuffer::isValid() const
{
    Q_D(const QGLPbuffer);
    return !d->invalid;;
}

QPaintEngine *QGLPbuffer::paintEngine() const
{
    if (!d_ptr->paintEngine)
        d_ptr->paintEngine = new QOpenGLPaintEngine();
    return d_ptr->paintEngine;
}

extern int qt_defaultDpi();

/*!
    Returns the size for the specified \a metric on the device.
*/
int QGLPbuffer::metric(PaintDeviceMetric metric) const
{
    Q_D(const QGLPbuffer);

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
        qWarning("QGLPbuffer::metric(), Unhandled metric type: %d\n", metric);
        break;
    }
    return 0;
}


