/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.h#12 $
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
    QPixMap( int w, int h, const char *data, bool isXbitmap );
    QPixMap( QImageData * );
   ~QPixMap();

    int	   width()  const { return pw; }
    int	   height() const { return ph; }
    QSize  size()   const { return QSize(pw,ph); }
    QRect  rect()   const { return QRect(0,0,pw,ph); }
    int	   depth()  const { return pd; }

#if defined(_WS_X11_)
    bool   isNull() const { return hd == 0; }
#else
    bool   isNull() const { return hbm == 0; }
#endif

    void   fill( const QColor &fillColor=white );

    void   createPixMap( QImageData * );
    void   getPixMap( QImageData * );
    static QPixMap *grabWindow( WId, int x=0, int y=0, int w=-1, int h=-1 );

    static QPixMap *find( const char *key );	// pixmap dict functions
    static bool     insert( const char *key, QPixMap * );
    static void	    setCacheSize( long );
    static void	    cleanup();

    QPixMap *xForm( const QWorldMatrix & );	// transform bitmap
    static QWorldMatrix trueMatrix( const QWorldMatrix &, int w, int h );

protected:
    long   metric( int ) const;			// get metric information

private:
    QCOOT  pw, ph;				// pixmap width,height
    int	   pd;					// pixmap depth
#if defined(_WS_WIN_)
    HANDLE allocMemDC();
    void   freeMemDC();
    HANDLE hbm;
#elif defined(_WS_PM_)
    HANDLE hdcmem;
    HANDLE hbm;
#endif
};


#endif // QPIXMAP_H
