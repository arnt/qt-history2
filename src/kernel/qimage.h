/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.h#1 $
**
** Definition of QImage class
**
** Author  : Haavard Nord
** Created : 950207
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QIMAGE_H
#define QIMAGE_H

#include "qpixmap.h"
#include "qstring.h"


struct QImageInfo;
struct QImageIO;
typedef void (*image_io_handler)( QImageIO * );	// image IO handler


class QImage					// image class
{
public:
    QImage();
    QImage( int w, int h, int depth=-1 );
    QImage( const QPixMap & );
    QImage( const QImage & );
    QImage( const QImageInfo * );
   ~QImage();
    QImage     &operator=( const QPixMap & );
    QImage     &operator=( const QImage & );

    bool    	operator==( const QImage & ) const;

    bool	isNull()	const	{ return data->pm == 0; }

    QImage	copy()		const;

    int		width()		const;
    int		height()	const;
    QSize	size()		const;
    QRect	rect()		const;

    int		depth()		const;
    int		numColors()	const;

    bool	trueColor()	const;

    uchar      *bits()		const;

    void	createImage( const QImageInfo * );

    void	resize( int width, int height );
    void	resize( const QSize & );
    void	fill( const QColor & = white );

    static void	defineIOHandler( const char *format,
				 const char *header,
				 image_io_handler read_image,
				 image_io_handler write_image );

    static const char *imageType( const char *fileName );
    bool	load( const char *fileName, const char *format=0 );
    bool	save( const char *fileName, const char *format=0 ) const;

		operator QPixMap &() const { return *data->pm; }
		operator QPixMap *() const { return data->pm; }

    static int red( ulong rgb )   { return (int)(rgb & 0xff); }
    static int green( ulong rgb ) { return (int)((rgb >> 8) & 0xff); }
    static int blue( ulong rgb )  { return (int)((rgb >> 16) & 0xff); }
    static ulong setRGB( int r, int g, int b )
	{ return (uchar)r | ((ushort)g << 8) | ((ulong)b << 16); }

    friend QDataStream &operator<<( QDataStream &, const QImage & );
    friend QDataStream &operator>>( QDataStream &, QImage & );

private:
    struct QImageData : QShared {		// image data
	QPixMap *pm;
    } *data;
};


inline void QImage::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}


// --------------------------------------------------------------------------
// QImage stream functions
//

QDataStream &operator<<( QDataStream &, const QImage & );
QDataStream &operator>>( QDataStream &, QImage & );


// --------------------------------------------------------------------------
// Abstract image description for image processing and storage.
//

struct QImageInfo {
    QImageInfo();
   ~QImageInfo();
    enum ImageSpec { Any, TrueColor, ColorMap, GrayScale, BitMap };
    ImageSpec  spec;				// image specification
    int	       width;				// image width
    int	       height;				// image height
    int	       depth;				// image depth
    int	       ncols;				// number of colors
    ulong     *carr;				// color array
    uchar     *bits;				// image data
    bool       bigend;				// big endian byte ordering
};

struct QIODevice;

struct QImageIO : public QImageInfo {
    QImageIO();
   ~QImageIO();
    int	       status;				// IO status
    QString    format;				// image format
    QIODevice *iodev;				// IO device
    long       length;				// length of input stream
    QString    fname;				// file name
    QString    params;				// image parameters
    QString    descr;				// image description
};


#endif // QIMAGE_H
