/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.h#5 $
**
** Definition of QPixMap class
**
** Author  : Haavard Nord
** Created : 940501
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPIXMAP_H
#define QPIXMAP_H

#include "qpaintd.h"


class QPixMap : public QPaintDevice		// pixmap class
{
friend class QPaintDevice;
friend class QPainter;
public:
    QPixMap( int w, int h, int nPlanes=-1 );
   ~QPixMap();

    QSize  size()   const { return sz; }
    QRect  rect()   const { return QRect(0,0,sz.width(),sz.height()); }
    int	   planes() const { return bitPlanes; }

#if defined(_WS_X11_)
    bool   isValid() const { return hd != 0; }
#else
    bool   isValid() const { return hbm != 0; }
#endif

    void   erase();

protected:
    QPixMap( int w, int h, const char *data );
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
