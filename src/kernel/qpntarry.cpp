/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpntarry.cpp#3 $
**
** Implementation of QPointArray class
**
** Author  : Haavard Nord
** Created : 940213
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qpntarry.h"
#include "qrect.h"
#include "qbitarry.h"
#include "qdstream.h"
#if !defined(_WS_X11_)
#include <math.h>
#else
double qsincos( double, bool calcCos );		// def. in qptr_x11.cpp
#endif

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpntarry.cpp#3 $";
#endif


// --------------------------------------------------------------------------
// QPointArray class
//

QPointArray::QPointArray( const QRect &r )
{
    resize( 4 );
    setPoint( 0, r.left(),  r.top() );
    setPoint( 1, r.right(), r.top() );
    setPoint( 2, r.right(), r.bottom() );
    setPoint( 3, r.left(),  r.bottom() );
}

QPointArray::QPointArray( const QCOOT *points, int nPoints )
{
    setPoints( points, nPoints );
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

bool QPointArray::setPoints( const QCOOT *points, int nPoints )
{
    if ( !resize( nPoints ) )			// allocate space for points
	return FALSE;
    int i = 0;
    while ( nPoints-- ) {			// make array of points
	setPoint( i++, *points, *(points+1) );
	points++;
	points++;
    }
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

inline int adjust_angle_huff( int a )
{
    if ( a > 16*360 )
	a %= 16*360;
    else if ( a < -16*360 )
	a = -((-a) % 16*360);
    return a;
}

inline int adjust_angle( int a )
{
    if ( a > 16*360 )
	a %= 16*360;
    else if ( a < -16*360 )
	a = -((-a) % 16*360);
    if ( a < 0 )	// FJERN SENERE
	a = 16*360 + a;
    return a;
}

static const double deg16_2_rad = 3.14159265358979323/2880.0;

void QPointArray::makeArc( int x, int y, int w, int h, int a1, int a2 )
{
    a1 = adjust_angle( a1 );
    a2 = adjust_angle( a2 );
    int a3 = a2 > 0 ? a2 : -a2;			// abs angle
    makeEllipse( x, y, w, h );
    int start = a1*size()/(16*360);
    int npts = a3*size()/(16*360);
    QPointArray a(npts);
    for ( int i=0; i<npts; i++ ) {
	a.setPoint(i,point(i+start));
	if ( i + start > npts )
	    start -= npts;
    }
    *this = a;
    return;
#if 0
    int npts = (w+h)*a3/(16*360);
    resize( npts );
    if ( !npts )
	return;
    double a = a1*deg16_2_rad;
    double a_end = (a1+a2)*deg16_2_rad;
    double a_inc = (a_end-a)/npts;
    int xx, yy;
    resize( npts );
    w /= 2;
    h /= 2;
    x += w;
    y += h;
    for ( int i=0; i<npts; i++ ) {		// make elliptic point array
	xx = x + (int)(qsin(a)*w);
	yy = y - (int)(qcos(a)*h);
	setPoint( i, xx, yy );
	a += a_inc;
    }
#endif
}


void QPointArray::makeEllipse( int xx, int yy, int w, int h )
{
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
    int s = (w+h)/2;				// max size of x,y array
    int *px = new int[s];			// 1/8th of ellipse
    int *py = new int[s];
    int x, y, d, r;
    r = w > h ? w/2 : h/2;
    x = 0;
    y = r;
    d = 1 - r;
    px[x] = x;
    py[x] = y;
    while ( x < y ) {				// bresenham, 1/8 ellipse
	if ( d < 0 )
	    d += x*2 + 3;
	else {
	    d += (x-y)*2 + 5;
	    y--;
	}
	x++;
	px[x] = x;
	py[x] = y;
    }
    s = x;
    resize( 8*s );				// make full point array
    xx += w/2;
    yy += h/2;
    int dw, dh;
    dw = w & 1 ? 0 : 1;
    dh = h & 1 ? 0 : 1;
    dw = dh = 0;
    for ( int i=0; i<s; i++ ) {			// mirror
	x = px[i];
	y = py[i];
	setPoint( i, xx+y, yy-x );
	setPoint( 2*s-1-i, xx+x, yy-y );
	setPoint( 2*s+i, xx-x, yy-y );
	setPoint( 4*s-1-i, xx-y, yy-x );
	setPoint( 4*s+i, xx-y, yy+x );
	setPoint( 6*s-1-i, xx-x, yy+y );
	setPoint( 6*s+i, xx+x+dw, yy+y );
	setPoint( 8*s-1-i, xx+y+dw, yy+x );
    }
    if ( w != h ) {				// scale ellipse
	int e1, e2;
	if ( h > w ) {
	    e1 = w;
	    e2 = h;
	}
	else {
	    e1 = h;
	    e2 = w;
	}
	for ( i=0; i<size(); i++ ) {
	    point( i, &x, &y );
	    y = y < 0 ? y+yy : y-yy;
	    setPoint( i, x, y*e1/e2+yy );
	}
    }
    delete[] px;
    delete[] py;
}

void QPointArray::smoothArc()			// make smooth arc
{
    int s = size();
    if ( s < 3 )
	return;
    register QPointData *p = data();
    int i=2, nskipped=0;
    QBitArray b( s );
    while ( i < s ) {
	bool xe, ye;
	if ( (xe =(p[2].x == p[1].x)) || (ye=(p[2].y == p[1].y)) ) {
	    bool skip_point=FALSE;
	    if ( xe && ye )
		skip_point = TRUE;
	    else if ( xe ) {
		if ( p[1].y == p[0].y ) {
		    int xd = p[1].x-p[2].x;
		    skip_point = xd == 1 || xd == -1;
		}
	    }
	    else {
		if ( p[1].x == p[0].x ) {		    
		    int yd = p[1].y-p[2].y;
		    skip_point = yd == 1 || yd == -1;
		}
	    }
	    if ( skip_point ) {
		b.setBit( i-1 );
		nskipped++;
	    }
	}
	i++;
	p++;
    }
    if ( nskipped ) {
	QPointData *p2;
	p = p2 = data();
	i = 0;
	while ( i<s ) {
	    if ( b.testBit(i++) )
		*p = *p2++;
	    else
		*p++ = *p2++;
	}
	debug( "SMOOTH %d points, %d skipped", s, nskipped );
	resize( s-nskipped );
    }
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
