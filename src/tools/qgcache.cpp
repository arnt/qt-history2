/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.cpp#43 $
**
** Implementation of QGCache and QGCacheIterator classes
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

#include "qgcache.h"
#include "qlist.h"
#include "qdict.h"
#include "qstring.h"				/* used for statistics */

/*!
  \class QGCache qgcache.h

  \brief The QGCache class is an internal class for implementing QCache and QIntCache.

  QGCache is a strictly internal class that acts as a base class for the
  \link collection.html collection classes\endlink QCache and QIntCache.
*/


/*****************************************************************************
  QGCacheItem class (internal cache item)
 *****************************************************************************/

struct QCacheItem
{
    QCacheItem( const char *k, QCollection::Item d, int c, short p )
	: priority(p), skipPriority(p), cost(c), key(k), data(d), node(0) {}
    short	priority;
    short	skipPriority;
    int		cost;
    const char *key;
    QCollection::Item data;
    QLNode     *node;
};


/*****************************************************************************
  QCList class (internal list of cache items)
 *****************************************************************************/

class QCList : private QList<QCacheItem>
{
friend class QGCacheIterator;
friend class QCListIt;
public:
    QCList()	{}
   ~QCList();

    void	insert( QCacheItem * );		// insert according to priority
    void	insert( int, QCacheItem * );
    void	take( QCacheItem * );
    void	reference( QCacheItem * );

    void	setAutoDelete( bool del ) { QCollection::setAutoDelete(del); }

    bool	removeFirst()	{ return QList<QCacheItem>::removeFirst(); }
    bool	removeLast()	{ return QList<QCacheItem>::removeLast(); }

    QCacheItem *first()		{ return QList<QCacheItem>::first(); }
    QCacheItem *last()		{ return QList<QCacheItem>::last(); }
    QCacheItem *prev()		{ return QList<QCacheItem>::prev(); }
    QCacheItem *next()		{ return QList<QCacheItem>::next(); }

#if defined(DEBUG)
    int		inserts;			// variables for statistics
    int		insertCosts;
    int		insertMisses;
    int		finds;
    int		hits;
    int		hitCosts;
    int		dumps;
    int		dumpCosts;
#endif
};


QCList::~QCList()
{
#if defined(DEBUG)
    ASSERT( count() == 0 );
#endif
}


void QCList::insert( QCacheItem *ci )
{
    QCacheItem *item = first();
    while( item && item->skipPriority > ci->priority ) {
	item->skipPriority--;
	item = next();
    }
    if ( item )
	QList<QCacheItem>::insert( at(), ci );
    else
	append( ci );
#if defined(DEBUG)
    ASSERT( ci->node == 0 );
#endif
    ci->node = currentNode();
}

inline void QCList::insert( int i, QCacheItem *ci )
{
    QList<QCacheItem>::insert( i, ci );
#if defined(DEBUG)
    ASSERT( ci->node == 0 );
#endif
    ci->node = currentNode();
}


void QCList::take( QCacheItem *ci )
{
    if ( ci ) {
#if defined(DEBUG)
	ASSERT( ci->node != 0 );
#endif
	takeNode( ci->node );
	ci->node = 0;
    }
}


inline void QCList::reference( QCacheItem *ci )
{
#if defined(DEBUG)
    ASSERT( ci != 0 && ci->node != 0 );
#endif
    ci->skipPriority = ci->priority;
    relinkNode( ci->node );			// relink as first item
}


class QCListIt: public QListIterator<QCacheItem>
{
public:
    QCListIt( const QCList * asdf ): QListIterator<QCacheItem>( *asdf ) {}
    QCListIt( const QCListIt * asdf ): QListIterator<QCacheItem>( *asdf ) {}
};


/*****************************************************************************
  QCDict class (internal dictionary of cache items)
 *****************************************************************************/

//
// Since we need to decide if the dictionary should use an int or const
// char * key (the "bool trivial" argument in the constructor below)
// we cannot use the macro/template dict, but inherit directly from QGDict.
//

class QCDict : public QGDict
{
public:
    QCDict( uint size, bool caseSensitive, bool copyKeys, bool trivial )
	: QGDict( size, caseSensitive, copyKeys, trivial ) {}

    QCacheItem *find(const char *key) const
		  { return (QCacheItem*)((QCDict*)this)->look(key, 0, 0); }
    QCacheItem *take(const char *key)
		  { return (QCacheItem*)QGDict::take(key); }
    bool  insert( const char *key, const QCacheItem *ci )
		  { return QGDict::look(key,(Item)ci,1)!=0;}
    bool  removeItem( QCacheItem* item ) 
	          { return QGDict::removeItem( item->key, item ); } 
    void  statistics()			{ QGDict::statistics(); }
};


/*****************************************************************************
  QGDict member functions
 *****************************************************************************/

/*!
  \internal
  Constructs a cache.
*/

QGCache::QGCache( int maxCost, uint size,
		  bool caseS, bool copyKeys, bool trivial )
{
    lruList = new QCList;
    CHECK_PTR( lruList );
    lruList->setAutoDelete( TRUE );
    copyK   = copyKeys;
    dict    = new QCDict( size, caseS, FALSE, trivial );
    CHECK_PTR( dict );
    mCost   = maxCost;
    tCost   = 0;
#if defined(DEBUG)
    lruList->inserts	  = 0;
    lruList->insertCosts  = 0;
    lruList->insertMisses = 0;
    lruList->finds	  = 0;
    lruList->hits	  = 0;
    lruList->hitCosts	  = 0;
    lruList->dumps	  = 0;
    lruList->dumpCosts	  = 0;
#endif
}

/*!
  \internal
  Cannot copy a cache.
*/

QGCache::QGCache( const QGCache & )
    : QCollection()
{
#if defined(CHECK_NULL)
    fatal( "QGCache::QGCache(QGCache &): Cannot copy a cache" );
#endif
}

/*!
  \internal
  Removes all items from the cache and destroys it.
*/

QGCache::~QGCache()
{
    clear();					// delete everything first
    delete dict;
    delete lruList;
}

/*!
  \internal
  Cannot assign a cache.
*/

QGCache &QGCache::operator=( const QGCache & )
{
#if defined(CHECK_NULL)
    fatal( "QGCache::operator=: Cannot copy a cache" );
#endif
    return *this;				// satisfy the compiler
}


/*!
  \fn uint QGCache::count() const
  \internal
  Returns the number of items in the cache.
*/

/*!
  \fn uint QGCache::size() const
  \internal
  Returns the size of the hash array.
*/

/*!
  \fn int QGCache::maxCost() const
  \internal
  Returns the maximum cache cost.
*/

/*!
  \fn int QGCache::totalCost() const
  \internal
  Returns the total cache cost.
*/

/*!
  \internal
  Sets the maximum cache cost.
*/

void QGCache::setMaxCost( int maxCost )
{
    if ( maxCost < tCost ) {
	if ( !makeRoomFor(tCost - maxCost) )	// remove excess cost
	    return;
    }
    mCost = maxCost;
}


/*!
  \internal
  Inserts an item into the cache.

  \warning If this function returns FALSE, you must delete \a data
  yourself.  Additionally, be very careful about using \a data after
  calling this function, as any other insertions into the cache, from
  anywhere in the application, or within Qt itself, could cause the
  data to be discarded from the cache, and the pointer to become
  invalid.

*/

bool QGCache::insert( const char *key, Item data, int cost, int priority )
{
#if defined(CHECK_NULL)
    ASSERT( key != 0 && data != 0 );
#endif
    if ( tCost + cost > mCost ) {
	if ( !makeRoomFor(tCost + cost - mCost, priority) ) {
#if defined(DEBUG)
	    lruList->insertMisses++;
#endif
	    return FALSE;
	}
    }
#if defined(DEBUG)
    lruList->inserts++;
    lruList->insertCosts += cost;
#endif
    if ( copyK )
	key = qstrdup( key );
    if ( priority < -32768 )
	priority = -32768;
    else if ( priority > 32767 )
	priority = 32677;
    QCacheItem *ci = new QCacheItem( key, newItem(data), cost,
				     (short)priority );
    CHECK_PTR( ci );
    lruList->insert( 0, ci );
    dict->insert( key, ci );
    tCost += cost;
    return TRUE;
}

/*!
  \internal
  Removes an item from the cache.
*/

bool QGCache::remove( const char *key )
{
#if defined(CHECK_NULL)
    ASSERT( key != 0 );
#endif
    Item d = take( key );
    if ( d )
	deleteItem( d );
    return d != 0;
}

/*!
  \internal
  Takes an item out of the cache (no delete).
*/

QCollection::Item QGCache::take( const char *key )
{
#if defined(CHECK_NULL)
    ASSERT( key != 0 );
#endif
    QCacheItem *ci = dict->take( key );		// take from dict
    Item d;
    if ( ci ) {
	d = ci->data;
	tCost -= ci->cost;
	if ( copyK )
	    delete [] (char *)ci->key;
	lruList->take( ci );			// take from list
	delete ci;
    }
    else
	d = 0;
    return d;
}

/*!
  \internal
  Clears the cache.
*/

void QGCache::clear()
{
    register QCacheItem *ci;
    while ( (ci = lruList->first()) ) {
	dict->removeItem( ci );			// remove from dict
	deleteItem( ci->data );			// delete data
	if ( copyK )
	    delete [] (char *)ci->key;
	lruList->removeFirst();			// remove from list
    }
    tCost = 0;
}


/*!
  \internal
  Finds an item in the cache.
*/

QCollection::Item QGCache::find( const char *key, bool ref ) const
{
#if defined(CHECK_NULL)
    ASSERT( key != 0 );
#endif
    QCacheItem *ci = dict->find( key );
#if defined(DEBUG)
    lruList->finds++;
#endif
    if ( ci ) {
#if defined(DEBUG)
	lruList->hits++;
	lruList->hitCosts += ci->cost;
#endif
	if ( ref )
	    lruList->reference( ci );
	return ci->data;
    }
    return 0;
}


/*!
  \internal
  Allocates cache space for one or more items.
*/

bool QGCache::makeRoomFor( int cost, int priority )
{
    if ( cost > mCost )				// cannot make room for more
	return FALSE;				//   than maximum cost
    if ( priority == -1 )
	priority = 32767;
    register QCacheItem *ci = lruList->last();
    int cntCost = 0;
    int dumps	= 0;				// number of items to dump
    while ( cntCost < cost && ci && ci->skipPriority <= priority ) {
	cntCost += ci->cost;
	ci	 = lruList->prev();
	dumps++;
    }
    if ( cntCost < cost )			// can enough cost be dumped?
	return FALSE;				// no
#if defined(DEBUG)
    ASSERT( dumps > 0 );
#endif
    while ( dumps-- ) {
	ci = lruList->last();
#if defined(DEBUG)
	lruList->dumps++;
	lruList->dumpCosts += ci->cost;
#endif
	dict->removeItem( ci );			// remove from dict
	if ( copyK )
	    delete [] (char *)ci->key;
	deleteItem( ci->data );			// delete data
	lruList->removeLast();			// remove from list
    }
    tCost -= cntCost;
    return TRUE;
}


/*!
  \internal
  Outputs debug statistics.
*/

void QGCache::statistics() const
{
#if defined(DEBUG)
    QString line;
    line.fill( '*', 80 );
    debug( line );
    debug( "CACHE STATISTICS:" );
    debug( "cache contains %d item%s, with a total cost of %d",
	   count(), count() != 1 ? "s" : "", tCost );
    debug( "maximum cost is %d, cache is %d%% full.",
	   mCost, (200*tCost + mCost) / (mCost*2) );
    debug( "find() has been called %d time%s",
	   lruList->finds, lruList->finds != 1 ? "s" : "" );
    debug( "%d of these were hits, items found had a total cost of %d.",
	   lruList->hits,lruList->hitCosts );
    debug( "%d item%s %s been inserted with a total cost of %d.",
	   lruList->inserts,lruList->inserts != 1 ? "s" : "",
	   lruList->inserts != 1 ? "have" : "has", lruList->insertCosts );
    debug( "%d item%s %s too large or had too low priority to be inserted.",
	   lruList->insertMisses, lruList->insertMisses != 1 ? "s" : "",
	   lruList->insertMisses != 1 ? "were" : "was" );
    debug( "%d item%s %s been thrown away with a total cost of %d.",
	   lruList->dumps, lruList->dumps != 1 ? "s" : "",
	   lruList->dumps != 1 ? "have" : "has", lruList->dumpCosts );
    debug( "Statistics from internal dictionary class:" );
    dict->statistics();
    debug( line );
#endif
}


/*****************************************************************************
  QGCacheIterator member functions
 *****************************************************************************/

/*!
  \class QGCacheIterator qgcache.h

  \brief An internal class for implementing QCacheIterator and QIntCacheIterator.

  QGCacheIterator is a strictly internal class that does the heavy work for
  QCacheIterator and QIntCacheIterator.
*/

/*!
  \internal
  Constructs an iterator that operates on the cache \e c.
*/

QGCacheIterator::QGCacheIterator( const QGCache &c )
{
    it = new QCListIt( c.lruList );
#if defined(DEBUG)
    ASSERT( it != 0 );
#endif
}

/*!
  \internal
  Constructs an iterator that operates on the same cache as \e ci.
*/

QGCacheIterator::QGCacheIterator( const QGCacheIterator &ci )
{
    it = new QCListIt( ci.it );
#if defined(DEBUG)
    ASSERT( it != 0 );
#endif
}

/*!
  \internal
  Destroys the iterator.
*/

QGCacheIterator::~QGCacheIterator()
{
    delete it;
}

/*!
  \internal
  Assigns the iterator \e ci to this cache iterator.
*/

QGCacheIterator &QGCacheIterator::operator=( const QGCacheIterator &ci )
{
    *it = *ci.it;
    return *this;
}

/*!
  \internal
  Returns the number of items in the cache.
*/

uint QGCacheIterator::count() const
{
    return it->count();
}

/*!
  \internal
  Returns TRUE if the iterator points to the first item.
*/

bool  QGCacheIterator::atFirst() const
{
    return it->atFirst();
}

/*!
  \internal
  Returns TRUE if the iterator points to the last item.
*/

bool QGCacheIterator::atLast() const
{
    return it->atLast();
}

/*!
  \internal
  Sets the list iterator to point to the first item in the cache.
*/

QCollection::Item QGCacheIterator::toFirst()
{
    register QCacheItem *item = it->toFirst();
    return item ? item->data : 0;
}

/*!
  \internal
  Sets the list iterator to point to the last item in the cache.
*/

QCollection::Item QGCacheIterator::toLast()
{
    register QCacheItem *item = it->toLast();
    return item ? item->data : 0;
}

/*!
  \internal
  Returns the current item.
*/

QCollection::Item QGCacheIterator::get() const
{
    register QCacheItem *item = it->current();
    return item ? item->data : 0;
}

/*!
  \internal
  Returns the key of the current item.
*/

QString QGCacheIterator::getKey() const
{
    register QCacheItem *item = it->current();
    return item ? QString::fromUtf8(item->key) : QString::null;
}

/*!
  \internal
  Returns the key of the current item, as a long.
*/

long QGCacheIterator::getKeyLong() const
{
    register QCacheItem *item = it->current();
    return item ? (long)item->key : 0L;
}


/*!
  \internal
  Moves to the next item (postfix).
*/

QCollection::Item QGCacheIterator::operator()()
{
    register QCacheItem *item = it->operator()();
    return item ? item->data : 0;
}

/*!
  \internal
  Moves to the next item (prefix).
*/

QCollection::Item QGCacheIterator::operator++()
{
    register QCacheItem *item = it->operator++();
    return item ? item->data : 0;
}

/*!
  \internal
  Moves \e jumps positions forward.
*/

QCollection::Item QGCacheIterator::operator+=( uint jump )
{
    register QCacheItem *item = it->operator+=(jump);
    return item ? item->data : 0;
}

/*!
  \internal
  Moves to the previous item (prefix).
*/

QCollection::Item QGCacheIterator::operator--()
{
    register QCacheItem *item = it->operator--();
    return item ? item->data : 0;
}

/*!
  \internal
  Moves \e jumps positions backward.
*/

QCollection::Item QGCacheIterator::operator-=( uint jump )
{
    register QCacheItem *item = it->operator-=(jump);
    return item ? item->data : 0;
}


bool QGCache::insert( const QString& key, Item i, int cost, int priority )
{
    if ( !copyK )
	fatal("QGCache: attempted to insert QString without copying key");
    QCString kutf8 = key.utf8();
    return insert( kutf8.data(), i, cost, priority );
}

bool QGCache::remove( const QString& key )
{
    QCString kutf8 = key.utf8();
    return remove( kutf8.data() );
}

QGCache::Item QGCache::take( const QString& key )
{
    QCString kutf8 = key.utf8();
    return take( kutf8.data() );
}

QGCache::Item QGCache::find( const QString& key, bool ref ) const
{
    QCString kutf8 = key.utf8();
    return find( kutf8.data(), ref );
}

