/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.h#7 $
**
** Definition of QGCache and QGCacheIterator classes
**
** Author  : Eirik Eng
** Created : 950208
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QGCACHE_H
#define QGCACHE_H

#include "qcollect.h"
#include "qglist.h"
#include "qgdict.h"


class QCList;					// internal classes
class QCDict;


// --------------------------------------------------------------------------
// QGCache class
//

class QGCache : public QCollection		// LRU cache class
{
friend class QGCacheIterator;
protected:
    QGCache( long maxCost, uint size,bool caseS, bool copyKeys, bool trivial );
    QGCache( const QGCache & );			// not allowed, calls fatal()
   ~QGCache();
    QGCache &operator=( const QGCache & );	// not allowed, calls fatal()

    uint    count()	const	{ return ((QGDict*)dict)->count(); }
    uint    size()	const	{ return ((QGDict*)dict)->size(); }
    long    maxCost()	const	{ return mCost; }
    long    totalCost() const	{ return tCost; }
    void    setMaxCost( long maxCost );

    bool    insert( const char *key, GCI, long cost, int priority );
    bool    remove( const char *key );
    GCI	    take( const char *key );
    void    clear();

    GCI	    reference( const char *key );
    GCI	    find( const char *key ) const;

    void    statistics() const;			// output debug statistics

private:
    bool    makeRoomFor( long cost, int priority = -1 );
    QCList *lruList;
    QCDict *dict;
    long    mCost;
    long    tCost;
    bool    copyK;
};


// --------------------------------------------------------------------------
// QGCacheIterator class
//

class QListIteratorM_QCacheItem;

class QGCacheIterator				// QGCache iterator
{
protected:
    QGCacheIterator( const QGCache & );
    QGCacheIterator( const QGCacheIterator & );
   ~QGCacheIterator();
    QGCacheIterator &operator=( const QGCacheIterator & );

    uint  count()   const;			// number of items in cache
    bool  atFirst() const;			// test if at first item
    bool  atLast()  const;			// test if at last item
    GCI	  toFirst();				// move to first item
    GCI	  toLast();				// move to last item

    GCI	  get() const;				// get current item
    const char *getKey() const;			// get current key
    GCI	  operator()();				// get current and move to next
    GCI	  operator++();				// move to next item (prefix)
    GCI	  operator+=( uint );			// move n positions forward
    GCI	  operator--();				// move to prev item (prefix)
    GCI	  operator-=( uint );			// move n positions backward

protected:
    QListIteratorM_QCacheItem *it;		// iterator on cache list
};


#endif // QGCACHE_H
