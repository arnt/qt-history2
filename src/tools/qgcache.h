/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.h#15 $
**
** Definition of QGCache and QGCacheIterator classes
**
** Created : 950208
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGCACHE_H
#define QGCACHE_H

#ifndef QT_H
#include "qcollect.h"
#include "qglist.h"
#include "qgdict.h"
#endif // QT_H


class QCList;					// internal classes
class QCDict;


/*****************************************************************************
  QGCache class
 *****************************************************************************/

class QGCache : public QCollection		// LRU cache class
{
friend class QGCacheIterator;
protected:
    QGCache( int maxCost, uint size,bool caseS, bool copyKeys, bool trivial );
    QGCache( const QGCache & );			// not allowed, calls fatal()
   ~QGCache();
    QGCache &operator=( const QGCache & );	// not allowed, calls fatal()

    uint    count()	const	{ return ((QGDict*)dict)->count(); }
    uint    size()	const	{ return ((QGDict*)dict)->size(); }
    int	    maxCost()	const	{ return mCost; }
    int	    totalCost() const	{ return tCost; }
    void    setMaxCost( int maxCost );

    bool    insert( const char *key, GCI, int cost, int priority );
    bool    remove( const char *key );
    GCI	    take( const char *key );
    void    clear();

    GCI	    find( const char *key, bool ref=TRUE ) const;

    void    statistics() const;			// output debug statistics

private:
    bool    makeRoomFor( int cost, int priority = -1 );
    QCList *lruList;
    QCDict *dict;
    int	    mCost;
    int	    tCost;
    bool    copyK;
};


/*****************************************************************************
  QGCacheIterator class
 *****************************************************************************/

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
