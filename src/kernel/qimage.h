/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.h#10 $
**
** Definition of QImage and QImageIO classes
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


class QImage
{
public:
    const IgnoreEndian = 0;			// bit/byte order values
    const BigEndian    = 1;
    const LittleEndian = 2;

    QImage();
    QImage( int width, int height, int depth, int numColors=0,
	    int bitOrder=IgnoreEndian );
    QImage( const QImage & );
    QImage( const QPixmap & );
    virtual ~QImage();

    QImage     &operator=( const QImage & );
    QImage     &operator=( const QPixmap & );
    void	detach();
    QImage	copy()		const;

    bool	isNull()	const	{ return data->bits == 0; }

    int		width()		const	{ return data->w; }
    int		height()	const	{ return data->h; }
    QSize	size()		const	{ return QSize(data->w,data->h); }
    QRect	rect()		const	{ return QRect(0,0,data->w,data->h); }
    int		depth()		const	{ return data->d; }
    int		numColors()	const	{ return data->ncols; }
    int		bitOrder()	const	{ return data->bitordr; }

    ulong	color( int i )	const;
    void	setColor( int i, ulong c );
    void	setNumColors( int );

    uchar      *bits()		const;
    uchar      *scanline( int )	const;
    uchar     **jumpTable()	const;
    bool	contiguousBits()const;
    ulong      *colorTable()	const;
    long	numBytes()	const;
    int		bytesPerLine()	const;

    bool	create( int width, int height, int depth, int numColors=0,
			int bitOrder=IgnoreEndian );
    void	reset();

    bool	convertDepth( int, QImage * )	 const;
    bool	convertBitOrder( int, QImage * ) const;

    static int	systemBitOrder();
    static int	systemByteOrder();

private:
    void	freeBits();

public:
    struct QImageData : QShared {		// internal image data
	int	w;				// image width
	int	h;				// image height
	int	d;				// image depth
	int	ncols;				// number of colors
	long	nbytes;				// number of bytes data
	int	bitordr;			// bit order (1 bit depth)
	ulong  *ctbl;				// color table
	uchar **bits;				// image data
#if defined(_WS_WIN16_)
	bool	contig;
#endif
    };

protected:
    QImageData *data;
    QImage( int, int );
    void    copyTo( QImage * ) const;
    virtual QImageData *newData();
    virtual void	deleteData( QImageData * );
};


struct QIODevice;
typedef void (*image_io_handler)( QImageIO * );	// image IO handler


class QImageIO : public QImage
{
public:
    QImageIO();
    QImageIO( int width, int height, int depth, int numColors=0,
	      int bitOrder=IgnoreEndian );
    QImageIO( const QImageIO & );
    QImageIO( const QPixmap & );
   ~QImageIO();

    QImageIO   &operator=( const QImageIO & );
    QImageIO   &operator=( const QPixmap & );
    void	detach();
    QImageIO	copy()		const;

    int		status()	const;
    const char *format()	const;
    QIODevice  *ioDevice()	const;
    const char *fileName()	const;
    const char *parameters()	const;
    const char *description()	const;

    void	setStatus( int );
    void	setFormat( const char * );
    void	setIODevice( QIODevice * );
    void	setFileName( const char * );
    void	setParameters( const char * );
    void	setDescription( const char * );

    bool	read();
    bool	write();

    static const char *imageFormat( const char *fileName );
    static const char *imageFormat( QIODevice * );

    static void defineIOHandler( const char *format,
				 const char *header,
				 const char *flags,
				 image_io_handler read_image,
				 image_io_handler write_image );

private:
    struct QImageIOData : QImageData {
	int	    status;			// IO status
	QString	    format;			// image format
	QIODevice  *iodev;			// IO device
	QString	    fname;			// file name
	QString	    params;			// image parameters
	QString	    descr;			// image description
    };
    QImageData *newData();
    void	deleteData( QImageData * );
};


inline uchar **QImage::jumpTable() const
{
    return data->bits;
}

inline ulong *QImage::colorTable() const
{
    return data->ctbl;
}

inline long QImage::numBytes() const
{
    return data->nbytes;
}

inline int QImage::bytesPerLine() const
{
    return data->h ? (int)(data->nbytes/data->h) : 0;
}

inline uchar *QImage::bits() const
{
    return data->bits ? data->bits[0] : 0;
}

inline bool QImage::contiguousBits() const
{
#if defined(_WS_WIN16_)
    return data->contig;
#else
    return TRUE;
#endif
}


#if !(defined(QIMAGE_C) || defined(DEBUG))

inline ulong QImage::color( int i ) const
{
    return data->ctbl ? data->ctbl[i] : -1L;
}

inline void QImage::setColor( int i, ulong c )
{
    if ( data->ctbl )
	data->ctbl[i] = c;
}

inline uchar *QImage::scanline( int i ) const
{
    return data->bits ? data->bits[i] : 0;
}

#endif


#endif // QIMAGE_H
