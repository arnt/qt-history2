/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmapcache.cpp#8 $
**
** Implementation of QPixmapCache class
**
** Author  : Haavard Nord
** Created : 950504
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpmcache.h"
#include "qcache.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpixmapcache.cpp#8 $");


/*----------------------------------------------------------------------------
  \class QPixmapCache qpmcache.h

  \brief The QPixmapCache class provides an application-global cache for
  pixmaps.

  This class is a tool for optimized drawing with \link QPixmap
  QPixmaps\endlink.  Here follows an example.  The QRadioButton widget has
  a non-trivial visual representation.  In the \link
  QRadioButton::drawButton() drawButton()\endlink function, we do not draw
  the radio button directly. Instead, we first check the global pixmap
  cache if a pixmap called "$qt_radio_s_" exists. The \c s is a numerical
  value that specifies the the radio button state.  If a pixmap is found,
  we bitBlt() it onto the widget and return. Otherwise, we create a new
  pixmap, draw the radio button in the pixmap and finally insert the
  pixmap in the global pixmap cache, using the key above.  The bitBlt() is
  10 times faster than drawing the radio button.  All radio buttons in the
  program share the cached pixmap since QPixmapCache is
  application-global.

  QPixmapCache contains no member data, only static functions to access
  the global pixmap cache.  It creates an internal QCache for caching the
  pixmaps.

  The cache associates a pixmap with a normal string (key).  If two
  pixmaps are inserted into the cache using equal keys, then the last
  pixmap will hide the first pixmap. The QDict and QCache classes do
  exactly the same.

  The cache becomes full when the total size of all pixmaps in the cache
  exceeds the cache limit.  The initial cache limit is 1024 KByte (1
  MByte).  A pixmap takes roughly width*height*depth/8 bytes of memory.

  See the QCache documentation for a more details about the cache mechanism.
 ----------------------------------------------------------------------------*/

typedef declare(QCacheM,QPixmap) QPMCache;
static QPMCache *pm_cache = 0;			// global pixmap cache
const  int cache_size	  = 61;			// size of internal hash array
static int cache_limit	  = 1024;		// 1024 KB cache limit


/*----------------------------------------------------------------------------
  Returns the pixmap associated with \e key in the cache, or null if there
  is no such pixmap.
 ----------------------------------------------------------------------------*/

QPixmap *QPixmapCache::find( const char *key )
{
    return pm_cache ? pm_cache->find(key) : 0;
}

/*----------------------------------------------------------------------------
  Inserts the pixmap \e pm associated with \e key into the cache.
  Returns TRUE if successful, or FALSE if the pixmap is too big for the cache.

  All pixmaps inserted by the Qt library has a key starting with "$qt..".
  Use something else for you pixmaps.

  When a pixmap is inserted and the cache is about to exceed its limit, it
  removes pixmaps until there is enough room for the pixmap to be inserted.

  The oldest pixmaps (least recently accessed in the cache) are deleted
  when more space is needed.

  \sa setCacheLimit().
 ----------------------------------------------------------------------------*/

bool QPixmapCache::insert( const char *key, QPixmap *pm )
{
    if ( !pm_cache ) {				// create pixmap cache
	pm_cache = new QPMCache( 1024*cache_limit, cache_size );
	CHECK_PTR( pm_cache );
	pm_cache->setAutoDelete( TRUE );
    }
    return pm_cache->insert( key, pm, pm->width()*pm->height()*pm->depth()/8 );
}

/*----------------------------------------------------------------------------
  Returns the cache limit (in kilobytes).

  The default setting is 1024 kilobytes.

  \sa setCacheLimit().
 ----------------------------------------------------------------------------*/

int QPixmapCache::cacheLimit()
{
    return cache_limit;
}

/*----------------------------------------------------------------------------
  Sets the cache limit to \e n kilobytes.

  The default setting is 1024 kilobytes.

  \sa cacheLimit()
 ----------------------------------------------------------------------------*/

void QPixmapCache::setCacheLimit( int n )
{
    cache_limit = n;
    if ( pm_cache )
	pm_cache->setMaxCost( 1024*cache_limit );
}


/*----------------------------------------------------------------------------
  Removes all pixmaps from the cache.
 ----------------------------------------------------------------------------*/

void QPixmapCache::clear()
{
    delete pm_cache;
    pm_cache = 0;
}
