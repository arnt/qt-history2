/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.cpp#13 $
**
** Implementation of QGCache and QGCacheIterator classes
**
** Author  : Eirik Eng
** Created : 950208
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qgcache.h"
#include "qlist.h"
#include "qdict.h"
#include "qstring.h"				/* used for statistics */

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qgcache.cpp#13 $";
#endif


// --------------------------------------------------------------------------
// Internal classes for QGCache
//

struct QCacheItem {
    QCacheItem( const char *k, GCI d, long c, short p )
	{ key=k; data=d; cost=c; skipPriority=priority=p; }
    short	priority;
    short	skipPriority;
    long	cost;
    const char *key;
    GCI		data;
};

declare(QListM,QCacheItem);

class QCList : public QListM(QCacheItem)	// internal cache list
{
public:
    void  reference( QCacheItem * );		// reference cache item
    void  insert( QCacheItem * );		// uses priority to place item
    void  insert( int i,QCacheItem *c ){QListM(QCacheItem)::insert(i,c);}
#if defined(DEBUG)
    long  inserts;				// variables for statistics
    long  insertCosts;
    long  insertMisses;
    long  finds;
    long  hits;
    long  hitCosts;
    long  dumps;
    long  dumpCosts;
#endif
};

void QCList::reference( QCacheItem *ci )
{
    if ( ci == get() || findRef( ci ) >= 0 ) {
	take();
	ci->skipPriority = ci->priority;
	insert( ci );
    }
}

void QCList::insert( QCacheItem *ci )
{
    QCacheItem *tmp = first();
    while( tmp && tmp->skipPriority > ci->priority ) {
	tmp->skipPriority--;
	tmp = next();
    }
    if ( tmp )
	insert( at(), ci );
    else
	append( ci );
}

//
// Since we need to decide if the dictionary should use an int or const
// char * key (the "bool trivial" argument in the constructor below)
// we cannot use the macro/template dict, but inherit directly from QGDict.
//

class QCDict : public QGDict {			// internal cache dict
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
    fatal( "QGCache::QGCache(QGCache &): You can't copy a cache" );
}

QGCache::~QGCache()
{
    clear();					// delete everything first
    delete dict;
    delete lruList;
}

QGCache &QGCache::operator=( const QGCache & )
{
    fatal( "QGCache::operator=: You can't copy a cache" );
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


GCI QGCache::find( const char *key ) const
{
    QCacheItem *tmp = dict->find( key );
    if ( tmp )
	lruList->reference( tmp );
#if defined(DEBUG)
    lruList->finds++;
    if ( tmp ) {
	lruList->hits++;
	lruList->hitCosts += tmp->cost;
    }
#endif
    return tmp ? tmp->data : 0;
}

bool QGCache::insert( const char *key, GCI data, long cost, int priority )
{
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
	key = strdup( key );
    QCacheItem *ci = new QCacheItem( key, newItem(data), cost,
				     (short)priority );
    CHECK_PTR( ci );
    lruList->insert( ci );
    dict->insert( key, ci );
    tCost += cost;
    return TRUE;
}

void QGCache::reference( GCI data ) const
{
    if ( !data )
	return;
    register QCacheItem *tmp = lruList->first();
    while( tmp ) {
	if ( tmp->data == data ) {
	    lruList->take();
	    tmp->skipPriority = tmp->priority;
	    lruList->insert( tmp );
	    return;
	}
	tmp = lruList->next();
    }
}

bool QGCache::remove( const char *key )
{
    GCI tmp = take( key );
    if ( tmp )
	deleteItem( tmp );
    return tmp != 0;
}

GCI QGCache::take( const char *key )
{
    QCacheItem *tmp = dict->take( key );	// remove from dict
    GCI gci = tmp ? tmp->data : 0;
    if ( tmp ) {
	tCost -= tmp->cost;
	ASSERT( lruList->findRef( tmp ) >= 0 );
	lruList->remove();			// remove from list and delete
    }
    return gci;
}

void QGCache::clear()
{
    register QCacheItem *ci;
    while ( (ci = lruList->first()) ) {
	dict->remove( ci->key );		// remove from dict
	deleteItem( ci->data );			// delete data
	if ( copyK )
	    delete (char*)ci->key;
	lruList->removeFirst();			// remove from list
    }
    tCost = 0;
}

bool QGCache::makeRoomFor( long cost, int priority )
{
    if ( cost > mCost )				// cannot make room for more
	return FALSE;				//   than maximum cost
    if ( priority == -1 )
	priority = 32767;			// use const from qglobal.h???
    register QCacheItem *tmp = lruList->last();
    long cntCost = 0;
    int	 dumps	 = 0;				// number of items to dump
    while ( cntCost < cost && tmp && tmp->skipPriority <= priority ) {
	cntCost += tmp->cost;
	tmp	 = lruList->prev();
	dumps++;
    }
    if ( cntCost < cost )			// can enough cost be dumped?
	return FALSE;				// no
    ASSERT( dumps > 0 );
    register QCacheItem *ci;
    while ( dumps-- ) {
	ci = lruList->last();
#if defined(DEBUG)
	lruList->dumps++;
	lruList->dumpCosts += ci->cost;
#endif
	dict->remove( ci->key );		// remove from dict
	if ( copyK )
	    delete (char*)ci->key;
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
    debug( "cache contains %i item%s, with a total cost of %i",
	   count(), count() != 1 ? "s" : "", tCost );
    debug( "maximum cost is %i, cache is %i%% full.",
	   mCost, (200*tCost + mCost) / (mCost*2) );
    debug( "find() has been called %i time%s",
	   lruList->finds, lruList->finds != 1 ? "s" : "" );
    debug( "%i of these were hits, items found had a total cost of %i.",
	   lruList->hits,lruList->hitCosts );
    debug( "%i item%s %s been inserted with a total cost of %i.",
	   lruList->inserts,lruList->inserts != 1 ? "s" : "",
	   lruList->inserts != 1 ? "have" : "has", lruList->insertCosts );
    debug( "%i item%s %s too large or had too low priority to be inserted.",
	   lruList->insertMisses, lruList->insertMisses != 1 ? "s" : "",
	   lruList->insertMisses != 1 ? "were" : "was" );
    debug( "%i item%s %s been thrown away with a total cost of %i.",
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
    it = new QListIteratorM(QCacheItem)(*c.lruList);
    CHECK_PTR( it );
}

QGCacheIterator::QGCacheIterator( const QGCacheIterator &ci )
{
    it = new QListIteratorM(QCacheItem)(*ci.it);
    CHECK_PTR( it );
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
    register QCacheItem *tmp = it->toFirst();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::toLast()
{
    register QCacheItem *tmp = it->toLast();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::get() const
{
    register QCacheItem *tmp = it->current();
    return tmp ? tmp->data : 0;
}

const char *QGCacheIterator::getKey() const
{
    register QCacheItem *tmp = it->current();
    return tmp ? tmp->key : 0;
}

GCI QGCacheIterator::operator()()
{
    register QCacheItem *tmp = it->operator()();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::operator++()
{
    register QCacheItem *tmp = it->operator++();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::operator+=( uint i )
{
    register QCacheItem *tmp = it->operator+=(i);
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::operator--()
{
    register QCacheItem *tmp = it->operator--();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::operator-=( uint i )
{
    register QCacheItem *tmp = it->operator-=(i);
    return tmp ? tmp->data : 0;
}
