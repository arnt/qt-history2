/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion_win.cpp#3 $
**
** Implementation of QRegion class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940801
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qregion_win.cpp#3 $";
#endif


QRegion::QRegion()				// create empty region
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = 0;
}

QRegion::QRegion( const QRect &r, RegionType t )
{						// create region from rect
    data = new QRegionData;
    CHECK_PTR( data );
    if ( t == Ellipse )				// elliptic region
	data->rgn = CreateEllipticRgn( r.left(), r.top(),
				       r.right(), r.bottom() );
    else					// rectangular region
	data->rgn = CreateRectRgn( r.left(), r.top(), r.right(), r.bottom() );
}

QRegion::QRegion( const QPointArray &a )	// create region from pt array
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = CreatePolygonRgn( (POINT*)a.data(), a.size(), WINDING );
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
	    DeleteObject( data->rgn );
	delete data;
    }
}

QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of p = p
    if ( data->deref() ) {
	if ( data->rgn )
	    DeleteObject( data->rgn );
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
    return data->rgn ? PtInRegion( data->rgn, p.x(), p.y() ) : FALSE;
}

bool QRegion::contains( const QRect &r ) const
{
    if ( !data->rgn )
	return FALSE;
    RECT rect;
    SetRect( &rect, r.left(), r.top(), r.right(), r.bottom() );
    return RectInRegion( data->rgn, &rect );
}


void QRegion::move( int dx, int dy )
{
    OffsetRgn( data->rgn, dx, dy );
}


QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result;
    result.data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    if ( data->rgn && r.data->rgn )
	CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_OR );
    else if ( data->rgn )
	CombineRgn( result.data->rgn, data->rgn, 0, RGN_COPY );
    else if ( r.data->rgn )
	CombineRgn( result.data->rgn, r.data->rgn, 0, RGN_COPY );
    return result;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result;
    result.data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    if ( data->rgn && r.data->rgn )
	CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_AND );
    return result;
}

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result;
    result.data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    if ( data->rgn && r.data->rgn )
	CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_DIFF );
    else if ( data->rgn )
	CombineRgn( result.data->rgn, data->rgn, 0, RGN_COPY );
    return result;
}

QRegion QRegion::xor( const QRegion &r ) const
{
    QRegion result;
    result.data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    if ( data->rgn && r.data->rgn )
	CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_XOR );
    else if ( data->rgn )
	CombineRgn( result.data->rgn, data->rgn, 0, RGN_COPY );
    else if ( r.data->rgn )
	CombineRgn( result.data->rgn, r.data->rgn, 0, RGN_COPY );
    return result;
}
