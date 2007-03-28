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

class tst_Assert: public QObject
{
    Q_OBJECT

private slots:
    void testNumber1() const;
    void testNumber2() const;
    void testNumber3() const;
};

void tst_Assert::testNumber1() const
{
}

void tst_Assert::testNumber2() const
{
    Q_ASSERT(false);
}

void tst_Assert::testNumber3() const
{
}

QTEST_MAIN(tst_Assert)

#include "tst_assert.moc"
