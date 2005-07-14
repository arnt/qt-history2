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

#ifndef Q3PICTURE_H
#define Q3PICTURE_H

#include <QtGui/qpicture.h>

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3Picture : public QPicture
{
public:
    Q3Picture() : QPicture(-1) { }
    Q3Picture(const QPicture &pic) : QPicture(pic) { }
    bool load(QIODevice *dev, const char *format = 0);
    bool load(const QString &fileName, const char *format = 0);
    bool save(QIODevice *dev, const char *format = 0);
    bool save(const QString &fileName, const char *format = 0);
};

#endif
