/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrgn_win.cpp#10 $
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

RCSTAG("$Id: //depot/qt/main/src/kernel/qrgn_win.cpp#10 $")


static void *empty_region = 0;

static void cleanup_empty_region()
{
    delete empty_region;
    empty_region = 0;
}


QRegion::QRegion()
{
    if ( !empty_region ) {			// avoid too many allocs
	qAddPostRoutine( cleanup_empty_region );
	empty_region = new QRegion( TRUE );
	CHECK_PTR( empty_region );
    }
    data = (QRegionData*)empty_region;
    data->ref();
}

QRegion::QRegion( bool )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = 0;
}

QRegion::QRegion( const QRect &r, RegionType t )
{
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
    QRegion r( TRUE );
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
    return data->rgn ? PtInRegion(data->rgn, p.x(), p.y()) : FALSE;
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
    if ( !data->rgn )
	return;
    detach();
    OffsetRgn( data->rgn, dx, dy );
    QPoint p( dx, dy );
    cmd( QRGN_MOVE, &p );
}


#define RGN_NOP -1

/*----------------------------------------------------------------------------
  Performs the actual OR, AND, SUB and XOR operation between regions.
  Sets the resulting region handle to 0 to indicate an empty region.
 ----------------------------------------------------------------------------*/

QRegion QRegion::winCombine( const QRegion &r, int op )
{
    int both=RGN_NOP, left=RGN_NOP, right=RGN_NOP;
    switch ( op ) {
	case QRGN_OR:
	    both = RGN_OR;
	    left = right = RGN_COPY;
	    break;
	case QRGN_AND:
	    both = RGN_AND;
	    break;
	case QRGN_SUB:
	    both = RGN_DIFF;
	    left = RGN_COPY;
	    break;
	case QRGN_XOR:
	    both = RGN_XOR;
	    left = right = RGN_COPY;
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QRegion: Internal error in winCombine" );
#endif
    }

    QRegion result( TRUE );
    result.data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    int res = NULLREGION;
    if ( data->rgn && r.data->rgn )
	res = CombineRgn( result.data->rgn, data->rgn, r.data->rgn, both );
    else if ( data->rgn && left != RGN_NOP )
	res = CombineRgn( result.data->rgn, data->rgn, 0, left );
    else if ( r.data->rgn && right != RGN_NOP )
	res = CombineRgn( result.data->rgn, r.data->rgn, 0, right );
    result.cmd( op, 0, this, &r );
    if ( res == NULLREGION ) {
	if ( result.data->rgn )
	    DeleteObject( result.data->rgn );
	result.data->rgn = 0;			// empty region
    }
    return result;
}

QRegion QRegion::unite( const QRegion &r ) const
{
    return winCombine( r, QRGN_OR );
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    return winCombine( r, QRGN_AND );
}

QRegion QRegion::subtract( const QRegion &r ) const
{
    return winCombine( r, QRGN_SUB );
}

QRegion QRegion::xor( const QRegion &r ) const
{
    return winCombine( r, QRGN_XOR );
}


bool QRegion::operator==( const QRegion &r ) const
{
    if ( data == r.data() )			// share the same data
	return TRUE;
    if ( (data->rgn == 0) ^ (r.data->rgn == 0)) // one is empty, not both
	return FALSE;
    return data->rgn == 0 ?
	TRUE :					// both empty
	EqualRgn( data->rgn, r.data->rgn );	// both non-empty
}
