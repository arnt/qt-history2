/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qapplication.h>
#include <qdebug.h>
#include <q3progressdialog.h>


//TESTED_CLASS=
//TESTED_FILES=q3progressdialog.h

class tst_Q3ProgressDialog : public QObject
{
Q_OBJECT

public:
    tst_Q3ProgressDialog();
    virtual ~tst_Q3ProgressDialog();

private slots:
    void getSetCheck();
};

tst_Q3ProgressDialog::tst_Q3ProgressDialog()
{
}

tst_Q3ProgressDialog::~tst_Q3ProgressDialog()
{
}

// Testing get/set functions
void tst_Q3ProgressDialog::getSetCheck()
{
    Q3ProgressDialog obj1;
    // bool Q3ProgressDialog::autoReset()
    // void Q3ProgressDialog::setAutoReset(bool)
    obj1.setAutoReset(false);
    QVERIFY(!obj1.autoReset());
    obj1.setAutoReset(true);
    QVERIFY(obj1.autoReset());

    // bool Q3ProgressDialog::autoClose()
    // void Q3ProgressDialog::setAutoClose(bool)
    obj1.setAutoClose(false);
    QVERIFY(!obj1.autoClose());
    obj1.setAutoClose(true);
    QVERIFY(obj1.autoClose());

    // int Q3ProgressDialog::totalSteps()
    // void Q3ProgressDialog::setTotalSteps(int)
    obj1.setTotalSteps(0);
    QCOMPARE(obj1.totalSteps(), 0);
    obj1.setTotalSteps(INT_MIN);
    QCOMPARE(obj1.totalSteps(), INT_MIN);
    obj1.setTotalSteps(INT_MAX);
    QCOMPARE(obj1.totalSteps(), INT_MAX);

    // int Q3ProgressDialog::minimumDuration()
    // void Q3ProgressDialog::setMinimumDuration(int)
    obj1.setMinimumDuration(0);
    QCOMPARE(obj1.minimumDuration(), 0);
    obj1.setMinimumDuration(INT_MIN);
    // It makes no sense with negative duration, but we cannot change this
    // behavior in a Qt3Support classs.
    QCOMPARE(obj1.minimumDuration(), INT_MIN);
    obj1.setMinimumDuration(INT_MAX);
    QCOMPARE(obj1.minimumDuration(), INT_MAX);
}

QTEST_MAIN(tst_Q3ProgressDialog)
#include "tst_q3progressdialog.moc"
