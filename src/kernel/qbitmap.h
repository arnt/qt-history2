/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#1 $
**
** Definition of QBitMap class
**
** Author  : Haavard Nord
** Created : 941020
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBITMAP_H
#define QBITMAP_H

#include "qpixmap.h"


class QBitMap : public QPixMap			// bitmap class
{
public:
    QBitMap( int w, int h ) : QPixMap(w,h,1) {}
    QBitMap( int w, int h, const char *data ) : QPixMap(w,h,data) {}
    ~QBitMap() {}

    bool   testBit( int x, int y ) const;	// test if bit set
    void   setBit( int x, int y );		// set bit
    void   setBit( int x, int y, bool v )	// set bit to value
		{ if ( v ) setBit(x,y); else clearBit(x,y); }
    void   clearBit( int x, int y );		// clear bit
    bool   toggleBit( int x, int y );		// toggle/invert bit

    QBitMap *rotate( int degrees );

protected:
    char  *data;
};


#endif // QBITMAP_H
