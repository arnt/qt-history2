/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgvector.h#14 $
**
** Definition of QGVector class
**
** Created : 930907
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGVECTOR_H
#define QGVECTOR_H

#ifndef QT_H
#include "qcollect.h"
#endif // QT_H


class QGVector : public QCollection		// generic vector
{
friend class QGList;				// needed by QGList::toVector
public:
    QDataStream &read( QDataStream & );		// read vector from stream
    QDataStream &write( QDataStream & ) const;	// write vector to stream

    virtual int compareItems( GCI, GCI );

protected:
    QGVector();					// create empty vector
    QGVector( uint size );			// create vector with nullptrs
    QGVector( const QGVector &v );		// make copy of other vector
   ~QGVector();

    QGVector &operator=( const QGVector &v );	// assign from other vector

    GCI	 *data()    const	{ return vec; }
    uint  size()    const	{ return len; }
    uint  count()   const	{ return numItems; }

    bool  insert( uint index, GCI );		// insert item at index
    bool  remove( uint index );			// remove item
    GCI	  take( uint index );			// take out item

    void  clear();				// clear vector
    bool  resize( uint newsize );		// resize vector

    bool  fill( GCI, int flen );		// resize and fill vector

    void  sort();				// sort vector
    int	  bsearch( GCI ) const;			// binary search (when sorted)

    int	  findRef( GCI, uint index ) const;	// find exact item in vector
    int	  find( GCI, uint index ) const;	// find equal item in vector
    uint  containsRef( GCI ) const;		// get number of exact matches
    uint  contains( GCI ) const;		// get number of equal matches

    GCI	  at( uint index ) const
#if defined(CHECK_RANGE) || defined(QGVECTOR_CPP)
	;					// safe (impl. in qcvector.cpp)
#else
	{ return vec[index]; }			// fast
#endif
    bool insertExpand( uint index, GCI );	// insert, expand if necessary

    void toList( QGList * ) const;		// put items in list

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    GCI	 *vec;
    uint  len;
    uint  numItems;
};


/*****************************************************************************
  QGVector stream functions
 *****************************************************************************/

QDataStream &operator>>( QDataStream &, QGVector & );
QDataStream &operator<<( QDataStream &, const QGVector & );


#endif // QGVECTOR_H
