/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "parser.h"
#include <qobject.h>
#include <qstringlist.h>

QString Parser::cleanArgs(const QString &func)
{
    QString slot(func);
    int begin = slot.indexOf("(") + 1;
    QString args = slot.mid(begin);
    args = args.left(args.indexOf(")"));
    QStringList lst = args.split(',');
    QString res = slot.left(begin);
    for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
        if (it != lst.begin())
            res += ",";
        QString arg = *it;
        int pos = 0;
        if ((pos = arg.indexOf("&")) != -1) {
            arg = arg.left(pos + 1);
        } else if ((pos = arg.indexOf("*")) != -1) {
            arg = arg.left(pos + 1);
        } else {
            arg = arg.simplified();
            if ((pos = arg.indexOf(':')) != -1)
                arg = arg.left(pos).simplified() + ":" + arg.mid(pos + 1).simplified();
            QStringList l = arg.split(' ');
            if (l.count() == 2) {
                if (l[0] != "const" && l[0] != "unsigned" && l[0] != "var")
                    arg = l[0];
            } else if (l.count() == 3) {
                arg = l[0] + " " + l[1];
            }
        }
        res += arg;
    }
    res += ")";
    return res;
}
