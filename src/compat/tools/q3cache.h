/****************************************************************************
**
** Definition of Q3Cache template class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3CACHE_H
#define Q3CACHE_H

#ifndef QT_H
#include "qgcache.h"
#endif // QT_H

template<class type>
class Q3Cache
#ifdef Q_QDOC
        : public QPtrCollection
#else
        : public QGCache
#endif
{
public:
    Q3Cache(const Q3Cache<type> &c) : QGCache(c) {}
    Q3Cache(int maxCost=100, int size=17, bool caseSensitive=true)
        : QGCache(maxCost, size, StringKey, caseSensitive, false) {}
   ~Q3Cache()                                { clear(); }
    Q3Cache<type> &operator=(const Q3Cache<type> &c)
                        { return (Q3Cache<type>&)QGCache::operator=(c); }
    int          maxCost()   const                { return QGCache::maxCost(); }
    int          totalCost() const                { return QGCache::totalCost(); }
    void  setMaxCost(int m)                { QGCache::setMaxCost(m); }
    uint  count()     const                { return QGCache::count(); }
    uint  size()      const                { return QGCache::size(); }
    bool  isEmpty()   const                { return QGCache::count() == 0; }
    void  clear()                        { QGCache::clear(); }
    bool  insert(const QString &k, const type *d, int c=1, int p=0)
                        { return QGCache::insert_string(k,(Item)d,c,p);}
    bool  remove(const QString &k)
                        { return QGCache::remove_string(k); }
    type *take(const QString &k)
                        { return (type *)QGCache::take_string(k); }
    type *find(const QString &k, bool ref=true) const
                        { return (type *)QGCache::find_string(k,ref);}
    type *operator[](const QString &k) const
                        { return (type *)QGCache::find_string(k);}
    void  statistics() const              { QGCache::statistics(); }
private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3Cache<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void Q3Cache<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete (type *)d;
}

template<class type>
class Q3CacheIterator : public QGCacheIterator
{
public:
    Q3CacheIterator(const Q3Cache<type> &c):QGCacheIterator((QGCache &)c) {}
    Q3CacheIterator(const Q3CacheIterator<type> &ci)
                                : QGCacheIterator((QGCacheIterator &)ci) {}
    Q3CacheIterator<type> &operator=(const Q3CacheIterator<type>&ci)
        { return (Q3CacheIterator<type>&)QGCacheIterator::operator=(ci); }
    uint  count()   const     { return QGCacheIterator::count(); }
    bool  isEmpty() const     { return QGCacheIterator::count() == 0; }
    bool  atFirst() const     { return QGCacheIterator::atFirst(); }
    bool  atLast()  const     { return QGCacheIterator::atLast(); }
    type *toFirst()              { return (type *)QGCacheIterator::toFirst(); }
    type *toLast()              { return (type *)QGCacheIterator::toLast(); }
    operator type *() const   { return (type *)QGCacheIterator::get(); }
    type *current()   const   { return (type *)QGCacheIterator::get(); }
    QString currentKey() const{ return QGCacheIterator::getKeyString(); }
    type *operator()()              { return (type *)QGCacheIterator::operator()();}
    type *operator++()              { return (type *)QGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGCacheIterator::operator+=(j);}
    type *operator--()              { return (type *)QGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGCacheIterator::operator-=(j);}
};

#endif // Q3CACHE_H
