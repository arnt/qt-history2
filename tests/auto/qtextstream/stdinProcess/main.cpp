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
    if (!qin.atEnd()) {
      int a, b, c;
      qin >> a >> b >> c;
      qDebug("%d %d %d", a, b, c);
    }
    return 0;
}
