/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpntarry.cpp#47 $
**
** Implementation of QPointArray class
**
** Created : 940213
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpntarry.h"
#include "qrect.h"
#include "qbitarry.h"
#include "qdstream.h"
#include <stdarg.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qpntarry.cpp#47 $");

/*!
  \class QPointVal qpntarry.h
  \brief The QPointVal class is an internal class, used with QPointArray.

  The QPointVal is required by the indexing [] operator on point arrays.
  It is probably a bad idea to use it in any other context.
*/

/*! \fn QPointVal::QPointVal (QPointData* ptr) 

  Constructs a reference to an element in a QPointArray.  This is
  what QPointArray::operator[] contructs its return value with.
*/

/*! \fn QPointVal& QPointVal::operator= (const QPoint& point) 
  Assigns a given point to the value referenced by this QPointVal.
*/

/*! \fn QPointVal& QPointVal::operator+= (const QPoint& point) 
  Vector-adds a given point to the value referenced by this QPointVal.
*/

/*! \fn QPointVal& QPointVal::operator-= (const QPoint& point) 
  Vector-subtracts a given point to the value referenced by this QPointVal.
*/

/*! \fn QPointVal::operator QPoint() const
  Returns the value of the point referenced by this QPointVal.
*/

/*! \fn int QPointVal::x () const 
  Returns the X coordinate of the point referenced by this QPointVal.
*/

/*! \fn int QPointVal::y () const 
  Returns the Y coordinate of the point referenced by this QPointVal.
*/


/*!
  \class QPointArray qpntarry.h
  \brief The QPointArray class provides an array of points.

  \inherit QArray

  QPointArray is used by the QPainter to draw
  \link QPainter::drawLineSegments() line segments\endlink,
  \link QPainter::drawPolyline() polylines\endlink,
  \link QPainter::drawPolygon() polygons\endlink and
  \link QPainter::drawQuadBezier() Bezier curves\endlink.

  The QPointArray is not an array of QPoint, instead it contains a
  platform dependent point type to make the QPainter functions more
  efficient (no conversion needed).  On the other hand, QPointArray
  has member functions that operate on QPoint to make programming
  easier.

  Note that since this class is a QArray, it is
  \link shclass.html explicitly shared\endlink
  and works with shallow copies by default.
*/


/*****************************************************************************
  QPointArray member functions
 *****************************************************************************/

/*!
  \fn QPointArray::QPointArray()
  Constructs a null point array.
  \sa isNull()
*/

/*!
  \fn QPointArray::QPointArray( int size )
  Constructs a point array with room for \e size points.
  Makes a null array if \e size == 0.

  \sa resize(), isNull()
*/

/*!
  \fn QPointArray::QPointArray( const QPointArray &a )
  Constructs a 
  \link shclass.html shallow copy\endlink of the point array \e a.

  \sa copy()
*/

/*!
  Constructs a point array from the rectangle \e r.

  If \e closed is FALSE, then the point array will contain the
  following four points (in the listed order):
  <ol>
  <li> r.topLeft()
  <li> r.topRight()
  <li> r.bottomRight()
  <li> r.bottomLeft()
  </ol>

  If \e closed is TRUE, then a fifth point is set to r.topLeft() to
  close the point array.
*/

QPointArray::QPointArray( const QRect &r, bool closed )
{
    setPoints( 4, r.left(),  r.top(),
		  r.right(), r.top(),
		  r.right(), r.bottom(),
		  r.left(),  r.bottom() );
    if ( closed ) {
	resize( 5 );
	setPoint( 4, r.left(), r.top() );
    }
}

/*!
  Constructs a point array with \e nPoints points, taken from the
  \e points array.

  Equivalent to setPoints(nPoints,points).
*/

QPointArray::QPointArray( int nPoints, const QCOORD *points )
{
    setPoints( nPoints, points );
}


/*!
  \fn QPointArray &QPointArray::operator=( const QPointArray &a )
  Assigns a 
  \link shclass.html shallow copy\endlink of \e a to this point array
  and returns a reference to this point array.

  Equivalent to assign( a ).

  \sa copy()
*/

/*!
  \fn QPointArray QPointArray::copy() const

  Creates a 
  \link shclass.html deep copy\endlink of the array.
*/

/*!
  Fills the point array with the point \e p.
  If \e size is specified as different from -1, then the array will be
  resized before filled.

  Returns TRUE if successful, or FALSE if the memory cannot be allocated
  (only when \e size != -1).

  \sa resize()
*/

bool QPointArray::fill( const QPoint &p, int size )
{
    QPointData p2( p.x(), p.y() );
    return QArrayM(QPointData)::fill( p2, size );
}


/*!
  Translates all points in the array \e (dx,dy).
*/

void QPointArray::translate( int dx, int dy )
{
    register QPointData *p = data();
    register int i = size();
    while ( i-- ) {
	p->x += (Qpnta_t)dx;
	p->y += (Qpnta_t)dy;
	p++;
    }
}


/*!
  Returns the point at position \e index in the array in \e *x and \e *y.
*/

void QPointArray::point( uint index, int *x, int *y ) const
{
    QPointData p = QArrayM(QPointData)::at( index );
    *x = (int)p.x;
    *y = (int)p.y;
}

/*!
  Returns the point at position \e index in the array.
*/

QPoint QPointArray::point( uint index ) const
{
    QPointData p = QArrayM(QPointData)::at( index );
    return QPoint( (QCOORD)p.x, (QCOORD)p.y );
}

/*!
  Sets the point at position \e index in the array to \e (x,y).
*/

void QPointArray::setPoint( uint index, int x, int y )
{
    QPointData p;
    p.x = (Qpnta_t)x;
    p.y = (Qpnta_t)y;
    QArrayM(QPointData)::at( index ) = p;
}

/*!
  Resizes the array to \e nPoints and sets the points in the array to
  the values taken from \e points.

  Returns TRUE if successful, or FALSE if the array could not be resized.

  Example:
  \code
    static QCOORD points[] = { 1,2, 3,4 };
    QPointArray a;
    a.setPoints( 2, points );
  \endcode

  The example code creates an array with two points (1,2) and (3,4).

  \sa resize(), putPoints()
*/

bool QPointArray::setPoints( int nPoints, const QCOORD *points )
{
    if ( !resize(nPoints) )
	return FALSE;
    int i = 0;
    while ( nPoints-- ) {			// make array of points
	setPoint( i++, *points, *(points+1) );
	points++;
	points++;
    }
    return TRUE;
}

/*!
  \fn void QPointArray::setPoint( uint i, const QPoint &p )

  Equivalent to setPoint( i, p.x(), p.y() ).
*/

/*!
  Resizes the array to \e nPoints and sets the points in the array to
  the values taken from the variable argument list.

  Returns TRUE if successful, or FALSE if the array could not be resized.

  Example:
  \code
    QPointArray a;
    a.setPoints( 2, 1,2, 3,4 );
  \endcode

  The example code creates an array with two points (1,2) and (3,4).

  \sa resize(), putPoints()
*/

bool QPointArray::setPoints( int nPoints, int firstx, int firsty,
			     ... )
{
    va_list ap;
    if ( !resize(nPoints) )
	return FALSE;
    setPoint( 0, firstx, firsty );		// set first point
    int i = 1, x, y;
    nPoints--;
    va_start( ap, firsty );
    while ( nPoints-- ) {
	x = va_arg( ap, int );
	y = va_arg( ap, int );
	setPoint( i++, x, y );
    }
    va_end( ap );
    return TRUE;
}

/*!
  Copies \e nPoints points from the \e points array into this point array.
  Will resize this point array if <code>index+nPoints</code> exceeds
  the size of the array.

  Returns TRUE if successful, or FALSE if the array could not be resized.

  Example:
  \code
    QPointArray a( 1 );
    a[0] = QPoint( 1, 2 );
    static QCOORD points[] = { 3,4, 5,6 };
    a.putPoints( 1, 2, points );
  \endcode

  The example code creates an array with two points (1,2), (3,4) and (5,6).

  This function differs from setPoints() because it does not resize the
  array unless the array size is exceeded.

  \sa resize(), setPoints()
*/

bool QPointArray::putPoints( int index, int nPoints, const QCOORD *points )
{
    if ( index + nPoints > (int)size() ) {	// extend array
	if ( !resize( index + nPoints ) )
	    return FALSE;
    }
    int i = index;
    while ( nPoints-- ) {			// make array of points
	setPoint( i++, *points, *(points+1) );
	points++;
	points++;
    }
    return TRUE;
}

/*!
  Copies \e nPoints points from the variable argument list into this point
  array. Will resize this point array if <code>index+nPoints</code> exceeds
  the size of the array.

  Returns TRUE if successful, or FALSE if the array could not be resized.

  Example:
  \code
    QPointArray a( 1 );
    a[0] = QPoint( 1, 2 );
    a.putPoints( 1, 2, 3,4, 5,6 );
  \endcode

  The example code creates an array with two points (1,2), (3,4) and (5,6).

  This function differs from setPoints() because it does not resize the
  array unless the array size is exceeded.

  \sa resize(), setPoints()
*/

bool QPointArray::putPoints( int index, int nPoints, int firstx, int firsty,
			     ... )
{
    va_list ap;
    if ( index + nPoints > (int)size() ) {	// extend array
	if ( !resize(index + nPoints) )
	    return FALSE;
    }
    if ( nPoints <= 0 )
	return TRUE;
    setPoint( index, firstx, firsty );		// set first point
    int i = index + 1, x, y;
    nPoints--;
    va_start( ap, firsty );
    while ( nPoints-- ) {
	x = va_arg( ap, int );
	y = va_arg( ap, int );
	setPoint( i++, x, y );
    }
    va_end( ap );
    return TRUE;
}


/*!
  Returns the point at position \e index in the array.
  \sa operator[]
*/

QPoint QPointArray::at( uint index ) const
{
    QPointData p = QArrayM(QPointData)::at( index );
    return QPoint( (QCOORD)p.x, (QCOORD)p.y );
}

/*!
  \fn QPointVal QPointArray::operator[]( int index )

  Returns a reference to the point at position \e index in the array.
*/

/*!
  \fn QPointVal QPointArray::operator[]( uint index )

  Returns a reference to the point at position \e index in the array.
*/


/*!
  Returns the bounding rectangle of the points in the array, or
  QRect(0,0,0,0) if the array is empty.
*/

QRect QPointArray::boundingRect() const
{
    if ( isEmpty() )
	return QRect( 0, 0, 0, 0 );		// null rectangle
    register QPointData *pd = data();
    int minx, maxx, miny, maxy;
    minx = maxx = pd->x;
    miny = maxy = pd->y;
    pd++;
    for ( int i=1; i<(int)size(); i++ ) {	// find min+max x and y
	if ( pd->x < minx )
	    minx = pd->x;
	else if ( pd->x > maxx )
	    maxx = pd->x;
	if ( pd->y < miny )
	    miny = pd->y;
	else if ( pd->y > maxy )
	    maxy = pd->y;
	pd++;
    }
    return QRect( QPoint(minx,miny), QPoint(maxx,maxy) );
}


static inline int fix_angle( int a )
{
    if ( a > 16*360 )
	a %= 16*360;
    else if ( a < -16*360 )
	a = -((-a) % 16*360);
    return a;
}

/*!
  Sets the points of the array to those describing an arc of an
  ellipse with size \a w by \a h and position (\a x, \a y ), starting
  from angle \a1, spanning \a a2.
  Angles are specified in 16ths of a degree,
  i.e. a full circle equals 5760 (16*360). Positive values mean
  counter-clockwise while negative values mean clockwise direction.
  Zero degrees is at the 3'o clock position.
*/

void QPointArray::makeArc( int x, int y, int w, int h, int a1, int a2 )
{
    a1 = fix_angle( a1 );
    if ( a1 < 0 )
	a1 += 16*360;
    a2 = fix_angle( a2 );
    int a3 = a2 > 0 ? a2 : -a2;			// abs angle
    makeEllipse( x, y, w, h );
    int npts = a3*size()/(16*360);		// # points in arc array
    QPointArray a(npts);
    int i, j, inc;
    i = a1*size()/(16*360);
    if ( a2 > 0 ) {
	j = 0;
	inc = 1;
    } else {
	j = npts - 1;
	inc = -1;
    }
    while ( npts-- ) {
	if ( i >= (int)size() )			// wrap index
	    i = 0;
	a.QArrayM(QPointData)::at( j ) = QArrayM(QPointData)::at( i );
	i++;
	j += inc;
    }
    *this = a;
    return;
}

/*!
  Sets the points of the array to those describing an ellipse with
  size \a w by \a h and position (\a x, \a y ).
*/
void QPointArray::makeEllipse( int xx, int yy, int w, int h )
{						// midpoint, 1/4 ellipse
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 ) {
	    resize( 0 );
	    return;
	}
	if ( w < 0 ) {				// negative width
	    w = -w;
	    xx -= w;
	}
	if ( h < 0 ) {				// negative height
	    h = -h;
	    yy -= h;
	}
    }
    int s = (w+h+2)/2;				// max size of x,y array
    int *px = new int[s];			// 1/4th of ellipse
    int *py = new int[s];
    int x, y, i=0;
    double d1, d2;
    double a2=(w/2)*(w/2),  b2=(h/2)*(h/2);
    x = 0;
    y = int(h/2);
    d1 = b2 - a2*(h/2) + 0.25*a2;
    px[i] = x;
    py[i] = y;
    i++;
    while ( a2*(y-0.5) > b2*(x+0.5) ) {		// region 1
	if ( d1 < 0 ) {
	    d1 = d1 + b2*(3.0+2*x);
	    x++;
	} else {
	    d1 = d1 + b2*(3.0+2*x) + 2.0*a2*(1-y);
	    x++;
	    y--;
	}
	px[i] = x;
	py[i] = y;
	i++;
    }
    d2 = b2*(x+0.5)*(x+0.5) + a2*(y-1)*(y-1) - a2*b2;
    while ( y > 0 ) {				// region 2
	if ( d2 < 0 ) {
	    d2 = d2 + 2.0*b2*(x+1) + a2*(3-2*y);
	    x++;
	    y--;
	} else {
	    d2 = d2 + a2*(3-2*y);
	    y--;
	}
	px[i] = x;
	py[i] = y;
	i++;
    }
    s = i;
    resize( 4*s );				// make full point array
    xx += w/2;
    yy += h/2;
    for ( i=0; i<s; i++ ) {			// mirror
	x = px[i];
	y = py[i];
	setPoint( s-i-1, xx+x, yy-y );
	setPoint( s+i, xx-x, yy-y );
	setPoint( 3*s-i-1, xx-x, yy+y );
	setPoint( 3*s+i, xx+x, yy+y );
    }
    delete[] px;
    delete[] py;
}


/*!
  Returns the Bezier points for the four control points in this array.
*/

QPointArray QPointArray::quadBezier() const
{
    if ( size() != 4 ) {
#if defined(CHECK_RANGE)
	warning( "QPointArray::bezier: The array must have 4 control points" );
#endif
	QPointArray p;
	return p;
    }
    int v;
    const int n = 3;				// n + 1 control points
    float xvec[4];
    float yvec[4];
    for ( v=0; v<=n; v++ ) {			// store all x,y in xvec,yvec
	int x, y;
	point( v, &x, &y );
	xvec[v] = (float)x;
	yvec[v] = (float)y;
    }

    QRect r = boundingRect();
    int m = QMAX(r.width(),r.height())/2;
    m = QMIN(m,30);				// m = number of result points
    if ( m < 2 )				// at least two points
	m = 2;
    QPointArray p( m );				// p = Bezier point array
    register QPointData *pd = p.data();

    float x0 = xvec[0],	 y0 = yvec[0];
    float dt = 1.0F/m;
    float cx = 3.0F * (xvec[1] - x0);
    float bx = 3.0F * (xvec[2] - xvec[1]) - cx;
    float ax = xvec[3] - (x0 + cx + bx);
    float cy = 3.0F * (yvec[1] - y0);
    float by = 3.0F * (yvec[2] - yvec[1]) - cy;
    float ay = yvec[3] - (y0 + cy + by);
    float t = dt;

    pd->x = (Qpnta_t)xvec[0];
    pd->y = (Qpnta_t)yvec[0];
    pd++;
    m -= 2;

    while ( m-- ) {
	pd->x = (Qpnta_t)qRound( ((ax * t + bx) * t + cx) * t + x0 );
	pd->y = (Qpnta_t)qRound( ((ay * t + by) * t + cy) * t + y0 );
	pd++;
	t += dt;
    }

    pd->x = (Qpnta_t)xvec[3];
    pd->y = (Qpnta_t)yvec[3];

    return p;
}


/*****************************************************************************
  QPointArray stream functions
 *****************************************************************************/

/*!
  \relates QPointArray
  Writes a point array to the stream and returns a reference to the stream.

  The serialization format is:
  <ol>
  <li> The array size (UINT32)
  <li> The array points (QPoint)
  </ol>
*/

QDataStream &operator<<( QDataStream &s, const QPointArray &a )
{
    register uint i;
    uint len = a.size();
    s << len;					// write size of array
    for ( i=0; i<len; i++ )			// write each point
	s << a.point( i );
    return s;
}

/*!
  \relates QPointArray
  Reads a point array from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QPointArray &a )
{
    register uint i;
    uint len;
    s >> len;					// read size of array
    if ( !a.resize( len ) )			// no memory
	return s;
    QPoint p;
    for ( i=0; i<len; i++ ) {			// read each point
	s >> p;
	a.setPoint( i, p );
    }
    return s;
}
