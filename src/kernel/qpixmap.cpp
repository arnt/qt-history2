/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#22 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#22 $";
#endif


/*!
  Detaches the pixmap from shared data.
*/

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

/*!
  Returns a deep copy of the pixmap using the bitBlt() function to copy
  the pixels.
  \sa operator=()
*/

QPixmap QPixmap::copy() const
{
    QPixmap tmp( data->w, data->h, data->d );
    bitBlt( &tmp, 0,0, this, 0,0, data->w, data->h );
    tmp.data->optim = data->optim;		// copy optim flag
    pm.data->bitmap = data->bitmap;		// copy bitmap flag
    return tmp;
}


/*!
  Converts the image \e image to a pixmap that is assigned to this pixmap.
  Returns a reference to the pixmap.
  \sa convertFromImage().
*/

QPixmap &QPixmap::operator=( const QImage &image )
{
    convertFromImage( im );
    return *this;
}


/*!
  \fn bool QPixmap::isNull() const
  Returns TRUE if it is a null pixmap.

  A null pixmap has zero width, zero height and no contents.
  You cannot draw in a null pixmap or bitBlt() anything to it.
  \sa isNull()
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
  Returns the enclosing rectangle of the pixmap.
  \sa width(), height(), size()
*/

/*!
  \fn int QPixmap::depth() const
  Returns the depth of the image. <br>
  The pixmap depth is also called bits per pixel (bpp) or bit planes
  of a pixmap.
  \sa numColors()
*/

/*!
  \fn int QPixmap::numColors() const
  Returns the maximum number of colors that can be used for the pixmap.<br>
  Equivalent to 2^depth().
*/


/*!
  \fn void QPixmap::resize( const QSize &size )
  Overloaded resize(); takes a QSize parameter instead of \e (w,h).
*/

/*!
  Resizes the pixmap to \e w width and \e h height. <br>
  New pixels will be uninitialized (random) if the pixmap is expanded. <br>
  A valid pixmap will be created if this is a null pixmap. <br>
*/

void QPixmap::resize( int w, int h )
{
    QPixmap pm( w, h, depth() );		// create new pixmap
    if ( !data->uninit && !isNull() )		// has existing pixmap
	bitBlt( &pm, 0, 0, this, 0, 0,		// copy old pixmap
		QMIN(width(), w),
		QMIN(height(),h) );
    pm.data->optim  = data->optim;		// keep optim flag
    pm.data->bitmap = data->bitmap;		// keep bitmap flag
    *this = pm;
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


/*!
  Loads an image from the file \e fileName into the pixmap.
  Returns TRUE if successful, or FALSE if the image could not be loaded.

  If \e format is specified, then the loader will try to read the image
  using the specified format.  If \e format is not specified (default),
  the loader reads a few bytes from the header to guess the file format.

  The QImageIO documentation lists the supported image formats.

  \sa save(), imageFormat()
*/

bool QPixmap::load( const char *fileName, const char *format )
{
    QImageIO io;
    io.setFileName( fileName );
    io.setFormat( format );
    if ( io.read() ) {
	detach();
	convertFromImage( io.image() );
	return TRUE;
    }
    return FALSE;
}

/*!
  Saves the pixmap to the file \e fileName, using the image file format
  \e format.  Returns TRUE if successful, or FALSE if the image could not
  be saved.
  \sa load(), imageFormat()
*/

bool QPixmap::save( const char *fileName, const char *format ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io;
    io.setImage( convertToImage() );
    io.setFileName( fileName );
    io.setFormat( format );
    return io.write();
}


// --------------------------------------------------------------------------
// QPixmap stream functions
//

/*!
  \relates QPixmap
  Writes a pixmap to the stream as a BMP image.
*/

QDataStream &operator<<( QDataStream &s, const QPixmap &pixmap )
{
    QImageIO io;
    io.setImage( pixmap.convertToImage() );
    io.setIODevice( s.device() );
    io.setFormat( "BMP" );
    io.write();
    return s;
}

/*!
  \relates QPixmap
  Reads a pixmap from the stream as a BMP image.
*/

QDataStream &operator>>( QDataStream &s, QPixmap &pixmap )
{
    QImageIO io;
    io.setIODevice( s.device() );
    io.setFormat( "BMP" );
    io.read();
    pixmap.convertFromImage( io.image() );
    return s;
}
