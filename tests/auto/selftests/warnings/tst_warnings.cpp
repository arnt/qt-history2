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

class tst_Warnings: public QObject
{
    Q_OBJECT
private slots:
    void testWarnings();
    void testMissingWarnings();
};

void tst_Warnings::testWarnings()
{
    printf("testWarnings 1: next line prints \"Warning\"\n");
    qWarning("Warning");
    printf("testWarnings 2: next line prints nothing\n");

    QTest::ignoreMessage(QtWarningMsg, "Warning");
    qWarning("Warning");
    printf("testWarnings 3: next line prints \"Warning\"\n");

    qWarning("Warning");
    printf("testWarnings 4: over\n");

    printf("testWarnings 5: next line prints \"Debug\"\n");
    qDebug("Debug");
    printf("testWarnings 6: next line prints nothing\n");

    QTest::ignoreMessage(QtDebugMsg, "Debug");
    qDebug("Debug");
    printf("testWarnings 7: next line prints \"Debug\"\n");

    qDebug("Debug");
    printf("testWarnings 8: next comes \"Baba\" twice\n");

    QTest::ignoreMessage(QtDebugMsg, "Bubu");
    qDebug("Baba");
    qDebug("Bubu");
    qDebug("Baba");

}

void tst_Warnings::testMissingWarnings()
{
    QTest::ignoreMessage(QtWarningMsg, "Warning0");
    QTest::ignoreMessage(QtWarningMsg, "Warning1");
    QTest::ignoreMessage(QtWarningMsg, "Warning2");

    qWarning("Warning2");
}

QTEST_MAIN(tst_Warnings)

#include "tst_warnings.moc"
