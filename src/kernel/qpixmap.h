/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.h#2 $
**
** Definition of QPixMap class
**
** Author  : Haavard Nord
** Created : 940501
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QPIXMAP_H
#define QPIXMAP_H

#include "qpaintd.h"
#include "qsize.h"


class QPixMap : public QPaintDevice		// pixmap class
{
friend class QPainter;
public:
    QPixMap( const QSize &size, int nPlanes=-1 );
   ~QPixMap();

    QSize  size()   const { return sz; }
    int	   planes() const { return bitPlanes; }

#if defined(_WS_X11_)
    bool   isValid() const { return hd != 0; }
#else
    bool   isValid() const { return hbm != 0; }
#endif

private:
    QSize  sz;					// size of pixmap
    int	   bitPlanes;				// # bit planes
#if defined(_WS_WIN_)
    HANDLE allocMemDC();
    void   freeMemDC();
    HANDLE hbm;					// bitmap
#elif defined(_WS_PM_)
    HANDLE hdcmem;
    HANDLE hbm;
#endif
};


#endif // QPIXMAP_H
