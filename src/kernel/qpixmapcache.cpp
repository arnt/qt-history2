/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmapcache.cpp#4 $
**
** Implementation of QPixmapCache class
**
** Author  : Haavard Nord
** Created : 950504
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qpmcache.h"
#include "qcache.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpixmapcache.cpp#4 $")


/*!
\class QPixmapCache qpmcache.h

\brief The QPixmapCache class provides an application-global cache for pixmaps.

The QPixmapCache is a class that contains no real members, only static
functions to access the global pixmap cache.  It creates an internal QCache
for caching the pixmaps.

The cache associates a pixmap with a normal string (key).  If two pixmaps
are inserted into the cache using equal keys, then the last pixmap will hide
the first pixmap. The QDict and QCache classes do exactly the same.

The cache becomes full when the total size of all pixmaps in the cache
exceeds the cache limit.  The initial cache limit is 1024 KByte (1 MByte).
A pixmap takes roughly width*height*depth/8 bytes of memory.

See the QCache documentation for a more details about the cache mechanism.
*/

typedef declare(QCacheM,QPixmap) QPMCache;
static QPMCache *pm_cache = 0;			// global pixmap cache
const  int cache_size	  = 61;			// size of internal hash array
static int cache_limit	  = 1024;		// 1024 KB cache limit


/*!
Returns the pixmap associated with \e key in the cache, or 0 if there is no
such pixmap.
*/

QPixmap *QPixmapCache::find( const char *key )
{
    return pm_cache ? pm_cache->find(key) : 0;
}

/*!
Inserts the pixmap \e pm associated with \e key into the cache.
Returns TRUE if successful, or FALSE if the pixmap is too big for the cache.

All pixmaps inserted by the Qt library has a key starting with "$qt.."
(so let your keys start with something else).

When a pixmap is inserted and the cache is about to exceed its limit,
it removes pixmaps until there is enough room for the pixmap to be inserted.

The oldest pixmaps (least recently accessed in the cache) will be thrown
away when more space is needed.

\sa setCacheLimit().
*/

bool QPixmapCache::insert( const char *key, QPixmap *pm )
{
    if ( !pm_cache ) {				// create pixmap cache
	pm_cache = new QPMCache( 1024L*cache_limit, cache_size );
	CHECK_PTR( pm_cache );
	pm_cache->setAutoDelete( TRUE );
    }
    return pm_cache->insert( key, pm, pm->width()*pm->height()*pm->depth()/8 );
}

/*!
Returns the cache limit (in kilobytes).

\sa setCacheLimit().
*/

int QPixmapCache::cacheLimit()
{
    return cache_limit;
}

/*!
Sets the cache limit to \e n kilobytes.

The default setting is 1024 kilobytes.

\sa cacheLimit().
*/

void QPixmapCache::setCacheLimit( int n )
{
    cache_limit = n;
    if ( pm_cache )
	pm_cache->setMaxCost( 1024L*cache_limit );
}


/*!
Removes all pixmaps from the cache.
*/

void QPixmapCache::clear()
{
    delete pm_cache;
    pm_cache = 0;
}
