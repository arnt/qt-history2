/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.cpp#3 $
**
** Implementation of QBitmap class
**
** Author  : Haavard Nord
** Created : 941020
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qbitmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qbitmap.cpp#3 $";
#endif


QBitmap::QBitmap( int w, int h, const char *bits, bool isXbitmap )
     : QPixmap( w, h, bits, isXbitmap )
{
}


bool QBitmap::isBitmap() const
{
    return TRUE;
}
