/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.cpp#5 $
**
** Implementation of QPoint class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QPOINT_C
#include "qpoint.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpoint.cpp#5 $";
#endif


// --------------------------------------------------------------------------
// QPoint member functions
//

QPoint::QPoint( QCOORD xpos, QCOORD ypos )
{
    xp=xpos; yp=ypos;
}

QPoint &QPoint::operator+=( const QPoint &p )
{
    xp+=p.xp; yp+=p.yp; return *this;
}

QPoint &QPoint::operator-=( const QPoint &p )
{
    xp-=p.xp; yp-=p.yp; return *this;
}

QPoint &QPoint::operator*=( int c )
{
    xp*=c; yp*=c; return *this;
}

QPoint &QPoint::operator*=( float c )
{
    xp=(QCOORD)(xp*c); yp=(QCOORD)(yp*c); return *this;
}

QPoint &QPoint::operator/=( int c )
{
    if ( c == 0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return *this;
    }
    xp/=c; yp/=c; return *this;
}

QPoint &QPoint::operator/=( float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return *this;
    }
    xp=(QCOORD)(xp/c); yp=(QCOORD)(yp/c); return *this;
}

bool operator==( const QPoint & p1, const QPoint & p2 )
{
    return p1.xp == p2.xp && p1.yp == p2.yp;
}

bool operator!=( const QPoint & p1, const QPoint & p2 )
{
    return p1.xp != p2.xp || p1.yp != p2.yp;
}

QPoint operator+( const QPoint & p1, const QPoint & p2 )
{
    return QPoint( p1.xp+p2.xp, p1.yp+p2.yp );
}

QPoint operator-( const QPoint & p1, const QPoint & p2 )
{
    return QPoint( p1.xp-p2.xp, p1.yp-p2.yp );
}

QPoint operator*( const QPoint &p, int c )
{
    return QPoint( p.xp*c, p.yp*c );
}

QPoint operator*( int c, const QPoint &p )
{
    return QPoint( p.xp*c, p.yp*c );
}

QPoint operator*( const QPoint &p, float c )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

QPoint operator*( float c, const QPoint &p )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

QPoint operator/( const QPoint &p, int c )
{
    if ( c == 0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return p;
    }
    return QPoint( p.xp/c, p.yp/c );
}

QPoint operator/( const QPoint &p, float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return p;
    }
    return QPoint( (QCOORD)(p.xp/c), (QCOORD)(p.yp/c) );
}

QPoint operator-( const QPoint &p )
{
    return QPoint( -p.xp, -p.yp );
}


// --------------------------------------------------------------------------
// QPoint stream functions
//

QDataStream &operator<<( QDataStream &s, const QPoint &p )
{
    return s << (INT16)p.x() << (INT16)p.y();
}

QDataStream &operator>>( QDataStream &s, QPoint &p )
{
    INT16 x, y;
    s >> x;  p.rx() = x;
    s >> y;  p.ry() = y;
    return s;
}
