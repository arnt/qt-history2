/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#11 $
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
    QBitmap();
    QBitmap( int w, int h,    bool clear = FALSE );
    QBitmap( const QSize &sz, bool clear = FALSE  );
    QBitmap( int w, int h,    const char *bits, bool isXbitmap=FALSE );
    QBitmap( const QSize &sz, const char *bits, bool isXbitmap=FALSE );
    QBitmap( const QBitmap & );

    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QImage  & );
    QBitmap  copy() const;
};


inline QBitmap &QBitmap::operator=( const QBitmap &bm )
{
    return (QBitmap&)QPixmap::operator=(bm);
}


#endif // QBITMAP_H
