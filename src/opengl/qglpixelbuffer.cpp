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

/*!
    \class QGLPixelBuffer
    \brief The QGLPixelBuffer class encapsulates an OpenGL pbuffer.
    \since 4.1

    \ingroup multimedia

    Rendering into a pbuffer is normally done using full hardware
    acceleration. This can be significantly faster than rendering
    into a QPixmap.

    There are three approaches to using this class:

    \list 1
    \o \bold{We can draw into the pbuffer and convert it to a QImage
       using toImage().} This is normally much faster than calling
       QGLWidget::renderPixmap().

    \o \bold{We can draw into the pbuffer and copy the contents into
       an OpenGL texture using updateDynamicTexture().} This allows
       us to create dynamic textures and works on all systems
       with pbuffer support.

    \o \bold{On systems that support it, we can bind the pbuffer to
       an OpenGL texture.} The texture is then updated automatically
       when the pbuffer contents change, eliminating the need for
       additional copy operations. This is supported only on Windows
       and Mac OS X systems that provide the \c render_texture
       extension.
    \endlist

    Pbuffers are provided by the OpenGL \c pbuffer extension; call
    hasOpenGLPbuffer() to find out if the system provides pbuffers.

    \sa {opengl/pbuffers}{Pbuffers Example}
*/

#include <qglpixelbuffer.h>
#include <private/qglpixelbuffer_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <qimage.h>


void QGLPixelBufferPrivate::common_init(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
{
    Q_Q(QGLPixelBuffer);
    if(init(size, f, shareWidget)) {
        req_size = size;
        req_format = f;
        req_shareWidget = shareWidget;
        invalid = false;
        qctx = new QGLContext(f);
        qctx->d_func()->sharing = (shareWidget != 0);
        qctx->d_func()->paintDevice = q;
    }
}

/*! \fn QGLPixelBuffer::QGLPixelBuffer(const QSize &size,
                                       const QGLFormat &format,
                                       QGLWidget *shareWidget)

    Constructs an OpenGL pbuffer of the given \a size. If no \a
    format is specified, the \l{QGLFormat::defaultFormat()}{default
    format} is used. If the \a shareWidget parameter points to a
    valid QGLWidget, the pbuffer will share its context with \a
    shareWidget.

    If you intend to bind this pbuffer as a dynamic texture, the width
    and height components of \c size must be powers of two (e.g., 512
    x 128).

    \sa size(), format()
*/
QGLPixelBuffer::QGLPixelBuffer(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
    : d_ptr(new QGLPixelBufferPrivate)
{
    Q_D(QGLPixelBuffer);
    d->common_init(size, f, shareWidget);
}


/*! \overload

    Constructs an OpenGL pbuffer with the \a width and \a height. If
    no \a format is specified, the
    \l{QGLFormat::defaultFormat()}{default format} is used. If the \a
    shareWidget parameter points to a valid QGLWidget, the pbuffer
    will share its context with \a shareWidget.

    If you intend to bind this pbuffer as a dynamic texture, the width
    and height components of \c size must be powers of two (e.g., 512
    x 128).

    \sa size(), format()
*/
QGLPixelBuffer::QGLPixelBuffer(int width, int height, const QGLFormat &f, QGLWidget *shareWidget)
    : d_ptr(new QGLPixelBufferPrivate)
{
    Q_D(QGLPixelBuffer);
    d->common_init(QSize(width, height), f, shareWidget);
}


/*! \fn QGLPixelBuffer::~QGLPixelBuffer()

    Destroys the pbuffer and frees any allocated resources.
*/
QGLPixelBuffer::~QGLPixelBuffer()
{
    Q_D(QGLPixelBuffer);
    d->cleanup();
    delete d->qctx;
    delete d_ptr;
}

/*! \fn bool QGLPixelBuffer::makeCurrent()

    Makes this pbuffer the current OpenGL rendering context. Returns
    true on success; otherwise returns false.

    \sa QGLContext::makeCurrent(), doneCurrent()
*/

/*! \fn bool QGLPixelBuffer::doneCurrent()

    Makes no context the current OpenGL context. Returns true on
    success; otherwise returns false.
*/

/*!
    Generates and binds a 2D GL texture that is the same size as the
    pbuffer, and returns the texture's ID. This can be used in
    conjunction with bindToDynamicTexture() and
    updateDynamicTexture().

    \sa size()
*/

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
GLuint QGLPixelBuffer::generateDynamicTexture() const
{
    Q_D(const QGLPixelBuffer);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, d->req_size.width(), d->req_size.height(), 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return texture;
}
#endif

/*! \fn bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)

    Binds the texture specified by \a texture_id to this pbuffer.
    Returns true on success; otherwise returns false.

    The texture must be of the same size and format as the pbuffer.

    To unbind the texture, call releaseFromDynamicTexture(). While
    the texture is bound, it is updated automatically when the
    pbuffer contents change, eliminating the need for additional copy
    operations.

    Example:

    \code
        QGLPixelBuffer pbuffer(...);
        ...
        pbuffer.makeCurrent();
        GLuint dynamicTexture = pbuffer.generateDynamicTexture();
        pbuffer.bindToDynamicTexture(dynamicTexture);
        ...
        pbuffer.releaseFromDynamicTexture();
    \endcode

    \warning This function uses the \c {render_texture} extension,
    which is currently not supported under X11. An alternative that
    works on all systems (including X11) is to manually copy the
    pbuffer contents to a texture using updateDynamicTexture().

    \warning For the bindToDynamicTexture() call to succeed on the
    Mac OS X, the pbuffer needs a shared context, i.e. the
    QGLPixelBuffer must be created with a share widget.

    \sa generateDynamicTexture(), releaseFromDynamicTexture()
*/

/*! \fn void QGLPixelBuffer::releaseFromDynamicTexture()

    Releases the pbuffer from any previously bound texture. Returns
    true on success; otherwise returns false.

    \sa bindToDynamicTexture()
*/

/*! \fn bool QGLPixelBuffer::hasOpenGLPbuffers()

    Returns true if the OpenGL \c pbuffer extension is present on
    this system; otherwise returns false.
*/

/*!
    Copies the pbuffer contents into the texture specified with \a
    texture_id.

    The texture must be of the same size and format as the pbuffer.

    Example:

    \code
        QGLPixelBuffer pbuffer(...);
        ...
        pbuffer.makeCurrent();
        GLuint dynamicTexture = pbuffer.generateDynamicTexture();
        ...
        pbuffer.updateDynamicTexture(dynamicTexture);
    \endcode

    An alternative on Windows and Mac OS X systems that support the
    \c render_texture extension is to use bindToDynamicTexture() to
    get dynamic updates of the texture.

    \sa generateDynamicTexture(), bindToDynamicTexture()
*/
void QGLPixelBuffer::updateDynamicTexture(GLuint texture_id) const
{
    Q_D(const QGLPixelBuffer);
    if (d->invalid)
        return;
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, d->req_size.width(), d->req_size.height(), 0);
}

/*!
    Returns the size of the pbuffer.
*/
QSize QGLPixelBuffer::size() const
{
    Q_D(const QGLPixelBuffer);
    return d->req_size;
}

/*!
    Returns the contents of the pbuffer as a QImage.
*/
QImage QGLPixelBuffer::toImage() const
{
    Q_D(const QGLPixelBuffer);
    if (d->invalid)
        return QImage();

    const_cast<QGLPixelBuffer *>(this)->makeCurrent();
    QImage img(d->req_size, QImage::Format_ARGB32);
    int w = d->req_size.width();
    int h = d->req_size.height();
    glReadPixels(0, 0, d->req_size.width(), d->req_size.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
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

/*!
    Returns the native pbuffer handle.
*/
Qt::HANDLE QGLPixelBuffer::handle() const
{
    Q_D(const QGLPixelBuffer);
    if (d->invalid)
        return 0;
    return d->pbuf;
}

/*!
    Returns true if this pbuffer is valid; otherwise returns false.
*/
bool QGLPixelBuffer::isValid() const
{
    Q_D(const QGLPixelBuffer);
    return !d->invalid;
}

Q_GLOBAL_STATIC(QOpenGLPaintEngine, qt_buffer_paintengine)

/*! \reimp */
QPaintEngine *QGLPixelBuffer::paintEngine() const
{
    return qt_buffer_paintengine();
}

extern int qt_defaultDpi();

/*! \reimp */
int QGLPixelBuffer::metric(PaintDeviceMetric metric) const
{
    Q_D(const QGLPixelBuffer);

    float dpmx = qt_defaultDpi()*100./2.54;
    float dpmy = qt_defaultDpi()*100./2.54;
    int w = d->req_size.width();
    int h = d->req_size.height();
    switch (metric) {
    case PdmWidth:
        return w;

    case PdmHeight:
        return h;

    case PdmWidthMM:
        return qRound(w * 1000 / dpmx);

    case PdmHeightMM:
        return qRound(h * 1000 / dpmy);

    case PdmNumColors:
        return 0;

    case PdmDepth:
        return 32;//d->depth;

    case PdmDpiX:
        return (int)(dpmx * 0.0254);

    case PdmDpiY:
        return (int)(dpmy * 0.0254);

    case PdmPhysicalDpiX:
        return (int)(dpmx * 0.0254);

    case PdmPhysicalDpiY:
        return (int)(dpmy * 0.0254);

    default:
        qWarning("QGLPixelBuffer::metric(), Unhandled metric type: %d\n", metric);
        break;
    }
    return 0;
}

/*!
    Generates and binds a 2D GL texture to the current context, based
    on \a image. The generated texture id is returned and can be used
    in later glBindTexture() calls.

    The \a target parameter specifies the texture target.

    Equivalent to calling QGLContext::bindTexture().

    \sa deleteTexture()
*/
GLuint QGLPixelBuffer::bindTexture(const QImage &image, GLenum target)
{
    Q_D(QGLPixelBuffer);
    return d->qctx->bindTexture(image, target, GL_RGBA8);
}


/*! \overload

    Generates and binds a 2D GL texture based on \a pixmap.

    Equivalent to calling QGLContext::bindTexture().

    \sa deleteTexture()
*/
GLuint QGLPixelBuffer::bindTexture(const QPixmap &pixmap, GLenum target)
{
    Q_D(QGLPixelBuffer);
    return d->qctx->bindTexture(pixmap, target, GL_RGBA8);
}

/*! \overload

    Reads the DirectDrawSurface (DDS) compressed file \a fileName and
    generates a 2D GL texture from it.

    Equivalent to calling QGLContext::bindTexture().

    \sa deleteTexture()
*/
GLuint QGLPixelBuffer::bindTexture(const QString &fileName)
{
    Q_D(QGLPixelBuffer);
    return d->qctx->bindTexture(fileName);
}

/*!
    Removes the texture identified by \a texture_id from the texture cache.

    Equivalent to calling QGLContext::deleteTexture().
 */
void QGLPixelBuffer::deleteTexture(GLuint texture_id)
{
    Q_D(QGLPixelBuffer);
    d->qctx->deleteTexture(texture_id);
}

/*!
    Returns the format of the pbuffer. The format may be different
    from the one that was requested.
*/
QGLFormat QGLPixelBuffer::format() const
{
    Q_D(const QGLPixelBuffer);
    return d->format;
}

/*! \fn int QGLPixelBuffer::devType() const

    \reimp
*/
