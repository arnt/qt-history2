/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_FailInit: public QObject
{
Q_OBJECT
private slots:
    void initTestCase() const;
    void aTestFunction() const;
};

void tst_FailInit::initTestCase() const
{
    QVERIFY(false);
}

/*! \internal
  This function should never be run because initTestCase fails.
 */
void tst_FailInit::aTestFunction() const
{
    qDebug() << "ERROR: this function is NOT supposed to be run.";
}

QTEST_APPLESS_MAIN(tst_FailInit)

#include "tst_failinit.moc"
