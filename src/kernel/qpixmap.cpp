/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#77 $
**
** Implementation of QPixmap class
**
** Created : 950301
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qdstream.h"
#include "qbuffer.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpixmap.cpp#77 $");


/*!
  \class QPixmap qpixmap.h
  \brief The QPixmap class is an off-screen buffer paint device.

  \ingroup drawing
  \ingroup shared

  A common use of the QPixmap class is to enable smooth updating of
  widgets.  Whenever something complex needs to be drawn, you can use
  a pixmap to obtain flicker-free drawing.

  <ol plain>
  <li> Create a pixmap with the same size as the widget.
  <li> Fill the pixmap with the widget background color.
  <li> Paint the pixmap.
  <li> bitBlt() the pixmap contents onto the widget.
  </ol>

  Example of flicker-free update:
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPixmap	 pm( size() );			// create pixmap
	QPainter p;				// our painter
	pm.fill( backgroundColor() );		// initialize pixmap
	p.begin( &pm );				// start painting pixmap
	...					// draw something
	p.end();				// painting done
	bitBlt( this, 0,0, &pm );		// copy pixmap to widget
    }
  \endcode

  The bitBlt() function has quite a few arguments that are not used in
  this example.

  Pixel data in a pixmap is internal and managed by the underlying window
  system.  Pixels can only be accessed through QImage, QPainter functions
  and the bitBlt().

  You can \link load() load\endlink and \link save() save\endlink
  pixmaps using several image formats.	You can display a QPixmap on
  the screen easily using QLabel::setPixmap(), and all the \link
  QButton button classes \endlink support pixmap use.

  A pixmap can be converted to a QImage to get direct access to the pixels.
  A QImage can also be converted back to a pixmap.

  The QPixmap class is optimized by the use of \link shclass.html implicit
  sharing\endlink, so it is very efficient to pass QPixmap objects as
  arguments.

  \sa QBitmap, QImage, QImageIO, \link shclass.html Shared Classes\endlink
*/


/*!
  Constructs a null pixmap.
  \sa isNull()
*/

QPixmap::QPixmap()
    : QPaintDevice( PDT_PIXMAP )
{
    init( 0, 0, 0 );
}

/*!
  Constructs a pixmap with \e w width, \e h height and of \e depth bits per
  pixels.

  The contents of the pixmap is uninitialized.

  The \e depth can be either 1 (monochrome) or the depth of the
  current video mode.  If \e depth is negative, then the hardware
  depth of the current video mode will be used.

  If either \e width or \e height is zero, a null pixmap is constructed.

  \sa isNull()
*/

QPixmap::QPixmap( int w, int h, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    init( w, h, depth );
}

/*!
  \overload QPixmap::QPixmap( const QSize &size, int depth )
*/

QPixmap::QPixmap( const QSize &size, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    init( size.width(), size.height(), depth );
}

/*!
  Constructs a pixmap from the file \e fileName. If the file does not
  exist, or is of an unknown format, the pixmap becomes a null pixmap.

  The parameters are passed on to load().

  \sa isNull(), load(), loadFromData(), save(), imageFormat()
*/

QPixmap::QPixmap( const char *fileName, const char *format,
	int conversion_flags )
    : QPaintDevice( PDT_PIXMAP )
{
    init( 0, 0, 0 );
    load( fileName, format, conversion_flags );
}

/*!
  Constructs a pixmap from the file \e fileName. If the file does not
  exist, or is of an unknown format, the pixmap becomes a null pixmap.

  The parameters are passed on to load().

  \sa isNull(), load(), loadFromData(), save(), imageFormat()
*/

QPixmap::QPixmap( const char *fileName, const char *format, ColorMode mode )
    : QPaintDevice( PDT_PIXMAP )
{
    init( 0, 0, 0 );
    load( fileName, format, mode );
}

/*!
  Constructs a pixmap from \a xpm, which must be a valid XPM image.
*/

QPixmap::QPixmap( const char *xpm[] )
    : QPaintDevice( PDT_PIXMAP )
{
    init( 0, 0, 0 );
    QImage image( xpm );
    if ( !image.isNull() )
	convertFromImage( image );
}


/*!
  Special-purpose function that detaches the pixmap from shared pixmap data.

  A pixmap is automatically detached by Qt whenever its contents is about
  to change.  This is done in all QPixmap member functions that modify the
  pixmap (fill(), resize(), convertFromImage(), load() etc.), in bitBlt()
  for the destination pixmap and in QPainter::begin() on a pixmap.

  It is possible to modify a pixmap without letting Qt know.
  You can first obtain the \link handle() system-dependent handle\endlink
  and then call system-specific functions (for instance BitBlt under Windows)
  that modifies the pixmap contents.  In this case, you can call detach()
  to cut the pixmap loose from other pixmaps that share data with this one.

  detach() returns immediately if there is just a single reference or if
  the pixmap has not been initialized yet.
*/

void QPixmap::detach()
{
    if ( data->optim )
	data->dirty = TRUE;
    if ( data->uninit || data->count == 1 ) {
	data->uninit = FALSE;
	return;
    }
    *this = copy();
}

/*!
  Returns a 
  \link shclass.html deep copy\endlink of the pixmap using the bitBlt()
  function to copy the pixels.
  \sa operator=()
*/

QPixmap QPixmap::copy() const
{
    QPixmap tmp( data->w, data->h, data->d );
    tmp.data->optim  = data->optim;		// copy optim flag
    tmp.data->bitmap = data->bitmap;		// copy bitmap flag
    if ( !tmp.isNull() ) {			// copy the bitmap
	bitBlt( &tmp, 0,0, this, 0,0, data->w, data->h, CopyROP, TRUE );
	if ( data->mask )			// copy the mask
	    tmp.setMask( *data->mask );
    }
    return tmp;
}


/*!
  Converts the image \e image to a pixmap that is assigned to this pixmap.
  Returns a reference to the pixmap.
  \sa convertFromImage().
*/

QPixmap &QPixmap::operator=( const QImage &image )
{
    convertFromImage( image );
    return *this;
}


/*!
  \fn bool QPixmap::isQBitmap() const
  Returns TRUE if this is a QBitmap, otherwise FALSE.
*/

/*!
  \fn bool QPixmap::isNull() const
  Returns TRUE if it is a null pixmap.

  A null pixmap has zero width, zero height and no contents.
  You cannot draw in a null pixmap or bitBlt() anything to it.

  Resizing an existing pixmap to (0,0) makes a pixmap into a null
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
  Returns the depth of the image.

  The pixmap depth is also called bits per pixel (bpp) or bit planes
  of a pixmap.	A null pixmap has depth 0.

  \sa defaultDepth(), isNull(), QImage::convertDepth()
*/


/*!
  \fn void QPixmap::fill( const QWidget *widget, const QPoint &ofs )
  Fills the pixmap with the widget's background color or pixmap.
  If the background is empty, nothing is done.

  The \e ofs point is an offset in the widget.

  The point \a ofs is a point in the widget's coordinate system. The
  pixmap's top left pixel will be mapped to the point \a ofs in the
  widget. This is significant if the widget has a background pixmap,
  otherwise the pixmap will simply be filled with the background color of
  the widget.

  Example:
  \code
  void CuteWidget::paintEvent( QPaintEvent *e )
  {
    QRect ur = e->rect();		// rectangle to update

    QPixmap  pix( ur.size() );	       	// Pixmap for double-buffering

    pix.fill( this, ur.topLeft() );	// fill with widget background

    QPainter p( &pix );
    p.translate( -ur.x(), -ur.y() );	// use widget coordinate system
                                        // when drawing on pixmap
    //    ... draw on pixmap ...

    p.end();

    bitBlt( this, ur.topLeft(), &pix );
  }
  \endcode
*/

/*!
  \overload void QPixmap::fill( const QWidget *widget, int xofs, int yofs )
*/

void QPixmap::fill( const QWidget *widget, int xofs, int yofs )
{
    const QPixmap* bgpm = widget->backgroundPixmap();
    if ( bgpm ) {
	if ( !bgpm->isNull() ) {
	    QPainter p;
	    p.begin( this );
	    p.setPen( NoPen );
	    p.setBrush( QBrush( black,*widget->backgroundPixmap() ) );
	    p.setBrushOrigin( -xofs, -yofs );
	    p.drawRect( 0, 0, width(), height() );
	    p.end();
	}
    } else {
	fill( widget->backgroundColor() );
    }
}


/*!
  \overload void QPixmap::resize( const QSize &size )
*/

/*!
  Resizes the pixmap to \e w width and \e h height.  If either \e w
  or \e h is less than 1, the pixmap becomes a null pixmap.

  If both \e w and \e h are greater than 0, a valid pixmap is created.
  New pixels will be uninitialized (random) if the pixmap is expanded.
*/

void QPixmap::resize( int w, int h )
{
    if ( w < 1 || h < 1 ) {			// becomes null
	QPixmap pm;
	pm.data->optim	= data->optim;		// keep optimization flag
	pm.data->bitmap = data->bitmap;		// keep is-a flag
	*this = pm;
	return;
    }

    int d;
    if ( depth() > 0 )
	d = depth();
    else
	d = isQBitmap() ? 1 : -1;
    QPixmap pm( w, h, d );			// create new pixmap
    if ( !data->uninit && !isNull() )		// has existing pixmap
	bitBlt( &pm, 0, 0, this, 0, 0,		// copy old pixmap
		QMIN(width(), w),
		QMIN(height(),h), CopyROP, TRUE );
    pm.data->optim  = data->optim;		// keep optim flag
    pm.data->bitmap = data->bitmap;		// keep bitmap flag
    if ( data->mask ) {				// resize mask as well
	QBitmap m = *data->mask;
	m.resize( w, h );
	pm.setMask( m );
    }
    *this = pm;
}


/*!
  \fn const QBitmap *QPixmap::mask() const
  Returns the mask bitmap, or null if no mask has been set.

  \sa setMask(), QBitmap
*/

/*!
  Sets a mask bitmap.

  The \e mask bitmap defines the clip mask for this pixmap. Every pixel in
  \e mask corresponds to a pixel in this pixmap. Pixel value 1 means opaque
  and pixel value 0 means transparent. The mask must have the same size as
  this pixmap.

  Setting a \link isNull() null\endlink mask resets the mask,

  \sa mask(), createHeuristicMask(), QBitmap
*/

void QPixmap::setMask( const QBitmap &mask )
{
    if ( mask.handle() && mask.handle() == handle() ) {
	const QPixmap *tmp = &mask;		// dec cxx bug
	QPixmap m = tmp->copy();
	setMask( *((QBitmap*)&m) );
	data->selfmask = TRUE;			// mask == pixmap
	return;
    }
    detach();
    data->selfmask = FALSE;
    if ( mask.isNull() ) {			// reset the mask
	delete data->mask;
	data->mask = 0;
	return;
    }
    if ( mask.width() != width() || mask.height() != height() ) {
#if defined(CHECK_RANGE)
	warning( "QPixmap::setMask: The pixmap and the mask must have "
		 "the same size" );
#endif
	return;
    }
    delete data->mask;
    data->mask = new QBitmap( mask );
}


/*!
  \fn bool QPixmap::selfMask() const
  Returns TRUE if the pixmap's mask is identical to the pixmap
  itself.
  \sa mask()
*/


/*!
  Creates and returns a heuristic mask for this pixmap. It works by
  selecting a color from one of the corners, then chipping away pixels of
  that color, starting at all the edges.

  The mask may not be perfect but should be reasonable, so you can do
  things like:
  \code
    pm->setMask( pm->createHeuristicMask() );
  \endcode;

  This function is slow because it involves transformation to a QImage,
  non-trivial computations and a transformation back to QBitmap.

  \sa QImage::createHeuristicMask()
*/

QBitmap QPixmap::createHeuristicMask( bool clipTight ) const
{
    QBitmap m;
    m.convertFromImage( convertToImage().createHeuristicMask(clipTight) );
    return m;
}


/*!
  Returns a string that specifies the image format of the file \e fileName,
  or null if the file cannot be read or if the format cannot be recognized.

  The QImageIO documentation lists the supported image formats.

  \sa load(), save()
*/

const char *QPixmap::imageFormat( const char *fileName )
{
    return QImageIO::imageFormat(fileName);
}


static bool can_handle_bmp = FALSE;
static bool did_handle_bmp = FALSE;

bool qt_image_native_bmp()
{
    if ( can_handle_bmp ) {
	did_handle_bmp = TRUE;
	return TRUE;
    }
    return FALSE;
}

bool qt_image_did_native_bmp()
{
    return did_handle_bmp;
}

/*!
  Loads a pixmap from the file \e fileName.
  Returns TRUE if successful, or FALSE if the pixmap could not be loaded.

  If \e format is specified, the loader attempts to read the pixmap using the
  specified format. If \e format is not specified (default),
  the loader reads a few bytes from the header to guess the file format.

  See the convertFromImage() documentation for a description
  of the \e conversion_flags argument.

  The QImageIO documentation lists the supported image formats and
  explains how to add extra formats.

  \sa loadFromData(), save(), imageFormat(), QImage::load(), QImageIO
*/

bool QPixmap::load( const char *fileName, const char *format,
		    int conversion_flags )
{
    QImageIO io( fileName, format );
#if defined(_WS_WIN_)
    can_handle_bmp = TRUE;
#endif
    bool result = io.read();
    if ( result ) {
	detach();
	result = convertFromImage( io.image(), conversion_flags );
    }
#if defined(_WS_WIN_)
    can_handle_bmp = did_handle_bmp = FALSE;
#endif
    return result;
}

/*!
  \overload
*/
bool QPixmap::load( const char *fileName, const char *format,
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
	;// Nothing.
    }
    return load( fileName, format, conversion_flags );
}

/*!
  \overload
*/
bool QPixmap::convertFromImage( const QImage &img, ColorMode mode )
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
	;// Nothing.
    }
    return convertFromImage( img, conversion_flags );
}


/*!
  Loads a pixmap from the binary data in \e buf (\e len bytes).
  Returns TRUE if successful, or FALSE if the pixmap could not be loaded.

  If \e format is specified, the loader attempts to read the pixmap using the
  specified format. If \e format is not specified (default),
  the loader reads a few bytes from the header to guess the file format.

  See the convertFromImage() documentation for a description
  of the \a conversion_flags argument.

  The QImageIO documentation lists the supported image formats and
  explains how to add extra formats.

  \sa load(), save(), imageFormat(), QImage::loadFromData(), QImageIO
*/

bool QPixmap::loadFromData( const uchar *buf, uint len, const char *format,
			    int conversion_flags )
{
    QByteArray a;
    a.setRawData( (char *)buf, len );
    QBuffer b( a );
    b.open( IO_ReadOnly );
    QImageIO io( &b, format );
#if defined(_WS_WIN_)
    can_handle_bmp = TRUE;
#endif
    bool result = io.read();
    b.close();
    a.resetRawData( (char *)buf, len );
    if ( result ) {
	detach();
	result = convertFromImage( io.image(), conversion_flags );
    }
#if defined(_WS_WIN_)
    can_handle_bmp = did_handle_bmp = FALSE;
#endif
    return result;
}

/*!
  \overload
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
	;// Nothing.
    }
    return loadFromData( buf, len, format, conversion_flags );
}

/*!
  Saves the pixmap to the file \e fileName, using the image file format
  \e format.  Returns TRUE if successful, or FALSE if the pixmap could not
  be saved.
  \sa load(), loadFromData(), imageFormat(), QImage::save(), QImageIO
*/

bool QPixmap::save( const char *fileName, const char *format ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io( fileName, format );
    io.setImage( convertToImage() );
    return io.write();
}


/*!
  \fn int QPixmap::serialNumber() const

  Returns a number that uniquely identifies this QPixmap object. The
  serial number is very useful for caching.

  \sa QPixmapCache
*/


/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/

/*!
  \relates QPixmap
  Writes a pixmap to the stream as a BMP image.
  \sa QPixmap::save()
*/

QDataStream &operator<<( QDataStream &s, const QPixmap &pixmap )
{
    QImageIO io( s.device(), "BMP" );
    io.setImage( pixmap.convertToImage() );
    io.write();
    return s;
}

/*!
  \relates QPixmap
  Reads a pixmap from the stream.
  \sa QPixmap::load()
*/

QDataStream &operator>>( QDataStream &s, QPixmap &pixmap )
{
    QImageIO io( s.device(), 0 );
    if ( io.read() )
	pixmap.convertFromImage( io.image() );
    return s;
}
