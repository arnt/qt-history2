/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.h#2 $
**
** Definition of QPoint class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QPOINT_H
#define QPOINT_H

#include "qwindefs.h"


class QPoint					// point class
{
public:
    QPoint()	{}				// undefined init values
    QPoint( QCOOT xpos, QCOOT ypos );		// set x=xpos, y=ypos
    QPoint( QCOOT xpos );			// set x=xpos, y=0

    bool   isNull()	const	{ return xp==0 && yp==0; }

    QCOOT &x()			{ return xp; }	// get/set x position
    QCOOT &y()			{ return yp; }	// get/set y position
    QCOOT  getX()	const	{ return xp; }	// x, when const is needed
    QCOOT  getY()	const	{ return yp; }	// y, when const is needed

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
    QCOOT yp;					// y position
    QCOOT xp;					// x position
#else
    QCOOT xp;					// x position
    QCOOT yp;					// y position
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

inline QPoint::QPoint( QCOOT xpos, QCOOT ypos )
{
    xp=xpos; yp=ypos;
}

inline QPoint::QPoint( QCOOT xpos )
{
    xp=xpos; yp=0;
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
    xp=(QCOOT)(xp*c); yp=(QCOOT)(yp*c); return *this;
}

inline QPoint &QPoint::operator/=( int c )
{
    xp/=c; yp/=c; return *this;
}

inline QPoint &QPoint::operator/=( float c )
{
    xp=(QCOOT)(xp/c); yp=(QCOOT)(yp/c); return *this;
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
    return QPoint( (QCOOT)(p.xp*c), (QCOOT)(p.yp*c) );
}

inline QPoint operator*( float c, const QPoint &p )
{
    return QPoint( (QCOOT)(p.xp*c), (QCOOT)(p.yp*c) );
}

inline QPoint operator/( const QPoint &p, int c )
{
    return QPoint( p.xp/c, p.yp/c );
}

inline QPoint operator/( const QPoint &p, float c )
{
    return QPoint( (QCOOT)(p.xp/c), (QCOOT)(p.yp/c) );
}

inline QPoint operator-( const QPoint &p )
{
    return QPoint( -p.xp, -p.yp );
}

#endif // inline functions


#endif // QPOINT_H
