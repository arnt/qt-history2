/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpntarry.h#28 $
**
** Definition of QPointArray class
**
** Created : 940213
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPNTARRY_H
#define QPNTARRY_H

#include "qarray.h"
#include "qpoint.h"


/*****************************************************************************
  QPointData struct; platform dependent element i QPointArray
 *****************************************************************************/

#if defined(_WS_WIN32_) || defined(_WS_PM_)
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

class QPointVal
{
public:
    QPointVal( QPointData *ptr ) : p(ptr) {}
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

Q_DECLARE(QArrayM,QPointData);

class QPointArray : public QArrayM(QPointData)
{
public:
    QPointArray() {}
    QPointArray( int size ) : QArrayM(QPointData)( size ) {}
    QPointArray( const QPointArray &a ) : QArrayM(QPointData)( a ) {}
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

    QPointArray quadBezier() const;
};


/*****************************************************************************
  QPointArray stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QPointArray & );
QDataStream &operator>>( QDataStream &, QPointArray & );


/*****************************************************************************
  Misc. QPointArray functions
 *****************************************************************************/

inline void QPointArray::setPoint( uint i, const QPoint &p )
{
    setPoint( i, p.x(), p.y() );
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


#endif // QPNTARRY_H
