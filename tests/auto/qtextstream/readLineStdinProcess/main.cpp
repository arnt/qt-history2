/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>

int main()
{
    QTextStream qin(stdin);
    QTextStream qerr(stderr);
    QString line;
    do {
        line = qin.readLine();
        if (!line.isNull())
            qerr << line << flush;
    } while (!line.isNull());
    return 0;
}
