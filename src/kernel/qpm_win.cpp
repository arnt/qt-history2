/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpm_win.cpp#1 $
**
** Implementation of QPixMap class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940501
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpm_win.cpp#1 $";
#endif


QPixMap::QPixMap( const QSize &size, int nPlanes )
{
    setDevType( PDT_PIXMAP );
    sz = size;
    bitPlanes = nPlanes;
    if ( bitPlanes < 0 ) {			// screen-compatible
	HDC gdc = GetDC( 0 );
	hbm = CreateCompatibleBitmap( gdc, sz.width(), sz.height() );
	bitPlanes = GetDeviceCaps( gdc, PLANES );
	ReleaseDC( 0, gdc );
    }
    else if ( bitPlanes == 1 )			// monochrome
	hbm = CreateBitmap( sz.width(), sz.height(), 1, 1, 0 );
    else
	hbm = 0;
    hdc = 0;
}

QPixMap::~QPixMap()
{
    freeMemDC();
    DeleteObject( hbm );
}


HANDLE QPixMap::allocMemDC()
{
    if ( !hdc ) {
	HANDLE h = GetDC( 0 );
	hdc = CreateCompatibleDC( hdc );
	SelectObject( hdc, hbm );
	ReleaseDC( 0, h );
    }
    return hdc;
}

void QPixMap::freeMemDC()
{
    if ( hdc ) {
	DeleteDC( hdc );
	hdc = 0;
    }
}
