/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#7 $
**
** Definition of QBitMap class
**
** Author  : Haavard Nord
** Created : 941020
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBITMAP_H
#define QBITMAP_H

#include "qimage.h"


class QBitMap : public QImage			// bitmap class
{
public:
    QBitMap()					{}
    QBitMap( int w, int h ) : QImage( w, h, 1 )	{}
    QBitMap( int w, int h, const char *data, bool isXbitmap=FALSE );
    QBitMap( const QPixMap &pm ) : QImage( pm )	{}
    QBitMap( QPixMap *pm ) : QImage( pm )	{}
    QBitMap( const QImage &i ) : QImage( i )	{}
    QBitMap( QImageData *i ) : QImage( i )	{}
protected:
    bool  isBitMap() const;
};


#endif // QBITMAP_H
