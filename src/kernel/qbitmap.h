/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#17 $
**
** Definition of QBitmap class
**
** Author  : Haavard Nord
** Created : 941020
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBITMAP_H
#define QBITMAP_H

#include "qpixmap.h"


class QBitmap : public QPixmap			// bitmap class
{
public:
    QBitmap();
    QBitmap( int w, int h,  bool clear = FALSE );
    QBitmap( const QSize &, bool clear = FALSE	);
    QBitmap( int w, int h,  const char *bits, bool isXbitmap=FALSE );
    QBitmap( const QSize &, const char *bits, bool isXbitmap=FALSE );
    QBitmap( const QBitmap & );

    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QPixmap & );
    QBitmap &operator=( const QImage  & );

    QBitmap  xForm( const QWMatrix & ) const;
};


#endif // QBITMAP_H
