/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpntarry.cpp#10 $
**
** Implementation of QPointArray class
**
** Author  : Haavard Nord
** Created : 940213
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpntarry.h"
#include "qrect.h"
#include "qbitarry.h"
#include "qdstream.h"
#include <stdarg.h>
#if !defined(_WS_X11_)
#include <math.h>
#else
double qsincos( double, bool calcCos );		// def. in qptr_x11.cpp
#endif

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpntarry.cpp#10 $";
#endif


// --------------------------------------------------------------------------
// QPointArray class
//

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

QPointArray::QPointArray( int nPoints, const QCOOT *points )
{
    setPoints( nPoints, points );
}


bool QPointArray::fill( int x, int y, int size )// fill array with point value
{
    QPointData p( x, y );
    return QArrayM(QPointData)::fill( p, size );
}


void QPointArray::move( int dx, int dy )	// add dx,dy to all points
{
    register QPointData *p = data();
    register int i = size();
    while ( i-- ) {
	p->x += (Qpnta_t)dx;
	p->y += (Qpnta_t)dy;
	p++;
    }
}


void QPointArray::point( uint i, int *x, int *y ) const
{						// get i'th point in array
    QPointData p = QArrayM(QPointData)::at( i );
    *x = (int)p.x;
    *y = (int)p.y;
}

QPoint QPointArray::point( uint i ) const	// get i'th point in array
{
    QPointData p = QArrayM(QPointData)::at( i );
    return QPoint( (QCOOT)p.x, (QCOOT)p.y );
}

void QPointArray::setPoint( uint i, int x, int y )
{						// set i'th point in array
    QPointData p;
    p.x = (Qpnta_t)x;
    p.y = (Qpnta_t)y;
    QArrayM(QPointData)::at( i ) = p;
}

bool QPointArray::setPoints( int nPoints, const QCOOT *points )
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

bool QPointArray::putPoints( int index, int nPoints, const QCOOT *points )
{
    if ( index + nPoints > size() ) {		// extend array
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

bool QPointArray::putPoints( int index, int nPoints, int firstx, int firsty,
			     ... )
{
    va_list ap;
    if ( index + nPoints > size() ) {		// extend array
	if ( !resize( index + nPoints ) )
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


QPoint QPointArray::at( uint i ) const		// get i'th point in array
{
    QPointData p = QArrayM(QPointData)::at( i );
    return QPoint( (QCOOT)p.x, (QCOOT)p.y );
}


#if defined(_WS_X11_)
inline double qsin( double d ) { return qsincos(d,TRUE); }
inline double qcos( double d ) { return qsincos(d,FALSE); }
#else
#define qsin sin
#define qcos cos
#endif

static inline int fix_angle( int a )
{
    if ( a > 16*360 )
	a %= 16*360;
    else if ( a < -16*360 )
	a = -((-a) % 16*360);
    return a;
}

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
    i = a1*size()/(16*360);	// THIS IS NOT SUFFICIENT. SCAN FOR START AND
    if ( a2 > 0 ) {		// STOP VALUES !!!
	j = 0;
	inc = 1;
    }
    else {
	j = npts - 1;
	inc = -1;
    }
#if 0
    if ( a1 == 90*16 ) {
	debug( "a1=%d, a2=%d, a3=%d, size=%d, npts=%d, i=%d",
	       a1,a2,a3,size(),npts,i);
    }
#endif
    while ( npts-- ) {
	if ( i >= size() )			// wrap index
	    i = 0;
	a.QArrayM(QPointData)::at( j ) = QArrayM(QPointData)::at( i );
	i++;
	j += inc;
    }
    *this = a;
    return;
}


static inline int d2i_round( double d )
{
    return d > 0 ? int(d+0.5) : int(d-0.5);
}

#if 1	/* bresenham */
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
    double a=0.5*w, b=0.5*h;
    double a2=a*a,  b2=b*b;
    x = 0;
    y = int(b);
    d1 = b2 - a2*b + 0.25*a2;
    px[i] = x;
    py[i] = y;
    i++;
    while ( a2*(y-0.5) > b2*(x+0.5) ) {		// region 1
	if ( d1 < 0 ) {
	    d1 = d1 + b2*(3.0+2*x);
	    x++;
	}
	else {
	    d1 = d1 + b2*(3.0+2*x) + 2.0*a2*(1-y);
	    x++;
	    y--;
	}
	px[i] = x;
	py[i] = y;
	i++;
    }
    double t1 = x+0.5;
    double t2 = y-1;
    d2 = b2*t1*t1 + a2*t2*t2 - a2*b2;
    while ( y > 0 ) {				// region 2
	if ( d2 < 0 ) {
	    d2 = d2 + 2.0*b2*(x+1) + a2*(3-2*y);
	    x++;
	    y--;
	}
	else {
	    d2 = d2 + a2*(3-2*y);
	    y--;
	}
	px[i] = x;
	py[i] = y;
	i++;
    }
    if ( i > s ) {
	debug( "i > s!!!    i=%d,  w=%d, h=%d", i, w, h );
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
#else /* my own experimental */
void QPointArray::makeEllipse( int xx, int yy, int w, int h )
{
    int s = (w+h)/2;				// max size of x,y array
    int *px = new int[s];			// 1/4th of ellipse
    int *py = new int[s];
    int x, y, y2, i, a, b, a2, b2;
    a = w/2;
    b = h/2;
    a2 = a*a;
    b2 = b*b;
    i = 0;
    x = a;
    y = 0;
    px[i] = x;
    py[i] = y;
    i++;
    while ( x > 0 ) {
	x--;
	y2 = (a2*b2-x*x*b2)/a2;
	while ( y*y < y2 ) {
	    px[i] = x;
	    py[i] = y;
	    y++;
	}
	px[i] = x;
	py[i] = y;
	i++;
    }
    ASSERT( i <= s );
    s = i;
    resize( 4*s );				// make full point array
    xx += w/2;
    yy += h/2;
    for ( i=0; i<s; i++ ) {			// mirror
	x = px[i];
	y = py[i];
	setPoint( i, xx+x, yy-y );
	setPoint( 2*s-i-1, xx-x, yy-y );
	setPoint( 3*s+i, xx-x, yy+y );
	setPoint( 4*s-i-1, xx+x, yy+y );
    }
    delete[] px;
    delete[] py;
}
#endif


const max_bezcontrols = 20;			// max Bezier control points

//
// We're using Pascal's triangle to calculate the binomial
// coefficients C(n,k) once.
// The advantages are that it is much faster than calculating on demand,
// and that we don't get any numerical overflow.
//

const max_bico  = max_bezcontrols;		// max binomial coefficient n
const num_bicos = max_bico*(max_bico+1)/2;	// 1+2+3+...+max_bico
static long bicot[num_bicos];			// Pascal's triangle

#define BICO(n,k) bicot[ (n)*((n)+1)/2 + (k) ];

static void init_bicot()			// initialize Pascal's triangle
{
    static bool initialized = FALSE;
    if ( initialized )
	return;
    initialized = TRUE;
    long *p, *c;
    int n, k;
    memset( bicot, sizeof(bicot), 0 );
    for ( n=0; n<max_bico; n++ ) {		// fill edges with 1's
	c = &BICO(n,0);
	c[0] = c[n] = 1;
    }
    for ( n=2; n<max_bico; n++ ) {		// compute sums
	p = &BICO(n-1,0);
	c = &BICO(n,1);
	for ( k=1; k<n; k++ ) {
	    *c++ = *p + *(p+1);
	    p++;
	}
    }
}


QPointArray QPointArray::bezier()		// calculate Bezier curve
{
    if ( size() <= 2 || size() > max_bezcontrols ) {
	QPointArray p;
	if ( size() == 2 )			// trivial
	    p = copy();
	return p;
    }
    int n = size() - 1;				// n + 1 control points
    int m = 0;					// m = # Bezier points
    for ( int q=0; q<n; q++ ) {
	int x1, y1, x2, y2;
	point( q,   &x1, &y1 );
	point( q+1, &x2, &y2 );
	x1 -= x2;
	y1 -= y2;
	if ( x1 < 0 ) x1 = -x1;
	if ( y1 < 0 ) y1 = -y1;
	m += x1 > y1 ? x1 : y1;			// minimal # Bezier points
    }
    QPointArray p( m );				// p = Bezier point array
    m--;
    double bv,u;
    init_bicot();
    long *bico = &BICO(n,0);
    double uv1[max_bezcontrols];		// contains: uv1[i] = u^i
    double uv2[max_bezcontrols];		// contains: uv2[i] = (1-u)^i
    double xvec[max_bezcontrols];
    double yvec[max_bezcontrols];
    uv1[0] = uv2[0] = 1;
    for ( int v=0; v<=n; v++ ) {		// store all x,y in xvec,yvec
	int x, y;
	point( v, &x, &y );
	xvec[v] = x;
	yvec[v] = y;
    }
    double xf, yf;
    int	   i, b, k;
    QPointData *pd = p.data();
    for ( i=0; i<=m; i++ ) {			// for each Bezier point...
	u = (double)i/m;
	for ( b=1; b<=n; b++ ) {		// compute u^1, u^2, ... u^n
	    uv1[b] = u*uv1[b-1];		//   and (1-u)^1, ... (1-u)^n
	    uv2[b] = (1.0-u)*uv2[b-1];
	}
	xf = yf = 0;
	for ( k=0; k<=n; k++ ) {		// add control point influence
	    bv = uv1[k]*uv2[n-k]*bico[k];	// compute blending value
	    xf += bv*xvec[k];
	    yf += bv*yvec[k];
	}
	pd->x = (Qpnta_t)(xf + (xf > 0 ? 0.5 : -0.5));
	pd->y = (Qpnta_t)(yf + (yf > 0 ? 0.5 : -0.5));
	pd++;
    }
    return p;
}


// --------------------------------------------------------------------------
// QPointArray stream functions
//

QDataStream &operator<<( QDataStream &s, const QPointArray &a )
{
    register uint i;
    uint len = a.size();
    s << len;					// write size of array
    for ( i=0; i<len; i++ )			// write each point
	s << a.point( i );
    return s;
}

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
