/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpmcache.h#1 $
**
** Definition of QPixmapCache class
**
** Author  : Haavard Nord
** Created : 950501
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPMCACHE_H
#define QPMCACHE_H

#include "qpixmap.h"


class QPixmapCache				// global pixmap cache
{
public:
    static  int		cacheLimit();
    static  void	setCacheLimit( int );
    static  QPixmap    *find( const char *key );
    static  bool	insert( const char *key, QPixmap * );
    static  void	clear();
};


#endif // QPMCACHE_H
