/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgcache.h#1 $
**
** Definition of QGCache class
**
** Author  : Eirik Eng
** Created : 950208
**
** Copyright (C) 1995 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QGCACHE_H
#define QGCACHE_H

#include "qcollect.h"


class QCList;		                        // internal classes
class QCDict;

// --------------------------------------------------------------------------
// QGCache class
//

class QGCache : public QCollection		// LRU cache class
{
public:
    uint    count() const;
    uint    size()  const;
    long    maxCost()    const;
    long    totalCost()  const;
    long    setMaxCost( long maxC);

protected:
    QGCache( long maxCost, uint size,bool caseS, bool copyKeys, bool trivial );
   ~QGCache();

    GCI     find( const char *key ) const;
    void    insert( const char *key, GCI, long cost, int priority );

    bool    remove( const char *key );
    GCI	    take( const char *key );
    void    clear();				// delete all items

    void    statistics() const;			// output statistics
private:
    bool    makeRoomFor( long cost, short priority = -1 );

    QCList *lruList;
    QCDict *dict;

    long   mCost;
    long   tCost;
    uint   copyK : 1;
};

#endif // QGCACHE_H

