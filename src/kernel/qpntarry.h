/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpntarry.h#1 $
**
** Definition of QPointArray class
**
** Author  : Haavard Nord
** Created : 940213
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#ifndef QPNTARRY_H
#define QPNTARRY_H

#include "qarray.h"
#include "qpoint.h"


// --------------------------------------------------------------------------
// QPointData struct; platform dependent element i QPointArray
//

#if defined(_WS_WIN32_) || defined(_WS_PM_)
typedef long Qpnta_t;
#else
typedef short Qpnta_t;
#endif

struct QPointData {				// platform dependent point
    QPointData() {}
    QPointData( int xp, int yp )  { x=(Qpnta_t)xp; y=(Qpnta_t)yp; }
    QPointData( const QPoint &p ) { x=(Qpnta_t)p.getX(); y=(Qpnta_t)p.getY(); }
#if defined(_OS_MAC_)
    Qpnta_t y;
    Qpnta_t x;
#else
    Qpnta_t x;
    Qpnta_t y;
#endif
};


// --------------------------------------------------------------------------
// QPointVal class; a context class for QPointArray::operator[]
//

class QPointArray;

class QPointVal
{
private:
    QPointArray *array;
    uint    index;
public:
    QPointVal( QPointArray *a, uint i ) : array(a), index(i) {}
	    operator QPoint();
    QPointVal &operator=( const QPointVal &v );
    QPointVal &operator=( const QPoint &p );
};


// --------------------------------------------------------------------------
// QPointArray class
//

declare(QArrayM,QPointData);

class QPointArray : public QArrayM(QPointData)
{
public:
    QPointArray() {}
    QPointArray( uint size ) : QArrayM(QPointData)( size ) {}
    QPointArray( const QPointArray &a ) : QArrayM(QPointData)( a ) {}
    QPointArray( const QCOOT *points, int nPoints );

    QPointArray	 &operator=( const QPointArray &a )
	{ return (QPointArray&)assign( a ); }

    bool    fill( int x, int y, int size = -1 );
    bool    fill( const QPoint &p, int size = -1 );

    QPointArray copy() const			// make deep copy
	{ QPointArray tmp; return *((QPointArray*)&tmp.duplicate(*this)); }

    void    move( int dx, int dy );		// move offset

    void    point( uint i, int *x, int *y ) const;
    QPoint  point( uint i ) const;
    void    setPoint( uint i, int x, int y );
    void    setPoint( uint i, const QPoint &p );
    bool    setPoints( const QCOOT *points, int nPoints );

    QPoint  at( uint i ) const;			// access point
    QPointVal operator[]( int i )		// get/set point
		{ return QPointVal( (QPointArray*)this, i ); }
};


// --------------------------------------------------------------------------
// QPointArray stream functions
//

QStream &operator<<( QStream &, const QPointArray & );
QStream &operator>>( QStream &, QPointArray & );


// --------------------------------------------------------------------------
// Misc. QPointArray functions
//

inline bool QPointArray::fill( const QPoint &p, int size )
{
    return fill( p.getX(), p.getY(), size );
}

inline void QPointArray::setPoint( uint i, const QPoint &p )
{
    setPoint( i, p.getX(), p.getY() );
}

inline QPointVal::operator QPoint()
{
    return array->point( index );
}

inline QPointVal &QPointVal::operator=( const QPointVal &v )
{
    array->setPoint( index, v.array->point(v.index) );
    return *this;
}

inline QPointVal &QPointVal::operator=( const QPoint &p )
{
    array->setPoint( index, p );
    return *this;
}


#endif // QPNTARRY_H
