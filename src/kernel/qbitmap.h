/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#9 $
**
** Definition of QBitmap class
**
** Author  : Haavard Nord
** Created : 941020
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBITMAP_H
#define QBITMAP_H

#include "qpixmap.h"


class QBitmap : public QPixmap			// bitmap class
{
public:
    QBitmap()						{}
    QBitmap( int w, int h )	 : QPixmap( w, h, 1 )	{}
    QBitmap( int w, int h, const char *bits, bool isXbitmap=FALSE )
				 : QPixmap( w, h, bits, isXbitmap ) {}
    QBitmap( const QBitmap &bm ) : QPixmap( bm )	{}
    QBitmap( const QImage  &im ) : QPixmap( im )	{}

    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QImage  & );

protected:
    bool  isBitmap() const;
};


inline QBitmap &QBitmap::operator=( const QBitmap &bm )
{
    return (QBitmap&)QPixmap::operator=(bm);
}

inline QBitmap &QBitmap::operator=( const QImage  &im )
{
    return (QBitmap&)QPixmap::operator=(im);
}


#endif // QBITMAP_H
