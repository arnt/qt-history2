/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglist.cpp#15 $
**
** Implementation of QGList and QGListIterator classes
**
** Author  : Haavard Nord
** Created : 920624
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qglist.h"
#include "qgvector.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/tools/qglist.cpp#15 $")


// --------------------------------------------------------------------------
// Default implementation of virtual functions
//

int QGList::compareItems( GCI d1, GCI d2 )
{
    return d1 != d2;				// compare pointers
}

QDataStream &QGList::read( QDataStream &s, GCI &d )
{						// read item from stream
    d = 0;
    return s;
}

QDataStream &QGList::write( QDataStream &s, GCI ) const
{						// write item to stream
    return s;
}


// --------------------------------------------------------------------------
// QGList member functions
//

QGList::QGList()				// create empty list
{
    firstNode = lastNode = curNode = 0;		// initialize list
    curIndex = numNodes = 0;
    iterators = 0;				// initialize iterator list
}

QGList::QGList( const QGList & list )		// make copy of other list
{
    firstNode = lastNode = curNode = 0;		// initialize list
    curIndex = numNodes = 0;
    iterators = 0;				// initialize iterator list
    register Qdnode *n = list.firstNode;
    while ( n ) {				// copy all items from list
	append( n->getData() );
	n = n->next;
    }
}

QGList::~QGList()
{
    clear();
    if ( !iterators )				// no iterators for this list
	return;
    register QGListIterator *i = (QGListIterator*)iterators->first();
    while ( i ) {				// notify all iterators that
	i->list = 0;				// this list is deleted
	i = (QGListIterator*)iterators->next();
    }
    delete iterators;
}


QGList& QGList::operator=( const QGList &list ) // assign from other list
{
    clear();
    register Qdnode *n = list.firstNode;
    while ( n ) {				// copy all items from list
	append( n->getData() );
	n = n->next;
    }
    curNode = firstNode;
    curIndex = 0;
    return *this;
}


Qdnode *QGList::locate( uint index )		// get node at i'th position
{
    if ( index == curIndex )			// current node ?
	return curNode;
    register Qdnode *node;
    int	 distance = index - curIndex;		// node distance to cur node
    bool forward;				// direction to traverse

    if ( index >= numNodes ) {
#if defined(CHECK_RANGE)
	warning( "QGList::locate: Index %d out of range", index );
#endif
	return 0;
    }

    if ( distance < 0 )
	distance = -distance;
    if ( (uint)distance < index && (uint)distance < numNodes - index ) {
	node =	curNode;
	forward = index > curIndex;		// start from current node
    }
    else
    if ( index < numNodes - index ) {		// start from first node
	node = firstNode;
	distance = index;
	forward = TRUE;
    }
    else {					// start from last node
	node = lastNode;
	distance = numNodes - index - 1;
	if ( distance < 0 )
	    distance = 0;
	forward = FALSE;
    }
    if ( forward ) {				// now run through nodes
	while ( distance-- )
	    node = node->next;
    }
    else {
	while ( distance-- )
	    node = node->prev;
    }
    curIndex = index;				// must update index
    return curNode = node;
}

void QGList::inSort( GCI d )			// add sorted in list
{
    int index = 0;
    register Qdnode *n = firstNode;
    while ( n && compareItems(n->data,d) < 0 ){ // find position in list
	n = n->next;
	index++;
    }
    insertAt( index, d );
}

void QGList::append( GCI d )			// add at list tail
{
    register Qdnode *n = new Qdnode( newItem( d ) );
    CHECK_PTR( n );
    CHECK_PTR( n->data );
    n->next = 0;
    if ( (n->prev = lastNode) )			// list is not empty
	lastNode->next = n;
    else					// initialize list
	firstNode = n;
    lastNode = curNode = n;			// curNode affected
    curIndex = numNodes;
    numNodes++;
}

bool QGList::insertAt( uint index, GCI d )	// add at i'th position
{
    if ( index == 0 ) {				// insert at head of list
	prepend( d );
	return TRUE;
    }
    else if ( index == numNodes ) {		// append at tail of list
	append( d );
	return TRUE;
    }
    Qdnode *nextNode = locate( index );
    if ( !nextNode )				// illegal position
	return FALSE;
    Qdnode *prevNode = nextNode->prev;
    register Qdnode *n = new Qdnode( newItem( d ) );
    CHECK_PTR( n );
    CHECK_PTR( n->data );
    nextNode->prev = n;
    prevNode->next = n;
    n->prev = prevNode;				// link new node into list
    n->next = nextNode;
    curNode = n;
    numNodes++;
    return TRUE;
}


Qdnode *QGList::unlink()			// unlink current node
{
    if ( !count() ) {				// no items left
#if defined(CHECK_NULL)
	warning( "QGList::unlink: Cannot unlink from empty list" );
#endif
	return 0;
    }
    register Qdnode *n = curNode;		// unlink this node
    if ( n == firstNode ) {			// removing first node ?
	if ( (firstNode = n->next) )
	    firstNode->prev = 0;
	else
	    lastNode = curNode = 0;		// list becomes empty
    }
    else {
	if ( n == lastNode ) {			// removing last node ?
	    lastNode = n->prev;
	    lastNode->next = 0;
	}
	else {					// neither last nor first node
	    n->prev->next = n->next;
	    n->next->prev = n->prev;
	}
    }
    if ( n->next )				// change current node
	curNode = n->next;
    else
    if ( n->prev ) {
	curNode = n->prev;
	curIndex--;
    }
    if ( iterators ) {				// update iterators
	register QGListIterator *i = (QGListIterator*)iterators->first();
	while ( i ) {				// fix all iterators that
	    if ( i->curNode == n )		// refers to pending node
		i->curNode = curNode;
	    i = (QGListIterator*)iterators->next();
	}
    }
    numNodes--;
    return n;
}

bool QGList::removeNode( Qdnode *n )		// remove one item
{
    CHECK_PTR( n );
    if ( n->prev && n->prev->next != n ||
	 n->next && n->next->prev != n ) {
#if defined(CHECK_NULL)
	warning( "QGList::remove(Qdnode): Attempt to remove invalid node." );
#endif
	return FALSE;
    }
    curNode = n;
    unlink();					// unlink node
    deleteItem( n->getData() );			// deallocate this node
    delete n;
    curNode  = firstNode;
    curIndex = 0;
    return TRUE;
}

bool QGList::remove( GCI d )			// remove one item
{
    if ( d ) {					// find the item
	if ( find( d ) == -1 )
	    return FALSE;
    }
    Qdnode *n = unlink();			// unlink node
    if ( !n )
	return FALSE;
    deleteItem( n->getData() );			// deallocate this node
    delete n;
    return TRUE;
}

bool QGList::removeAt( uint index )		// remove at i'th position
{
    if ( !locate(index) )
	return FALSE;
    Qdnode *n = unlink();			// unlink node
    if ( !n )
	return FALSE;
    deleteItem( n->getData() );			// deallocate this node
    delete n;
    return TRUE;
}

GCI QGList::take()				// take current item out
{
    Qdnode *n = unlink();			// unlink node
    GCI d = n ? n->getData() : 0;
    delete n;					// delete node, keep contents
    return d;
}

GCI QGList::takeAt( uint index )		// take at i'th item
{
    if ( !locate(index) )
	return 0;
    Qdnode *n = unlink();			// unlink node
    GCI d = n ? n->getData() : 0;
    delete n;					// delete node, keep contents
    return d;
}

GCI QGList::takeFirst()				// take first item out
{
    first();
    Qdnode *n = unlink();			// unlink node
    GCI d = n ? n->getData() : 0;
    delete n;
    return d;
}

GCI QGList::takeLast()				// take last item out
{
    last();
    Qdnode *n = unlink();			// unlink node
    GCI d = n ? n->getData() : 0;
    delete n;
    return d;
}


void QGList::clear()				// remove all items
{
    register Qdnode *n = firstNode;
    Qdnode *prevNode;
    while ( n ) {				// for all nodes ...
	deleteItem( n->getData() );		// deallocate data
	prevNode = n;
	n = n->next;
	delete prevNode;			// deallocate node
    }
    firstNode = lastNode = curNode = 0;		// initialize list
    curIndex = numNodes = 0;
    if ( !iterators )				// no iterators for this list
	return;
    register QGListIterator *i = (QGListIterator*)iterators->first();
    while ( i ) {				// notify all iterators that
	i->curNode = 0;				// this list is empty
	i = (QGListIterator*)iterators->next();
    }
}


int QGList::findRef( GCI d, bool fromStart )	// find exact item in list
{
    register Qdnode *n;
    int	     index;
    if ( fromStart ) {				// start from first node
	n = firstNode;
	index = 0;
    }
    else {					// start from current node
	n = curNode;
	index = curIndex;
    }
    while ( n && n->getData() != d ) {		// find exact match
	n = n->next;
	index++;
    }
    if ( n )					// item was found
	curNode = n;
    return n ? (int)(curIndex=index) : -1;	// return position of item
}

int QGList::find( GCI d, bool fromStart )	// find equal item in list
{
    register Qdnode *n;
    int	     index;
    if ( fromStart ) {				// start from first node
	n = firstNode;
	index = 0;
    }
    else {					// start from current node
	n = curNode;
	index = curIndex;
    }
    while ( n && compareItems(n->getData(),d) ){// find equal match
	n = n->next;
	index++;
    }
    if ( n )					// item was found
	curNode = n;
    return n ? (int)(curIndex=index) : -1;	// return position of item
}

uint QGList::containsRef( GCI d )		// get number of exact matches
{
    register Qdnode *n = firstNode;
    uint     count = 0;
    while ( n ) {				// for all nodes...
	if ( n->getData() == d )		// count # exact matches
	    count++;
	n = n->next;
    }
    return count;
}

uint QGList::contains( GCI d )			// get number of equal matches
{
    register Qdnode *n = firstNode;
    uint     count = 0;
    while ( n ) {				// for all nodes...
	if ( !compareItems(n->getData(),d) )	// count # equal matches
	    count++;
	n = n->next;
    }
    return count;
}


GCI QGList::first()				// get first item in list
{
    if ( firstNode ) {
	curIndex = 0;
	return (curNode=firstNode)->getData();
    }
    return 0;
}

GCI QGList::last()				// get last item in list
{
    if ( lastNode ) {
	curIndex = numNodes-1;
	return (curNode=lastNode)->getData();
    }
    return 0;
}

GCI QGList::next()				// get next item in list
{
    if ( curNode && curNode->next ) {
	curIndex++;
	return (curNode = curNode->next)->getData();
    }
    return 0;
}

GCI QGList::prev()				// get previous item in list
{
    if ( curNode && curNode->prev ) {
	curIndex--;
	return (curNode = curNode->prev)->getData();
    }
    return 0;
}


void QGList::toVector( QGVector *vector ) const // store items in vector
{
    vector->clear();
    if ( !vector->resize( count() ) )
	return;
    register Qdnode *n = firstNode;
    uint i = 0;
    while ( n ) {
	vector->insert( i, n->getData() );
	n = n->next;
	i++;
    }
}


// --------------------------------------------------------------------------
// QGList stream functions
//

QDataStream &operator>>( QDataStream &s, QGList &list )
{						// read list
    return list.read( s );
}

QDataStream &operator<<( QDataStream &s, const QGList &list )
{						// write list
    return list.write( s );
}

QDataStream &QGList::read( QDataStream &s )	// read list from stream
{
    uint num;
    s >> num;					// read number of items
    clear();					// clear list
    while ( num-- ) {				// read all items
	GCI d;
	read( s, d );
	CHECK_PTR( d );
	if ( !d )				// no memory
	    break;
	Qdnode *n = new Qdnode( d );
	CHECK_PTR( n );
	if ( !n )				// no memory
	    break;
	n->next = 0;
	if ( (n->prev = lastNode) )		// list is not empty
	    lastNode->next = n;
	else					// initialize list
	    firstNode = n;
	lastNode = n;
	numNodes++;
    }
    curNode = firstNode;
    curIndex = 0;
    return s;
}

QDataStream &QGList::write( QDataStream &s ) const
{						// write list to stream
    s << count();				// write number of items
    Qdnode *n = firstNode;
    while ( n ) {				// write all items
	write( s, n->getData() );
	n = n->next;
    }
    return s;
}


// --------------------------------------------------------------------------
// QGListIterator member functions
//

QGListIterator::QGListIterator( const QGList &l )
{
    list = (QGList *)&l;			// get reference to list
    curNode = list->firstNode;			// set to first node
    if ( !list->iterators ) {
	list->iterators = new QGList;		// create iterator list
	CHECK_PTR( list->iterators );
    }
    list->iterators->append( this );		// notify list about iterator
}

QGListIterator::~QGListIterator()
{
    if ( list )
	list->iterators->remove( this );	// remove iterator from list
}


GCI QGListIterator::toFirst()			// move to first item
{
    if ( !list ) {
#if defined(CHECK_NULL)
	warning( "QGListIterator::toFirst: List has been deleted" );
#endif
	return 0;
    }
    return list->firstNode ? (curNode = list->firstNode)->getData() : 0;
}

GCI QGListIterator::toLast()			// move to last item
{
    if ( !list ) {
#if defined(CHECK_NULL)
	warning( "QGListIterator::toLast: List has been deleted" );
#endif
	return 0;
    }
    return list->lastNode ? (curNode = list->lastNode)->getData() : 0;
}


GCI QGListIterator::operator()()		// get current and move to next
{
    if ( !curNode )
	return 0;
    GCI d = curNode->getData();
    curNode = curNode->next;
    return  d;
}

GCI QGListIterator::operator++()		// move to next item (prefix)
{
    if ( !curNode )
	return 0;
    curNode = curNode->next;
    return curNode ? curNode->getData() : 0;
}

GCI QGListIterator::operator+=( uint jumps )	// move n positions forward
{
    while ( curNode && jumps-- )
	curNode = curNode->next;
    return curNode ? curNode->getData() : 0;
}

GCI QGListIterator::operator--()		// move to prev item (prefix)
{
    if ( !curNode )
	return 0;
    curNode = curNode->prev;
    return curNode ? curNode->getData() : 0;
}

GCI QGListIterator::operator-=( uint jumps )	// move n positions backward
{
    while ( curNode && jumps-- )
	curNode = curNode->prev;
    return curNode ? curNode->getData() : 0;
}

void QGList::prepend( GCI d )			// add at list head
{
    register Qdnode *n = new Qdnode( newItem( d ) );
    CHECK_PTR( n );
    CHECK_PTR( n->data );
    n->prev = 0;
    if ( (n->next = firstNode) )		// list is not empty
	firstNode->prev = n;
    else					// initialize list
	lastNode = n;
    firstNode = curNode = n;			// curNode affected
    numNodes++;
    curIndex = 0;
}
