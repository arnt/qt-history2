/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#11 $
**
** Implementation of QPixmap class
**
** Author  : Haavard Nord
** Created : 950301
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include "qimage.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#11 $";
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
Returns a deep copy of the pixmap. All pixels are copied using bitBlt.
*/

QPixmap QPixmap::copy() const			// deep copy
{
    QPixmap tmp( data->w, data->h, data->d );
    bitBlt( &tmp, 0,0, this, 0,0, data->w, data->h );
    return tmp;
}


/*!
Resizes the pixmap to \e w X \e h.
*/

void QPixmap::resize( int w, int h )
{
    if ( !data->uninit && !isNull() ) {		// has existing pixmap
	QPixmap pm( w, h, depth() );
	bitBlt( &pm, 0, 0, this, 0, 0,		// copy old pixmap
		QMIN(width(), w),
		QMIN(height(),h) );
	*this = pm;
    }
    else					// create new pixmap
	*this = QPixmap( w, h, isBitmap() ? 1 : -1 );
}


/*!
Returns FALSE for instances of QPixmap and returns TRUE for instances of
QBitmap.
*/

bool QPixmap::isBitmap() const			// reimplemented in QBitmap
{
    return FALSE;
}


/*!
Returns a string that specifies the image format of the file \e fileName,
or null if the file could not be read or the format could not be recognized.
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

The QImageIO documentation describes the different image formats.
*/

bool QPixmap::load( const char *fileName, const char *format )
{
    QImageIO io;
    io.setFileName( fileName );
    io.setFormat( format );
    if ( io.read() ) {
	detach();
	QImage im = io.image();
	convertFromImage( &im );
	return TRUE;
    }
    return FALSE;
}

/*!
Saves the pixmap to the file \e fileName, using the image file format
\e format.
*/

bool QPixmap::save( const char *fileName, const char *format ) const
{
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io;
    QImage   im;
    convertToImage( &im );
    io.setImage( im );
    io.setFileName( fileName );
    io.setFormat( format );
    return io.write();
}


/*!
Writes a pixmap to the stream as a BMP image.
*/

QDataStream &operator<<( QDataStream &s, const QPixmap &pixmap )
{
    QImageIO io;
    QImage   im;
    pixmap.convertToImage( &im );
    io.setImage( im );
    io.setIODevice( s.device() );
    io.setFormat( "BMP" );
    io.write();
    return s;
}

/*!
Reads a pixmap from the stream as a BMP image.
*/

QDataStream &operator>>( QDataStream &s, QPixmap &pixmap )
{
    QImageIO io;
    QImage   im;
    io.setIODevice( s.device() );
    io.setFormat( "BMP" );
    io.read();
    im = io.image();
    pixmap.convertFromImage( &im );
    return s;
}
