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

#ifndef QGCACHE_H
#define QGCACHE_H

#include "qptrcollection.h"
#include "qglist.h"
#include "qgdict.h"


class QCList;                                        // internal classes
class QCListIt;
class QCDict;


class Q_COMPAT_EXPORT QGCache : public QPtrCollection        // generic LRU cache
{
friend class QGCacheIterator;
protected:
    enum KeyType { StringKey, AsciiKey, IntKey, PtrKey };
      // identical to QGDict's, but PtrKey is not used at the moment

    QGCache(int maxCost, uint size, KeyType kt, bool caseSensitive,
             bool copyKeys);
    QGCache(const QGCache &);                        // not allowed, calls fatal()
   ~QGCache();
    QGCache &operator=(const QGCache &);        // not allowed, calls fatal()

    uint    count()        const;
    uint    size()        const;
    int            maxCost()        const        { return mCost; }
    int            totalCost() const        { return tCost; }
    void    setMaxCost(int maxCost);
    void    clear();

    bool    insert_string(const QString &key, QPtrCollection::Item,
                           int cost, int priority);
    bool    insert_other(const char *key, QPtrCollection::Item,
                          int cost, int priority);
    bool    remove_string(const QString &key);
    bool    remove_other(const char *key);
    QPtrCollection::Item take_string(const QString &key);
    QPtrCollection::Item take_other(const char *key);

    QPtrCollection::Item find_string(const QString &key, bool ref=true) const;
    QPtrCollection::Item find_other(const char *key, bool ref=true) const;

    void    statistics() const;

private:
    bool    makeRoomFor(int cost, int priority = -1);
    KeyType keytype;
    QCList *lruList;
    QCDict *dict;
    int            mCost;
    int            tCost;
    bool    copyk;
};


class Q_COMPAT_EXPORT QGCacheIterator                        // generic cache iterator
{
protected:
    QGCacheIterator(const QGCache &);
    QGCacheIterator(const QGCacheIterator &);
   ~QGCacheIterator();
    QGCacheIterator &operator=(const QGCacheIterator &);

    uint              count()   const;
    bool              atFirst() const;
    bool              atLast()  const;
    QPtrCollection::Item toFirst();
    QPtrCollection::Item toLast();

    QPtrCollection::Item get() const;
    QString              getKeyString() const;
    const char       *getKeyAscii()  const;
    long              getKeyInt()    const;

    QPtrCollection::Item operator()();
    QPtrCollection::Item operator++();
    QPtrCollection::Item operator+=(uint);
    QPtrCollection::Item operator--();
    QPtrCollection::Item operator-=(uint);

protected:
    QCListIt *it;                                // iterator on cache list
};


#endif // QGCACHE_H
