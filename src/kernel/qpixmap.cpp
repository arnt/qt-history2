/****************************************************************************
**
** Implementation of QPixmap class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpixmap.h"

#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qapplication.h"
#include <private/qinternal_p.h>
#include "qmime.h"
#include "qdragobject.h"
#include "qevent.h"
#include "qfile.h"

/*!
    \class QPixmap qpixmap.h
    \brief The QPixmap class is an off-screen, pixel-based paint device.

    \ingroup graphics
    \ingroup images
    \ingroup shared
    \mainclass

    QPixmap is one of the two classes Qt provides for dealing with
    images; the other is QImage. QPixmap is designed and optimized
    for drawing; QImage is designed and optimized for I/O and for
    direct pixel access/manipulation. There are (slow) functions to
    convert between QImage and QPixmap: convertToImage() and
    convertFromImage().

    One common use of the QPixmap class is to enable smooth updating
    of widgets. Whenever something complex needs to be drawn, you can
    use a pixmap to obtain flicker-free drawing, like this:

    \list 1
    \i Create a pixmap with the same size as the widget.
    \i Fill the pixmap with the widget background color.
    \i Paint the pixmap.
    \i bitBlt() the pixmap contents onto the widget.
    \endlist

    Pixel data in a pixmap is internal and is managed by the
    underlying window system. Pixels can be accessed only through
    QPainter functions, through bitBlt(), and by converting the
    QPixmap to a QImage.

    You can easily display a QPixmap on the screen using
    QLabel::setPixmap(). For example, all the QButton subclasses
    support pixmap use.

    The QPixmap class uses \link shclass.html copy-on-write\endlink,
    so it is practical to pass QPixmap objects by value.

    You can retrieve the width(), height(), depth() and size() of a
    pixmap. The enclosing rectangle is given by rect(). Pixmaps can be
    filled with fill() and resized with resize(). You can create and
    set a mask with createHeuristicMask() and setMask(). Use
    selfMask() to see if the pixmap is identical to its mask.

    In addition to loading a pixmap from file using load() you can
    also loadFromData(). You can control optimization with
    setOptimization() and obtain a transformed version of the pixmap
    using xForm()

    Note regarding Windows 95 and 98: on Windows 9x the system crashes
    if you create more than about 1000 pixmaps, independent of the
    size of the pixmaps or installed RAM. Windows NT and 2000 do not
    have this limitation.

    Qt tries to work around the resource limitation. If you set the
    pixmap optimization to \c QPixmap::MemoryOptim and the width of
    your pixmap is less than or equal to 128 pixels, Qt stores the
    pixmap in a way that is very memory-efficient when there are many
    pixmaps.

    If your application uses dozens or hundreds of pixmaps (for
    example on tool bar buttons and in popup menus), and you plan to
    run it on Windows 95 or Windows 98, we recommend using code like
    this:

    \code
	QPixmap::setDefaultOptimization( QPixmap::MemoryOptim );
	while ( ... ) {
	    // load tool bar pixmaps etc.
	    QPixmap *pixmap = new QPixmap(fileName);
	}
	QPixmap::setDefaultOptimization( QPixmap::NormalOptim );
    \endcode

    \sa QBitmap, QImage, QImageIO, \link shclass.html Shared Classes\endlink
*/

/*!
    \enum QPixmap::ColorMode

    This enum type defines the color modes that exist for converting
    QImage objects to QPixmap.

    \value Auto  Select \c Color or \c Mono on a case-by-case basis.
    \value Color Always create colored pixmaps.
    \value Mono  Always create bitmaps.
*/

/*!
    \enum QPixmap::Optimization

    QPixmap has the choice of optimizing for speed or memory in a few
    places; the best choice varies from pixmap to pixmap but can
    generally be derived heuristically. This enum type defines a
    number of optimization modes that you can set for any pixmap to
    tweak the speed/memory tradeoffs:

    \value DefaultOptim  Whatever QPixmap::defaultOptimization()
	returns. A pixmap with this optimization will have whatever
	the current default optimization is. If the default
	optimization is changed using setDefaultOptimization(), then
	this will not effect any pixmaps that have already been
	created.

    \value NoOptim  No optimization (currently the same as \c
	MemoryOptim).

    \value MemoryOptim  Optimize for minimal memory use.

    \value NormalOptim  Optimize for typical usage. Often uses more
	memory than \c MemoryOptim, and is often faster.

    \value BestOptim  Optimize for pixmaps that are drawn very often
	and where performance is critical. Generally uses more memory
	than \c NormalOptim and may provide a little more speed.

    We recommend using \c DefaultOptim.

*/


QPixmap::Optimization QPixmap::defOptim = QPixmap::NormalOptim;


/*!
  \Internal
  Private constructor which takes the bitmap flag, the optimization.and a screen.
*/

QPixmap::QPixmap( int w, int h, int depth, bool bitmap,
		  Optimization optimization )
    : QPaintDevice( QInternal::Pixmap )
{
    init( w, h, depth, bitmap, optimization );
}


/*!
    Constructs a null pixmap.

    \sa isNull()
*/

QPixmap::QPixmap()
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
}

/*!
    Constructs a pixmap from the QImage \a image.

    \sa convertFromImage()
*/

QPixmap::QPixmap( const QImage& image )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    convertFromImage( image );
}

/*!
    Constructs a pixmap with \a w width, \a h height and \a depth bits
    per pixel. The pixmap is optimized in accordance with the \a
    optimization value.

    The contents of the pixmap is uninitialized.

    The \a depth can be either 1 (monochrome) or the depth of the
    current video mode. If \a depth is negative, then the hardware
    depth of the current video mode will be used.

    If either \a w or \a h is zero, a null pixmap is constructed.

    \sa isNull() QPixmap::Optimization
*/

QPixmap::QPixmap( int w, int h, int depth, Optimization optimization )
    : QPaintDevice( QInternal::Pixmap )
{
    init( w, h, depth, FALSE, optimization );
}

/*!
    \overload QPixmap::QPixmap( const QSize &size, int depth, Optimization optimization )

    Constructs a pixmap of size \a size, \a depth bits per pixel,
    optimized in accordance with the \a optimization value.
*/

QPixmap::QPixmap( const QSize &size, int depth, Optimization optimization )
    : QPaintDevice( QInternal::Pixmap )
{
    init( size.width(), size.height(), depth, FALSE, optimization );
}

#ifndef QT_NO_IMAGEIO
/*!
    Constructs a pixmap from the file \a fileName. If the file does
    not exist or is of an unknown format, the pixmap becomes a null
    pixmap.

    The \a fileName, \a format and \a conversion_flags parameters are
    passed on to load(). This means that the data in \a fileName is
    not compiled into the binary. If \a fileName contains a relative
    path (e.g. the filename only) the relevant file must be found
    relative to the runtime working directory.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a
    conversion_flags to specify how you'd prefer this to happen.

    \sa Qt::ImageConversionFlags isNull(), load(), loadFromData(), save(), imageFormat()
*/

QPixmap::QPixmap( const QString& fileName, const char *format,
	int conversion_flags )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    load( fileName, format, conversion_flags );
}

/*!
    Constructs a pixmap from the file \a fileName. If the file does
    not exist or is of an unknown format, the pixmap becomes a null
    pixmap.

    The \a fileName, \a format and \a mode parameters are passed on to
    load(). This means that the data in \a fileName is not compiled
    into the binary. If \a fileName contains a relative path (e.g. the
    filename only) the relevant file must be found relative to the
    runtime working directory.

    \sa QPixmap::ColorMode isNull(), load(), loadFromData(), save(), imageFormat()
*/

QPixmap::QPixmap( const QString& fileName, const char *format, ColorMode mode )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    load( fileName, format, mode );
}

/*!
    Constructs a pixmap from \a xpm, which must be a valid XPM image.

    Errors are silently ignored.

    Note that it's possible to squeeze the XPM variable a little bit
    by using an unusual declaration:

    \code
	static const char * const start_xpm[]={
	    "16 15 8 1",
	    "a c #cec6bd",
	....
    \endcode

    The extra \c const makes the entire definition read-only, which is
    slightly more efficient (for example, when the code is in a shared
    library) and ROMable when the application is to be stored in ROM.

    In order to use that sort of declaration you must cast the
    variable back to \c{const char **} when you create the QPixmap.
*/

QPixmap::QPixmap( const char * const xpm[] )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    QImage image( xpm );
    if ( !image.isNull() )
	convertFromImage( image );
}
#endif //QT_NO_IMAGEIO

/*!
    Constructs a pixmap that is a copy of \a pixmap.
*/

QPixmap::QPixmap( const QPixmap &pixmap )
    : QPaintDevice( QInternal::Pixmap )
{
    if ( pixmap.paintingActive() ) {		// make a deep copy
	data = 0;
	operator=( pixmap.copy() );
    } else {
	data = pixmap.data;
	data->ref();
	devFlags = pixmap.devFlags;		// copy QPaintDevice flags
#if defined(Q_WS_WIN)
	hdc = pixmap.hdc;			// copy Windows device context
#elif defined(Q_WS_X11)
	hd = pixmap.hd;				// copy X11 drawable
	rendhd = pixmap.rendhd;
	copyX11Data( &pixmap );			// copy x11Data
#elif defined(Q_WS_MAC)
	hd = pixmap.hd;
#endif
    }
}


/*!
    Destroys the pixmap.
*/

QPixmap::~QPixmap()
{
    deref();
}

/*! Convenience function. Gets the data associated with the absolute
  name \a abs_name from the default mime source factory and decodes it
  to a pixmap.

  \sa QMimeSourceFactory, QImage::fromMimeSource(), QImageDrag::decode()
*/

#ifndef QT_NO_MIME
QPixmap QPixmap::fromMimeSource( const QString &abs_name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( abs_name );
    if ( !m ) {
	if ( QFile::exists( abs_name ) )
	    return QPixmap( abs_name );
	if ( !abs_name.isEmpty() )
	    qWarning( "QPixmap::fromMimeSource: Cannot find pixmap \"%s\" in the mime source factory",
		      abs_name.latin1() );
	return QPixmap();
    }
    QPixmap pix;
    QImageDrag::decode( m, pix );
    return pix;
}
#endif

/*!
    Returns a \link shclass.html deep copy\endlink of the pixmap using
    the bitBlt() function to copy the pixels.

    \sa operator=()
*/

QPixmap QPixmap::copy( bool ignoreMask ) const
{
#if defined(Q_WS_X11)
    int old = x11SetDefaultScreen( x11Screen() );
#endif // Q_WS_X11

    QPixmap pm( data->w, data->h, data->d, data->bitmap, data->optim );

    if ( !pm.isNull() ) {			// copy the bitmap
#if defined(Q_WS_X11)
	pm.cloneX11Data( this );
#endif // Q_WS_X11

	if ( ignoreMask )
	    bitBlt( &pm, 0, 0, this, 0, 0, data->w, data->h, Qt::CopyROP, TRUE );
	else
	    copyBlt( &pm, 0, 0, this, 0, 0, data->w, data->h );
    }

#if defined(Q_WS_X11)
    x11SetDefaultScreen( old );
#endif // Q_WS_X11

    return pm;
}


/*!
    Assigns the pixmap \a pixmap to this pixmap and returns a
    reference to this pixmap.
*/

QPixmap &QPixmap::operator=( const QPixmap &pixmap )
{
    if ( paintingActive() ) {
	qWarning("QPixmap::operator=: Cannot assign to pixmap during painting");
	return *this;
    }
    pixmap.data->ref();				// avoid 'x = x'
    deref();
    if ( pixmap.paintingActive() ) {		// make a deep copy
	init( pixmap.width(), pixmap.height(), pixmap.depth(),
	      pixmap.data->bitmap, pixmap.data->optim );
	data->uninit = FALSE;
	if ( !isNull() )
	    copyBlt( this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height() );
	pixmap.data->deref();
    } else {
	data = pixmap.data;
	devFlags = pixmap.devFlags;		// copy QPaintDevice flags
#if defined(Q_WS_WIN)
	hdc = pixmap.hdc;
#elif defined(Q_WS_X11)
	hd = pixmap.hd;				// copy QPaintDevice drawable
	rendhd = pixmap.rendhd;
	copyX11Data( &pixmap );			// copy x11Data
#elif defined(Q_WS_MAC)
	hd = pixmap.hd;
#endif
	deviceGC = pixmap.deviceGC;
    }
    return *this;
}


/*!
    \overload

    Converts the image \a image to a pixmap that is assigned to this
    pixmap. Returns a reference to the pixmap.

    \sa convertFromImage().
*/

QPixmap &QPixmap::operator=( const QImage &image )
{
    convertFromImage( image );
    return *this;
}

/*!
    Returns the actual matrix used for transforming a pixmap with \a w
    width and \a h height and matrix \a m.

    When transforming a pixmap with xForm(), the transformation matrix
    is internally adjusted to compensate for unwanted translation,
    i.e. xForm() returns the smallest pixmap containing all
    transformed points of the original pixmap.

    This function returns the modified matrix, which maps points
    correctly from the original pixmap into the new pixmap.

    \sa xForm(), QWMatrix
*/
#ifndef QT_NO_PIXMAP_TRANSFORMATION
QWMatrix QPixmap::trueMatrix( const QWMatrix &m, int w, int h )
{
    return QImage::trueMatrix(m, w, h);
}
#endif

/*!
    \fn bool QPixmap::isQBitmap() const

    Returns TRUE if this is a QBitmap; otherwise returns FALSE.
*/

/*!
    \fn bool QPixmap::isNull() const

    Returns TRUE if this is a null pixmap; otherwise returns FALSE.

    A null pixmap has zero width, zero height and no contents. You
    cannot draw in a null pixmap or bitBlt() anything to it.

    Resizing an existing pixmap to (0, 0) makes a pixmap into a null
    pixmap.

    \sa resize()
*/

/*!
    \fn int QPixmap::width() const

    Returns the width of the pixmap.

    \sa height(), size(), rect()
*/

/*!
    \fn int QPixmap::height() const

    Returns the height of the pixmap.

    \sa width(), size(), rect()
*/

/*!
    \fn QSize QPixmap::size() const

    Returns the size of the pixmap.

    \sa width(), height(), rect()
*/

/*!
    \fn QRect QPixmap::rect() const

    Returns the enclosing rectangle (0,0,width(),height()) of the pixmap.

    \sa width(), height(), size()
*/

/*!
    \fn int QPixmap::depth() const

    Returns the depth of the pixmap.

    The pixmap depth is also called bits per pixel (bpp) or bit planes
    of a pixmap. A null pixmap has depth 0.

    \sa defaultDepth(), isNull(), QImage::convertDepth()
*/


/*!
    \overload void QPixmap::resize( const QSize &size )

    Resizes the pixmap to size \a size.
*/

/*!
    Resizes the pixmap to \a w width and \a h height. If either \a w
    or \a h is 0, the pixmap becomes a null pixmap.

    If both \a w and \a h are greater than 0, a valid pixmap is
    created. New pixels will be uninitialized (random) if the pixmap
    is expanded.
*/

void QPixmap::resize( int w, int h )
{
    if ( w < 1 || h < 1 ) {			// becomes null
	QPixmap pm( 0, 0, 0, data->bitmap, data->optim );
	*this = pm;
	return;
    }
    int d;
    if ( depth() > 0 )
	d = depth();
    else
	d = isQBitmap() ? 1 : -1;
    // Create new pixmap
    QPixmap pm( w, h, d, data->bitmap, data->optim );
#ifdef Q_WS_X11
    pm.x11SetScreen( x11Screen() );
#endif // Q_WS_X11
    if ( !data->uninit && !isNull() )		// has existing pixmap
	bitBlt( &pm, 0, 0, this, 0, 0,		// copy old pixmap
		qMin(width(), w),
		qMin(height(),h), CopyROP, TRUE );
#if defined(Q_WS_MAC)
    if(data->alphapm) {
	data->alphapm->resize(w, h);
    } else
#elif defined(Q_WS_X11) && !defined(QT_NO_XFTFREETYPE)
    if (data->alphapm)
	qWarning("QPixmap::resize: TODO: resize alpha data");
    else
#endif // Q_WS_X11
	if ( data->mask ) {				// resize mask as well
	    if ( data->selfmask ) {			// preserve self-mask
		pm.setMask( *((QBitmap*)&pm) );
	    } else {				// independent mask
		QBitmap m = *data->mask;
		m.resize( w, h );
		pm.setMask( m );
	    }
	}
    *this = pm;
}


/*!
    \fn const QBitmap *QPixmap::mask() const

    Returns the mask bitmap, or 0 if no mask has been set.

    \sa setMask(), QBitmap, hasAlpha()
*/

/*!
    Sets a mask bitmap.

    The \a newmask bitmap defines the clip mask for this pixmap. Every
    pixel in \a newmask corresponds to a pixel in this pixmap. Pixel
    value 1 means opaque and pixel value 0 means transparent. The mask
    must have the same size as this pixmap.

    \warning Setting the mask on a pixmap will cause any alpha channel
    data to be cleared. For example:
    \code
	QPixmap alpha( "image-with-alpha.png" );
	QPixmap alphacopy = alpha;
	alphacopy.setMask( *alphacopy.mask() );
    \endcode
    Now, alpha and alphacopy are visually different.

    Setting a \link isNull() null\endlink mask resets the mask.

    \sa mask(), createHeuristicMask(), QBitmap
*/

void QPixmap::setMask( const QBitmap &newmask )
{
    const QPixmap *tmp = &newmask;		// dec cxx bug
    if ( (data == tmp->data) ||
	 ( newmask.handle() && newmask.handle() == handle() ) ) {
	QPixmap m = tmp->copy( TRUE );
	setMask( *((QBitmap*)&m) );
	data->selfmask = TRUE;			// mask == pixmap
	return;
    }
    detach();
    data->selfmask = FALSE;
    if ( newmask.isNull() ) {			// reset the mask
	delete data->mask;
	data->mask = 0;
	return;
    }
    if ( newmask.width() != width() || newmask.height() != height() ) {
	qWarning( "QPixmap::setMask: The pixmap and the mask must have "
		  "the same size" );
	return;
    }
#if defined(Q_WS_MAC) || (defined(Q_WS_X11) && !defined(QT_NO_XFTFREETYPE))
    // when setting the mask, we get rid of the alpha channel completely
    delete data->alphapm;
    data->alphapm = 0;
#endif // Q_WS_X11 && !QT_NO_XFTFREETYPE

    delete data->mask;
    QBitmap* newmaskcopy;
    if ( newmask.mask() )
	newmaskcopy = (QBitmap*)new QPixmap( tmp->copy( TRUE ) );
    else
	newmaskcopy = new QBitmap( newmask );
#ifdef Q_WS_X11
    newmaskcopy->x11SetScreen( x11Screen() );
#endif
    data->mask = newmaskcopy;
}


/*!
    \fn bool QPixmap::selfMask() const

    Returns TRUE if the pixmap's mask is identical to the pixmap
    itself; otherwise returns FALSE.

    \sa mask()
*/

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
/*!
    Creates and returns a heuristic mask for this pixmap. It works by
    selecting a color from one of the corners and then chipping away
    pixels of that color, starting at all the edges.

    The mask may not be perfect but it should be reasonable, so you
    can do things such as the following:
    \code
    pm->setMask( pm->createHeuristicMask() );
    \endcode

    This function is slow because it involves transformation to a
    QImage, non-trivial computations and a transformation back to a
    QBitmap.

    If \a clipTight is TRUE the mask is just large enough to cover the
    pixels; otherwise, the mask is larger than the data pixels.

    \sa QImage::createHeuristicMask()
*/

QBitmap QPixmap::createHeuristicMask( bool clipTight ) const
{
    QBitmap m;
    m.convertFromImage( convertToImage().createHeuristicMask(clipTight) );
    return m;
}
#endif
#ifndef QT_NO_IMAGEIO
/*!
    Returns a string that specifies the image format of the file \a
    fileName, or 0 if the file cannot be read or if the format cannot
    be recognized.

    The QImageIO documentation lists the supported image formats.

    \sa load(), save()
*/

const char* QPixmap::imageFormat( const QString &fileName )
{
    return QImageIO::imageFormat(fileName);
}

/*!
    Loads a pixmap from the file \a fileName at runtime. Returns TRUE
    if successful; otherwise returns FALSE.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    See the convertFromImage() documentation for a description of the
    \a conversion_flags argument.

    The QImageIO documentation lists the supported image formats and
    explains how to add extra formats.

    \sa loadFromData(), save(), imageFormat(), QImage::load(),
    QImageIO
*/

bool QPixmap::load( const QString &fileName, const char *format,
		    int conversion_flags )
{
    QImageIO io( fileName, format );
    bool result = io.read();
    if ( result ) {
	detach(); // ###hanord: Why detach here, convertFromImage does it
	result = convertFromImage( io.image(), conversion_flags );
    }
    return result;
}

/*!
    \overload

    Loads a pixmap from the file \a fileName at runtime.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    The \a mode is used to specify the color mode of the pixmap.

    \sa QPixmap::ColorMode
*/

bool QPixmap::load( const QString &fileName, const char *format,
		    ColorMode mode )
{
    int conversion_flags = 0;
    switch (mode) {
      case Color:
	conversion_flags |= ColorOnly;
	break;
      case Mono:
	conversion_flags |= MonoOnly;
	break;
      default:
	break;// Nothing.
    }
    return load( fileName, format, conversion_flags );
}
#endif //QT_NO_IMAGEIO

/*!
    \overload

    Converts \a image and sets this pixmap using color mode \a mode.
    Returns TRUE if successful; otherwise returns FALSE.

    \sa QPixmap::ColorMode
*/

bool QPixmap::convertFromImage( const QImage &image, ColorMode mode )
{
    if ( image.isNull() ) {
	// convert null image to null pixmap
	*this = QPixmap();
	return TRUE;
    }

    int conversion_flags = 0;
    switch (mode) {
      case Color:
	conversion_flags |= ColorOnly;
	break;
      case Mono:
	conversion_flags |= MonoOnly;
	break;
      default:
	break;// Nothing.
    }
    return convertFromImage( image, conversion_flags );
}

#ifndef QT_NO_IMAGEIO
/*!
    Loads a pixmap from the binary data in \a buf (\a len bytes).
    Returns TRUE if successful; otherwise returns FALSE.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    See the convertFromImage() documentation for a description of the
    \a conversion_flags argument.

    The QImageIO documentation lists the supported image formats and
    explains how to add extra formats.

    \sa load(), save(), imageFormat(), QImage::loadFromData(),
    QImageIO
*/

bool QPixmap::loadFromData( const uchar *buf, uint len, const char *format,
			    int conversion_flags )
{
    QByteArray a;
    a.setRawData( (char *)buf, len );
    QBuffer b( a );
    b.open( IO_ReadOnly );
    QImageIO io( &b, format );
    bool result = io.read();
    b.close();
    a.resetRawData( (char *)buf, len );
    if ( result ) {
	detach();
	result = convertFromImage( io.image(), conversion_flags );
    }
    return result;
}

/*!
    \overload

    Loads a pixmap from the binary data in \a buf (\a len bytes) using
    color mode \a mode. Returns TRUE if successful; otherwise returns
    FALSE.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    \sa QPixmap::ColorMode
*/

bool QPixmap::loadFromData( const uchar *buf, uint len, const char *format,
			    ColorMode mode )
{
    int conversion_flags = 0;
    switch (mode) {
      case Color:
	conversion_flags |= ColorOnly;
	break;
      case Mono:
	conversion_flags |= MonoOnly;
	break;
      default:
	break;// Nothing.
    }
    return loadFromData( buf, len, format, conversion_flags );
}

/*!
    \overload
*/

bool QPixmap::loadFromData( const QByteArray &buf, const char *format,
			    int conversion_flags )
{
    return loadFromData( (const uchar *)buf.constData(), buf.size(), format, conversion_flags );
}


/*!
    Saves the pixmap to the file \a fileName using the image file
    format \a format and a quality factor \a quality. \a quality must
    be in the range [0,100] or -1. Specify 0 to obtain small
    compressed files, 100 for large uncompressed files, and -1 to use
    the default settings. Returns TRUE if successful; otherwise
    returns FALSE.

    \sa load(), loadFromData(), imageFormat(), QImage::save(),
    QImageIO
*/

bool QPixmap::save( const QString &fileName, const char *format, int quality ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io( fileName, format );
    return doImageIO( &io, quality );
}

/*!
    \overload

    This function writes a QPixmap to the QIODevice, \a device. This
    can be used, for example, to save a pixmap directly into a
    QByteArray:
    \code
    QPixmap pixmap;
    QByteArray ba;
    QBuffer buffer( ba );
    buffer.open( IO_WriteOnly );
    pixmap.save( &buffer, "PNG" ); // writes pixmap into ba in PNG format
    \endcode
*/

bool QPixmap::save( QIODevice* device, const char* format, int quality ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io( device, format );
    return doImageIO( &io, quality );
}

/*! \internal
*/

bool QPixmap::doImageIO( QImageIO* io, int quality ) const
{
    if ( !io )
	return FALSE;
    io->setImage( convertToImage() );
    if ( quality > 100  || quality < -1 )
	qWarning( "QPixmap::save: quality out of range [-1,100]" );
    if ( quality >= 0 )
	io->setQuality( qMin(quality,100) );
    return io->write();
}

#endif //QT_NO_IMAGEIO

/*!
    \fn int QPixmap::serialNumber() const

    Returns a number that uniquely identifies the contents of this
    QPixmap object. This means that multiple QPixmap objects can have
    the same serial number as long as they refer to the same contents.

    An example of where this is useful is for caching QPixmaps.

    \sa QPixmapCache
*/


/*!
    Returns the default pixmap optimization setting.

    \sa setDefaultOptimization(), setOptimization(), optimization()
*/

QPixmap::Optimization QPixmap::defaultOptimization()
{
    return defOptim;
}

/*!
    Sets the default pixmap optimization.

    All \e new pixmaps that are created will use this default
    optimization. You may also set optimization for individual pixmaps
    using the setOptimization() function.

    The initial default \a optimization setting is \c QPixmap::Normal.

    \sa defaultOptimization(), setOptimization(), optimization()
*/

void QPixmap::setDefaultOptimization( Optimization optimization )
{
    if ( optimization != DefaultOptim )
	defOptim = optimization;
}

/*
  fills \a buf with \a r in \a widget. Then blits \a buf on \a res at
  position \a offset
 */
static void grabWidget_helper(QWidget *widget, QPixmap &res, QPixmap &buf,
			      const QRect &r, const QPoint &offset)
{
    buf.fill(widget, r.topLeft());
    QPainter::setRedirected(widget, &buf, r.topLeft());
    QPaintEvent e(r & widget->rect());
    QApplication::sendEvent(widget, &e);
    QPainter::restoreRedirected(widget);
    ::bitBlt(&res, offset, &buf, QRect(QPoint(), r.size()));

    const QObjectList children = widget->children();
    for (int i = 0; i < children.size(); ++i) {
	QWidget *child = static_cast<QWidget*>(children.at(i));
	if (!child->isWidgetType() || child->isTopLevel()
	    || child->isHidden() || !child->geometry().intersects(r))
	    continue;
	QRect cr = r & child->geometry();
	cr.moveBy(-child->pos());
	grabWidget_helper(child, res, buf, cr, offset + child->pos());
    }
}

/*!
    Creates a pixmap and paints \a widget in it.

    If the \a widget has any children, then they are also painted in
    the appropriate positions.

    If you specify \a x, \a y, \a w or \a h, only the rectangle you
    specify is painted. The defaults are 0, 0 (top-left corner) and
    -1,-1 (which means the entire widget).

    (If \a w is negative, the function copies everything to the right
    border of the window. If \a h is negative, the function copies
    everything to the bottom of the window.)

    If \a widget is 0, or if the rectangle defined by \a x, \a y, the
    modified \a w and the modified \a h does not overlap the \a
    {widget}->rect(), this function will return a null QPixmap.

    This function actually asks \a widget to paint itself (and its
    children to paint themselves). QPixmap::grabWindow() grabs pixels
    off the screen, which is a bit faster and picks up \e exactly
    what's on-screen. This function works by calling paintEvent() with
    painter redirection turned on. If there are overlaying windows,
    grabWindow() will see them, but not this function.

    If there is overlap, it returns a pixmap of the size you want,
    containing a rendering of \a widget. If the rectangle you ask for
    is a superset of \a widget, the areas outside \a widget are
    covered with the widget's background.

    If an error occurs when trying to grab the widget, such as the
    size of the widget being too large to fit in memory, an isNull()
    pixmap is returned.

    \sa grabWindow() QWidget::paintEvent()
*/

QPixmap QPixmap::grabWidget( QWidget * widget, int x, int y, int w, int h )
{
    QPixmap res, buf;

    if ( !widget )
	return res;

    if ( w < 0 )
	w = widget->width() - x;
    if ( h < 0 )
	h = widget->height() - y;

    QRect r( x, y, w, h );
    if ( !r.intersects( widget->rect() ) )
	return res;

    res.resize(r.size());
    buf.resize(r.size());
    if(!res || !buf)
	return res;

    grabWidget_helper(widget, res, buf, r, QPoint());
    return res;
}







/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
/*!
    \relates QPixmap

    Writes the pixmap \a pixmap to the stream \a s as a PNG image.

    Note that writing the stream to a file will not produce a valid image file.

    \sa QPixmap::save()
    \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QPixmap &pixmap )
{
    s << pixmap.convertToImage();
    return s;
}

/*!
    \relates QPixmap

    Reads a pixmap from the stream \a s into the pixmap \a pixmap.

    \sa QPixmap::load()
    \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QPixmap &pixmap )
{
    QImage img;
    s >> img;
    pixmap.convertFromImage( img );
    return s;
}

#endif //QT_NO_DATASTREAM




