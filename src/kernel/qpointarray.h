/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpointarray.h#7 $
**
** Definition of QPointArray class
**
** Author  : Haavard Nord
** Created : 940213
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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
    QPointData( const QPoint &p ) { x=(Qpnta_t)p.x(); y=(Qpnta_t)p.y(); }
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
    QPointArray( const QRect &r, bool closed=FALSE );
    QPointArray( int nPoints, const QCOOT *points );

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
    bool    setPoints( int nPoints, const QCOOT *points );
    bool    setPoints( int nPoints, int firstx, int firsty, ... );
    bool    putPoints( int index, int nPoints, const QCOOT *points );
    bool    putPoints( int index, int nPoints, int firstx, int firsty, ... );

    QPoint  at( uint i ) const;			// access point
    QPointVal operator[]( int i )		// get/set point
		{ return QPointVal( (QPointArray*)this, i ); }

    void    makeArc( int x, int y, int w, int h, int a1, int a2 );
    void    makeEllipse( int x, int y, int w, int h );
    QPointArray bezier();
};


// --------------------------------------------------------------------------
// QPointArray stream functions
//

QDataStream &operator<<( QDataStream &, const QPointArray & );
QDataStream &operator>>( QDataStream &, QPointArray & );


// --------------------------------------------------------------------------
// Misc. QPointArray functions
//

inline bool QPointArray::fill( const QPoint &p, int size )
{
    return fill( p.x(), p.y(), size );
}

inline void QPointArray::setPoint( uint i, const QPoint &p )
{
    setPoint( i, p.x(), p.y() );
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
