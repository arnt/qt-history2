/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrgn_x11.cpp#22 $
**
** Implementation of QRegion class for X11
**
** Author  : Haavard Nord
** Created : 940729
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#include "qbuffer.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qrgn_x11.cpp#22 $");


static QRegion *empty_region = 0;

static void cleanup_empty_region()
{
    delete empty_region;
    empty_region = 0;
}


/*!
  Constructs an empty region.
*/

QRegion::QRegion()
{
    if ( !empty_region ) {			// avoid too many allocs
	qAddPostRoutine( cleanup_empty_region );
	empty_region = new QRegion( TRUE );
	CHECK_PTR( empty_region );
    }
    data = empty_region->data;
    data->ref();
}

/*!
  Internal constructor that creates an empty region.
*/

QRegion::QRegion( bool )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = XCreateRegion();
}

/*!
  Constructs a rectangular or elliptic region.

  \arg \e r is the region rectangle.
  \arg \e t is the region type: QRegion::Rectangle (default) or
  QRegion::Ellipse.
*/

QRegion::QRegion( const QRect &r, RegionType t )
{
    QRect rr = r.normalize();
    data = new QRegionData;
    CHECK_PTR( data );
    int id;
    if ( t == Rectangle ) {			// rectangular region
	data->rgn = XCreateRegion();
	XRectangle xr;
	xr.x = rr.x();
	xr.y = rr.y();
	xr.width  = rr.width();
	xr.height = rr.height();
	XUnionRectWithRegion( &xr, data->rgn, data->rgn );
	id = QRGN_SETRECT;
    }
    else if ( t == Ellipse ) {			// elliptic region
	QPointArray a;
	a.makeEllipse( rr.x(), rr.y(), rr.width(), rr.height() );
	id = QRGN_SETELLIPSE;
	data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(), EvenOddRule );
    }
    else {
#if defined(CHECK_RANGE)
	warning( "QRegion: Invalid region type" );
#endif
	return;
    }
    cmd( id, &rr );
}

/*!
  Constructs a polygon region from the point array \e a.

  If \e winding is TRUE, the polygon region uses the winding
  algorithm, otherwise the alternative (even-odd) algorithm
  will be used.
*/

QRegion::QRegion( const QPointArray &a, bool winding )
{
    int r, c;
    if ( winding ) {
	r = WindingRule;
	c = QRGN_SETPTARRAY_WIND;
    }
    else {
	r = EvenOddRule;
	c = QRGN_SETPTARRAY_ALT;
    }
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(), r );
    cmd( c, (QPointArray *)&a );
}

/*!
  Constructs a region which is a shallow copy of \e r.
*/

QRegion::QRegion( const QRegion &r )
{
    data = r.data;
    data->ref();
}

/*!
  Destroys the region.
*/

QRegion::~QRegion()
{
    if ( data->deref() ) {
	XDestroyRegion( data->rgn );
	delete data;
    }
}

/*!
  Assigns a shallow copy of \e r to this region and returns a reference to
  the region.
*/

QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of r = r
    if ( data->deref() ) {
	XDestroyRegion( data->rgn );
	delete data;
    }
    data = r.data;
    return *this;
}


/*!
  Returns a deep copy of the region.
*/

QRegion QRegion::copy() const
{
    QRegion r( TRUE );
    r.data->bop = data->bop.copy();
    XUnionRegion( data->rgn, r.data->rgn, r.data->rgn );
    return r;
}


/*!
  Returns TRUE if the region is a null region.
  \sa isEmpty()
*/

bool QRegion::isNull() const
{
    return data->bop.isNull();
}

/*!
  Returns TRUE if the region is empty, or FALSE if it is
  non-empty. \sa isNull()
*/

bool QRegion::isEmpty() const
{
    return XEmptyRegion( data->rgn );
}


/*!
  Returns TRUE if the region contains the point \e p, or FALSE if \e p is
  outside the region.
*/

bool QRegion::contains( const QPoint &p ) const
{
    return XPointInRegion( data->rgn, p.x(), p.y() );
}

/*!
  Returns TRUE if the region contains the rectangle \e r, or FALSE if \e r is
  outside the region.
*/

bool QRegion::contains( const QRect &r ) const
{
    return XRectInRegion( data->rgn, r.left(), r.right(),
			  r.width(), r.height() ) != RectangleOut;
}


/*!
  Translates the region \e dx along the X axis and \e dy along the Y axis.
*/

void QRegion::translate( int dx, int dy )
{
    if ( data == empty_region->data )
	return;
    detach();
    XOffsetRegion( data->rgn, dx, dy );
    QPoint p( dx, dy );
    cmd( QRGN_TRANSLATE, &p );
}


/*!
  Returns a region which is the union of this region and \e r.
*/

QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result( TRUE );
    XUnionRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_OR, 0, this, &r );
    return result;
}

/*!
  Returns a region which is the intersection of this region and \e r.
*/

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result( TRUE );
    XIntersectRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_AND, 0, this, &r );
    return result;
}

/*!
  Returns a region which is \e r subtracted from this region.
*/

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result( TRUE );
    XSubtractRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_SUB, 0, this, &r );
    return result;
}

/*!
  Returns a region which is this region XOR \e r.
*/

QRegion QRegion::xor( const QRegion &r ) const
{
    QRegion result( TRUE );
    XXorRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_XOR, 0, this, &r );
    return result;
}


/*!
  Returns TRUE if the region is equal to \e r, or FALSE if the regions are
  different.
*/

bool QRegion::operator==( const QRegion &r ) const
{
    return data == r.data ?
	TRUE : XEqualRegion( data->rgn, r.data->rgn );
}

/*!
  \fn bool QRegion::operator!=( const QRegion &r ) const
  Returns TRUE if the region is different from \e r, or FALSE if the regions
  are equal.
*/
