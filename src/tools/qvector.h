/****************************************************************************
** $Id: //depot/qt/main/src/tools/qvector.h#16 $
**
** Definition of QVector template/macro class
**
** Created : 930907
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

#ifndef QVECTOR_H
#define QVECTOR_H

#ifndef QT_H
#include "qgvector.h"
#endif // QT_H


template<class type> class QVector : public QGVector
{
public:
    QVector()				{}
    QVector( uint size ) : QGVector(size) {}
    QVector( const QVector<type> &v ) : QGVector(v) {}
   ~QVector()				{ clear(); }
    QVector<type> &operator=(const QVector<type> &v)
			{ return (QVector<type>&)QGVector::operator=(v); }
    type **data()   const		{ return (type **)QGVector::data(); }
    uint  size()    const		{ return QGVector::size(); }
    uint  count()   const		{ return QGVector::count(); }
    bool  isEmpty() const		{ return QGVector::count() == 0; }
    bool  isNull()  const		{ return QGVector::size() == 0; }     \
    bool  resize( uint size )		{ return QGVector::resize(size); }
    bool  insert( uint i, const type *d){ return QGVector::insert(i,(Item)d); }
    bool  remove( uint i )		{ return QGVector::remove(i); }
    type *take( uint i )		{ return (type *)QGVector::take(i); }
    void  clear()			{ QGVector::clear(); }
    bool  fill( const type *d, int size=-1 )
					{ return QGVector::fill((Item)d,size);}
    void  sort()			{ QGVector::sort(); }
    int	  bsearch( const type *d ) const{ return QGVector::bsearch((Item)d); }
    int	  findRef( const type *d, uint i=0 ) const
					{ return QGVector::findRef((Item)d,i);}
    int	  find( const type *d, uint i= 0 ) const
					{ return QGVector::find((Item)d,i); }
    uint  containsRef( const type *d ) const
				{ return QGVector::containsRef((Item)d); }
    uint  contains( const type *d ) const
					{ return QGVector::contains((Item)d); }
    type *operator[]( int i ) const	{ return (type *)QGVector::at(i); }
    type *at( uint i ) const		{ return (type *)QGVector::at(i); }
    void  toList( QGList *list ) const	{ QGVector::toList(list); }
private:
    void  deleteItem( Item d ) { if ( del_item ) delete (type *)d; }
};


#endif // QVECTOR_H
