/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.h#77 $
**
** Definition of QImage and QImageIO classes
**
** Created : 950207
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QIMAGE_H
#define QIMAGE_H

#ifndef QT_H
#include "qpixmap.h"
#include "qstrlist.h"
#endif // QT_H


class Q_EXPORT QImage
{
public:
    enum Endian { IgnoreEndian, BigEndian, LittleEndian };

    QImage();
    QImage( int width, int height, int depth, int numColors=0,
	    Endian bitOrder=IgnoreEndian );
    QImage( const QSize&, int depth, int numColors=0,
	    Endian bitOrder=IgnoreEndian );
    QImage( const QString &fileName, const char* format=0 );
    QImage( const char *xpm[] );
    QImage( const QByteArray &data );
    QImage( const QImage & );
   ~QImage();

    QImage     &operator=( const QImage & );
    QImage     &operator=( const QPixmap & );
    bool   	operator==( const QImage & ) const;
    bool   	operator!=( const QImage & ) const;
    void	detach();
    QImage	copy()		const;
    QImage	copy(int x, int y, int w, int h, int conversion_flags=0) const;
    QImage	copy(QRect&)	const;

    bool	isNull()	const	{ return data->bits == 0; }

    int		width()		const	{ return data->w; }
    int		height()	const	{ return data->h; }
    QSize	size()		const	{ return QSize(data->w,data->h); }
    QRect	rect()		const	{ return QRect(0,0,data->w,data->h); }
    int		depth()		const	{ return data->d; }
    int		numColors()	const	{ return data->ncols; }
    Endian 	bitOrder()	const	{ return (Endian) data->bitordr; }

    QRgb	color( int i )	const;
    void	setColor( int i, QRgb c );
    void	setNumColors( int );

    bool	hasAlphaBuffer() const;
    void	setAlphaBuffer( bool );

    bool	allGray() const;
    bool        isGrayscale() const;

    uchar      *bits()		const;
    uchar      *scanLine( int ) const;
    uchar     **jumpTable()	const;
    QRgb       *colorTable()	const;
    int		numBytes()	const;
    int		bytesPerLine()	const;

    bool	create( int width, int height, int depth, int numColors=0,
			Endian bitOrder=IgnoreEndian );
    bool	create( const QSize&, int depth, int numColors=0,
			Endian bitOrder=IgnoreEndian );
    void	reset();

    void	fill( uint pixel );
    void	invertPixels( bool invertAlpha = TRUE );

    QImage	convertDepth( int ) const;
    QImage	convertDepthWithPalette( int, QRgb* p, int pc, int cf=0 ) const;
    QImage	convertDepth( int, int conversion_flags ) const;
    QImage	convertBitOrder( Endian ) const;
    QImage	smoothScale(int width, int height) const;

    QImage	createAlphaMask( int conversion_flags=0 ) const;
    QImage	createHeuristicMask( bool clipTight=TRUE ) const;

    QImage	mirror() const;
    QImage	swapRGB() const;

    static Endian systemBitOrder();
    static Endian systemByteOrder();

    static const char* imageFormat( const QString &fileName );
    static QStrList inputFormats();
    static QStrList outputFormats();

    bool	load( const QString &fileName, const char* format=0 );
    bool	loadFromData( const uchar *buf, uint len,
			      const char *format=0 );
    bool	loadFromData( QByteArray data, const char *format=0 );
    bool	save( const QString &fileName, const char* format ) const;

    bool	valid( int x, int y ) const;
    int		pixelIndex( int x, int y ) const;
    QRgb	pixel( int x, int y ) const;
    void	setPixel( int x, int y, uint index_or_rgb );

private:
    void	init();
    void	freeBits();
    static void	warningIndexRange( const char *, int );

    struct QImageData : public QShared {	// internal image data
	int	w;				// image width
	int	h;				// image height
	int	d;				// image depth
	int	ncols;				// number of colors
	int	nbytes;				// number of bytes data
	int	bitordr;			// bit order (1 bit depth)
	QRgb   *ctbl;				// color table
	uchar **bits;				// image data
	bool	alpha;				// alpha buffer
    } *data;

    friend Q_EXPORT void bitBlt( QImage* dst, int dx, int dy,
				 const QImage* src, int sx, int sy,
				 int sw, int sh, int conversion_flags );
};


// QImage stream functions

Q_EXPORT QDataStream &operator<<( QDataStream &, const QImage & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QImage & );


class QIODevice;
typedef void (*image_io_handler)( QImageIO * ); // image IO handler


class Q_EXPORT QImageIO
{
public:
    QImageIO();
    QImageIO( QIODevice	 *ioDevice, const char *format );
    QImageIO( const QString &fileName, const char* format );
   ~QImageIO();


    const QImage &image()	const	{ return im; }
    int		status()	const	{ return iostat; }
    const char *format()	const	{ return frmt; }
    QIODevice  *ioDevice()	const	{ return iodev; }
    QString	fileName()	const	{ return fname; }
    const char *parameters()	const	{ return params; }
    QString	description()	const	{ return descr; }

    void	setImage( const QImage & );
    void	setStatus( int );
    void	setFormat( const char * );
    void	setIODevice( QIODevice * );
    void	setFileName( const QString &);
    void	setParameters( const char * );
    void	setDescription( const QString &);

    bool	read();
    bool	write();

    static const char* imageFormat( const QString &fileName );
    static const char *imageFormat( QIODevice * );
    static QStrList inputFormats();
    static QStrList outputFormats();

    static void defineIOHandler( const char *format,
				 const char *header,
				 const char *flags,
				 image_io_handler read_image,
				 image_io_handler write_image );

private:
    QImage	im;				// image
    int		iostat;				// IO status
    QCString	frmt;				// image format
    QIODevice  *iodev;				// IO device
    QString	fname;				// file name
    char       *params;				// image parameters
    QString     descr;				// image description

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QImageIO( const QImageIO & );
    QImageIO &operator=( const QImageIO & );
#endif
};


Q_EXPORT void bitBlt( QImage* dst, int dx, int dy, const QImage* src,
		      int sx=0, int sy=0, int sw=-1, int sh=-1,
		      int conversion_flags=0 );


/*****************************************************************************
  QImage member functions
 *****************************************************************************/

inline bool QImage::hasAlphaBuffer() const
{
    return data->alpha;
}

inline uchar *QImage::bits() const
{
    return data->bits ? data->bits[0] : 0;
}

inline uchar **QImage::jumpTable() const
{
    return data->bits;
}

inline QRgb *QImage::colorTable() const
{
    return data->ctbl;
}

inline int QImage::numBytes() const
{
    return data->nbytes;
}

inline int QImage::bytesPerLine() const
{
    return data->h ? data->nbytes/data->h : 0;
}

inline QImage QImage::copy(QRect& r) const
{
    return copy(r.x(), r.y(), r.width(), r.height());
}

inline QRgb QImage::color( int i ) const
{
#if defined(CHECK_RANGE)
    if ( i >= data->ncols )
	warningIndexRange( "color", i );
#endif
    return data->ctbl ? data->ctbl[i] : (QRgb)-1;
}

inline void QImage::setColor( int i, QRgb c )
{
#if defined(CHECK_RANGE)
    if ( i >= data->ncols )
	warningIndexRange( "setColor", i );
#endif
    if ( data->ctbl )
	data->ctbl[i] = c;
}

inline uchar *QImage::scanLine( int i ) const
{
#if defined(CHECK_RANGE)
    if ( i >= data->h )
	warningIndexRange( "scanLine", i );
#endif
    return data->bits ? data->bits[i] : 0;
}


#endif // QIMAGE_H
