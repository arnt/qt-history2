/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.h#6 $
**
** Definition of QPoint class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPOINT_H
#define QPOINT_H

#include "qwindefs.h"


class QPoint					// point class
{
public:
    QPoint()	{}				// undefined init values
    QPoint( QCOORD xpos, QCOORD ypos );		// set x=xpos, y=ypos

    bool   isNull()	const	{ return xp==0 && yp==0; }

    QCOORD x()		const	{ return xp; }	// get x
    QCOORD y()		const	{ return yp; }	// get y
    void   setX( QCOORD x )	{ xp=x; }	// set x
    void   setY( QCOORD y )	{ yp=y; }	// set y

    QCOORD &rx()		{ return xp; }	// get reference to x
    QCOORD &ry()		{ return yp; }	// get reference to y

    QPoint &operator+=( const QPoint &p );	// add point
    QPoint &operator-=( const QPoint &p );	// subtract point
    QPoint &operator*=( int c );		// multiply with scalar
    QPoint &operator*=( float c );		// multiply with scalar float
    QPoint &operator/=( int c );		// divide by scalar
    QPoint &operator/=( float c );		// divide by scalar float

    friend bool	  operator==( const QPoint &, const QPoint & );
    friend bool	  operator!=( const QPoint &, const QPoint & );
    friend QPoint operator+( const QPoint &, const QPoint & );
    friend QPoint operator-( const QPoint &, const QPoint & );
    friend QPoint operator*( const QPoint &, int );
    friend QPoint operator*( int, const QPoint & );
    friend QPoint operator*( const QPoint &, float );
    friend QPoint operator*( float, const QPoint & );
    friend QPoint operator/( const QPoint &, int );
    friend QPoint operator/( const QPoint &, float );
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

inline QPoint::QPoint( QCOORD xpos, QCOORD ypos )
{
    xp=xpos; yp=ypos;
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
    xp*=c; yp*=c; return *this;
}

inline QPoint &QPoint::operator*=( float c )
{
    xp=(QCOORD)(xp*c); yp=(QCOORD)(yp*c); return *this;
}

inline QPoint &QPoint::operator/=( int c )
{
    xp/=c; yp/=c; return *this;
}

inline QPoint &QPoint::operator/=( float c )
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

inline QPoint operator*( const QPoint &p, float c )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

inline QPoint operator*( float c, const QPoint &p )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

inline QPoint operator/( const QPoint &p, int c )
{
    return QPoint( p.xp/c, p.yp/c );
}

inline QPoint operator/( const QPoint &p, float c )
{
    return QPoint( (QCOORD)(p.xp/c), (QCOORD)(p.yp/c) );
}

inline QPoint operator-( const QPoint &p )
{
    return QPoint( -p.xp, -p.yp );
}

#endif // inline functions


#endif // QPOINT_H
