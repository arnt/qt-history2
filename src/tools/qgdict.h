/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgdict.h#11 $
**
** Definition of QGDict and QGDictIterator classes
**
** Author  : Haavard Nord
** Created : 920529
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGDICT_H
#define QGDICT_H

#include "qcollect.h"


class QBucket;					// internal classes
class QListM_QGDictIterator;
#define Qditlst QListM_QGDictIterator


class QGDict : public QCollection		// generic dictionary class
{
friend class QGDictIterator;
public:
    uint    	count() const	{ return numItems; }
    uint    	size()  const	{ return vlen; }
    GCI	    	look( const char *key, GCI, int );// find/insert/replace item

    QDataStream &read( QDataStream & );		// read dict from stream
    QDataStream &write( QDataStream & ) const;	// write dict to stream

protected:
    QGDict( uint len, bool cs, bool ck, bool th );
    QGDict( const QGDict & );
   ~QGDict();

    QGDict     &operator=( const QGDict & );

    bool    	remove( const char *key );
    GCI	    	take( const char *key );
    void    	clear();

    void    	statistics() const;		// output statistics (debug)

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    QBucket   **vec;
    uint    	vlen;
    uint    	numItems;
    uint    	cases	: 1;
    uint    	copyk	: 1;
    uint    	trivial	: 1;
    Qditlst    *iterators;
    QBucket    *unlink( const char * );
    virtual int hashKey( const char * );	// hash function
};


class QGDictIterator				// generic dictionary iterator
{
friend class QGDict;
public:
    QGDictIterator( const QGDict & );
   ~QGDictIterator();

    GCI	  	toFirst();

    GCI	  	get()	 const;
    const char *getKey() const;

    GCI	  	operator()();
    GCI	  	operator++();
    GCI	  	operator+=(uint);

protected:
    QGDict     *dict;

private:
    QBucket    *curNode;
    uint	curIndex;
};


#endif // QGDICT_H
