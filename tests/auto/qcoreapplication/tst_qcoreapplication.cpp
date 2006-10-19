/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

class tst_QCoreApplication: public QObject
{
    Q_OBJECT
private slots:
    void qAppName();
    void argc();
};

void tst_QCoreApplication::qAppName()
{
    int argc = 1;
    char *argv[] = { "tst_qcoreapplication" };
    QCoreApplication app(argc, argv);
    QVERIFY(!::qAppName().isEmpty());
}

void tst_QCoreApplication::argc()
{
    {
        int argc = 1;
        char *argv[] = { "tst_qcoreapplication" };
        QCoreApplication app(argc, argv);
        QCOMPARE(argc, 1);
        QCOMPARE(app.argc(), 1);
    }

    {
        int argc = 4;
        char *argv[] = { "tst_qcoreapplication", "arg1", "arg2", "arg3" };
        QCoreApplication app(argc, argv);
        QCOMPARE(argc, 4);
        QCOMPARE(app.argc(), 4);
    }

    {
        int argc = 0;
        char **argv = 0;
        QCoreApplication app(argc, argv);
        QCOMPARE(argc, 0);
        QCOMPARE(app.argc(), 0);
    }
}

QTEST_APPLESS_MAIN(tst_QCoreApplication)
#include "tst_qcoreapplication.moc"
