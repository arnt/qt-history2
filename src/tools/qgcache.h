/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.h#24 $
**
** Definition of QGCache and QGCacheIterator classes
**
** Created : 950208
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

#ifndef QGCACHE_H
#define QGCACHE_H

#ifndef QT_H
#include "qcollection.h"
#include "qglist.h"
#include "qgdict.h"
#endif // QT_H


class QCList;					// internal classes
class QCListIt;
class QCDict;


/*****************************************************************************
  QGCache class
 *****************************************************************************/

class Q_EXPORT QGCache : public QCollection		// LRU cache class
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
    void    clear();

    bool    insert( const char *key, Item, int cost, int priority );
    bool    remove( const char *key );
    Item	    take( const char *key );
    Item	    find( const char *key, bool ref=TRUE ) const;

    bool    insert( const QString& key, Item, int cost, int priority );
    bool    remove( const QString& key );
    Item	    take( const QString& key );
    Item	    find( const QString& key, bool ref=TRUE ) const;

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

class Q_EXPORT QGCacheIterator				// QGCache iterator
{
protected:
    QGCacheIterator( const QGCache & );
    QGCacheIterator( const QGCacheIterator & );
   ~QGCacheIterator();
    QGCacheIterator &operator=( const QGCacheIterator & );

    uint  count()   const;			// number of items in cache
    bool  atFirst() const;			// test if at first item
    bool  atLast()  const;			// test if at last item
    QCollection::Item toFirst();			// move to first item
    QCollection::Item toLast();			// move to last item

    QCollection::Item get() const;		// get current item
    QString getKey() const;			// get current key
    long getKeyLong() const;			// get current key as a long
    QCollection::Item operator()();		// get current and move to next
    QCollection::Item operator++();		// move to next item (prefix)
    QCollection::Item operator+=( uint );	// move n positions forward
    QCollection::Item operator--();		// move to prev item (prefix)
    QCollection::Item operator-=( uint );	// move n positions backward

protected:
    QCListIt *it;				// iterator on cache list
};


#endif // QGCACHE_H
