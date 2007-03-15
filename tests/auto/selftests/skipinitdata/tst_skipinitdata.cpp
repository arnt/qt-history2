/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_SkipInitData: public QObject
{
Q_OBJECT
private slots:
    void initTestCase_data() const;
    void initTestCase() const;
    void aTestFunction() const;
};

void tst_SkipInitData::initTestCase_data() const
{
    QSKIP("Skip inside initTestCase. This should skip all tests in the class.", SkipAll);
}

void tst_SkipInitData::initTestCase() const
{
}

/*! \internal
  This function should never be run because initTestCase fails.
 */
void tst_SkipInitData::aTestFunction() const
{
    qDebug() << "ERROR: this function is NOT supposed to be run.";
}

QTEST_APPLESS_MAIN(tst_SkipInitData)

#include "tst_skipinitdata.moc"
