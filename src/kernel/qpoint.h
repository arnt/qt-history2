/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.h#9 $
**
** Definition of QPoint class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPOINT_H
#define QPOINT_H

#include "qwindefs.h"


class QPoint					// point class
{
public:
    QPoint()	{}				// undefined init values
    QPoint( int xpos, int ypos );		// set x=xpos, y=ypos

    bool   isNull()	const	{ return xp==0 && yp==0; }

    int	   x()		const	{ return xp; }	// get x
    int	   y()		const	{ return yp; }	// get y
    void   setX( int x )	{ xp=(QCOORD)x;}// set x
    void   setY( int y )	{ yp=(QCOORD)y;}// set y

    QCOORD &rx()		{ return xp; }	// get reference to x
    QCOORD &ry()		{ return yp; }	// get reference to y

    QPoint &operator+=( const QPoint &p );	// add point
    QPoint &operator-=( const QPoint &p );	// subtract point
    QPoint &operator*=( int c );		// multiply with scalar
    QPoint &operator*=( double c );		// multiply with scalar double
    QPoint &operator/=( int c );		// divide by scalar
    QPoint &operator/=( double c );		// divide by scalar double

    friend bool	  operator==( const QPoint &, const QPoint & );
    friend bool	  operator!=( const QPoint &, const QPoint & );
    friend QPoint operator+( const QPoint &, const QPoint & );
    friend QPoint operator-( const QPoint &, const QPoint & );
    friend QPoint operator*( const QPoint &, int );
    friend QPoint operator*( int, const QPoint & );
    friend QPoint operator*( const QPoint &, double );
    friend QPoint operator*( double, const QPoint & );
    friend QPoint operator/( const QPoint &, int );
    friend QPoint operator/( const QPoint &, double );
    friend QPoint operator-( const QPoint & );

private:
#if defined(_OS_MAC_)
    QCOORD yp;					// y position
    QCOORD xp;					// x position
#else
    QCOORD xp;					// x position
    QCOORD yp;					// y position
#endif
};


// --------------------------------------------------------------------------
// QPoint stream functions
//

QDataStream &operator<<( QDataStream &, const QPoint & );
QDataStream &operator>>( QDataStream &, QPoint & );


#if !(defined(QPOINT_C) || defined(DEBUG))

// --------------------------------------------------------------------------
// QPoint member functions
//

inline QPoint::QPoint( int xpos, int ypos )
{
    xp=(QCOORD)xpos; yp=(QCOORD)ypos;
}

inline QPoint &QPoint::operator+=( const QPoint &p )
{
    xp+=p.xp; yp+=p.yp; return *this;
}

inline QPoint &QPoint::operator-=( const QPoint &p )
{
    xp-=p.xp; yp-=p.yp; return *this;
}

inline QPoint &QPoint::operator*=( int c )
{
    xp*=(QCOORD)c; yp*=(QCOORD)c; return *this;
}

inline QPoint &QPoint::operator*=( double c )
{
    xp=(QCOORD)(xp*c); yp=(QCOORD)(yp*c); return *this;
}

inline QPoint &QPoint::operator/=( int c )
{
    xp/=(QCOORD)c; yp/=(QCOORD)c; return *this;
}

inline QPoint &QPoint::operator/=( double c )
{
    xp=(QCOORD)(xp/c); yp=(QCOORD)(yp/c); return *this;
}

inline bool operator==( const QPoint & p1, const QPoint & p2 )
{
    return p1.xp == p2.xp && p1.yp == p2.yp;
}

inline bool operator!=( const QPoint & p1, const QPoint & p2 )
{
    return p1.xp != p2.xp || p1.yp != p2.yp;
}

inline QPoint operator+( const QPoint & p1, const QPoint & p2 )
{
    return QPoint( p1.xp+p2.xp, p1.yp+p2.yp );
}

inline QPoint operator-( const QPoint & p1, const QPoint & p2 )
{
    return QPoint( p1.xp-p2.xp, p1.yp-p2.yp );
}

inline QPoint operator*( const QPoint &p, int c )
{
    return QPoint( p.xp*c, p.yp*c );
}

inline QPoint operator*( int c, const QPoint &p )
{
    return QPoint( p.xp*c, p.yp*c );
}

inline QPoint operator*( const QPoint &p, double c )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

inline QPoint operator*( double c, const QPoint &p )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

inline QPoint operator/( const QPoint &p, int c )
{
    return QPoint( p.xp/c, p.yp/c );
}

inline QPoint operator/( const QPoint &p, double c )
{
    return QPoint( (QCOORD)(p.xp/c), (QCOORD)(p.yp/c) );
}

inline QPoint operator-( const QPoint &p )
{
    return QPoint( -p.xp, -p.yp );
}

#endif // inline functions


#endif // QPOINT_H
