/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.h#28 $
**
** Definition of QGCache and QGCacheIterator classes
**
** Created : 950208
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QGCACHE_H
#define QGCACHE_H

#ifndef QT_H
#include "qcollection.h"
#include "qglist.h"
#include "qgdict.h"
#endif // QT_H


class QCList;					// internal classes
class QCListIt;
class QCDict;


class Q_EXPORT QGCache : public QCollection	// generic LRU cache
{
friend class QGCacheIterator;
protected:
    enum KeyType { StringKey, AsciiKey, IntKey, PtrKey };
      // identical to QGDict's, but PtrKey is not used at the moment

    QGCache( int maxCost, uint size, KeyType kt, bool caseSensitive,
	     bool copyKeys );
    QGCache( const QGCache & );			// not allowed, calls fatal()
   ~QGCache();
    QGCache &operator=( const QGCache & );	// not allowed, calls fatal()

    uint    count()	const	{ return ((QGDict*)dict)->count(); }
    uint    size()	const	{ return ((QGDict*)dict)->size(); }
    int	    maxCost()	const	{ return mCost; }
    int	    totalCost() const	{ return tCost; }
    void    setMaxCost( int maxCost );
    void    clear();

    bool    insert_string( const QString &key, QCollection::Item,
			   int cost, int priority );
    bool    insert_other( const char *key, QCollection::Item,
			  int cost, int priority );
    bool    remove_string( const QString &key );
    bool    remove_other( const char *key );
    QCollection::Item take_string( const QString &key );
    QCollection::Item take_other( const char *key );

    QCollection::Item find_string( const QString &key, bool ref=TRUE ) const;
    QCollection::Item find_other( const char *key, bool ref=TRUE ) const;

    void    statistics() const;

private:
    bool    makeRoomFor( int cost, int priority = -1 );
    KeyType keytype;
    QCList *lruList;
    QCDict *dict;
    int	    mCost;
    int	    tCost;
    bool    copyk;
};


class Q_EXPORT QGCacheIterator			// generic cache iterator
{
protected:
    QGCacheIterator( const QGCache & );
    QGCacheIterator( const QGCacheIterator & );
   ~QGCacheIterator();
    QGCacheIterator &operator=( const QGCacheIterator & );

    uint	      count()   const;
    bool	      atFirst() const;
    bool	      atLast()  const;
    QCollection::Item toFirst();
    QCollection::Item toLast();

    QCollection::Item get() const;
    QString	      getKeyString() const;
    const char       *getKeyAscii()  const;
    long	      getKeyInt()    const;

    QCollection::Item operator()();
    QCollection::Item operator++();
    QCollection::Item operator+=( uint );
    QCollection::Item operator--();
    QCollection::Item operator-=( uint );

protected:
    QCListIt *it;				// iterator on cache list
};


#endif // QGCACHE_H
