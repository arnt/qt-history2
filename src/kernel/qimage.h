/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.h#7 $
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

class QIODevice;


class QImage					// image class
{
public:
    QImage();
    QImage( int w, int h, int depth=-1 );
    QImage( const QPixMap & );
    QImage( QPixMap * );
    QImage( const QImage & );
    QImage( const QImageData * );
    virtual ~QImage();
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

    void	resize( int width, int height );
    void	resize( const QSize & );
    void	fill( const QColor & = white );

    bool	getImageData( QImageData * ) const;
    bool	setImageData( const QImageData * );

    QPixMap    *pixmap()	     const { return data->pm; }
		operator QPixMap &() const;
		operator QPixMap *() const { return data->pm; }

    static void	defineIOHandler( const char *format,
				 const char *header,
				 const char *flags,
				 image_io_handler read_image,
				 image_io_handler write_image );

    static const char *imageType( const char *fileName );
    bool	load( const char *fileName, const char *format=0 );
    bool	save( const char *fileName, const char *format ) const;

    friend QDataStream &operator<<( QDataStream &, const QImage & );
    friend QDataStream &operator>>( QDataStream &, QImage & );

    virtual bool isBitMap() const;

#if defined(_WS_WIN_)
    HDC	     handle() const;
#elif defined(_WS_PM_)
    HPS	     handle() const;
#elif defined(_WS_X11_)
    WId	     handle() const;
#endif

protected:
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
    enum	{ IgnoreEndian, BigEndian, LittleEndian };

    QImageData();
   ~QImageData();
    int		width;				// image width
    int		height;				// image height
    int		depth;				// image depth
    int		ncols;				// number of colors
    ulong      *ctbl;				// color table
    uchar     **bits;				// image data
    int		bitOrder;			// bit order (1 bit depth)

    long	numBytes() const;
    void	allocBits();
    void	freeBits();
    void	clear();

    bool	contiguousBits()			const;
    bool	copyData( QImageData *dst )		const;
    bool	convertDepth( int, QImageData *dst )	const;
    void	convertBitOrder( int );
    static int	systemBitOrder();		// display HW bit order
    static int	systemByteOrder();		// client computer byte order
};

struct QIODevice;

struct QImageIO : public QImageData {
    QImageIO();
   ~QImageIO();
    int		status;				// IO status
    QString	format;				// image format
    QIODevice  *ioDevice;			// IO device
    QString	fileName;			// file name
    QString	params;				// image parameters
    QString	description;			// image description

    static const char *imageFormat( const char *fileName );
    static const char *imageFormat( QIODevice * );
    bool	read();
    bool	write();
};


#endif // QIMAGE_H
