/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.h#4 $
**
** Definition of QImage class
**
** Author  : Haavard Nord
** Created : 950207
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QIMAGE_H
#define QIMAGE_H

#include "qpixmap.h"
#include "qstring.h"


struct QImageData;
struct QImageIO;
typedef void (*image_io_handler)( QImageIO * );	// image IO handler


class QImage					// image class
{
public:
    QImage();
    QImage( int w, int h, int depth=-1 );
    QImage( const QPixMap & );
    QImage( const QImage & );
    QImage( const QImageData * );
   ~QImage();
    QImage     &operator=( const QPixMap & );
    QImage     &operator=( const QImage & );

    bool	operator==( const QImage & ) const;

    bool	isNull()	const	{ return data->pm == 0; }

    QImage	copy()		const;

    int		width()		const;
    int		height()	const;
    QSize	size()		const;
    QRect	rect()		const;

    int		depth()		const;
    int		numColors()	const;

    uchar      *bits()		const;

    void	createImage( const QImageData * );

    void	resize( int width, int height );
    void	resize( const QSize & );
    void	fill( const QColor & = white );

    static void	defineIOHandler( const char *format,
				 const char *header,
				 const char *flags,
				 image_io_handler read_image,
				 image_io_handler write_image );

    static const char *imageType( const char *fileName );
    bool	load( const char *fileName, const char *format=0 );
    bool	save( const char *fileName, const char *format=0 ) const;

		operator QPixMap &() const;
		operator QPixMap *() const { return data->pm; }

    friend QDataStream &operator<<( QDataStream &, const QImage & );
    friend QDataStream &operator>>( QDataStream &, QImage & );

private:
    struct QImagePix : QShared {		// image pixel data
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

struct QImageData {
    enum	{ BigEndian, LittleEndian, IgnoreEndian };

    QImageData();
   ~QImageData();
    int		width;				// image width
    int		height;				// image height
    int		depth;				// image depth
    int		ncols;				// number of colors
    ulong      *ctbl;				// color table
    uchar     **bits;				// image data
    int		bitOrder;			// bit order (1 bit depth)

    long	nBytes() const;
    void	allocBits();
    void	freeBits();

    bool	contiguousBits()  const;	// contiguous
    bool	convertDepth( int, QImageData *dest ) const;
    void	convertBitOrder( int );		// convert bit  order (1 bit)
    void	togglePix01();
    int		systemBitOrder()  const;	// display HW bit order
    static int	systemByteOrder();		// client computer byte order

    static int red( ulong rgb )	  { return (int)(rgb & 0xff); }
    static int green( ulong rgb ) { return (int)((rgb >> 8) & 0xff); }
    static int blue( ulong rgb )  { return (int)((rgb >> 16) & 0xff); }
    static int gray( int r, int g, int b )
	{ return (r*11 + g*16 + b*5) / 32; }
    static int gray( ulong rgb );
    static ulong setRGB( int r, int g, int b )
	{ return (uchar)r | ((ushort)g << 8) | ((ulong)b << 16); }
};

struct QIODevice;

struct QImageIO : public QImageData {
    QImageIO();
   ~QImageIO();
    int	       status;				// IO status
    QString    format;				// image format
    QIODevice *iodev;				// IO device
    QString    fname;				// file name
    QString    params;				// image parameters
    QString    descr;				// image description
};


#endif // QIMAGE_H
