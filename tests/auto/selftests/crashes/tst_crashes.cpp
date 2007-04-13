/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtTest/QtTest>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class tst_Crashes: public QObject
{
    Q_OBJECT

private slots:
    void crash();
};

void tst_Crashes::crash()
{
#ifdef Q_OS_WIN
   //we avoid the error dialogbox to appear on windows
   SetErrorMode( SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#endif
    int *i = 0;
    *i = 1;
}

QTEST_MAIN(tst_Crashes)

#include "tst_crashes.moc"
