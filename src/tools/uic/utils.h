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

#ifndef UTILS_H
#define UTILS_H

#include "ui4.h"

#include <qstring.h>
#include <qlist.h>
#include <qhash.h>

inline bool toBool(const QString &str)
{ return str.toLower() == QLatin1String("true"); }

inline QString toString(const DomString *str)
{ return str ? str->text() : QString(); }

inline QString fixString(const QString &str, bool encode=false)
{
    QString s;
    if (!encode) {
        s = str;
        s.replace(QLatin1String("\\"), QLatin1String("\\\\"));
        s.replace(QLatin1String("\""), QLatin1String("\\\""));
        s.replace(QLatin1String("\r"), QLatin1String(""));
        s.replace(QLatin1String("\n"), QLatin1String("\\n\"\n\""));
    } else {
        QByteArray utf8 = str.toUtf8();
        const int l = utf8.length();
        for (int i = 0; i < l; ++i)
            s += QLatin1String("\\x") + QString::number((uchar)utf8[i], 16);
    }

    return QLatin1String("\"") + s + QLatin1String("\"");
}

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties)
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}

inline QStringList unique(const QStringList &lst)
{
    QHash<QString, bool> h;
    for (int i=0; i<lst.size(); ++i)
        h.insert(lst.at(i), true);
    return h.keys();
}

#endif
