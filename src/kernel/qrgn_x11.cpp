/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrgn_x11.cpp#3 $
**
** Implementation of QRegion class for X11
**
** Author  : Haavard Nord
** Created : 940729
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qrgn_x11.cpp#3 $";
#endif


QRegion::QRegion()				// create empty region
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = XCreateRegion();
}

QRegion::QRegion( const QRect &rr, RegionType t )
{						// create region from rect
    QRect r = rr;
    data = new QRegionData;
    CHECK_PTR( data );
    QPointArray a;
    r.fixup();
    if ( t == Ellipse )				// elliptic region
	a.makeEllipse( r.x(), r.y(), r.width(), r.height() );
    else {					// rectangular region
	a.resize( 4 );
	a.setPoint( 0, r.topLeft() );
	a.setPoint( 1, r.topRight() );
	a.setPoint( 2, r.bottomRight() );
	a.setPoint( 3, r.bottomLeft() );
    }
    data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(), WindingRule );
}

QRegion::QRegion( const QPointArray &a )	// create region from pt array
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(), WindingRule );
}

QRegion::QRegion( const QRegion &r )
{
    data = r.data;
    data->ref();
}

QRegion::~QRegion()
{
    if ( data->deref() ) {
	XDestroyRegion( data->rgn );
	delete data;
    }
}

QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of p = p
    if ( data->deref() ) {
	XDestroyRegion( data->rgn );
	delete data;
    }
    data = r.data;
    return *this;
}


bool QRegion::isNull() const
{
    return XEmptyRegion( data->rgn );
}

bool QRegion::contains( const QPoint &p ) const
{
    return XPointInRegion( data->rgn, p.x(), p.y() );
}

bool QRegion::contains( const QRect &r ) const
{
    return XRectInRegion( data->rgn, r.left(), r.right(),
			  r.width(), r.height() ) == RectanglePart;
}


void QRegion::move( int dx, int dy )
{
    XOffsetRegion( data->rgn, dx, dy );
}


QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result;
    XUnionRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result;
    XIntersectRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result;
    XSubtractRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

QRegion QRegion::xor( const QRegion &r ) const
{
    QRegion result;
    XXorRegion( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}
