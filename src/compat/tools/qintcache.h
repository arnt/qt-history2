/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QINTCACHE_H
#define QINTCACHE_H

#ifndef QT_H
#include "qgcache.h"
#endif // QT_H


template<class type>
class QIntCache : public QGCache
{
public:
    QIntCache(const QIntCache<type> &c) : QGCache(c) {}
    QIntCache(int maxCost=100, int size=17)
        : QGCache(maxCost, size, IntKey, false, false) {}
   ~QIntCache()                { clear(); }
    QIntCache<type> &operator=(const QIntCache<type> &c)
                        { return (QIntCache<type>&)QGCache::operator=(c); }
    int          maxCost()   const        { return QGCache::maxCost(); }
    int          totalCost() const        { return QGCache::totalCost(); }
    void  setMaxCost(int m)        { QGCache::setMaxCost(m); }
    uint  count()     const        { return QGCache::count(); }
    uint  size()      const        { return QGCache::size(); }
    bool  isEmpty()   const        { return QGCache::count() == 0; }
    bool  insert(long k, const type *d, int c=1, int p=0)
                { return QGCache::insert_other((const char*)k,(Item)d,c,p); }
    bool  remove(long k)
                { return QGCache::remove_other((const char*)k); }
    type *take(long k)
                { return (type *)QGCache::take_other((const char*)k);}
    void  clear()                { QGCache::clear(); }
    type *find(long k, bool ref=true) const
                { return (type *)QGCache::find_other((const char*)k,ref);}
    type *operator[](long k) const
                { return (type *)QGCache::find_other((const char*)k); }
    void  statistics() const { QGCache::statistics(); }
private:
        void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QIntCache<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void QIntCache<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete (type *)d;
}

template<class type>
class QIntCacheIterator : public QGCacheIterator
{
public:
    QIntCacheIterator(const QIntCache<type> &c)
        : QGCacheIterator((QGCache &)c) {}
    QIntCacheIterator(const QIntCacheIterator<type> &ci)
                              : QGCacheIterator((QGCacheIterator &)ci) {}
    QIntCacheIterator<type> &operator=(const QIntCacheIterator<type>&ci)
        { return (QIntCacheIterator<type>&)QGCacheIterator::operator=(ci);}
    uint  count()   const     { return QGCacheIterator::count(); }
    bool  isEmpty() const     { return QGCacheIterator::count() == 0; }
    bool  atFirst() const     { return QGCacheIterator::atFirst(); }
    bool  atLast()  const     { return QGCacheIterator::atLast(); }
    type *toFirst()              { return (type *)QGCacheIterator::toFirst(); }
    type *toLast()              { return (type *)QGCacheIterator::toLast(); }
    operator type *()  const  { return (type *)QGCacheIterator::get(); }
    type *current()    const  { return (type *)QGCacheIterator::get(); }
    long  currentKey() const  { return (long)QGCacheIterator::getKeyInt();}
    type *operator()()              { return (type *)QGCacheIterator::operator()();}
    type *operator++()              { return (type *)QGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGCacheIterator::operator+=(j);}
    type *operator--()              { return (type *)QGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGCacheIterator::operator-=(j);}
};


#endif // QINTCACHE_H
