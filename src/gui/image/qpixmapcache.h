/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPIXMAPCACHE_H
#define QPIXMAPCACHE_H

#include "QtGui/qpixmap.h"

QT_MODULE(Gui)

class Q_GUI_EXPORT QPixmapCache                                // global pixmap cache
{
public:
    static int cacheLimit();
    static void setCacheLimit(int);
    static QPixmap *find(const QString &key);
    static bool find(const QString &key, QPixmap&);
    static bool insert(const QString &key, const QPixmap&);
    static void remove(const QString &key);
    static void clear();
};

#endif // QPIXMAPCACHE_H
