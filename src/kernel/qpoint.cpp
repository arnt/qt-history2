/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.cpp#1 $
**
** Implementation of QPoint class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#define QPOINT_C
#include "qpoint.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpoint.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// QPoint member functions
//

QPoint::QPoint( QCOOT xpos, QCOOT ypos )
{
    xp=xpos; yp=ypos;
}

QPoint::QPoint( QCOOT xpos )
{
    xp=xpos; yp=0;
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
    xp=(QCOOT)(xp*c); yp=(QCOOT)(yp*c); return *this;
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
    xp=(QCOOT)(xp/c); yp=(QCOOT)(yp/c); return *this;
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
    return QPoint( (QCOOT)(p.xp*c), (QCOOT)(p.yp*c) );
}

QPoint operator*( float c, const QPoint &p )
{
    return QPoint( (QCOOT)(p.xp*c), (QCOOT)(p.yp*c) );
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
    return QPoint( (QCOOT)(p.xp/c), (QCOOT)(p.yp/c) );
}

QPoint operator-( const QPoint &p )
{
    return QPoint( -p.xp, -p.yp );
}


// --------------------------------------------------------------------------
// QPoint stream functions
//

#include "qstream.h"

QStream &operator<<( QStream &s, const QPoint &p )
{
    return s << (INT16)p.getX() << (INT16)p.getY();
}

QStream &operator>>( QStream &s, QPoint &p )
{
    INT16 x, y;
    s >> x; p.x() = x;
    s >> y; p.y() = y;
    return s;
}
