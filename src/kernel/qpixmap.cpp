/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#116 $
**
** Implementation of QPixmap class
**
** Created : 950301
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qpixmap.h"

#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qobjectlist.h"
#include "qapplication.h"

/*!
  \class QPixmap qpixmap.h
  \brief The QPixmap class is an off-screen pixel-based paint device.

  \ingroup drawing
  \ingroup shared

  It is one of the two classes Qt provides for dealing with images,
  the other being QImage.  QPixmap is designed and optimized for
  drawing; QImage is designed and optimized for I/O and for direct
  pixel access/manipulation.  There are (slow) functions to convert
  between QImage and QPixmap; convertToImage() and convertFromImage().

  One common use of the QPixmap class is to enable smooth updating of
  widgets.  Whenever something complex needs to be drawn, you can use
  a pixmap to obtain flicker-free drawing, like this:

  <ol plain>
  <li> Create a pixmap with the same size as the widget.
  <li> Fill the pixmap with the widget background color.
  <li> Paint the pixmap.
  <li> bitBlt() the pixmap contents onto the widget.
  </ol>

  Pixel data in a pixmap is internal and managed by the underlying
  window system.  Pixels can only be accessed through QPainter
  functions, through bitBlt(), and by converting the QPixmap to a
  QImage.

  You can display a QPixmap on the screen easily using
  e.g. QLabel::setPixmap(), and all the \link QButton button classes
  \endlink support pixmap use.

  There are also convenience functions to get and set single pixels
  and to load and save the entire pixmap; these work by converting the
  pixmap to a QImage internally.

  The QPixmap class is optimized by the use of \link shclass.html implicit
  sharing\endlink, so it is very efficient to pass QPixmap objects as
  arguments.

  <strong>Note about Windows 95 and 98:</strong> Because of internal
  limitations in the operating system, Windows 9x easily crashes if you
  create around 1000 pixmaps or more.  Qt will work around this problem
  if you set the \link setOptimization() pixmap optimization\endlink
  to \c QPixmap::MemoryOptim.  Read more about this in the
  setOptimization() documentation.

  \sa QBitmap, QImage, QImageIO, \link shclass.html Shared Classes\endlink
*/


QPixmap::Optimization QPixmap::defOptim = QPixmap::NormalOptim;


/*!
  \internal
  Private constructor which takes the bitmap flag and the optimization.
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
  Constructs a pixmap with \e w width, \e h height and of \e depth bits per
  pixels.

  The contents of the pixmap is uninitialized.

  The \e depth can be either 1 (monochrome) or the depth of the
  current video mode.  If \e depth is negative, then the hardware
  depth of the current video mode will be used.

  If either \e width or \e height is zero, a null pixmap is constructed.

  \sa isNull()
*/

QPixmap::QPixmap( int w, int h, int depth, Optimization optimization )
    : QPaintDevice( QInternal::Pixmap )
{
    init( w, h, depth, FALSE, optimization );
}

/*!
  \overload QPixmap::QPixmap( const QSize &size, int depth, Optimization optimization )
*/

QPixmap::QPixmap( const QSize &size, int depth, Optimization optimization )
    : QPaintDevice( QInternal::Pixmap )
{
    init( size.width(), size.height(), depth, FALSE, optimization );
}


/*!
  Constructs a pixmap from the file \e fileName. If the file does not
  exist, or is of an unknown format, the pixmap becomes a null pixmap.

  The parameters are passed on to load().

  \sa isNull(), load(), loadFromData(), save(), imageFormat()
*/

QPixmap::QPixmap( const QString& fileName, const char *format,
	int conversion_flags )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    load( fileName, format, conversion_flags );
}

/*!
  Constructs a pixmap from the file \e fileName. If the file does not
  exist, or is of an unknown format, the pixmap becomes a null pixmap.

  The parameters are passed on to load().

  \sa isNull(), load(), loadFromData(), save(), imageFormat()
*/

QPixmap::QPixmap( const QString& fileName, const char *format, ColorMode mode )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    load( fileName, format, mode );
}

/*!
  Constructs a pixmap from \a xpm, which must be a valid XPM image.
*/

QPixmap::QPixmap( const char *xpm[] )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    QImage image( xpm );
    if ( !image.isNull() )
	convertFromImage( image );
}

/*!
  Constructs a pixmaps by loading from \a img_data.
  The data can be in any image format supported by Qt.

  \sa loadFromData()
*/

QPixmap::QPixmap( const QByteArray & img_data )
    : QPaintDevice( QInternal::Pixmap )
{
    init( 0, 0, 0, FALSE, defOptim );
    loadFromData( img_data );
}


/*!
  Constructs a pixmap which is a copy of \e pixmap.
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
#if defined(_WS_WIN_)
	hdc = pixmap.hdc;			// copy Windows device context
#elif defined(_WS_X11_)
	hd = pixmap.hd;				// copy X11 drawable
	copyX11Data( &pixmap );			// copy x11Data
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


/*!
  Returns a
  \link shclass.html deep copy\endlink of the pixmap using the bitBlt()
  function to copy the pixels.
  \sa operator=()
*/

QPixmap QPixmap::copy( bool ignoreMask ) const
{
    QPixmap pm( data->w, data->h, data->d, data->bitmap, data->optim );
    if ( !pm.isNull() ) {			// copy the bitmap
#if defined(_WS_X11_)
	pm.copyX11Data( this );
#endif
	bitBlt( &pm, 0,0, this, 0,0, data->w, data->h, CopyROP, TRUE );
	if ( !ignoreMask && data->mask )		// copy the mask
	    pm.setMask( data->selfmask ? *((QBitmap*)&pm) : *data->mask );
    }
    return pm;
}


/*!
  Assigns the pixmap \e pixmap to this pixmap and returns a reference to
  this pixmap.
*/

QPixmap &QPixmap::operator=( const QPixmap &pixmap )
{
    if ( paintingActive() ) {
#if defined(CHECK_STATE)
	qWarning("QPixmap::operator=: Cannot assign to pixmap during painting");
#endif
	return *this;
    }
    pixmap.data->ref();				// avoid 'x = x'
    deref();
    if ( pixmap.paintingActive() ) {		// make a deep copy
	init( pixmap.width(), pixmap.height(), pixmap.depth(),
	      pixmap.data->bitmap, pixmap.data->optim );
	data->uninit = FALSE;
	if ( !isNull() ) {
	    bitBlt( this, 0, 0, &pixmap, pixmap.width(), pixmap.height(),
		    CopyROP, TRUE );
	    if ( pixmap.mask() )
		setMask( pixmap.data->selfmask ? *((QBitmap*)(this))
					       : *pixmap.mask() );
	}
	pixmap.data->deref();
    } else {
	data = pixmap.data;
	devFlags = pixmap.devFlags;		// copy QPaintDevice flags
#if defined(_WS_WIN_)
	hdc = pixmap.hdc;
#elif defined(_WS_X11_)
	hd = pixmap.hd;				// copy QPaintDevice drawable
	copyX11Data( &pixmap );			// copy x11Data
#endif
    }
    return *this;
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
	    p.setBrush( QBrush( Qt::black,*widget->backgroundPixmap() ) );
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
    if ( !data->uninit && !isNull() )		// has existing pixmap
	bitBlt( &pm, 0, 0, this, 0, 0,		// copy old pixmap
		QMIN(width(), w),
		QMIN(height(),h), CopyROP, TRUE );
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

void QPixmap::setMask( const QBitmap &newmask )
{
    if ( (data == newmask.data) ||
	 ( newmask.handle() && newmask.handle() == handle() ) ) {
	const QPixmap *tmp = &newmask;		// dec cxx bug
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
#if defined(CHECK_RANGE)
	qWarning( "QPixmap::setMask: The pixmap and the mask must have "
		 "the same size" );
#endif
	return;
    }
    delete data->mask;
    QBitmap* newmaskcopy;
    if ( newmask.mask() ) {
	const QPixmap *tmp = &newmask;
	newmaskcopy = (QBitmap*)new QPixmap( tmp->copy( TRUE ) );
    }
    else {
	newmaskcopy = new QBitmap( newmask );
    }
    data->mask = newmaskcopy;
}


/*!
  \fn bool QPixmap::selfMask() const
  Returns TRUE if the pixmap's mask is identical to the pixmap itself.
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
  \endcode

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

const char* QPixmap::imageFormat( const QString &fileName )
{
    return QImageIO::imageFormat(fileName);
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
	;// Nothing.
    }
    return load( fileName, format, conversion_flags );
}


/*!
  \overload
*/

bool QPixmap::convertFromImage( const QImage &image, ColorMode mode )
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
    return convertFromImage( image, conversion_flags );
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
  \overload
*/

bool QPixmap::loadFromData( const QByteArray &buf, const char *format,
			    int conversion_flags )
{
    return loadFromData( (const uchar *)(buf.data()), buf.size(),
			 format, conversion_flags );
}


/*!
  Saves the pixmap to the file \e fileName, using the image file format
  \e format.  Returns TRUE if successful, or FALSE if the pixmap could not
  be saved.
  \sa load(), loadFromData(), imageFormat(), QImage::save(), QImageIO
*/

bool QPixmap::save( const QString &fileName, const char *format ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io( fileName, format );
    io.setImage( convertToImage() );
    return io.write();
}


/*!
  \fn int QPixmap::serialNumber() const

  Returns a number that uniquely identifies the contents of this QPixmap object.
  This means that multiple QPixmaps objects can have the same serial number
  as long as they refere to the same contents.
  The serial number is for example very useful for caching.

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

  All \e new pixmaps that are created will use this default optimization.
  You may also set optimization for individual pixmaps using the
  setOptimization() function.

  The initial default optimization setting is \c QPixmap::Normal.

  \sa defaultOptimization(), setOptimization(), optimization()
*/

void QPixmap::setDefaultOptimization( Optimization optimization )
{
    if ( optimization != DefaultOptim )
	defOptim = optimization;
}


// helper for next function
static QPixmap grabChildWidgets( QWidget * w )
{
    QPixmap res( w->width(), w->height() );
    res.fill( w, QPoint( 0, 0 ) );
    QPainter::redirect( w, &res ); // ### overwrites earlier redirect
    QPaintEvent e( w->rect(), FALSE );
    QApplication::sendEvent( w, &e );
    QPainter::redirect( w, 0 );
    debug( "arnt was here %s/%s", w->className(), w->name() );

    const QObjectList * children = w->children();
    if ( children ) {
	QPainter p( &res );
	QObjectListIt it( *children );
	QObject * child;
	while( (child=it.current()) != 0 ) {
	    ++it;
	    if ( child->isWidgetType() && !((QWidget *)child)->testWState( Qt::WState_ForceHide ) &&
		 ((QWidget *)child)->geometry().intersects( w->rect() ) ) {
		// those conditions aren't quite right, it's possible
		// to have a grandchild completely outside its
		// grandparent, but partially inside its parent.  no
		// point in optimizing for that.
		
		// make sure to evaluate pos() first - who knows what
		// the paint event(s) inside grabChildWidgets() will do.
		QPoint childpos = ((QWidget *)child)->pos();
		p.drawPixmap( childpos, grabChildWidgets( (QWidget *)child ) );
	    }
	}
    }
    return res;
}


/*!  Creates a pixmap and paints \a widget in it.

  If \a widget has children, they are painted too, appropriately located.

  If you specify \a x, \a y, \a w or \a h, only the rectangle you
  specify is painted.  The defaults are 0, 0 (top-left corner) and
  -1,-1 (which means the entire widget).

  (If \e w is negative, the function copies everything to the right
  border of the window.  If \e h is negative, the function copies
  everything to the bottom of the window.)

  If \a widget is 0, or if the rectangle defined by \a x, \a y, the
  modified \a w and the modified \a h does not overlap the \a
  widget->rect(), this function returns a null QPixmap.

  This function actually asks \a widget to paint itself (or
  themselves).  QPixmap::grabWindow() grabs pixels off the screen,
  which is a bit faster.  This function works by calling paintEvent()
  with painter redirection turned on.

  If there is overlap, it returns a pixmap of the size you want,
  containing a rendering of \a widget.  If the rectangle you ask for
  is a superset of \a widget, the area outside \a widget are covered
  with the widget's background.

  \sa grabWindow() QPainter::redirect() QWidget::paintEvent()
*/

QPixmap QPixmap::grabWidget( QWidget * widget, int x, int y, int w, int h )
{
    QPixmap res;
    if ( !widget )
	return res;

    if ( w < 0 )
	w = widget->width() - x;
    if ( h < 0 )
	h = widget->height() - y;

    QRect wr( x, y, w, h );
    if ( wr == widget->rect() )
	return grabChildWidgets( widget );
    if ( !wr.intersects( widget->rect() ) )
	return res;

    res.resize( w, h );
    res.fill( widget, QPoint( w,h ) );
    QPixmap tmp( grabChildWidgets( widget ) );
    ::bitBlt( &res, 0, 0, &tmp, x, y, w, h );
    return res;
}





/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/

/*!
  \relates QPixmap
  Writes a pixmap to the stream as a PNG image.
  \sa QPixmap::save()
*/

QDataStream &operator<<( QDataStream &s, const QPixmap &pixmap )
{
    QImageIO io;
    io.setIODevice( s.device() );
    if ( s.version() == 1 )
	io.setFormat( "BMP" );
    else
	io.setFormat( "PNG" );

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

