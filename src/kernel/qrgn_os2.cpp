/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrgn_os2.cpp#10 $
**
** Implementation of QRegion class for OS/2 PM
**
** Created : 940802
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#define	 INCL_PM
#include <os2.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qrgn_os2.cpp#10 $");


HPS QRegion::hps = 0;				// global presentation space

QRegion::QRegion()				// create empty region
{
    if ( !hps )
	hps = WinGetPS( HWND_DESKTOP );
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = 0;
}

QRegion::QRegion( const QRect &r, RegionType t )
{						// create region from rect
    if ( !hps )
	hps = WinGetPS( HWND_DESKTOP );
    data = new QRegionData;
    CHECK_PTR( data );
    if ( t == Ellipse ) {			// elliptic region
	data->rgn = 0;
    }
    else {					// rectangular region
	RECTL rect;
	rect.xLeft = r.left();
	rect.yBottom = r.bottom();
	rect.xRight = r.right();
	rect.yTop = r.top();
	data->rgn = GpiCreateRegion( hps, 1, &rect );
    }
}

QRegion::QRegion( const QPointArray &a )	// create region from pt array
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = 0;
}

QRegion::QRegion( const QRegion &r )
{
    data = r.data;
    data->ref();
}

QRegion::~QRegion()
{
    if ( data->deref() ) {
	if ( data->rgn )
	    GpiDestroyRegion( hps, data->rgn );
	delete data;
    }
}

QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of p = p
    if ( data->deref() ) {
	if ( data->rgn )
	    GpiDestroyRegion( hps, data->rgn );
	delete data;
    }
    data = r.data;
    return *this;
}


bool QRegion::isNull() const
{
    return data->rgn == 0;
}

bool QRegion::contains( const QPoint &p ) const
{
    bool res = FALSE;
    if ( data->rgn ) {
	POINTL pt;
	pt.x = p.x();
	pt.y = p.y();
	res = GpiPtInRegion( hps, data->rgn, &pt ) == PRGN_INSIDE;
    }
    return res;
}

bool QRegion::contains( const QRect &r ) const
{
    bool res;
    if ( data->rgn ) {
	RECTL rect;
	rect.xLeft = r.left();
	rect.yBottom = r.bottom();
	rect.xRight = r.right();
	rect.yTop = r.top();
	res = GpiRectInRegion( hps, data->rgn, &rect ) == RRGN_PARTIAL;
    }
    return res;
}


void QRegion::move( int dx, int dy )
{
    if ( data->rgn ) {
	POINTL pt;
	pt.x = dx;
	pt.y = dy;
	GpiOffsetRegion( hps, data->rgn, &pt );
    }
}


QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result( QRect(0, 0, 1, 1) );
    if ( data->rgn && r.data->rgn )
	GpiCombineRegion( hps, result.data->rgn, data->rgn, r.data->rgn, CRGN_OR );
    else if ( data->rgn )
	GpiCombineRegion( hps, result.data->rgn, data->rgn, 0, CRGN_COPY );
    else if ( r.data->rgn )
	GpiCombineRegion( hps, result.data->rgn, r.data->rgn, 0, CRGN_COPY );
    return result;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result( QRect(0, 0, 1, 1) );
    if ( data->rgn && r.data->rgn )
	GpiCombineRegion( hps, result.data->rgn, data->rgn, r.data->rgn, CRGN_AND );
    return result;
}

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result( QRect(0, 0, 1, 1) );
    if ( data->rgn && r.data->rgn )
	GpiCombineRegion( hps, result.data->rgn, data->rgn, r.data->rgn, CRGN_DIFF );
    else if ( data->rgn )
	GpiCombineRegion( hps, result.data->rgn, data->rgn, 0, CRGN_COPY );
    return result;
}

QRegion QRegion::eor( const QRegion &r ) const
{
    QRegion result( QRect(0, 0, 1, 1) );
    if ( data->rgn && r.data->rgn )
	GpiCombineRegion( hps, result.data->rgn, data->rgn, r.data->rgn, CRGN_XOR );
    else if ( data->rgn )
	GpiCombineRegion( hps, result.data->rgn, data->rgn, 0, CRGN_COPY );
    else if ( r.data->rgn )
	GpiCombineRegion( hps, result.data->rgn, r.data->rgn, 0, CRGN_COPY );
    return result;
}
