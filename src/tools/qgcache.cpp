/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.cpp#23 $
**
** Implementation of QGCache and QGCacheIterator classes
**
** Author  : Eirik Eng
** Created : 950208
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qgcache.h"
#include "qlist.h"
#include "qdict.h"
#include "qstring.h"				/* used for statistics */

RCSTAG("$Id: //depot/qt/main/src/tools/qgcache.cpp#23 $")


// --------------------------------------------------------------------------
// Internal classes for QGCache
//

struct QCacheItem
{
    QCacheItem( const char *k, GCI d, long c, short p )
	{ key=k; data=d; cost=c; skipPriority=priority=p; node=0; }
    short	priority;
    short	skipPriority;
    long	cost;
    const char *key;
    GCI		data;
    QLNode     *node;
};

declare(QListM,QCacheItem);

class QCList : private QListM(QCacheItem)	// internal cache list
{
friend class QGCacheIterator;
public:
    QCList()	{}
   ~QCList();

    void	insert( QCacheItem * );		// insert according to priority
    void	insert( int, QCacheItem * );
    void	take( QCacheItem * );
    void	reference( QCacheItem * );

    void	setAutoDelete( bool del ) { QCollection::setAutoDelete(del); }

    bool	removeFirst()	{ return QListM(QCacheItem)::removeFirst(); }
    bool	removeLast()	{ return QListM(QCacheItem)::removeLast(); }

    QCacheItem *first()		{ return QListM(QCacheItem)::first(); }
    QCacheItem *last()		{ return QListM(QCacheItem)::last(); }
    QCacheItem *prev()		{ return QListM(QCacheItem)::prev(); }
    QCacheItem *next()		{ return QListM(QCacheItem)::next(); }

#if defined(DEBUG)
    long	inserts;			// variables for statistics
    long	insertCosts;
    long	insertMisses;
    long	finds;
    long	hits;
    long	hitCosts;
    long	dumps;
    long	dumpCosts;
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
	QListM(QCacheItem)::insert( at(), ci );
    else
	append( ci );
#if defined(DEBUG)
    ASSERT( ci->node == 0 );
#endif
    ci->node = currentNode();
}

inline void QCList::insert( int i, QCacheItem *ci )
{
    QListM(QCacheItem)::insert( i, ci );
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


//
// Since we need to decide if the dictionary should use an int or const
// char * key (the "bool trivial" argument in the constructor below)
// we cannot use the macro/template dict, but inherit directly from QGDict.
//

class QCDict : public QGDict			// internal cache dict
{
public:
    QCDict( uint size, bool caseSensitive, bool copyKeys, bool trivial )
	: QGDict( size, caseSensitive, copyKeys, trivial ) {}

    QCacheItem *find(const char *key) const
		  { return (QCacheItem*)((QCDict*)this)->look(key, 0, 0); }
    QCacheItem *take(const char *key)
		  { return (QCacheItem*)QGDict::take(key); }
    bool  insert( const char *key, const QCacheItem *ci )
		  { return QGDict::look(key,(GCI)ci,1)!=0;}
    bool  remove( const char *key )	{ return QGDict::remove(key); }
    void  statistics()			{ QGDict::statistics(); }
};


// --------------------------------------------------------------------------
// QGCache member functions
//

QGCache::QGCache( long maxCost, uint size,
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

QGCache::QGCache( const QGCache & )
{
#if defined(CHECK_NULL)
    fatal( "QGCache::QGCache(QGCache &): Cannot copy a cache" );
#endif
}

QGCache::~QGCache()
{
    clear();					// delete everything first
    delete dict;
    delete lruList;
}

QGCache &QGCache::operator=( const QGCache & )
{
    fatal( "QGCache::operator=: Cannot copy a cache" );
    return *this;				// satisfy the compiler
}


void QGCache::setMaxCost( long maxCost )
{
    if ( maxCost < tCost ) {
	if ( !makeRoomFor(tCost - maxCost) )	// remove excess cost
	    return;
    }
    mCost = maxCost;
}


bool QGCache::insert( const char *key, GCI data, long cost, int priority )
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
    QCacheItem *ci = new QCacheItem( key, newItem(data), cost,
				     (short)priority );
    CHECK_PTR( ci );
    lruList->insert( 0, ci );
    dict->insert( key, ci );
    tCost += cost;
    return TRUE;
}

bool QGCache::remove( const char *key )
{
#if defined(CHECK_NULL)
    ASSERT( key != 0 );
#endif
    GCI d = take( key );
    if ( d )
	deleteItem( d );
    return d != 0;
}

GCI QGCache::take( const char *key )
{
#if defined(CHECK_NULL)
    ASSERT( key != 0 );
#endif
    QCacheItem *ci = dict->take( key );		// take from dict
    GCI d;
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

void QGCache::clear()
{
    register QCacheItem *ci;
    while ( (ci = lruList->first()) ) {
	dict->remove( ci->key );		// remove from dict
	deleteItem( ci->data );			// delete data
	if ( copyK )
	    delete [] (char *)ci->key;
	lruList->removeFirst();			// remove from list
    }
    tCost = 0;
}


GCI QGCache::find( const char *key, bool ref ) const
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


bool QGCache::makeRoomFor( long cost, int priority )
{
    if ( cost > mCost )				// cannot make room for more
	return FALSE;				//   than maximum cost
    if ( priority == -1 )
	priority = 32767;
    register QCacheItem *ci = lruList->last();
    long cntCost = 0;
    int	 dumps	 = 0;				// number of items to dump
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
	dict->remove( ci->key );		// remove from dict
	if ( copyK )
	    delete [] (char *)ci->key;
	deleteItem( ci->data );			// delete data
	lruList->removeLast();			// remove from list
    }
    tCost -= cntCost;
    return TRUE;
}


void QGCache::statistics() const
{
#if defined(DEBUG)
    QString line;
    line.fill( '*', 80 );
    debug( line );
    debug( "CACHE STATISTICS:" );
    debug( "cache contains %d item%s, with a total cost of %ld",
	   count(), count() != 1 ? "s" : "", tCost );
    debug( "maximum cost is %ld, cache is %ld%% full.",
	   mCost, (200*tCost + mCost) / (mCost*2) );
    debug( "find() has been called %ld time%s",
	   lruList->finds, lruList->finds != 1 ? "s" : "" );
    debug( "%ld of these were hits, items found had a total cost of %ld.",
	   lruList->hits,lruList->hitCosts );
    debug( "%ld item%s %s been inserted with a total cost of %ld.",
	   lruList->inserts,lruList->inserts != 1 ? "s" : "",
	   lruList->inserts != 1 ? "have" : "has", lruList->insertCosts );
    debug( "%ld item%s %s too large or had too low priority to be inserted.",
	   lruList->insertMisses, lruList->insertMisses != 1 ? "s" : "",
	   lruList->insertMisses != 1 ? "were" : "was" );
    debug( "%ld item%s %s been thrown away with a total cost of %ld.",
	   lruList->dumps, lruList->dumps != 1 ? "s" : "",
	   lruList->dumps != 1 ? "have" : "has", lruList->dumpCosts );
    debug( "Statistics from internal dictionary class:" );
    dict->statistics();
    debug( line );
#endif
}


// --------------------------------------------------------------------------
// QGCacheIterator member functions
//

declare(QListIteratorM,QCacheItem);


QGCacheIterator::QGCacheIterator( const QGCache &c )
{
    it = new QListIteratorM(QCacheItem)( *((QListM(QCacheItem)*)c.lruList) );
#if defined(DEBUG)
    ASSERT( it != 0 );
#endif
}

QGCacheIterator::QGCacheIterator( const QGCacheIterator &ci )
{
    it = new QListIteratorM(QCacheItem)( *ci.it );
#if defined(DEBUG)
    ASSERT( it != 0 );
#endif
}

QGCacheIterator::~QGCacheIterator()
{
    delete it;
}

QGCacheIterator &QGCacheIterator::operator=( const QGCacheIterator &ci )
{
    *it = *ci.it;
    return *this;
}

uint QGCacheIterator::count() const
{
    return it->count();
}

bool  QGCacheIterator::atFirst() const
{
    return it->atFirst();
}

bool QGCacheIterator::atLast() const
{
    return it->atLast();
}

GCI QGCacheIterator::toFirst()
{
    register QCacheItem *item = it->toFirst();
    return item ? item->data : 0;
}

GCI QGCacheIterator::toLast()
{
    register QCacheItem *item = it->toLast();
    return item ? item->data : 0;
}

GCI QGCacheIterator::get() const
{
    register QCacheItem *item = it->current();
    return item ? item->data : 0;
}

const char *QGCacheIterator::getKey() const
{
    register QCacheItem *item = it->current();
    return item ? item->key : 0;
}

GCI QGCacheIterator::operator()()
{
    register QCacheItem *item = it->operator()();
    return item ? item->data : 0;
}

GCI QGCacheIterator::operator++()
{
    register QCacheItem *item = it->operator++();
    return item ? item->data : 0;
}

GCI QGCacheIterator::operator+=( uint i )
{
    register QCacheItem *item = it->operator+=(i);
    return item ? item->data : 0;
}

GCI QGCacheIterator::operator--()
{
    register QCacheItem *item = it->operator--();
    return item ? item->data : 0;
}

GCI QGCacheIterator::operator-=( uint i )
{
    register QCacheItem *item = it->operator-=(i);
    return item ? item->data : 0;
}
