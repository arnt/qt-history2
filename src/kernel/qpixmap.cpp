/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#1 $
**
** Implementation of QPixMap class (pixmap cache)
**
** Author  : Haavard Nord
** Created : 950301
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include "qcache.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#1 $";
#endif


typedef declare(QCacheM,QPixMap) QPixMapCache;
static QPixMapCache *pmcache = 0;		// global pixmap cache
const long pmcache_maxcost   = 1024*1024;	// maximum cache cost
const int  pmcache_size      = 61;		// size of internal hash array


QPixMap *QPixMap::find( const char *key )	// find pixmap in cache
{
    return pmcache ? pmcache->find(key) : 0;
}

bool QPixMap::insert( const char *key, QPixMap *pm )
{
    if ( !pmcache ) {				// create pixmap cache
	pmcache = new QPixMapCache( pmcache_maxcost, pmcache_size );
	pmcache->setAutoDelete( TRUE );
	CHECK_PTR( pmcache );
    }
    return pmcache->insert( key, pm, pm->width()*pm->height()*pm->depth()/8 );
}

void QPixMap::setCacheSize( long maxCost )	// change the maxcost parameter
{
    if ( pmcache )
	pmcache->setMaxCost( maxCost );
    else {
	pmcache = new QPixMapCache( maxCost, pmcache_size );
	pmcache->setAutoDelete( TRUE );
	CHECK_PTR( pmcache );
    }
}

void QPixMap::cleanup()				// cleanup cache
{
    delete pmcache;
}
