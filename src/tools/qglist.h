/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglist.h#9 $
**
** Definition of QGList and QGListIterator classes
**
** Author  : Haavard Nord
** Created : 920624
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGLIST_H
#define QGLIST_H

#include "qcollect.h"


// --------------------------------------------------------------------------
// Qdnode class (internal doubly linked list node)
//

class Qdnode
{
friend class QGList;
friend class QGListIterator;
public:
    GCI	    getData()	{ return data; }
private:
    GCI	    data;
    Qdnode *prev;
    Qdnode *next;
    Qdnode( GCI d )	{ data = d; }
};


// --------------------------------------------------------------------------
// QGList class
//

class QGList : public QCollection		// doubly linked generic list
{
friend class QGListIterator;
friend class QGVector;				// needed by QGVector::toList
public:
    uint  count() const	 { return numNodes; }	// return number of nodes

    QDataStream &read( QDataStream & );		// read list from stream
    QDataStream &write( QDataStream & ) const;	// write list to stream

protected:
    QGList();					// create empty list
    QGList( const QGList & );			// make copy of other list
   ~QGList();

    QGList &operator=( const QGList & );	// assign from other list

    void  insert( GCI );			// add item at start of list
    void  inSort( GCI );			// add item sorted in list
    void  append( GCI );			// add item at end of list
    bool  insertAt( uint index, GCI );		// add item at i'th position
    bool  remove( Qdnode * );			// remove one item
    bool  remove( GCI = 0 );			// remove one item (0=current)
    bool  removeFirst();			// remove first item
    bool  removeLast();				// remove last item
    bool  removeAt( uint index );		// remove item at i'th position
    GCI	  take();				// take out current item
    GCI	  takeAt( uint index );			// take out item at i'th pos
    GCI	  takeFirst();				// take out first item
    GCI	  takeLast();				// take out last item

    void  clear();				// remove all items

    int	  findRef( GCI, bool = TRUE );		// find exact item in list
    int	  find( GCI, bool = TRUE );		// find equal item in list

    uint  containsRef( GCI );			// get number of exact matches
    uint  contains( GCI );			// get number of equal matches

    GCI	  at( uint index );			// access item at i'th pos
    uint  at() const	  { return curIndex; }	// get current index
    Qdnode *currentNode() { return curNode;  }  // get current node

    GCI	  get() const;				// get current item

    GCI	  cfirst() const;			// get ptr to first list item
    GCI	  clast()  const;			// get ptr to last list item
    GCI	  first();				// set first item in list curr
    GCI	  last();				// set last item in list curr
    GCI	  next();				// set next item in list curr
    GCI	  prev();				// set prev item in list curr

    void  toVector( QGVector & ) const;		// put items in vector

    int	  apply( GCF, void * ) const;		// apply function to all items

    virtual int compareItems( GCI, GCI );

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    Qdnode *firstNode;				// first node
    Qdnode *lastNode;				// last node
    Qdnode *curNode;				// current node
    uint    curIndex;				// current index
    uint    numNodes;				// number of nodes
    QGList *iterators;				// list of iterators

    Qdnode *locate( uint );			// get node at i'th pos
    Qdnode *unlink();				// unlink node
};


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

inline GCI QGList::at( uint index )
{
    Qdnode *n = locate( index );
    return n ? n->data : 0;
}

inline GCI QGList::get() const
{
    return curNode ? curNode->data : 0;
}

inline GCI QGList::cfirst() const
{
    return firstNode ? firstNode->data : 0;
}

inline GCI QGList::clast() const
{
    return lastNode ? lastNode->data : 0;
}


// --------------------------------------------------------------------------
// QGList stream functions
//

QDataStream &operator>>( QDataStream &, QGList & );
QDataStream &operator<<( QDataStream &, const QGList & );


// --------------------------------------------------------------------------
// QGListIterator class
//

class QGListIterator				// QGList iterator
{
friend class QGList;
protected:
    QGListIterator( const QGList & );
   ~QGListIterator();

    bool  atFirst() const;			// test if at first item
    bool  atLast()  const;			// test if at last item
    GCI	  toFirst();				// move to first item
    GCI	  toLast();				// move to last item

    GCI	  get() const;				// get current item
    GCI	  operator()();				// get current and move to next
    GCI	  operator++();				// move to next item (prefix)
    GCI	  operator+=(uint);			// move n positions forward
    GCI	  operator--();				// move to prev item (prefix)
    GCI	  operator-=(uint);			// move n positions backward

protected:
    QGList *list;				// reference to list

private:
    Qdnode  *curNode;				// current node in list
};


inline bool QGListIterator::atFirst() const
{
    return curNode == list->firstNode;
}

inline bool QGListIterator::atLast() const
{
    return curNode == list->lastNode;
}

inline GCI QGListIterator::get() const
{
    return curNode ? curNode->data : 0;
}


#endif	// QGLIST_H
