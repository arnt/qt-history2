/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#16 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#16 $";
#endif


/*!
Detaches from shared pixmap data and makes sure that this pixmap is the
only one referring the data.

If multiple pixmaps share common data, this pixmap dereferences the
data and gets a copy of the data. Nothing will be done if there is just
a single reference.
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
Returns a deep copy of the pixmap.  All pixels are copied using bitBlt().
\sa operator=().
*/

QPixmap QPixmap::copy() const
{
    QPixmap tmp( data->w, data->h, data->d );
    tmp.data->bitmap = data->bitmap;
    bitBlt( &tmp, 0,0, this, 0,0, data->w, data->h );
    return tmp;
}


/*!
\fn void QPixmap::resize( const QSize &s )
Synonymous resize() which takes a QSize parameter.
*/

/*!
Resizes the pixmap to \e w width and \e h height.

New pixels will be uninitialized (random) if the pixmap is expanded.

A valid pixmap will be created if it is a null pixmap.
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

\sa load() and save().
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

\sa save() and imageFormat().
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
\e format.  Returns TRUE if successful, or FALSE if the image could not
be saved.

\sa load() and imageFormat().
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


/*! Writes a pixmap to the stream as a BMP image. \related QPixmap */

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

/*! Reads a pixmap from the stream as a BMP image. \related QPixmap */

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
