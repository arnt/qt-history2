/****************************************************************************
** $Id: //depot/qt/main/src/tools/qptrdict.h#12 $
**
** Definition of QPtrDict template class
**
** Created : 970415
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
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
    QPtrDict(int size=17) : QGDict(size,PtrKey,0,0) {}
    QPtrDict( const QPtrDict<type> &d ) : QGDict(d) {}
   ~QPtrDict()				{ clear(); }
    QPtrDict<type> &operator=(const QPtrDict<type> &d)
			{ return (QPtrDict<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }
    void  insert( void *k, const type *d )
					{ QGDict::look_ptr(k,(Item)d,1); }
    void  replace( void *k, const type *d )
					{ QGDict::look_ptr(k,(Item)d,2); }
    bool  remove( void *k )		{ return QGDict::remove_ptr(k); }
    type *take( void *k )		{ return (type*)QGDict::take_ptr(k); }
    type *find( void *k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_ptr(k,0,0); }
    type *operator[]( void *k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_ptr(k,0,0); }
    void  clear()			{ QGDict::clear(); }
    void  resize( uint n )		{ QGDict::resize(n); }
    void  statistics() const		{ QGDict::statistics(); }
private:
    void  deleteItem( Item d )	{ if ( del_item ) delete (type *)d; }
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
    void *currentKey() const  { return QGDictIterator::getKeyPtr(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};


#endif // QPTRDICT_H
