/****************************************************************************
** $Id: //depot/qt/main/src/tools/qintdict.h#20 $
**
** Definition of QIntDict template/macro class
**
** Created : 940624
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

#ifndef QINTDICT_H
#define QINTDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H


template<class type> class Q_EXPORT QIntDict : public QGDict
{
public:
    QIntDict(int size=17) : QGDict(size,0,0,TRUE) {}
    QIntDict( const QIntDict<type> &d ) : QGDict(d) {}
   ~QIntDict()			{ clear(); }
    QIntDict<type> &operator=(const QIntDict<type> &d)
			{ return (QIntDict<type>&)QGDict::operator=(d); }
    uint  count()   const	{ return QGDict::count(); }
    uint  size()    const	{ return QGDict::size(); }
    bool  isEmpty() const	{ return QGDict::count() == 0; }
    void  insert( long k, const type *d )
				{ QGDict::look((const char*)k,(GCI)d,1); }
    void  replace( long k, const type *d )
				{ QGDict::look((const char*)k,(GCI)d,2); }
    bool  remove( long k )	{ return QGDict::remove((const char*)k); }
    type *take( long k )	{ return (type*)QGDict::take((const char*)k); }
    void  clear()		{ QGDict::clear(); }
    void  resize( uint n )	{ QGDict::resize(n); }
    type *find( long k )	const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    type *operator[]( long k ) const
	{ return (type *)((QGDict*)this)->QGDict::look((const char*)k,0,0); }
    void  statistics() const	{ QGDict::statistics(); }
private:
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }
};

template<class type> class Q_EXPORT QIntDictIterator : public QGDictIterator
{
public:
    QIntDictIterator(const QIntDict<type> &d) :QGDictIterator((QGDict &)d) {}
   ~QIntDictIterator()	      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *()  const  { return (type *)QGDictIterator::get(); }
    type *current()    const  { return (type *)QGDictIterator::get(); }
    long  currentKey() const  { return QGDictIterator::getKeyLong(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};


#endif // QINTDICT_H
