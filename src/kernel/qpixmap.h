/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.h#33 $
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


class QPixmap : public QPaintDevice		// pixmap class
{
friend class QPaintDevice;
friend class QPainter;
public:
    QPixmap();
    QPixmap( int w, int h,  int depth=-1 );
    QPixmap( const QSize &, int depth=-1 );
    QPixmap( const QPixmap & );
   ~QPixmap();

    QPixmap &operator=( const QPixmap & );
    QPixmap &operator=( const QImage  & );

    bool    isNull()	const;

    int	    width()	const { return data->w; }
    int	    height()	const { return data->h; }
    QSize   size()	const { return QSize(data->w,data->h); }
    QRect   rect()	const { return QRect(0,0,data->w,data->h); }
    int	    depth()	const { return data->d; }
    int	    numColors() const { return (1 << data->d); }

    void    fill( const QColor &fillColor=white );
    void    resize( int width, int height );
    void    resize( const QSize & );

    static  QPixmap grabWindow( WId, int x=0, int y=0, int w=-1, int h=-1 );

    bool    enableImageCache( bool enable );

    QPixmap		xForm( const Q2DMatrix & )	const;
    static  Q2DMatrix	trueMatrix( const Q2DMatrix &, int w, int h );

    QImage  convertToImage() const;
    bool    convertFromImage( const QImage & );

    static  const char *imageFormat( const char *fileName );
    bool    load( const char *fileName, const char *format=0 );
    bool    save( const char *fileName, const char *format ) const;

protected:
    QPixmap( int w, int h, const char *data, bool isXbitmap );
    long    metric( int ) const;		// get metric information

    virtual void detach();

#if defined(_WS_WIN_) || defined(_WS_PM_)
    HANDLE  hbm() const;
#endif

#if defined(_WS_WIN_)
    HANDLE allocMemDC();
    void   freeMemDC();
#endif

    struct QPixmapData : QShared {		// internal pixmap data
	QCOORD w, h;
	short  d;
	uint   dirty  : 1;
	uint   optim  : 1;
	uint   uninit : 1;
	uint   bitmap : 1;
#if defined(_WS_WIN_)
	HANDLE hbm;
#elif defined(_WS_PM_)
	HANDLE hdcmem;
	HANDLE hbm;
#elif defined(_WS_X11_)
	void  *ximage;
#endif
    } *data;

private:
    void    init();

    friend void bitBlt( QPaintDevice *, int, int, const QPaintDevice *,
			int, int, int, int, RasterOp ); // needs detach()
};


inline void QPixmap::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

inline bool QPixmap::isNull() const
{
#if defined(_WS_X11_)
    return hd == 0;
#else
    return data->hbm == 0;
#endif
}

#if defined(_WS_WIN_) || defined(_WS_PM_)
inline HANDLE QPixmap::hbm() const
{
    return data->hbm;
}
#endif


// --------------------------------------------------------------------------
// QPixmap stream functions
//

QDataStream &operator<<( QDataStream &, const QPixmap & );
QDataStream &operator>>( QDataStream &, QPixmap & );


#endif // QPIXMAP_H
