/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#21 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#21 $";
#endif


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
  Returns a deep copy of the pixmap.  All pixels are copied using bitBlt().
  \sa operator=().
*/

QPixmap QPixmap::copy() const
{
    QPixmap tmp( data->w, data->h, data->d );
    bitBlt( &tmp, 0,0, this, 0,0, data->w, data->h );
    return tmp;
}


/*!
  \fn void QPixmap::resize( const QSize &size )
  Synonymous resize() which takes a QSize parameter.
*/

/*!
  Resizes the pixmap to \e w width and \e h height. <br>
  New pixels will be uninitialized (random) if the pixmap is expanded. <br>
  A valid pixmap will be created if it is a null pixmap. <br>
*/

void QPixmap::resize( int w, int h )
{
    if ( !data->uninit && !isNull() ) {		// has existing pixmap
	QPixmap pm( w, h, depth() );
	pm.data->bitmap = data->bitmap;
	bitBlt( &pm, 0, 0, this, 0, 0,		// copy old pixmap
		QMIN(width(), w),
		QMIN(height(),h) );
	*this = pm;
    }
    else {					// create new pixmap
	QPixmap pm( w, h, data->bitmap ? 1 : -1 );
	pm.data->bitmap = data->bitmap;
	*this = pm;
    }
}


/*!
  Returns a string that specifies the image format of the file \e fileName,
  or null if the file cannot be read or if the format cannot be recognized.
  \sa load(), save()
*/

const char *QPixmap::imageFormat( const char *fileName )
{						// determine image format
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
