/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmapcache.h#15 $
**
** Definition of QPixmapCache class
**
** Created : 950501
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
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
