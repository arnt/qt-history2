/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#19 $
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
    QBitmap( int w, int h,  const uchar *bits, bool isXbitmap=FALSE );
    QBitmap( const QSize &, const uchar *bits, bool isXbitmap=FALSE );
#if defined(OBSOLETE)
    QBitmap( int w, int h,  const char *bits, bool isXbitmap=FALSE )
	: QPixmap( w, h, (const uchar *)bits, isXBitmap )
	{ data->bitmap = TRUE; }
    QBitmap( const QSize &s, const char *bits, bool isXbitmap=FALSE )
	: QPixmap( s.width(), s.height(), (const uchar *)bits, isXBitmap )
	{ data->bitmap = TRUE; }
#endif
    QBitmap( const QBitmap & );

    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QPixmap & );
    QBitmap &operator=( const QImage  & );

    QBitmap  xForm( const QWMatrix & ) const;
};


#endif // QBITMAP_H
