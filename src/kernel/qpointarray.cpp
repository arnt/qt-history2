/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpointarray.cpp#2 $
**
** Implementation of QPointArray class
**
** Author  : Haavard Nord
** Created : 940213
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#include "qpntarry.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpointarray.cpp#2 $";
#endif


// --------------------------------------------------------------------------
// QPointArray class
//

QPointArray::QPointArray( const QCOOT *points, int nPoints )
{
    setPoints( points, nPoints );
}


bool QPointArray::fill( int x, int y, int size )// fill array with point value
{
    QPointData p( x, y );
    return QArrayM(QPointData)::fill( p, size );
}


void QPointArray::move( int dx, int dy )	// add dx,dy to all points
{
    register QPointData *p = data();
    register int i = size();
    while ( i-- ) {
	p->x += (Qpnta_t)dx;
	p->y += (Qpnta_t)dy;
	p++;
    }
}


void QPointArray::point( uint i, int *x, int *y ) const
{						// get i'th point in array
    QPointData p = QArrayM(QPointData)::at( i );
    *x = (int)p.x;
    *y = (int)p.y;
}

QPoint QPointArray::point( uint i ) const	// get i'th point in array
{
    QPointData p = QArrayM(QPointData)::at( i );
    return QPoint( (QCOOT)p.x, (QCOOT)p.y );
}

void QPointArray::setPoint( uint i, int x, int y )
{						// set i'th point in array
    QPointData p;
    p.x = (Qpnta_t)x;
    p.y = (Qpnta_t)y;
    QArrayM(QPointData)::at( i ) = p;
}

bool QPointArray::setPoints( const QCOOT *points, int nPoints )
{
    if ( !resize( nPoints ) )			// allocate space for points
	return FALSE;
    int i = 0;
    while ( nPoints-- ) {			// make array of points
	setPoint( i++, *points, *(points+1) );
	points++;
	points++;
    }
    return TRUE;
}

QPoint QPointArray::at( uint i ) const		// get i'th point in array
{
    QPointData p = QArrayM(QPointData)::at( i );
    return QPoint( (QCOOT)p.x, (QCOOT)p.y );
}


// --------------------------------------------------------------------------
// QPointArray stream functions
//

QDataStream &operator<<( QDataStream &s, const QPointArray &a )
{
    register uint i;
    uint len = a.size();
    s << len;					// write size of array
    for ( i=0; i<len; i++ )			// write each point
	s << a.point( i );
    return s;
}

QDataStream &operator>>( QDataStream &s, QPointArray &a )
{
    register uint i;
    uint len;
    s >> len;					// read size of array
    if ( !a.resize( len ) )			// no memory
	return s;
    QPoint p;
    for ( i=0; i<len; i++ ) {			// read each point
	s >> p;
	a.setPoint( i, p );
    }
    return s;
}
