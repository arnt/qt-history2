/****************************************************************************
** $Id: //depot/qt/main/src/tools/qintdict.h#23 $
**
** Definition of QIntDict template class
**
** Created : 940624
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

#ifndef QINTDICT_H
#define QINTDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H


template<class type> class Q_EXPORT QIntDict : public QGDict
{
public:
    QIntDict(int size=17) : QGDict(size,IntKey,0,0) {}
    QIntDict( const QIntDict<type> &d ) : QGDict(d) {}
   ~QIntDict()				{ clear(); }
    QIntDict<type> &operator=(const QIntDict<type> &d)
			{ return (QIntDict<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }
    void  insert( long k, const type *d )
					{ QGDict::look_int(k,(Item)d,1); }
    void  replace( long k, const type *d )
					{ QGDict::look_int(k,(Item)d,2); }
    bool  remove( long k )		{ return QGDict::remove_int(k); }
    type *take( long k )		{ return (type*)QGDict::take_int(k); }
    type *find( long k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_int(k,0,0); }
    type *operator[]( long k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_int(k,0,0); }
    void  clear()			{ QGDict::clear(); }
    void  resize( uint n )		{ QGDict::resize(n); }
    void  statistics() const		{ QGDict::statistics(); }
private:
    void  deleteItem( Item d )	{ if ( del_item ) delete (type *)d; }
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
    long  currentKey() const  { return QGDictIterator::getKeyInt(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};


#endif // QINTDICT_H
