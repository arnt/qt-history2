/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcache.h#18 $
**
** Definition of QCache template/macro class
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

#ifndef QCACHE_H
#define QCACHE_H

#ifndef QT_H
#include "qgcache.h"
#endif // QT_H


template<class type> class Q_EXPORT QCache : public QGCache
{
public:
    QCache( const QCache<type> &c ) : QGCache(c) {}
    QCache( int maxCost=100, int size=17, bool cs=TRUE, bool ck=TRUE )
	: QGCache( maxCost, size, cs, ck, FALSE ) {}
   ~QCache()			      { clear(); }
    QCache<type> &operator=( const QCache<type> &c )
			{ return (QCache<type>&)QGCache::operator=(c); }
    int	  maxCost()   const	      { return QGCache::maxCost(); }
    int	  totalCost() const	      { return QGCache::totalCost(); }
    void  setMaxCost( int m )	      { QGCache::setMaxCost(m); }
    uint  count()     const	      { return QGCache::count(); }
    uint  size()      const	      { return QGCache::size(); }
    bool  isEmpty()   const	      { return QGCache::count() == 0; }
    void  clear()		      { QGCache::clear(); }

    bool  insert( const char *k, const type *d, int c=1, int p=0 )
				      { return QGCache::insert(k,(Item)d,c,p);}
    bool  remove( const char *k )     { return QGCache::remove(k); }
    type *take( const char *k )	      { return (type *)QGCache::take(k); }
    type *find( const char *k, bool ref=TRUE ) const
				      { return (type *)QGCache::find(k,ref);}
    type *operator[]( const char *k ) const
				      { return (type *)QGCache::find(k);}

    bool  insert( const QString& k, const type *d, int c=1, int p=0 )
				      { return QGCache::insert(k,(Item)d,c,p);}
    bool  remove( const QString& k )     { return QGCache::remove(k); }
    type *take( const QString& k )	      { return (type *)QGCache::take(k); }
    type *find( const QString& k, bool ref=TRUE ) const
				      { return (type *)QGCache::find(k,ref);}
    type *operator[]( const QString& k ) const
				      { return (type *)QGCache::find(k);}

    void  statistics() const	      { QGCache::statistics(); }
private:
    void  deleteItem( Item d )	      { if ( del_item ) delete (type *)d; }
};



template<class type> class Q_EXPORT QCacheIterator : public QGCacheIterator
{
public:
    QCacheIterator( const QCache<type> &c ):QGCacheIterator((QGCache &)c) {}
    QCacheIterator( const QCacheIterator<type> &ci)
				: QGCacheIterator( (QGCacheIterator &)ci ) {}
    QCacheIterator<type> &operator=(const QCacheIterator<type>&ci)
	{ return ( QCacheIterator<type>&)QGCacheIterator::operator=( ci ); }
    uint  count()   const     { return QGCacheIterator::count(); }
    bool  isEmpty() const     { return QGCacheIterator::count() == 0; }
    bool  atFirst() const     { return QGCacheIterator::atFirst(); }
    bool  atLast()  const     { return QGCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)QGCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)QGCacheIterator::toLast(); }
    operator type *() const   { return (type *)QGCacheIterator::get(); }
    type *current()   const   { return (type *)QGCacheIterator::get(); }
    long currentKeyLong() const
			      { return QGCacheIterator::getKeyLong();}
    QString currentKey() const
			      { return QGCacheIterator::getKey();}
    type *operator()()	      { return (type *)QGCacheIterator::operator()();}
    type *operator++()	      { return (type *)QGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGCacheIterator::operator-=(j);}
};


#endif // QCACHE_H
