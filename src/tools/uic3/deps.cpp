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

#include "ui3reader.h"

#include <QDomElement>
#include <QFile>

void Ui3Reader::computeDeps(const QDomElement &e,
        QStringList &globalIncludes,
        QStringList &localIncludes, bool impl)
{
    QDomNodeList nl;

    // additional includes (local or global) and forward declaractions
    nl = e.toElement().elementsByTagName(QLatin1String("include"));
    for (int i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();

        if (s.right(5) == QLatin1String(".ui.h") && !QFile::exists(s))
            continue;

        if (impl && n2.attribute(QLatin1String("impldecl"), QLatin1String("in implementation")) != QLatin1String("in implementation"))
            continue;

        if (n2.attribute(QLatin1String("location")) != QLatin1String("local"))
            globalIncludes += s;
        else
            localIncludes += s;
    }

    // do the local includes afterwards, since global includes have priority on clashes
    nl = e.toElement().elementsByTagName(QLatin1String("header"));
    for (int i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();
        if (n2.attribute(QLatin1String("location")) == QLatin1String("local") && !globalIncludes.contains(s)) {
            if (s.right(5) == QLatin1String(".ui.h") && !QFile::exists(s))
                continue;

            if (impl && n2.attribute(QLatin1String("impldecl"), QLatin1String("in implementation")) != QLatin1String("in implementation"))
                continue;

            localIncludes += s;
        }
    }

    // additional custom widget headers
    nl = e.toElement().elementsByTagName(QLatin1String("header"));
    for (int i = 0; i < (int) nl.length(); i++) {
        QDomElement n2 = nl.item(i).toElement();
        QString s = n2.firstChild().toText().data();

        if (n2.attribute(QLatin1String("location")) != QLatin1String("local"))
            globalIncludes += s;
        else
            localIncludes += s;
    }

    { // fix globalIncludes
        globalIncludes = unique(globalIncludes);
        QMutableStringListIterator it(globalIncludes);
        while (it.hasNext()) {
            QString v = it.next();

            if (v.isEmpty()) {
                it.remove();
                continue;
            }

            it.setValue(fixHeaderName(v));
        }
    }

    { // fix the localIncludes
        localIncludes = unique(localIncludes);
        QMutableStringListIterator it(localIncludes);
        while (it.hasNext()) {
            QString v = it.next();

            if (v.isEmpty()) {
                it.remove();
                continue;
            }

            it.setValue(fixHeaderName(v));
        }
    }
}
