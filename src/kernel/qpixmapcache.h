/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmapcache.h#9 $
**
** Definition of QPixmapCache class
**
** Created : 950501
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPIXMAPCACHE_H
#define QPIXMAPCACHE_H

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H


class QPixmapCache				// global pixmap cache
{
public:
    static  int		cacheLimit();
    static  void	setCacheLimit( int );
    static  QPixmap    *find( const char *key );
    static  bool	find( const char *key, QPixmap& );
    static  bool	insert( const char *key, QPixmap * );
    static  void	insert( const char *key, const QPixmap& );
    static  void	clear();
};


#endif // QPIXMAPCACHE_H
