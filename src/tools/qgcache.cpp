/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.cpp#2 $
**
** Implementation of QGCache class
**
** Author  : Eirik Eng
** Created : 950208
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qgcache.h"
#include "qlist.h"
#include "qdict.h"
#include "qstring.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qgcache.cpp#2 $";
#endif


struct CacheItem {
    CacheItem( const char *k, GCI d, long c, short p )
        { key=k; data=d; cost=c; skipPriority=priority=p; }
    short        priority;
    short        skipPriority;
    long         cost;
    const char  *key;
    GCI          data;
};

declare(QListM,CacheItem);

class QCList : public QListM(CacheItem)
{
public:
    void         reference( CacheItem * ); // reference cache item
    void         append( CacheItem * );    // Uses priority to place item
#if defined(DEBUG)
    long         inserts;                  // variables used for statistics
    long         insertCosts;
    long         insertMisses;
    long         finds;
    long         hits;
    long         hitCosts;
    long         dumps;
    long         dumpCosts;
#endif
};

// Since we need to decide if the dictionary should use an int or const
// char * key ( the "bool trivial" argument in the constructor below )
// we cannot use the macro/template dict, but inherit directly from QGDict 

class QCDict : public QGDict {
public:
    QCDict( uint size, bool caseSensitive, bool copyKeys, bool trivial )
        : QGDict( size, caseSensitive, copyKeys, trivial ){}

    CacheItem *find(const char *key) const 
                  { return (CacheItem*) ((QCDict*)this)->look( key, 0, 0); }
    CacheItem *take(const char *key) 
                  { return (CacheItem*) QGDict::take(key); } 
    bool  insert( const char *key, const CacheItem *ci )
		  { return QGDict::look(key,(GCI)ci,1)!=0;}
    bool  remove( const char *key )    { QGDict::remove(key); }
    void statistics()                  { QGDict::statistics(); }
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
    copyK   = copyKeys != 0;           // copyK is a bitfield, play safe
    dict    = new QCDict( size, caseS, FALSE, trivial );
    CHECK_PTR( dict );
    mCost   = maxCost;
#if defined(DEBUG)
    lruList->inserts      = 0;
    lruList->insertCosts  = 0;
    lruList->insertMisses = 0;
    lruList->finds        = 0;
    lruList->hits         = 0;
    lruList->hitCosts     = 0;
    lruList->dumps        = 0;
    lruList->dumpCosts    = 0;
#endif
}

QGCache::QGCache( const QGCache & )
{
    fatal("QGCache::QGCache( QGCache & ): You can't copy a cache.");
}

QGCache::~QGCache()
{
    clear();		      // delete everything
    delete dict;              // must call clear() first for destructors
    delete lruList;           // to be called for all cached items!!!
}

QGCache &QGCache::operator=( const QGCache & )
{
    fatal("QGCache::operator= : You can't copy a cache.");
    return *this;             // satisfy the compiler
}

uint QGCache::count() const
{
    return lruList->count();
}

uint QGCache::size() const
{
    return dict->size();
}

long QGCache::maxCost() const
{
    return mCost;
}

long QGCache::totalCost() const
{
    return tCost;
}

void QGCache::setMaxCost( long maxC )
{
    if ( maxC < totalCost() )
        makeRoomFor( totalCost() - maxC );  // remove excess cost
    mCost = maxC;
}


GCI QGCache::find( const char *key ) const
{
    CacheItem *tmp = dict->find( key );
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
        if ( !makeRoomFor( tCost + cost - mCost, priority ) ) {
#if defined(DEBUG)
            lruList->insertMisses++;
#endif
            return FALSE;
	}
#if defined(DEBUG)
    }
    lruList->inserts++;
    lruList->insertCosts += cost;
#endif
    if ( copyK )
        key = strdup( key );
    CacheItem *ci = new CacheItem( key, newItem(data), cost, priority );
    lruList->append( ci );
    dict->insert( key, ci );
    tCost += cost;
    return TRUE;
}

void QGCache::reference( GCI data ) const
{
    CacheItem *tmp = lruList->first();
    while( tmp ) {
        if ( tmp->data == data ) {
            lruList->reference( tmp );
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
    CacheItem *tmp = dict->take( key );  // remove from dict
    GCI gci = tmp ? tmp->data : 0;
    if ( tmp ) {
        tCost -= tmp->cost;
        if ( copyK )
            delete[] (char*) tmp->key;
        lruList->remove( tmp );         // remove from list and delete
    }
    return gci;
}

void QGCache::clear()
{
    CacheItem *f;
    while ( (f = lruList->first()) ) {
        deleteItem( f->data );      // delete data
        if ( copyK )
            delete[] (char*) f->key;
        dict->remove( f->key );     // remove from cache
        lruList->removeFirst();     //   --- "" ---
    }
    tCost = 0;
}

bool QGCache::makeRoomFor( long cost, short priority )
{
    if ( cost > mCost )
        return FALSE;   // cannot make room for more than maximum cost
    if ( priority == -1 )
        priority = 32767;   // ### Use const from qglobal.h???
    register CacheItem *tmp = lruList->first();
    long cntCost = 0;
    while ( cntCost < cost && tmp && tmp->skipPriority <= priority ) {
        cntCost += tmp->cost;
        tmp      = lruList->next();
    }
    if ( cntCost < cost ) // can enough cost be dumped?
        return FALSE;     // no
    int count = tmp ? lruList->at() : lruList->count(); // no. of items to dump
    CacheItem *f;
    while ( count-- ) {
        f = lruList->first();
#if defined(DEBUG)
        lruList->dumps++;
        lruList->dumpCosts += f->cost;
#endif
        deleteItem( f->data );      // delete data
        dict->remove( f->key );     // remove from cache
        lruList->removeFirst();     //   --- "" ---
    }
    tCost -= cntCost;
    return TRUE;
}

void QGCache::statistics() const
{
#if defined(DEBUG)
    QString line;
    line.fill( '*', 80 );
    double real, ideal;
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

void QCList::reference( CacheItem *ci )
{
    if ( findRef( ci ) >= 0 ) {
        take();
        ci->skipPriority = ci->priority;
        append( ci );
    }
}

void QCList::append( CacheItem *ci )
{
    CacheItem *tmp = last();
    while( tmp && tmp->skipPriority > ci->priority ) {
        tmp->skipPriority--;
        tmp = prev();
    }
    if ( tmp )
        insert( at() + 1, ci );
    else
        insert( ci );
}

// --------------------------------------------------------------------------
// QGListIterator member functions
//

declare(QListIteratorM,CacheItem);


QGCacheIterator::QGCacheIterator( const QGCache &c )
{
    it = new QListIteratorM(CacheItem)(*c.lruList);
    CHECK_PTR( it );
}

QGCacheIterator::QGCacheIterator( const QGCacheIterator &ci )
{
    it = new QListIteratorM(CacheItem)(*ci.it);
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
    register CacheItem *tmp = it->toFirst();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::toLast()
{
    register CacheItem *tmp = it->toLast();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::get() const
{
    register CacheItem *tmp = it->current();
    return tmp ? tmp->data : 0;
}

const char *QGCacheIterator::getKey() const
{
    register CacheItem *tmp = it->current();
    return tmp ? tmp->key : 0;
}

GCI QGCacheIterator::operator()()
{
    register CacheItem *tmp = it->operator()();
    return tmp ? tmp->data : 0;
}
	
GCI QGCacheIterator::operator++()
{
    register CacheItem *tmp = it->operator++();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::operator+=(uint i)
{
    register CacheItem *tmp = it->operator+=(i);
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::operator--()
{
    register CacheItem *tmp = it->operator--();
    return tmp ? tmp->data : 0;
}

GCI QGCacheIterator::operator-=(uint i)
{
    register CacheItem *tmp = it->operator-=(i);
    return tmp ? tmp->data : 0;
}

