/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#8 $
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
    QBitmap( int w, int h ) : QPixmap( w, h, 1 )	{}
    QBitmap( int w, int h, const char *data, bool isXbitmap=FALSE );
    QBitmap( const QPixmap &pm ) : QPixmap( pm )	{}
    QBitmap( QImageData *i ) : QPixmap( i )	{}
protected:
    bool  isBitmap() const;
};


#endif // QBITMAP_H
