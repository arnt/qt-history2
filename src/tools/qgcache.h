/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.h#2 $
**
** Definition of QGCache class
**
** Author  : Eirik Eng
** Created : 950208
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGCACHE_H
#define QGCACHE_H

#include "qcollect.h"


class QCList;		                        // internal classes
class QCDict;


// --------------------------------------------------------------------------
// QGCache class
//

class QGCache : public QCollection	    // LRU cache class
{
friend class QGCacheIterator;
public:
    uint    count() const;
    uint    size()  const;
    long    maxCost()    const;
    long    totalCost()  const;
    void    setMaxCost( long maxC);

protected:
    QGCache( long maxCost, uint size,bool caseS, bool copyKeys, bool trivial );
    QGCache( const QGCache & );             // not allowed, calls fatal()
   ~QGCache();
    QGCache &operator=( const QGCache & );  // not allowed, calls fatal()
    GCI      find( const char *key ) const;
    bool     insert( const char *key, GCI, long cost, int priority );
    void     reference( GCI ) const;        // make element most recently used

    bool     remove( const char *key );
    GCI	     take( const char *key );
    void     clear();			    // delete all items

    void     statistics() const;	    // output statistics
private:
    bool     makeRoomFor( long cost, short priority = -1 );

    QCList *lruList;
    QCDict *dict;

    long   mCost;
    long   tCost;
    uint   copyK : 1;
};

// --------------------------------------------------------------------------
// QGCacheIterator class
//

class QListIteratorM_CacheItem;

class QGCacheIterator				// QGCache iterator
{
protected:
    QGCacheIterator( const QGCache & );
    QGCacheIterator( const QGCacheIterator& );
   ~QGCacheIterator();
    QGCacheIterator &operator=( const QGCacheIterator &s );

    uint  count() const;                        // number of items in cache
    bool  atFirst() const;			// test if at first item
    bool  atLast()  const;			// test if at last item
    GCI	  toFirst();				// move to first item
    GCI	  toLast();				// move to last item

    GCI	  get() const;				// get current item
    const char *getKey() const;			// get current key
    GCI	  operator()();				// get current and move to next
    GCI	  operator++();				// move to next item (prefix)
    GCI	  operator+=(uint);			// move n positions forward
    GCI	  operator--();				// move to prev item (prefix)
    GCI	  operator-=(uint);			// move n positions backward

protected:
    QListIteratorM_CacheItem *it; 	        // iterator on cache list
};

#endif // QGCACHE_H
