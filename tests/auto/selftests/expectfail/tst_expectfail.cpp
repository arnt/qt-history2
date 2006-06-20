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

class tst_ExpectFail: public QObject
{
    Q_OBJECT

private slots:
    void expectAndContinue();
    void expectAndAbort();
};

void tst_ExpectFail::expectAndContinue()
{
    qDebug("begin");
    QEXPECT_FAIL("", "This should xfail", Continue);
    QVERIFY(false);
    qDebug("after");
}

void tst_ExpectFail::expectAndAbort()
{
    qDebug("begin");
    QEXPECT_FAIL("", "This should xfail", Abort);
    QVERIFY(false);
    qDebug("this should not be reached");
}

QTEST_MAIN(tst_ExpectFail)

#include "tst_expectfail.moc"
