/****************************************************************************
** $Id: //depot/qt/main/src/tools/qlist.h#30 $
**
** Definition of QList template/macro class
**
** Created : 920701
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

#ifndef QLIST_H
#define QLIST_H

#ifndef QT_H
#include "qglist.h"
#endif // QT_H


template<class type> class Q_EXPORT QList : public QGList
{
public:
    QList()				{}
    QList( const QList<type> &l ) : QGList(l) {}
   ~QList()				{ clear(); }
    QList<type> &operator=(const QList<type> &l)
			{ return (QList<type>&)QGList::operator=(l); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    bool  insert( uint i, const type *d){ return QGList::insertAt(i,(Item)d); }
    void  inSort( const type *d )	{ QGList::inSort((Item)d); }
    void  append( const type *d )	{ QGList::append((Item)d); }
    bool  remove( uint i )		{ return QGList::removeAt(i); }
    bool  remove()			{ return QGList::remove((Item)0); }
    bool  remove( const type *d )	{ return QGList::remove((Item)d); }
    bool  removeRef( const type *d )	{ return QGList::removeRef((Item)d); }
    void  removeNode( QLNode *n )	{ QGList::removeNode(n); }
    bool  removeFirst()			{ return QGList::removeFirst(); }
    bool  removeLast()			{ return QGList::removeLast(); }
    type *take( uint i )		{ return (type *)QGList::takeAt(i); }
    type *take()			{ return (type *)QGList::take(); }
    type *takeNode( QLNode *n )		{ return (type *)QGList::takeNode(n); }
    void  clear()			{ QGList::clear(); }
    void  sort()			{ QGList::sort(); }
    int	  find( const type *d )		{ return QGList::find((Item)d); }
    int	  findNext( const type *d )	{ return QGList::find((Item)d,FALSE); }
    int	  findRef( const type *d )	{ return QGList::findRef((Item)d); }
    int	  findNextRef( const type *d ){ return QGList::findRef((Item)d,FALSE);}
    uint  contains( const type *d ) const { return QGList::contains((Item)d); }
    uint  containsRef( const type *d ) const
					{ return QGList::containsRef((Item)d); }
    type *at( uint i )			{ return (type *)QGList::at(i); }
    int	  at() const			{ return QGList::at(); }
    type *current()  const		{ return (type *)QGList::get(); }
    QLNode *currentNode()  const	{ return QGList::currentNode(); }
    type *getFirst() const		{ return (type *)QGList::cfirst(); }
    type *getLast()  const		{ return (type *)QGList::clast(); }
    type *first()			{ return (type *)QGList::first(); }
    type *last()			{ return (type *)QGList::last(); }
    type *next()			{ return (type *)QGList::next(); }
    type *prev()			{ return (type *)QGList::prev(); }
    void  toVector( QGVector *vec )const{ QGList::toVector(vec); }
private:
    void  deleteItem( Item d ) { if ( del_item ) delete (type *)d; }
};

template<class type> class Q_EXPORT QListIterator : public QGListIterator
{
public:
    QListIterator(const QList<type> &l) :QGListIterator((QGList &)l) {}
   ~QListIterator()	      {}
    uint  count()   const     { return list->count(); }
    bool  isEmpty() const     { return list->count() == 0; }
    bool  atFirst() const     { return QGListIterator::atFirst(); }
    bool  atLast()  const     { return QGListIterator::atLast(); }
    type *toFirst()	      { return (type *)QGListIterator::toFirst(); }
    type *toLast()	      { return (type *)QGListIterator::toLast(); }
    operator type *() const   { return (type *)QGListIterator::get(); }
    type *operator*()         { return (type *)QGListIterator::get(); }

    // No good, since QList<char> (ie. QStrList fails...
    //
    // MSVC++ gives warning
    // Sunpro C++ 4.1 gives error
    //    type *operator->()        { return (type *)QGListIterator::get(); }

    type *current()   const   { return (type *)QGListIterator::get(); }
    type *operator()()	      { return (type *)QGListIterator::operator()();}
    type *operator++()	      { return (type *)QGListIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGListIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGListIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGListIterator::operator-=(j);}
    QListIterator<type>& operator=(const QListIterator<type>&it)
			      { QGListIterator::operator=(it); return *this; }
};


#endif // QLIST_H
