/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion_x11.cpp#8 $
**
** Implementation of QRegion class for X11
**
** Author  : Haavard Nord
** Created : 940729
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#include "qbuffer.h"
#include "qdstream.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qregion_x11.cpp#8 $";
#endif


/*!
\class QRegion qregion.h
\brief The QRegion class specified a clip region for the painter.

A region defines a clip region for a QPainter. A region can be
a rectangle, an ellipse, a polygon or a combination of these.

Regions are combined by creating a new region which is a
union, intersection or difference between any two regions.

The region XOR operation is defined as:
\code
  a XOR b = (a UNION b) - (a INTERSECTION b)
\endcode

Example of use:
\code
  QWidget  w;
  QPainter p;
  QRegion r1( QRect(100,100,200,80),	\/ r1 = elliptic region
              QRegion::Ellipse );
  QRegion r2( QRect(100,120,90,30) );	\/ r2 = rectangular region
  QRegion r3 = r1.intersect( r2 );	\/ r3 = intersection
  p.begin( &w );			\/ start painting widget
  p.setClipRegion( r3 );		\/ set clip region
  ...					\/ paint clipped graphics
  p.end();				\/ painting done
\endcode
*/


/*!
Constructs an empty region.
*/

QRegion::QRegion()				// create empty region
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = XCreateRegion();
}

/*!
Constructs a rectangular or elliptic region.

\arg \e rr is the region rectangle.
\arg \e t is the region type: QRegion::Rectangle (default) or QRegion::Ellipse.
*/

QRegion::QRegion( const QRect &rr, RegionType t )
{						// create region from rect
    QRect r = rr;
    data = new QRegionData;
    CHECK_PTR( data );
    r.fixup();
    int id;
    if ( t == Rectangle ) {			// rectangular region
	data->rgn = XCreateRegion();
	XRectangle xr;
	xr.x = r.x();
	xr.y = r.y();
	xr.width = r.width();
	xr.height = r.height();
	XUnionRectWithRegion( &xr, data->rgn, data->rgn );
	id = QRGN_SETRECT;
    }
    else if ( t == Ellipse ) {			// elliptic region
	QPointArray a;
	a.makeEllipse( r.x(), r.y(), r.width(), r.height() );
	id = QRGN_SETELLIPSE;
	data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(), EvenOddRule );
    }
    else {
#if defined(CHECK_RANGE)
	warning( "QRegion: Invalid region type" );
#endif
	return;
    }
    cmd( id, &r );
}

/*!
Constructs a polygon region from the point array \e a.
*/

QRegion::QRegion( const QPointArray &a )	// create region from pt array
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = XPolygonRegion( (XPoint*)a.data(), a.size(), EvenOddRule );
    cmd( QRGN_SETPTARRAY, (QPointArray *)&a );
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
    QRegion r;
    r.data->bop = data->bop.copy();
    XUnionRegion( data->rgn, r.data->rgn, r.data->rgn );
    return r;
}


/*!
Returns TRUE if the region is a null region.
*/

bool QRegion::isNull() const
{
    return data->bop.isNull();
}

/*!
Returns TRUE if the region is empty, or FALSE if it is non-empty.
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
Changes the offset of the region \e dx along the X axis and \e dy along the
Y axis.
*/

void QRegion::move( int dx, int dy )
{
    XOffsetRegion( data->rgn, dx, dy );
    QPoint p( dx, dy );
    cmd( QRGN_MOVE, &p );
}


/*!
Returns a region which is the union of this region and \e r.
*/

QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result;
    XUnionRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_OR, 0, this, &r );
    return result;
}

/*!
Returns a region which is the intersection of this region and \e r.
*/

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result;
    XIntersectRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_AND, 0, this, &r );
    return result;
}

/*!
Returns a region which is \e r subtracted from this region.
*/

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result;
    XSubtractRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_SUB, 0, this, &r );
    return result;
}

/*!
Returns a region which is this region XOR \e r.
*/

QRegion QRegion::xor( const QRegion &r ) const
{
    QRegion result;
    XXorRegion( data->rgn, r.data->rgn, result.data->rgn );
    result.cmd( QRGN_XOR, 0, this, &r );
    return result;
}


/*!
\fn bool QRegion::operator!=( const QRegion &r ) const
Returns TRUE if the region is different from \e r, or FALSE if the regions are
equal.
*/

/*!
Returns TRUE if the region is equal to \e r, or FALSE if the regions are
different.
*/

bool QRegion::operator==( const QRegion &r ) const
{
    return data->bop == r.data->bop ?
	TRUE : XEqualRegion( data->rgn, r.data->rgn );
}


void QRegion::cmd( int id, void *param, const QRegion *r1, const QRegion *r2 )
{
    QBuffer buf( data->bop );
    QDataStream s( &buf );
    buf.open( IO_WriteOnly );
    buf.at( buf.size() );
    s << id;
    switch ( id ) {
	case QRGN_SETRECT:
	case QRGN_SETELLIPSE:
	    s << *((QRect*)param);
	    break;
	case QRGN_SETPTARRAY:
	    s << *((QPointArray*)param);
	    break;
	case QRGN_MOVE:
	    s << *((QPoint*)param);
	    break;
	case QRGN_OR:
	case QRGN_AND:
	case QRGN_SUB:
	case QRGN_XOR:
	    s << r1->data->bop << r2->data->bop;
	    break;
#if defined(CHECK_RANGE)
	default:
	    warning( "QRegion: Internal cmd error" );
#endif
    }
    buf.close();
}


void QRegion::exec()
{
    QBuffer buf( data->bop );
    QDataStream s( &buf );
    buf.open( IO_ReadOnly );
    QRegion rgn;
#if defined(DEBUG)
    int test_cnt = 0;
#endif
    while ( !s.eos() ) {
	int id;
	s >> id;
#if defined(DEBUG)
	if ( test_cnt > 0 && id != QRGN_MOVE )
	    warning( "QRegion: Internal exec error" );
	test_cnt++;
#endif
	if ( id == QRGN_SETRECT || id == QRGN_SETELLIPSE ) {
	    QRect r;
	    s >> r;
	    rgn = QRegion( r, id == QRGN_SETRECT ? Rectangle : Ellipse );
	}
	else if ( id == QRGN_SETPTARRAY ) {
	    QPointArray a;
	    s >> a;
	    rgn = QRegion( a );
	}
	else if ( id == QRGN_MOVE ) {
	    QPoint p;
	    s >> p;
	    rgn = *this;
	    rgn.move( p.x(), p.y() );
	}
	else if ( id >= QRGN_OR && id <= QRGN_XOR ) {
	    QByteArray bop1, bop2;
	    s >> bop1;
	    s >> bop2;
	    QRegion r1, r2;
	    r1.data->bop = bop1;
	    r2.data->bop = bop2;
	    r1.exec();
	    r2.exec();
	    switch ( id ) {
		case QRGN_OR:
		    rgn = r1.unite( r2 );
		    break;
		case QRGN_AND:
		    rgn = r1.intersect( r2 );
		    break;
		case QRGN_SUB:
		    rgn = r1.subtract( r2 );
		    break;
		case QRGN_XOR:
		    rgn = r1.xor( r2 );
		    break;
	    }
	}
    }
    buf.close();
    *this = rgn;
}


// --------------------------------------------------------------------------
// QRegion stream functions
//

QDataStream &operator<<( QDataStream &s, const QRegion &r )
{
    return s << r.data->bop;
}

QDataStream &operator>>( QDataStream &s, QRegion &r )
{
    QRegion newr;
    QByteArray b;
    s >> b;
    newr.data->bop = b;
    newr.exec();
    r = newr;
    return s;
}
