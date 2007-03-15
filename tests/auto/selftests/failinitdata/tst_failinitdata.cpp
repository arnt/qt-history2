/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_FailInitData: public QObject
{
Q_OBJECT
private slots:
    void initTestCase() const;
    void initTestCase_data() const;
    void aTestFunction() const;
};

void tst_FailInitData::initTestCase_data() const
{
    QVERIFY(false);
}

/*! \internal
  This function should never be run because initTestCase_data fails.
 */
void tst_FailInitData::initTestCase() const
{
    qDebug() << "This function is NOT supposed to be called.";
}

/*! \internal
  This function should never be run because initTestCase_data fails.
 */
void tst_FailInitData::aTestFunction() const
{
    qDebug() << "This function is NOT supposed to be called.";
}

QTEST_APPLESS_MAIN(tst_FailInitData)

#include "tst_failinitdata.moc"
