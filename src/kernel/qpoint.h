/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.h#10 $
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
    QPoint( int xpos, int ypos );

    bool   isNull()	const;

    int	   x()		const;
    int	   y()		const;
    void   setX( int x );
    void   setY( int y );

    QCOORD &rx();
    QCOORD &ry();

    QPoint &operator+=( const QPoint &p );
    QPoint &operator-=( const QPoint &p );
    QPoint &operator*=( int c );
    QPoint &operator*=( double c );
    QPoint &operator/=( int c );
    QPoint &operator/=( double c );

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
    QCOORD yp;
    QCOORD xp;
#else
    QCOORD xp;
    QCOORD yp;
#endif
};


// --------------------------------------------------------------------------
// QPoint stream functions
//

QDataStream &operator<<( QDataStream &, const QPoint & );
QDataStream &operator>>( QDataStream &, QPoint & );


// --------------------------------------------------------------------------
// QPoint member functions
//

inline QPoint::QPoint( int xpos, int ypos )
{
    xp=(QCOORD)xpos; yp=(QCOORD)ypos;
}

inline bool QPoint::isNull() const
{
    return xp == 0 && yp == 0;
}

inline int QPoint::x() const
{
    return xp;
}

inline int QPoint::y() const
{
    return yp;
}

inline void QPoint::setX( int x )
{
    xp = (QCOORD)x;
}

inline void QPoint::setY( int y )
{
    yp = (QCOORD)y;
}

inline QCOORD &QPoint::rx()
{
    return xp;
}

inline QCOORD &QPoint::ry()
{
    return yp;
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

inline bool operator==( const QPoint &p1, const QPoint &p2 )
{
    return p1.xp == p2.xp && p1.yp == p2.yp;
}

inline bool operator!=( const QPoint &p1, const QPoint &p2 )
{
    return p1.xp != p2.xp || p1.yp != p2.yp;
}

inline QPoint operator+( const QPoint &p1, const QPoint &p2 )
{
    return QPoint( p1.xp+p2.xp, p1.yp+p2.yp );
}

inline QPoint operator-( const QPoint &p1, const QPoint &p2 )
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

inline QPoint operator-( const QPoint &p )
{
    return QPoint( -p.xp, -p.yp );
}

#if !(defined(QPOINT_C) || defined(DEBUG))

//
// These "dangerous" QPoint functions are inline if DEBUG is not defined.
// The debug implementation in qpoint.cpp checks c and gives a warning
// before dividing by zero.
//

inline QPoint &QPoint::operator/=( int c )
{
    xp/=(QCOORD)c; yp/=(QCOORD)c; return *this;
}

inline QPoint &QPoint::operator/=( double c )
{
    xp=(QCOORD)(xp/c); yp=(QCOORD)(yp/c); return *this;
}

inline QPoint operator/( const QPoint &p, int c )
{
    return QPoint( p.xp/c, p.yp/c );
}

inline QPoint operator/( const QPoint &p, double c )
{
    return QPoint( (QCOORD)(p.xp/c), (QCOORD)(p.yp/c) );
}


#endif // non-debug inline functions


#endif // QPOINT_H
