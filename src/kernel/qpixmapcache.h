/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmapcache.h#14 $
**
** Definition of QPixmapCache class
**
** Created : 950501
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

#ifndef QPIXMAPCACHE_H
#define QPIXMAPCACHE_H

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H


class Q_EXPORT QPixmapCache				// global pixmap cache
{
public:
    static  int		cacheLimit();
    static  void	setCacheLimit( int );
    static  QPixmap    *find( const QString &key );
    static  bool	find( const QString &key, QPixmap& );
    static  bool	insert( const QString &key, QPixmap * );
    static  void	insert( const QString &key, const QPixmap& );
    static  void	clear();
};


#endif // QPIXMAPCACHE_H
