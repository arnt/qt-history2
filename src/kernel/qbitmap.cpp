/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.cpp#2 $
**
** Implementation of QBitMap class
**
** Author  : Haavard Nord
** Created : 941020
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qbitmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qbitmap.cpp#2 $";
#endif


QBitMap::QBitMap( int w, int h, const char *bits, bool isXbitmap )
{
    data->pm = new QPixMap( w, h, bits, isXbitmap );
}


bool QBitMap::isBitMap() const
{
    return TRUE;
}
