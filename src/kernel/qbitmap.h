/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#27 $
**
** Definition of QBitmap class
**
** Created : 941020
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
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
    QBitmap( int w, int h,  const uchar *bits, bool isXbitmap=FALSE );
    QBitmap( const QSize &, const uchar *bits, bool isXbitmap=FALSE );
    QBitmap( const QBitmap & );
    QBitmap( const char *fileName, const char *format=0 );

    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QPixmap & );
    QBitmap &operator=( const QImage  & );

    QBitmap  xForm( const QWMatrix & ) const;
};


#endif // QBITMAP_H
