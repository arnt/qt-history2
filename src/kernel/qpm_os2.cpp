/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpm_os2.cpp#8 $
**
** Implementation of QPixMap class for OS/2 PM
**
** Created : 940804
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include <string.h>
#define	 INCL_PM
#include <os2.h>

QPixMap::QPixMap( const QSize &size, int nPlanes )
{
    setDevType( PDT_PIXMAP );
    sz = size;
    bitPlanes = nPlanes;
    HAB hab = qPMAppInst();
    hdcmem = DevOpenDC( hab, OD_MEMORY, "*", 0, 0, 0 );
    SIZEL s = { 0, 0 };
    hps = GpiCreatePS( hab, hdcmem, &s, PU_PELS |
		       GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );
    HDC gdc = WinOpenWindowDC( HWND_DESKTOP );
    long planes, bitcount;
    DevQueryCaps( gdc, CAPS_COLOR_PLANES, 1, &planes );
    DevQueryCaps( gdc, CAPS_COLOR_BITCOUNT, 1, &bitcount );
    if ( bitPlanes < 0 )
	bitPlanes = (int)planes;
    else if ( bitPlanes == 1 )
	planes = bitcount = 1;
    else {
	hbm = 0;
	return;
    }
    BITMAPINFOHEADER2 bmp;
    memset( &bmp, 0, sizeof(bmp) );
    bmp.cbFix = sizeof(bmp);
    bmp.cx = sz.width();
    bmp.cy = sz.height();
    bmp.cPlanes = planes;
    bmp.cBitCount = bitcount;
    hbm = GpiCreateBitmap( hps, &bmp, 0, 0, 0 );
    GpiSetBitmap( hps, hbm );
}

QPixMap::~QPixMap()
{
    GpiDestroyPS( hps );
    DevCloseDC( hdcmem );
    GpiDeleteBitmap( hbm );
}
