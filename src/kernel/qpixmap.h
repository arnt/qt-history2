/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.h#19 $
**
** Definition of QPixmap class
**
** Author  : Haavard Nord
** Created : 940501
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPIXMAP_H
#define QPIXMAP_H

#include "qpaintd.h"
#include "qcolor.h"
#include "qshared.h"
#include "qstring.h"


struct QImageData;
struct QImageIO;
typedef void (*image_io_handler)( QImageIO * );	// image IO handler

class QIODevice;


class QPixmap : public QPaintDevice		// pixmap class
{
friend class QPaintDevice;
friend class QPainter;
public:
    QPixmap();
    QPixmap( int w, int h, int depth=-1 );
    QPixmap( int w, int h, const char *data, bool isXbitmap );
    QPixmap( const QImageData * );
    QPixmap( const QPixmap & );
   ~QPixmap();

    QPixmap &operator=( const QPixmap & );

    int	    width()     const { return data->pw; }
    int	    height()    const { return data->ph; }
    QSize   size()      const { return QSize(data->pw,data->ph); }
    QRect   rect()      const { return QRect(0,0,data->pw,data->ph); }
    int	    depth()     const { return data->pd; }
    int	    numColors() const { return (1 << data->pd); }

#if defined(_WS_X11_)
    bool    isNull() const { return hd == 0; }
#else
    bool    isNull() const { return hbm == 0; }
#endif

    void    fill( const QColor &fillColor=white );
    void    resize( int width, int height );
    void    resize( const QSize & );
    QPixmap copy() const;

    bool    getImageData( QImageData * ) const;
    bool    setImageData( const QImageData * );
    static  QPixmap grabWindow( WId, int x=0, int y=0, int w=-1, int h=-1 );

  // Pixmap cache functions

    static  QPixmap    *find( const char *key );
    static  bool	insert( const char *key, QPixmap * );
    static  void	setCacheSize( long );
    static  void	cleanup();

  // Pixmap transformation

    QPixmap 	       *xForm( const Q2DMatrix & );
    static  Q2DMatrix	trueMatrix( const Q2DMatrix &, int w, int h );

    bool		cacheImageData( bool onOff );

    static  void	defineIOHandler( const char *format,
					 const char *header,
					 const char *flags,
					 image_io_handler read_image,
					 image_io_handler write_image );

    static  const char *imageType( const char *fileName );
    bool    load( const char *fileName, const char *format=0 );
    bool    save( const char *fileName, const char *format ) const;

    virtual bool isBitmap() const;

protected:
    long    metric( int ) const;		// get metric information

private:
    void    detach();

#if defined(_WS_WIN_)
    HANDLE allocMemDC();
    void   freeMemDC();
#endif

    struct QPixmapData : QShared {
        QCOORD pw, ph;				// pixmap width,height
        short  pd;				// pixmap depth
        uint   dirty  : 1;
        uint   optim  : 1;
        uint   virgin : 1;
#if defined(_WS_WIN_)
        HANDLE allocMemDC();
        void   freeMemDC();
        HANDLE hbm;
#elif defined(_WS_PM_)
        HANDLE hdcmem;
        HANDLE hbm;
#elif defined(_WS_X11_)
        void  *ximage;
#endif
    } *data;
};


// --------------------------------------------------------------------------
// QPixmap inline functions
//

inline void QPixmap::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}


// --------------------------------------------------------------------------
// QPixmap stream functions
//

QDataStream &operator<<( QDataStream &, const QPixmap & );
QDataStream &operator>>( QDataStream &, QPixmap & );


// --------------------------------------------------------------------------
// Abstract image description for image processing and storage.
//

struct QImageData {
    enum { IgnoreEndian, BigEndian, LittleEndian };

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


#endif // QPIXMAP_H
