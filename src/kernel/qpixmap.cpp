/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#32 $
**
** Implementation of QPixmap class
**
** Author  : Haavard Nord
** Created : 950301
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include "qimage.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#32 $";
#endif


/*----------------------------------------------------------------------------
  \class QPixmap qpixmap.h
  \brief The QPixmap class is an off-screen buffer paint device.

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
  pixmaps using several image formats.  You can display a QPixmap on
  the screen easily using QLabel::setPixmap(), and all the \link
  QButton button classes \endlink support pixmap use.

  A pixmap can be converted to a QImage to get direct access to the pixels.
  A QImage can also be converted back to a pixmap.

  The QPixmap class is optimized by the use of implicit sharing, so it
  is quite practical to pass QPixmap objects as arguments.

  \sa QBitmap, QImage, QImageIO, QPaintDevice, QLabel, QButton */


/*----------------------------------------------------------------------------
  Detaches the pixmap from shared data.
 ----------------------------------------------------------------------------*/

void QPixmap::detach()				// detach shared pixmap
{
    if ( data->optim )				// detach is called before
	data->dirty = TRUE;			//   pixmap is changed
    if ( data->uninit || data->count == 1 ) {
	data->uninit = FALSE;
	return;
    }
    *this = copy();
}

/*----------------------------------------------------------------------------
  Returns a deep copy of the pixmap using the bitBlt() function to copy
  the pixels.
  \sa operator=()
 ----------------------------------------------------------------------------*/

QPixmap QPixmap::copy() const
{
    QPixmap tmp( data->w, data->h, data->d );
    if ( !tmp.isNull() )
	bitBlt( &tmp, 0,0, this, 0,0, data->w, data->h );
    tmp.data->optim  = data->optim;		// copy optim flag
    tmp.data->bitmap = data->bitmap;		// copy bitmap flag
    return tmp;
}


/*----------------------------------------------------------------------------
  Converts the image \e image to a pixmap that is assigned to this pixmap.
  Returns a reference to the pixmap.
  \sa convertFromImage().
 ----------------------------------------------------------------------------*/

QPixmap &QPixmap::operator=( const QImage &image )
{
    convertFromImage( image );
    return *this;
}


/*----------------------------------------------------------------------------
  \fn bool QPixmap::isNull() const
  Returns TRUE if it is a null pixmap.

  A null pixmap has zero width, zero height and no contents.
  You cannot draw in a null pixmap or bitBlt() anything to it.

  Resizing an existing pixmap to (0,0) makes a pixmap into a null
  pixmap.

  \sa resize() */

/*----------------------------------------------------------------------------
  \fn int QPixmap::width() const
  Returns the width of the pixmap.
  \sa height(), size(), rect()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QPixmap::height() const
  Returns the height of the pixmap.
  \sa width(), size(), rect()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QSize QPixmap::size() const
  Returns the size of the pixmap.  A null pixmap has size (0,0).
  \sa width(), height(), rect(), isNull().
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QRect QPixmap::rect() const
  Returns the enclosing rectangle of the pixmap, or (0,0,0,0) for a
  null pixmap.
  \sa width(), height(), size(), isNull()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QPixmap::depth() const

  Returns the depth of the image.

  The pixmap depth is also called bits per pixel (bpp) or bit planes
  of a pixmap.  A null pixmap has depth 0.

  \sa numColors() isNull() QImage::convertDepth() convertToImage()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QPixmap::numColors() const
  Returns the maximum number of colors that can be used for the pixmap.
  Equivalent to 2^depth().
  \sa depth()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \overload void QPixmap::resize( const QSize &size )
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Resizes the pixmap to \e w width and \e h height.  If either \e w
  or \e h is less than 1, the pixmap becomes a null pixmap.

  If both \e w and \e h are greater than 0, a valid pixmap will be
  created.  New pixels will be uninitialized (random) if the pixmap is
  expanded.
 ----------------------------------------------------------------------------*/

void QPixmap::resize( int w, int h )
{
    if ( w<1 || h<1 ){				// will become null?
	QPixmap pm;
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
		QMIN(height(),h) );
    pm.data->optim  = data->optim;		// keep optim flag
    pm.data->bitmap = data->bitmap;		// keep bitmap flag
    *this = pm;
}


/*----------------------------------------------------------------------------
  Returns a string that specifies the image format of the file \e fileName,
  or null if the file cannot be read or if the format cannot be recognized.

  The QImageIO documentation lists the supported image formats.

  \sa load(), save()
 ----------------------------------------------------------------------------*/

const char *QPixmap::imageFormat( const char *fileName )
{
    return QImageIO::imageFormat(fileName);
}


static bool can_turn_scanlines = FALSE;
static bool did_turn_scanlines = FALSE;

bool qt_image_can_turn_scanlines()
{
    if ( can_turn_scanlines ) {
	did_turn_scanlines = TRUE;
	return TRUE;
    }
    return FALSE;
}

bool qt_image_did_turn_scanlines()
{
    return did_turn_scanlines;
}


/*----------------------------------------------------------------------------
  Loads an image from the file \e fileName into the pixmap.
  Returns TRUE if successful, or FALSE if the image could not be loaded.

  If \e format is specified, then the loader will try to read the image
  using the specified format.  If \e format is not specified (default),
  the loader reads a few bytes from the header to guess the file format.

  The QImageIO documentation lists the supported image formats and
  explains how to add extra formats.

  \sa save(), imageFormat()
 ----------------------------------------------------------------------------*/

bool QPixmap::load( const char *fileName, const char *format )
{
    QImageIO io( fileName, format );
#if defined(_WS_WIN_)
    can_turn_scanlines = TRUE;
#endif
    bool result = io.read();
    if ( io.read() ) {
	detach();
	result = convertFromImage( io.image() );
    }
#if defined(_WS_WIN_)
    can_turn_scanlines = did_turn_scanlines = FALSE;
#endif
    return result;
}

/*----------------------------------------------------------------------------
  Saves the pixmap to the file \e fileName, using the image file format
  \e format.  Returns TRUE if successful, or FALSE if the image could not
  be saved.
  \sa load(), imageFormat()
 ----------------------------------------------------------------------------*/

bool QPixmap::save( const char *fileName, const char *format ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io( fileName, format );
    io.setImage( convertToImage() );
    return io.write();
}


/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QPixmap
  Writes a pixmap to the stream as a BMP image.
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QPixmap &pixmap )
{
    QImageIO io( s.device(), "BMP" );
    io.setImage( pixmap.convertToImage() );
    io.write();
    return s;
}

/*----------------------------------------------------------------------------
  \relates QPixmap
  Reads a pixmap from the stream as a BMP image.
 ----------------------------------------------------------------------------*/

QDataStream &operator>>( QDataStream &s, QPixmap &pixmap )
{
    QImageIO io( s.device(), "BMP" );
    io.read();
    pixmap.convertFromImage( io.image() );
    return s;
}
