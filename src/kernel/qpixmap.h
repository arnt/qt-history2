/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.h#98 $
**
** Definition of QPixmap class
**
** Created : 940501
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

#ifndef QPIXMAP_H
#define QPIXMAP_H

#ifndef QT_H
#include "qpaintdevice.h"
#include "qcolor.h"
#include "qstring.h"
#include "qnamespace.h"
#endif // QT_H


class Q_EXPORT QPixmap : public QPaintDevice, public Qt	// pixmap class
{
friend class QPaintDevice;
friend class QPainter;
public:
    enum ColorMode { Auto, Color, Mono };

    QPixmap();
    QPixmap( int w, int h,  int depth=-1 );
    QPixmap( const QSize &, int depth=-1 );
    QPixmap( const QString& fileName, const char *format=0, ColorMode mode=Auto );
    QPixmap( const QString& fileName, const char *format, int conversion_flags );
    QPixmap( const char *xpm[] );
    QPixmap( const QByteArray &data );
    QPixmap( const QPixmap & );
   ~QPixmap();

    QPixmap    &operator=( const QPixmap & );
    QPixmap    &operator=( const QImage	 & );

    bool	isNull()	const;

    int		width()		const { return data->w; }
    int		height()	const { return data->h; }
    QSize	size()		const { return QSize(data->w,data->h); }
    QRect	rect()		const { return QRect(0,0,data->w,data->h); }
    int		depth()		const { return data->d; }
    static int	defaultDepth();

    void	fill( const QColor &fillColor=Qt::white );
    void	fill( const QWidget *, int xofs, int yofs );
    void	fill( const QWidget *, const QPoint &ofs );
    void	resize( int width, int height );
    void	resize( const QSize & );

    const QBitmap *mask() const;
    void	setMask( const QBitmap & );
    bool	selfMask() const;
    QBitmap	createHeuristicMask( bool clipTight = TRUE ) const;

    static  QPixmap grabWindow( WId, int x=0, int y=0, int w=-1, int h=-1 );

    QPixmap	    xForm( const QWMatrix & ) const;
    static QWMatrix trueMatrix( const QWMatrix &, int w, int h );

    QImage	convertToImage() const;
    bool	convertFromImage( const QImage &, ColorMode mode=Auto );
    bool	convertFromImage( const QImage &, int conversion_flags );

    static QString imageFormat( const QString &fileName );
    bool	load( const QString& fileName, const char *format=0,
		      ColorMode mode=Auto );
    bool	load( const QString& fileName, const char *format,
		      int conversion_flags );
    bool	loadFromData( const uchar *buf, uint len,
			      const char* format=0,
			      ColorMode mode=Auto );
    bool	loadFromData( const uchar *buf, uint len,
			      const char* format,
			      int conversion_flags );
    bool	loadFromData( QByteArray data,
			      const char* format=0,
			      int conversion_flags=0 );
    bool	save( const QString& fileName, const char* format ) const;

#if defined(_WS_WIN_)
    HBITMAP	hbm()		const;
#endif

    int		serialNumber()	const;

    enum Optimization { NoOptim, NormalOptim, BestOptim };

    Optimization	optimization() const;
    void		setOptimization( Optimization );
    static Optimization defaultOptimization();
    static void		setDefaultOptimization( Optimization );

    virtual void detach();

    bool	isQBitmap() const;

#if defined(_WS_WIN_)
    HDC		allocMemDC();
    void	freeMemDC();
#endif

protected:
    QPixmap( int w, int h, const uchar *data, bool isXbitmap );
    int		metric( int ) const;

    struct QPixmapData : public QShared {	// internal pixmap data
	QCOORD	w, h;
	short	d;
	uint	uninit	 : 1;
	uint	bitmap	 : 1;
	uint	selfmask : 1;
	uint	unused	 : 1;
	int	ser_no;
	QBitmap *mask;
#if defined(_WS_WIN_)
	HBITMAP	hbm;
	void   *bits;
	QPixmap *maskpm;
#elif defined(_WS_X11_)
	void   *ximage;
	void   *maskgc;
#endif
	Optimization optim;
    } *data;

private:
    void	init( int, int, int );
    void	deref();
    QPixmap	copy() const;
    static Optimization defOptim;
    friend Q_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QPaintDevice *,
				 int, int, int, int, RasterOp, bool );
    friend Q_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QImage* src,
				 int, int, int, int, int conversion_flags );
};


inline bool QPixmap::isNull() const
{
#if defined(_WS_X11_)
    return hd == 0;
#else
    return data->hbm == 0;
#endif
}

inline void QPixmap::fill( const QWidget *w, const QPoint &ofs )
{
    fill( w, ofs.x(), ofs.y() );
}

inline void QPixmap::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

inline const QBitmap *QPixmap::mask() const
{
    return data->mask;
}

inline bool QPixmap::selfMask() const
{
    return data->selfmask;
}

#if defined(_WS_WIN_)
inline HBITMAP QPixmap::hbm() const
{
    return data->hbm;
}
#endif

inline int QPixmap::serialNumber() const
{
    return data->ser_no;
}

inline QPixmap::Optimization QPixmap::optimization() const
{
    return data->optim;
}

inline bool QPixmap::isQBitmap() const
{
    return data->bitmap;
}


/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPixmap & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPixmap & );


#endif // QPIXMAP_H
