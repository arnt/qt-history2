/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.h#11 $
**
** Definition of QPixMap class
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


class QImageData;


class QPixMap : public QPaintDevice		// pixmap class
{
friend class QPaintDevice;
friend class QPainter;
public:
    QPixMap( int w, int h, int depth=-1 );
    QPixMap( const QImageData * );
   ~QPixMap();

    int	   width()  const { return sz.width(); }
    int	   height() const { return sz.height(); }
    QSize  size()   const { return sz; }
    QRect  rect()   const { return QRect(0,0,sz.width(),sz.height()); }
    int	   depth()  const { return bitPlanes; }

#if defined(_WS_X11_)
    bool   isNull() const { return hd == 0; }
#else
    bool   isNull() const { return hbm == 0; }
#endif

    void   fill( const QColor &fillColor=white );

    void   createPixMap( const QImageData * );
    void   getPixMap( QImageData * );
    static QPixMap *grabWindow( WId, int x=0, int y=0, int w=-1, int h=-1 );

    static QPixMap *find( const char *key );
    static bool     insert( const char *key, QPixMap * );
    static void	    setCacheSize( long );
    static void	    cleanup();

protected:
    QPixMap( int w, int h, const char *data );
    long   metric( int ) const;			// get metric information
    QSize  sz;					// size of pixmap
    int	   bitPlanes;				// # bit planes
    bool   dirty;				// dirty/needs reconfig
#if defined(_WS_WIN_)
    HANDLE allocMemDC();
    void   freeMemDC();
    HANDLE hbm;					// bitmap
#elif defined(_WS_PM_)
    HANDLE hdcmem;
    HANDLE hbm;
#elif defined(_WS_X11_)

#endif
};


#endif // QPIXMAP_H
