/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgdict.h#30 $
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


class QBucket					// internal dict node
{
public:
    char   *getKey()		{ return key; }
    char   *setKey( char *k )	{ return key = k; }
    GCI	    getData()		{ return data; }
    GCI	    setData( GCI d )	{ return data = d; }
    QBucket *getNext()		{ return next; }
    void    setNext( QBucket *n){ next = n; }
private:
    char   *key;
    GCI	    data;
    QBucket *next;
};


class Q_EXPORT QGDict : public QCollection	// generic dictionary class
{
friend class QGDictIterator;
public:
    uint	count() const	{ return numItems; }
    uint	size()	const	{ return vlen; }
    GCI		look( const char *key, GCI, int );
    GCI		look( QString key, GCI, int );

    QDataStream &read( QDataStream & );
    QDataStream &write( QDataStream & ) const;

protected:
    QGDict( uint len, bool cs, bool ck, bool th );
    QGDict( const QGDict & );
   ~QGDict();

    QGDict     &operator=( const QGDict & );

    bool	remove( const char *key );
    bool	removeItem( const char *key, GCI item );
    GCI		take( const char *key );
    bool	remove( QString key );
    bool	removeItem( QString key, GCI item );
    GCI		take( QString key );

    void	clear();
    void	resize( uint );

    virtual int hashKey( const char * );

    void	statistics() const;

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    QBucket   **vec;
    uint	vlen;
    uint	numItems;
    uint	cases	: 1;
    uint	copyk	: 1;
    uint	triv	: 1;
    QGDItList  *iterators;
    QBucket    *unlink( const char *, GCI item = 0 );
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

    GCI		toFirst();

    GCI		get()	 const { return curNode ? curNode->getData() : 0; }
    long        getKeyLong() const
			       { return curNode ? (long)curNode->getKey() : 0; }
    QString     getKey() const;

    GCI		operator()();
    GCI		operator++();
    GCI		operator+=(uint);

protected:
    QGDict     *dict;

private:
    QBucket    *curNode;
    uint	curIndex;
};


#endif // QGDICT_H
