/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrect.cpp#10 $
**
** Implementation of QRect class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QRECT_C
#include "qrect.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qrect.cpp#10 $";
#endif

#undef min
#undef max

static inline QCOORD min( QCOORD a, QCOORD b )	// replaces min macro
{
    return a < b ? a : b;
}

static inline QCOORD max( QCOORD a, QCOORD b )	// replaces max macro
{
    return a > b ? a : b;
}


// --------------------------------------------------------------------------
// QRect member functions
//

QRect::QRect( const QPoint &topleft, const QPoint &bottomright )
{
    x1 = topleft.x();
    y1 = topleft.y();
    x2 = bottomright.x();
    y2 = bottomright.y();
}

QRect::QRect( const QPoint &topleft, const QSize &size )
{
    x1 = topleft.x();
    y1 = topleft.y();
    x2 = x1+size.width()-1;
    y2 = y1+size.height()-1;
}

QRect::QRect( QCOORD left, QCOORD top, QCOORD width, QCOORD height )
{
    x1 = left;
    y1 = top;
    x2 = left+width-1;
    y2 = top+height-1;
}

bool QRect::isNull() const
{
    return x2 == x1-1 && y2 == y1-1;
}

bool QRect::isEmpty() const
{
    return x1 > x2 || y1 > y2;
}

bool QRect::isValid() const
{
    return x1 <= x2 && y1 <= y2;
}

void QRect::fixup()
{
    QCOORD t;
    if ( x2 < x1 ) {				// swap bad x values
	t = x1;
	x1 = x2;
	x2 = t;
    }
    if ( y2 < y1 ) {				// swap bad y values
	t = y1;
	y1 = y2;
	y2 = t;
    }
}

void QRect::setLeft( QCOORD pos )
{
    x1=pos;
}

void QRect::setTop( QCOORD pos )
{
    y1=pos;
}

void QRect::setX( QCOORD x )
{
    x1 = x;
}

void QRect::setY( QCOORD y )
{
    y1 = y;
}

void QRect::setRight( QCOORD pos )
{
    x2=pos;
}

void QRect::setBottom( QCOORD pos )
{
    y2=pos;
}

QPoint QRect::topLeft() const
{
    return QPoint( x1, y1 );
}

QPoint QRect::bottomRight() const
{
    return QPoint( x2, y2 );
}

QPoint QRect::topRight() const
{
    return QPoint( x2, y1 );
}

QPoint QRect::bottomLeft() const
{
    return QPoint( x1, y2 );
}

QPoint QRect::center() const
{
    return QPoint( (x1+x2)/2, (y1+y2)/2 );
}

void QRect::rect( int *x, int *y, int *w, int *h ) const
{
    *x = x1;
    *y = y1;
    *w = x2-x1+1;
    *h = y2-y1+1;
}

void QRect::coords( int *xp1, int *yp1, int *xp2, int *yp2 ) const
{
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

void QRect::setTopLeft( const QPoint &p )
{
    x2 += (p.x() - x1);
    y2 += (p.y() - y1);
    x1 = p.x();
    y1 = p.y();
}

void QRect::setBottomRight( const QPoint &p )
{
    x1 += (p.x() - x2);
    y1 += (p.y() - y2);
    x2 = p.x();
    y2 = p.y();
}

void QRect::setTopRight( const QPoint &p )
{
    x1 += (p.x() - x2);
    y2 += (p.y() - y1);
    x2 = p.x();
    y1 = p.y();
}

void QRect::setBottomLeft( const QPoint &p )
{
    x2 += (p.x() - x1);
    y1 += (p.y() - y2);
    x1 = p.x();
    y2 = p.y();
}

void QRect::setCenter( const QPoint &p )
{
    QCOORD w = x2 - x1;
    QCOORD h = y2 - y1;
    x1 = p.x() - w/2;
    y1 = p.y() - h/2;
    x2 = x1 + w;
    y2 = y1 + h;
}

void QRect::move( int dx, int dy )
{
    x1 += dx;
    y1 += dy;
    x2 += dx;
    y2 += dy;
}

void QRect::setRect( int x, int y, int w, int h )
{
    x1 = x;
    y1 = y;
    x2 = x+w-1;
    y2 = y+h-1;
}

void QRect::setCoords( int xp1, int yp1, int xp2, int yp2 )
{
    x1 = xp1;
    y1 = yp1;
    x2 = xp2;
    y2 = yp2;
}

QSize QRect::size() const
{
    return QSize( x2-x1+1, y2-y1+1 );
}

QCOORD QRect::width() const
{
    return x2-x1+1;
}

QCOORD QRect::height() const
{
    return y2-y1+1;
}

void QRect::setSize( const QSize &s )
{
    x2 = x1+s.width()-1;
    y2 = y1+s.height()-1;
}

bool QRect::contains( const QPoint &p, bool proper ) const
{
    if ( proper )
	return p.x() > x1 && p.x() < x2 &&
	       p.y() > y1 && p.y() < y2;
    else
	return p.x() >= x1 && p.x() <= x2 &&
	       p.y() >= y1 && p.y() <= y2;
}

bool QRect::contains( const QRect &r, bool proper ) const
{
    if ( proper )
	return r.x1 > x1 && r.x2 < x2 && r.y1 > y1 && r.y2 < y2;
    else
	return r.x1 >= x1 && r.x2 <= x2 && r.y1 >= y1 && r.y2 <= y2;
}

QRect QRect::unite( const QRect &r ) const
{
    QRect tmp;
    tmp.setLeft( min( x1, r.x1 ) );
    tmp.setRight( max( x2, r.x2 ) );
    tmp.setTop( min( y1, r.y1 ) );
    tmp.setBottom( max( y2, r.y2 ) );
    return tmp;
}

QRect QRect::intersect(const QRect &r ) const
{
    QRect tmp;
    tmp.x1 = max( x1, r.x1 );
    tmp.x2 = min( x2, r.x2 );
    tmp.y1 = max( y1, r.y1 );
    tmp.y2 = min( y2, r.y2 );
    return tmp;
}

bool QRect::intersects(const QRect &r ) const
{
    return ( max( x1, r.x1 ) <= min( x2, r.x2 ) && 
             max( y1, r.y1 ) <= min( y2, r.y2 ) );
}

bool operator==( const QRect &r1, const QRect &r2 )
{
    return r1.x1==r2.x1 && r1.x2==r2.x2 && r1.y1==r2.y1 && r1.y2==r2.y2;
}

bool operator!=( const QRect &r1, const QRect &r2 )
{
    return r1.x1!=r2.x1 || r1.x2!=r2.x2 || r1.y1!=r2.y1 || r1.y2!=r2.y2;
}


// --------------------------------------------------------------------------
// QRect stream functions
//

QDataStream &operator<<( QDataStream &s, const QRect &r )
{
    return s << (INT16)r.left() << (INT16)r.top()
	     << (INT16)r.right() << (INT16)r.bottom();
}

QDataStream &operator>>( QDataStream &s, QRect &r )
{
    INT16 x1, y1, x2, y2;
    s >> x1; s >> y1; s >> x2; s >> y2;
    r.setCoords( x1, y1, x2, y2 );
    return s;
}
