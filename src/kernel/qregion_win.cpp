/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion_win.cpp#9 $
**
** Implementation of QRegion class for Windows
**
** Author  : Haavard Nord
** Created : 940801
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#include "qbuffer.h"
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qregion_win.cpp#9 $")


QRegion::QRegion()				// create empty region
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = 0;
}

QRegion::QRegion( const QRect &r, RegionType t )
{						// create region from rect
    QRect rr = r;
    rr.fixup();
    data = new QRegionData;
    CHECK_PTR( data );
    int id;
    if ( t == Rectangle ) {			// rectangular region
	data->rgn = CreateRectRgn( rr.left(),	 rr.top(),
				   rr.right()+1, rr.bottom()+1 );
	id = QRGN_SETRECT;
    }
    else if ( t == Ellipse )	{		// elliptic region
	data->rgn = CreateEllipticRgn( rr.left(),    rr.top(),
				       rr.right()+1, rr.bottom()+1 );
	id = QRGN_SETELLIPSE;
    }
    else {
#if defined(CHECK_RANGE)
	warning( "QRegion: Invalid region type" );
#endif
	return;
    }
    cmd( id, &rr );
}

QRegion::QRegion( const QPointArray &a, bool winding )
{
    int r, c;
    if ( winding ) {
	r = WINDING;
	c = QRGN_SETPTARRAY_WIND;
    }
    else {
	r = ALTERNATE;
	c = QRGN_SETPTARRAY_ALT;
    }
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = CreatePolygonRgn( (POINT*)a.data(), a.size(), r );
    cmd( c, (QPointArray *)&a );
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


QRegion QRegion::copy() const
{
    QRegion r;
    r.data->bop = data->bop.copy();
    if ( data->rgn )
	CombineRgn( r.data->rgn, data->rgn, 0, RGN_COPY );
    return r;
}


bool QRegion::isNull() const
{
    return data->bop.isNull();
}

bool QRegion::isEmpty() const
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
    QPoint p( dx, dy );
    cmd( QRGN_MOVE, &p );
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
    result.cmd( QRGN_OR, 0, this, &r );
    return result;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result;
    result.data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    if ( data->rgn && r.data->rgn )
	CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_AND );
    result.cmd( QRGN_AND, 0, this, &r );
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
    result.cmd( QRGN_SUB, 0, this, &r );
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
    result.cmd( QRGN_XOR, 0, this, &r );
    return result;
}


bool QRegion::operator==( const QRegion &r ) const
{
    return data == r.data ?
	TRUE : EqualRgn( data->rgn, r.data->rgn );
}
