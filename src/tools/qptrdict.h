/****************************************************************************
** $Id: //depot/qt/main/src/tools/qptrdict.h#9 $
**
** Definition of QPtrDict template/macro class
**
** Created : 970415
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

#ifndef QPTRDICT_H
#define QPTRDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H


template<class type> class Q_EXPORT QPtrDict : public QGDict
{
public:
    QPtrDict(int size=17) : QGDict(size,0,0,TRUE) {}
    QPtrDict( const QPtrDict<type> &d ) : QGDict(d) {}
   ~QPtrDict()			{ clear(); }
    QPtrDict<type> &operator=(const QPtrDict<type> &d)
			{ return (QPtrDict<type>&)QGDict::operator=(d); }
    uint  count()   const	{ return QGDict::count(); }
    uint  size()    const	{ return QGDict::size(); }
    bool  isEmpty() const	{ return QGDict::count() == 0; }
    void  insert( void *k, const type *d )
				{ QGDict::look((const char*)k,(GCI)d,1); }
    void  replace( void *k, const type *d )
				{ QGDict::look((const char*)k,(GCI)d,2); }
    bool  remove( void *k )	{ return QGDict::remove((const char*)k); }
    type *take( void *k )	{ return (type*)QGDict::take((const char*)k); }
    void  clear()		{ QGDict::clear(); }
    void  resize( uint n )	{ QGDict::resize(n); }
    type *find( void *k )	const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    type *operator[]( void *k ) const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    void  statistics() const	{ QGDict::statistics(); }
private:
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }
};


template<class type> class Q_EXPORT QPtrDictIterator : public QGDictIterator
{
public:
    QPtrDictIterator(const QPtrDict<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QPtrDictIterator()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *()  const  { return (type *)QGDictIterator::get(); }
    type *current()    const  { return (type *)QGDictIterator::get(); }
    void *currentKey() const  { return (void *)QGDictIterator::getKeyLong(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};


#endif // QPTRDICT_H
