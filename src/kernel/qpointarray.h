/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpointarray.h#39 $
**
** Definition of QPointArray class
**
** Created : 940213
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPOINTARRAY_H
#define QPOINTARRAY_H

#ifndef QT_H
#include "qarray.h"
#include "qpoint.h"
#endif // QT_H


/*****************************************************************************
  QPointData struct; platform dependent element i QPointArray
 *****************************************************************************/

#if defined(_WS_WIN32_)
typedef long Qpnta_t;
#else
typedef short Qpnta_t;
#endif

struct QPointData {				// platform dependent point
    QPointData() {}
    QPointData( int xp, int yp )  { x=(Qpnta_t)xp; y=(Qpnta_t)yp; }
    QPointData( const QPoint &p ) { x=(Qpnta_t)p.x(); y=(Qpnta_t)p.y(); }
#if defined(_WS_MAC_)
    Qpnta_t y;
    Qpnta_t x;
#else
    Qpnta_t x;
    Qpnta_t y;
#endif
};


/*****************************************************************************
  QPointVal class; a context class for QPointArray::operator[]
 *****************************************************************************/

class QPointArray;

class Q_EXPORT QPointVal
{
public:
    QPointVal( QPointData *ptr ) : p(ptr) {}
    bool operator==( const QPointVal &point ) const;
    bool operator!=( const QPointVal &point ) const;
    QPointVal &operator=( const QPointVal &point );
    QPointVal &operator=( const QPoint &point );
    QPointVal &operator+=( const QPoint &point );
    QPointVal &operator-=( const QPoint &point );
	       operator QPoint() const	{ return QPoint(p->x,p->y); }
    int	       x() const		{ return (int)p->x; }
    int	       y() const		{ return (int)p->y; }
private:
    QPointData *p;
};


/*****************************************************************************
  QPointArray class
 *****************************************************************************/

#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QArray<QPointData>;
#endif

class Q_EXPORT QPointArray : public QArray<QPointData>
{
public:
    QPointArray() {}
    QPointArray( int size ) : QArray<QPointData>( size ) {}
    QPointArray( const QPointArray &a ) : QArray<QPointData>( a ) {}
    QPointArray( const QRect &r, bool closed=FALSE );
    QPointArray( int nPoints, const QCOORD *points );

    QPointArray	 &operator=( const QPointArray &a )
	{ return (QPointArray&)assign( a ); }

    bool    fill( const QPoint &p, int size = -1 );

    QPointArray copy() const
	{ QPointArray tmp; return *((QPointArray*)&tmp.duplicate(*this)); }

    void    translate( int dx, int dy );

    void    point( uint i, int *x, int *y ) const;
    QPoint  point( uint i ) const;
    void    setPoint( uint i, int x, int y );
    void    setPoint( uint i, const QPoint &p );
    bool    setPoints( int nPoints, const QCOORD *points );
    bool    setPoints( int nPoints, int firstx, int firsty, ... );
    bool    putPoints( int index, int nPoints, const QCOORD *points );
    bool    putPoints( int index, int nPoints, int firstx, int firsty, ... );

    QPoint  at( uint i ) const;
    QPointVal operator[]( int i )
		{ return QPointVal( data()+i ); }
    QPointVal operator[]( uint i )
		{ return QPointVal( data()+i ); }
    QPoint operator[]( int i ) const
		{ return (QPoint)QPointVal( data()+i ); }
    QPoint operator[]( uint i ) const
		{ return (QPoint)QPointVal( data()+i ); }

    QRect   boundingRect() const;

    void    makeArc( int x, int y, int w, int h, int a1, int a2 );
    void    makeEllipse( int x, int y, int w, int h );
    void    makeArc( int x, int y, int w, int h, int a1, int a2, const QWMatrix& );

    QPointArray quadBezier() const;
};


/*****************************************************************************
  QPointArray stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPointArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPointArray & );


/*****************************************************************************
  Misc. QPointArray functions
 *****************************************************************************/

inline void QPointArray::setPoint( uint i, const QPoint &p )
{
    setPoint( i, p.x(), p.y() );
}

inline bool QPointVal::operator==( const QPointVal &pointval ) const
{
    return p->x == pointval.p->x && p->y == pointval.p->y;
}

inline bool QPointVal::operator!=( const QPointVal &pointval ) const
{
    return p->x != pointval.p->x && p->y != pointval.p->y;
}

inline QPointVal &QPointVal::operator=( const QPointVal &pointval )
{
    p->x = pointval.p->x;
    p->y = pointval.p->y;
    return *this;
}

inline QPointVal &QPointVal::operator=( const QPoint &point )
{
    p->x = (Qpnta_t)point.x();
    p->y = (Qpnta_t)point.y();
    return *this;
}

inline QPointVal &QPointVal::operator+=( const QPoint &point )
{
    p->x += (Qpnta_t)point.x();
    p->y += (Qpnta_t)point.y();
    return *this;
}

inline QPointVal &QPointVal::operator-=( const QPoint &point )
{
    p->x -= (Qpnta_t)point.x();
    p->y -= (Qpnta_t)point.y();
    return *this;
}


#endif // QPOINTARRAY_H
