/****************************************************************************
** $Id: //depot/qt/main/src/tools/qintcache.h#18 $
**
** Definition of QIntCache template/macro class
**
** Created : 950209
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

#ifndef QINTCACHE_H
#define QINTCACHE_H

#ifndef QT_H
#include "qgcache.h"
#endif // QT_H


template<class type> class Q_EXPORT QIntCache : public QGCache
{
public:
    QIntCache( const QIntCache<type> &c ) : QGCache(c) {}
    QIntCache( int maxCost=100, int size=17 )
	: QGCache( maxCost, size, FALSE, FALSE, TRUE ) {}
   ~QIntCache()	     { clear(); }
    QIntCache<type> &operator=( const QIntCache<type> &c )
			{ return (QIntCache<type>&)QGCache::operator=(c); }
    int	  maxCost()   const	{ return QGCache::maxCost(); }
    int	  totalCost() const	{ return QGCache::totalCost(); }
    void  setMaxCost( int m)	{ QGCache::setMaxCost(m); }
    uint  count()     const	{ return QGCache::count(); }
    uint  size()      const	{ return QGCache::size(); }
    bool  isEmpty()   const	{ return QGCache::count() == 0; }
    bool  insert( long k, const type *d, int c=1, int p=0 )
			{ return QGCache::insert((const char*)k,(GCI)d,c,p); }
    bool  remove( long k )	{ return QGCache::remove((const char*)k); }
    type *take( long k )    { return (type *)QGCache::take((const char*)k);}
    void  clear()	    { QGCache::clear(); }
    type *find( long k, bool ref=TRUE ) const
			{ return (type *)QGCache::find( (const char*)k,ref);}
    type *operator[]( long k ) const
			{ return (type *)QGCache::find( (const char*)k); }
    void  statistics() const { QGCache::statistics(); }
private:
    void  deleteItem( GCI d )	{ if ( del_item ) delete (type *)d; }
};


template<class type> class Q_EXPORT QIntCacheIterator : public QGCacheIterator
{
public:
    QIntCacheIterator( const QIntCache<type> &c )
	: QGCacheIterator( (QGCache &)c ) {}
    QIntCacheIterator( const QIntCacheIterator<type> &ci )
			      : QGCacheIterator((QGCacheIterator &)ci) {}
    QIntCacheIterator<type> &operator=( const QIntCacheIterator<type>&ci )
	{ return ( QIntCacheIterator<type>&)QGCacheIterator::operator=( ci );}
    uint  count()   const     { return QGCacheIterator::count(); }
    bool  isEmpty() const     { return QGCacheIterator::count() == 0; }
    bool  atFirst() const     { return QGCacheIterator::atFirst(); }
    bool  atLast()  const     { return QGCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)QGCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)QGCacheIterator::toLast(); }
    operator type *()  const  { return (type *)QGCacheIterator::get(); }
    type *current()    const  { return (type *)QGCacheIterator::get(); }
    long  currentKey() const  { return (long)QGCacheIterator::getKey();}
    type *operator()()	      { return (type *)QGCacheIterator::operator()();}
    type *operator++()	      { return (type *)QGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGCacheIterator::operator-=(j);}
};


#endif // QINTCACHE_H
