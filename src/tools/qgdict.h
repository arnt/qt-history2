/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgdict.h#3 $
**
** Definition of QGDict and QGDictIterator classes
**
** Author  : Haavard Nord
** Created : 920529
**
** Copyright (C) 1992-1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QGDICT_H
#define QGDICT_H

#include "qcollect.h"


class Qbucket;					// internal classes
class QListM_QGDictIterator;
#define Qditlst QListM_QGDictIterator


// --------------------------------------------------------------------------
// QGDict class
//

class QGDict : public QCollection		// hash dictionary class
{
friend class QGDictIterator;
public:
    uint    count() const { return numItems; }	// return number of items
    GCI	    look( const char *key, GCI, bool);	// find/insert item

    QDataStream &read( QDataStream & );		// read dict from stream
    QDataStream &write( QDataStream & ) const;	// write dict to stream

protected:
    QGDict( uint sz, bool cs, bool ck, bool th );
   ~QGDict();

    bool    remove( const char *key );
    void    clear();				// delete all items

    void    statistics() const;			// output statistics

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    Qbucket **vec;				// hash array
    uint    size;				// size of array
    uint    numItems;				// number of items
    int	    cases	: 1;			// case sensitive
    int	    copyk	: 1;			// copy keys
    int	    trivial	: 1;			// trivial hashing
    Qditlst *iterators;				// list of iterators
    virtual int	  hashKey( const char * );	// hash function
};


// --------------------------------------------------------------------------
// QGDict stream functions
//

QDataStream &operator>>( QDataStream &, QGDict & );
QDataStream &operator<<( QDataStream &, const QGDict & );


// --------------------------------------------------------------------------
// QGDictIterator class
//

class QGDictIterator				// QGDict iterator
{
friend class QGDict;
public:
    QGDictIterator( const QGDict & );
   ~QGDictIterator();

    GCI	  toFirst();				// move to first item

    GCI	  get() const;				// get current item
    GCI	  operator()();				// get current and move to next
    GCI	  operator++();				// move to next item (prefix)
    GCI	  operator+=(uint);

protected:
    QGDict  *dict;				// reference to dict

private:
    Qbucket *curNode;				// current node in dict
    uint     curIndex;				// current index in array
};


#endif // QGDICT_H
