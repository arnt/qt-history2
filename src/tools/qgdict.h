/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgdict.h#34 $
**
** Definition of QGDict and QGDictIterator classes
**
** Created : 920529
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

#ifndef QGDICT_H
#define QGDICT_H

#ifndef QT_H
#include "qlist.h"
#include "qstring.h"
#endif // QT_H

class QGDictIterator;
class QGDItList;


class QBucketPrivate					// internal dict node
{
public:
    char   *getKey()		{ return key; }
    char   *setKey( char *k )	{ return key = k; }
    QCollection::Item getData() { return data; }
    QCollection::Item setData( QCollection::Item d ) { return data = d; }
    QBucketPrivate *getNext()		{ return next; }
    void    setNext( QBucketPrivate *n){ next = n; }
private:
    char   *key;
    QCollection::Item	    data;
    QBucketPrivate *next;
};


class Q_EXPORT QGDict : public QCollection	// generic dictionary class
{
friend class QGDictIterator;
public:
    uint	count() const	{ return numItems; }
    uint	size()	const	{ return vlen; }
    Item		look( const char *key, Item, int );
    Item		look( const QString& key, Item, int );

    QDataStream &read( QDataStream & );
    QDataStream &write( QDataStream & ) const;

protected:
    QGDict( uint len, bool cs, bool ck, bool th );
    QGDict( const QGDict & );
   ~QGDict();

    QGDict     &operator=( const QGDict & );

    bool	remove( const char *key );
    bool	removeItem( const char *key, Item item );
    Item		take( const char *key );
    bool	remove( const QString& key );
    bool	removeItem( const QString& key, Item item );
    Item		take( const QString& key );

    void	clear();
    void	resize( uint );

    virtual int hashKey( const char * );

    void	statistics() const;

    virtual QDataStream &read( QDataStream &, Item & );
    virtual QDataStream &write( QDataStream &, Item ) const;

private:
    QBucketPrivate   **vec;
    uint	vlen;
    uint	numItems;
    uint	cases	: 1;
    uint	copyk	: 1;
    uint	triv	: 1;
    QGDItList  *iterators;
    QBucketPrivate    *unlink( const char *, Item item = 0 );
    void        init( uint );
};


class Q_EXPORT QGDictIterator			// generic dictionary iterator
{
friend class QGDict;
public:
    QGDictIterator( const QGDict & );
    QGDictIterator( const QGDictIterator & );
    QGDictIterator &operator=( const QGDictIterator & );
   ~QGDictIterator();

    QCollection::Item		toFirst();

    QCollection::Item		get()	 const { return curNode ? curNode->getData() : 0; }
    long        getKeyLong() const
			       { return curNode ? (long)curNode->getKey() : 0; }
    QString     getKey() const;

    QCollection::Item		operator()();
    QCollection::Item		operator++();
    QCollection::Item		operator+=(uint);

protected:
    QGDict     *dict;

private:
    QBucketPrivate    *curNode;
    uint	curIndex;
};


#endif // QGDICT_H
