/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglist.h#34 $
**
** Definition of QGList and QGListIterator classes
**
** Created : 920624
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

#ifndef QGLIST_H
#define QGLIST_H

#ifndef QT_H
#include "qcollection.h"
#endif // QT_H


/*****************************************************************************
  QLNode class (internal doubly linked list node)
 *****************************************************************************/

class Q_EXPORT QLNode
{
friend class QGList;
friend class QGListIterator;
public:
    QCollection::Item getData()	{ return data; }
private:
    QCollection::Item data;
    QLNode *prev;
    QLNode *next;
    QLNode( QCollection::Item d ) { data = d; }
};


/*****************************************************************************
  QGList class
 *****************************************************************************/

class Q_EXPORT QGList : public QCollection	// doubly linked generic list
{
friend class QGListIterator;
friend class QGVector;				// needed by QGVector::toList
public:
    uint  count() const;			// return number of nodes

    QDataStream &read( QDataStream & );		// read list from stream
    QDataStream &write( QDataStream & ) const;	// write list to stream

protected:
    QGList();					// create empty list
    QGList( const QGList & );			// make copy of other list
    virtual ~QGList();

    QGList &operator=( const QGList & );	// assign from other list

    void  inSort( Item );			// add item sorted in list
    void  append( Item );			// add item at end of list
    bool  insertAt( uint index, Item );		// add item at i'th position
    void  relinkNode( QLNode * );		// relink as first item
    bool  removeNode( QLNode * );		// remove node
    bool  remove( Item = 0 );			// remove item (0=current)
    bool  removeRef( Item = 0 );			// remove item (0=current)
    bool  removeFirst();			// remove first item
    bool  removeLast();				// remove last item
    bool  removeAt( uint index );		// remove item at i'th position
    Item	  takeNode( QLNode * );			// take out node
    Item	  take();				// take out current item
    Item	  takeAt( uint index );			// take out item at i'th pos
    Item	  takeFirst();				// take out first item
    Item	  takeLast();				// take out last item

    void  sort();                               // sort all items;
    void  clear();				// remove all items

    int	  findRef( Item, bool = TRUE );		// find exact item in list
    int	  find( Item, bool = TRUE );		// find equal item in list

    uint  containsRef( Item )	const;		// get number of exact matches
    uint  contains( Item )	const;		// get number of equal matches

    Item	  at( uint index );			// access item at i'th pos
    int	  at() const;				// get current index
    QLNode *currentNode() const;		// get current node

    Item	  get() const;				// get current item

    Item	  cfirst() const;			// get ptr to first list item
    Item	  clast()  const;			// get ptr to last list item
    Item	  first();				// set first item in list curr
    Item	  last();				// set last item in list curr
    Item	  next();				// set next item in list curr
    Item	  prev();				// set prev item in list curr

    void  toVector( QGVector * ) const;		// put items in vector

    virtual int compareItems( Item, Item );

    virtual QDataStream &read( QDataStream &, Item & );
    virtual QDataStream &write( QDataStream &, Item ) const;

private:
    void  prepend( Item );			// add item at start of list

    void heapSortPushDown( QCollection::Item* heap, int first, int last );

    QLNode *firstNode;				// first node
    QLNode *lastNode;				// last node
    QLNode *curNode;				// current node
    int	    curIndex;				// current index
    uint    numNodes;				// number of nodes
    QGList *iterators;				// list of iterators

    QLNode *locate( uint );			// get node at i'th pos
    QLNode *unlink();				// unlink node
};


inline uint QGList::count() const
{
    return numNodes;
}

inline bool QGList::removeFirst()
{
    first();
    return remove();
}

inline bool QGList::removeLast()
{
    last();
    return remove();
}

inline int QGList::at() const
{
    return curIndex;
}

inline QCollection::Item QGList::at( uint index )
{
    QLNode *n = locate( index );
    return n ? n->data : 0;
}

inline QLNode *QGList::currentNode() const
{
    return curNode;
}

inline QCollection::Item QGList::get() const
{
    return curNode ? curNode->data : 0;
}

inline QCollection::Item QGList::cfirst() const
{
    return firstNode ? firstNode->data : 0;
}

inline QCollection::Item QGList::clast() const
{
    return lastNode ? lastNode->data : 0;
}


/*****************************************************************************
  QGList stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator>>( QDataStream &, QGList & );
Q_EXPORT QDataStream &operator<<( QDataStream &, const QGList & );


/*****************************************************************************
  QGListIterator class
 *****************************************************************************/

class Q_EXPORT QGListIterator			// QGList iterator
{
friend class QGList;
protected:
    QGListIterator( const QGList & );
    QGListIterator( const QGListIterator & );
    QGListIterator &operator=( const QGListIterator & );
   ~QGListIterator();

    bool  atFirst() const;			// test if at first item
    bool  atLast()  const;			// test if at last item
    QCollection::Item	  toFirst();				// move to first item
    QCollection::Item	  toLast();				// move to last item

    QCollection::Item	  get() const;				// get current item
    QCollection::Item	  operator()();				// get current and move to next
    QCollection::Item	  operator++();				// move to next item (prefix)
    QCollection::Item	  operator+=(uint);			// move n positions forward
    QCollection::Item	  operator--();				// move to prev item (prefix)
    QCollection::Item	  operator-=(uint);			// move n positions backward

protected:
    QGList *list;				// reference to list

private:
    QLNode  *curNode;				// current node in list
};


inline bool QGListIterator::atFirst() const
{
    return curNode == list->firstNode;
}

inline bool QGListIterator::atLast() const
{
    return curNode == list->lastNode;
}

inline QCollection::Item QGListIterator::get() const
{
    return curNode ? curNode->data : 0;
}


#endif	// QGLIST_H
