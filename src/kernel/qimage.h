/****************************************************************************
**
** Definition of QImage and QImageIO classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIMAGE_H
#define QIMAGE_H

#ifndef QT_H
#include "qlist.h"
#include "qstringlist.h"
#include "qiodevice.h"
#include "qshared.h"
#include "qrgb.h"
#include "qrect.h"
#ifndef QT_BUILD_KERNEL_LIB
class QPixmap;
#endif
#endif // QT_H

class QImageDataMisc; // internal
#ifndef QT_NO_IMAGE_TEXT
class Q_KERNEL_EXPORT QImageTextKeyLang {
public:
    QImageTextKeyLang(const char* k, const char* l) : key(k), lang(l) { }
    QImageTextKeyLang() { }

    QByteArray key;
    QByteArray lang;

    bool operator< (const QImageTextKeyLang& other) const
	{ return key < other.key || key==other.key && lang < other.lang; }
    bool operator== (const QImageTextKeyLang& other) const
	{ return key==other.key && lang==other.lang; }
};
#endif //QT_NO_IMAGE_TEXT


class Q_KERNEL_EXPORT QImage
{
public:
    enum Endian { IgnoreEndian, BigEndian, LittleEndian };

    QImage();
    QImage( int width, int height, int depth, int numColors=0,
	    Endian bitOrder=IgnoreEndian );
    QImage( const QSize&, int depth, int numColors=0,
	    Endian bitOrder=IgnoreEndian );
#ifndef QT_NO_IMAGEIO
    QImage( const QString &fileName, const char* format=0 );
    QImage( const char * const xpm[] );
    QImage( const QByteArray &data );
#endif
    QImage( uchar* data, int w, int h, int depth,
		QRgb* colortable, int numColors,
		Endian bitOrder );
#ifdef Q_WS_QWS
    QImage( uchar* data, int w, int h, int depth, int pbl,
		QRgb* colortable, int numColors,
		Endian bitOrder );
#endif
    QImage( const QImage & );
   ~QImage();

    QImage     &operator=( const QImage & );
    bool	operator==( const QImage & ) const;
    bool	operator!=( const QImage & ) const;
    void	detach();
    QImage	copy()		const;
    QImage	copy(int x, int y, int w, int h, int conversion_flags=0) const;
    QImage	copy(const QRect&)	const;
#ifndef QT_NO_MIME
#ifndef QT_BUILD_KERNEL_LIB
    static QImage fromMimeSource( const QString& abs_name );
#endif
#endif
    bool	isNull()	const	{ return data->bits == 0; }

    int		width()		const	{ return data->w; }
    int		height()	const	{ return data->h; }
    QSize	size()		const	{ return QSize(data->w,data->h); }
    QRect	rect()		const	{ return QRect(0,0,data->w,data->h); }
    int		depth()		const	{ return data->d; }
    int		numColors()	const	{ return data->ncols; }
    Endian	bitOrder()	const	{ return (Endian) data->bitordr; }

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

#ifdef Q_WS_QWS
    QGfx * graphicsContext();
#endif

    bool	create( int width, int height, int depth, int numColors=0,
			Endian bitOrder=IgnoreEndian );
    bool	create( const QSize&, int depth, int numColors=0,
			Endian bitOrder=IgnoreEndian );
    void	reset();

    void	fill( uint pixel );
    void	invertPixels( bool invertAlpha = TRUE );

    QImage	convertDepth( int ) const;
#ifndef QT_NO_IMAGE_TRUECOLOR
    QImage	convertDepthWithPalette( int, QRgb* p, int pc, int cf=0 ) const;
#endif
    QImage	convertDepth( int, int conversion_flags ) const;
    QImage	convertBitOrder( Endian ) const;

    enum ScaleMode {
	ScaleFree,
	ScaleMin,
	ScaleMax
    };
#ifndef QT_NO_IMAGE_SMOOTHSCALE
    QImage smoothScale( int w, int h, ScaleMode mode=ScaleFree ) const;
    QImage smoothScale( const QSize& s, ScaleMode mode=ScaleFree ) const;
#endif
#ifndef QT_NO_IMAGE_TRANSFORMATION
    QImage scale( int w, int h, ScaleMode mode=ScaleFree ) const;
    QImage scale( const QSize& s, ScaleMode mode=ScaleFree ) const;
    QImage scaleWidth( int w ) const;
    QImage scaleHeight( int h ) const;
    QImage xForm( const QWMatrix &matrix ) const;
    static QWMatrix trueMatrix( const QWMatrix &, int w, int h );
#endif

#ifndef QT_NO_IMAGE_DITHER_TO_1
    QImage	createAlphaMask( int conversion_flags=0 ) const;
#endif
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
    QImage	createHeuristicMask( bool clipTight=TRUE ) const;
#endif
#ifndef QT_NO_IMAGE_MIRROR
    QImage	mirror() const;
    QImage	mirror(bool horizontally, bool vertically) const;
#endif
    QImage	swapRGB() const;

#ifndef QT_BUILD_KERNEL_LIB
    static Endian systemBitOrder();
#endif
    static Endian systemByteOrder();

#ifndef QT_NO_IMAGEIO
    static const char* imageFormat( const QString &fileName );
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();
#ifndef QT_NO_STRINGLIST
    static QStringList inputFormatList();
    static QStringList outputFormatList();
#endif
    bool	load( const QString &fileName, const char* format=0 );
    bool	loadFromData( const uchar *buf, uint len,
			      const char *format=0 );
    bool	loadFromData( QByteArray data, const char* format=0 );
    bool	save( const QString &fileName, const char* format,
		      int quality=-1 ) const;
    bool	save( QIODevice * device, const char* format,
		      int quality=-1 ) const;
#endif //QT_NO_IMAGEIO

    bool	valid( int x, int y ) const;
    int		pixelIndex( int x, int y ) const;
    QRgb	pixel( int x, int y ) const;
    void	setPixel( int x, int y, uint index_or_rgb );

    // Auxiliary data
    int dotsPerMeterX() const;
    int dotsPerMeterY() const;
    void setDotsPerMeterX(int);
    void setDotsPerMeterY(int);
    QPoint offset() const;
    void setOffset(const QPoint&);
#ifndef QT_NO_IMAGE_TEXT
    QList<QImageTextKeyLang> textList() const;
    QStringList textLanguages() const;
    QStringList textKeys() const;
    QString text(const char* key, const char* lang=0) const;
    QString text(const QImageTextKeyLang&) const;
    void setText(const char* key, const char* lang, const QString&);
#endif
private:
    void	init();
    void	reinit();
    void	freeBits();

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
	int	dpmx;				// dots per meter X (or 0)
	int	dpmy;				// dots per meter Y (or 0)
	QPoint	offset;				// offset in pixels
#ifndef QT_NO_IMAGE_TEXT
	QImageDataMisc* misc;			// less common stuff
#endif
	bool    ctbl_mine;			// this allocated ctbl
    } *data;
#ifndef QT_NO_IMAGE_TEXT
    QImageDataMisc& misc() const;
#endif
#ifndef QT_NO_IMAGEIO
    bool doImageIO( QImageIO* io, int quality ) const;
#endif
    friend Q_KERNEL_EXPORT void bitBlt( QImage* dst, int dx, int dy,
				 const QImage* src, int sx, int sy,
				 int sw, int sh, int conversion_flags );
};


// QImage stream functions

#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
Q_KERNEL_EXPORT QDataStream &operator<<( QDataStream &, const QImage & );
Q_KERNEL_EXPORT QDataStream &operator>>( QDataStream &, QImage & );
#endif

#ifndef QT_NO_IMAGEIO
class QIODevice;
typedef void (*image_io_handler)( QImageIO * ); // image IO handler


struct QImageIOData;


class Q_KERNEL_EXPORT QImageIO
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
    int		quality()	const;
    QString	description()	const	{ return descr; }
    const char *parameters()	const;
    float gamma() const;

    void	setImage( const QImage & );
    void	setStatus( int );
    void	setFormat( const char * );
    void	setIODevice( QIODevice * );
    void	setFileName( const QString & );
    void	setQuality( int );
    void	setDescription( const QString & );
    void	setParameters( const char * );
    void	setGamma( float );

    bool	read();
    bool	write();

    static QByteArray imageFormat( const QString &fileName );
    static QByteArray imageFormat( QIODevice * );
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();

    static void defineIOHandler( const char *format,
				 const char *header,
				 const char *flags,
				 image_io_handler read_image,
				 image_io_handler write_image );

private:
    void	init();

    QImage	im;				// image
    int		iostat;				// IO status
    QByteArray	frmt;				// image format
    QIODevice  *iodev;				// IO device
    QString	fname;				// file name
    char       *params;				// image parameters //### change to QImageIOData *d in 3.0
    QString     descr;				// image description
    QImageIOData *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QImageIO( const QImageIO & );
    QImageIO &operator=( const QImageIO & );
#endif
};

#endif //QT_NO_IMAGEIO

#ifndef QT_NO_PIXMAP_TRANSFORMATION
#  define QT_XFORM_TYPE_MSBFIRST 0
#  define QT_XFORM_TYPE_LSBFIRST 1
#  if defined(Q_WS_WIN)
#    define QT_XFORM_TYPE_WINDOWSPIXMAP 2
#  endif
Q_KERNEL_EXPORT bool qt_xForm_helper( const QWMatrix&, int, int, int, uchar*, int, int, int, uchar*, int, int, int );
#endif

Q_KERNEL_EXPORT void bitBlt( QImage* dst, int dx, int dy, const QImage* src,
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

inline QImage QImage::copy(const QRect& r) const
{
    return copy(r.x(), r.y(), r.width(), r.height());
}

inline QRgb QImage::color( int i ) const
{
    Q_ASSERT(i < numColors());
    return data->ctbl ? data->ctbl[i] : (QRgb)-1;
}

inline void QImage::setColor( int i, QRgb c )
{
    Q_ASSERT(i < numColors());
    if ( data->ctbl )
	data->ctbl[i] = c;
}

inline uchar *QImage::scanLine( int i ) const
{
    Q_ASSERT(i < height());
    return data->bits ? data->bits[i] : 0;
}

inline int QImage::dotsPerMeterX() const
{
    return data->dpmx;
}

inline int QImage::dotsPerMeterY() const
{
    return data->dpmy;
}

inline QPoint QImage::offset() const
{
    return data->offset;
}

#ifndef QT_BUILD_KERNEL_LIB

#ifndef QT_NO_MIME
Q_GUI_EXPORT QImage qFromMimeSource_helper( const QString &abs_name );
inline QImage QImage::fromMimeSource( const QString &abs_name )
{
    return qFromMimeSource_helper(abs_name);
}
#endif


inline QImage::Endian QImage::systemBitOrder()
{
#if defined(Q_WS_X11)
    return BitmapBitOrder(qt_xdisplay()) == MSBFirst ? BigEndian :LittleEndian;
#else
    return BigEndian;
#endif
}

#endif // QT_GUI_LIB


#endif // QIMAGE_H
