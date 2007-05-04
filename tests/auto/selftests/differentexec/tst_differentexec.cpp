/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_TestA : public QObject
{
    Q_OBJECT

private slots:
    void nothingA() const
    {
        QVERIFY(true);
    }
};

class tst_TestB : public QObject
{
    Q_OBJECT

private slots:
    void nothingB() const
    {
        QVERIFY(true);
    }
};

int main(int argc, char *argv[])
{
    tst_TestA testA;
    QTest::qExec(&testA, argc, argv);

    tst_TestB testB;
    QTest::qExec(&testB, argc, argv);

    return 0;
}

#include "tst_differentexec.moc"
