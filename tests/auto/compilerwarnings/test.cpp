/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>
#include <QtXml/QtXml>
#include <QtSql/QtSql>
#include <QtGui/QtGui>

#ifndef QT_NO_OPENGL
#include <QtOpenGL/QtOpenGL>
#endif

#include <QtDesigner/QtDesigner>

#include <QtTest/QtTest>

#if !defined(QT_NO_DBUS) && defined(Q_OS_UNIX)
#include <QtDBus/QtDBus>
#endif

#ifndef Q_OS_MAC
int main(int, char **)
{
    return 0;
}
#endif

