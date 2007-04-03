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
    void expectAndContinue() const;
    void expectAndAbort() const;
    void xfailWithQString() const;
};

void tst_ExpectFail::expectAndContinue() const
{
    qDebug("begin");
    QEXPECT_FAIL("", "This should xfail", Continue);
    QVERIFY(false);
    qDebug("after");
}

void tst_ExpectFail::expectAndAbort() const
{
    qDebug("begin");
    QEXPECT_FAIL("", "This should xfail", Abort);
    QVERIFY(false);
    qDebug("this should not be reached");
}

void tst_ExpectFail::xfailWithQString() const
{
    QEXPECT_FAIL("", QString("A string").toLatin1().constData(), Continue);
    QVERIFY(false);

    int bugNo = 5;
    QString msg("The message");
    QEXPECT_FAIL( "", QString("Bug %1 (%2)").arg(bugNo).arg(msg).toLatin1().constData(), Continue);
    QVERIFY(false);
}

QTEST_MAIN(tst_ExpectFail)

#include "tst_expectfail.moc"
