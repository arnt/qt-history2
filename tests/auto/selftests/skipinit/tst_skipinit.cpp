/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_SkipInit: public QObject
{
Q_OBJECT
private slots:
    void initTestCase() const;
    void aTestFunction() const;
};

void tst_SkipInit::initTestCase() const
{
    QSKIP("Skip inside initTestCase. This should skip all tests in the class.", SkipAll);
}

/*! \internal
  This function should never be run because initTestCase fails.
 */
void tst_SkipInit::aTestFunction() const
{
    qDebug() << "ERROR: This function is NOT supposed to be run.";
}

QTEST_APPLESS_MAIN(tst_SkipInit)

#include "tst_skipinit.moc"
