/****************************************************************************
** $Id: //depot/qt/main/src/tools/qasciicache.h#1 $
**
** Definition of QAsciiCache template/macro class
**
** Created : 950209
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QASCIICACHE_H
#define QASCIICACHE_H

#ifndef QT_H
#include "qgcache.h"
#endif // QT_H


template<class type> class Q_EXPORT QAsciiCache : public QGCache
{
public:
    QAsciiCache( const QAsciiCache<type> &c ) : QGCache(c) {}
    QAsciiCache( int maxCost=100, int size=17, bool caseSensitive=TRUE,
		 bool copyKeys=TRUE )
	: QGCache( maxCost, size, AsciiKey, caseSensitive, copyKeys ) {}
   ~QAsciiCache()			{ clear(); }
    QAsciiCache<type> &operator=( const QAsciiCache<type> &c )
			{ return (QAsciiCache<type>&)QGCache::operator=(c); }
    int	  maxCost()   const		{ return QGCache::maxCost(); }
    int	  totalCost() const		{ return QGCache::totalCost(); }
    void  setMaxCost( int m )		{ QGCache::setMaxCost(m); }
    uint  count()     const		{ return QGCache::count(); }
    uint  size()      const		{ return QGCache::size(); }
    bool  isEmpty()   const		{ return QGCache::count() == 0; }
    void  clear()			{ QGCache::clear(); }
    bool  insert( const char *k, const type *d, int c=1, int p=0 )
			{ return QGCache::insert_other(k,(Item)d,c,p);}
    bool  remove( const char *k )
			{ return QGCache::remove_other(k); }
    type *take( const char *k )
			{ return (type *)QGCache::take_other(k); }
    type *find( const char *k, bool ref=TRUE ) const
			{ return (type *)QGCache::find_other(k,ref);}
    type *operator[]( const char *k ) const
			{ return (type *)QGCache::find_other(k);}
    void  statistics() const	      { QGCache::statistics(); }
private:
    void  deleteItem( Item d )	      { if ( del_item ) delete (type *)d; }
};



template<class type> class Q_EXPORT QAsciiCacheIterator : public QGCacheIterator
{
public:
    QAsciiCacheIterator( const QAsciiCache<type> &c ):QGCacheIterator((QGCache &)c) {}
    QAsciiCacheIterator( const QAsciiCacheIterator<type> &ci)
				: QGCacheIterator( (QGCacheIterator &)ci ) {}
    QAsciiCacheIterator<type> &operator=(const QAsciiCacheIterator<type>&ci)
	{ return ( QAsciiCacheIterator<type>&)QGCacheIterator::operator=( ci ); }
    uint  count()   const     { return QGCacheIterator::count(); }
    bool  isEmpty() const     { return QGCacheIterator::count() == 0; }
    bool  atFirst() const     { return QGCacheIterator::atFirst(); }
    bool  atLast()  const     { return QGCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)QGCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)QGCacheIterator::toLast(); }
    operator type *() const   { return (type *)QGCacheIterator::get(); }
    type *current()   const   { return (type *)QGCacheIterator::get(); }
    const char *currentKey() const { return QGCacheIterator::getKeyAscii(); }
    type *operator()()	      { return (type *)QGCacheIterator::operator()();}
    type *operator++()	      { return (type *)QGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGCacheIterator::operator-=(j);}
};


#endif // QASCIICACHE_H
