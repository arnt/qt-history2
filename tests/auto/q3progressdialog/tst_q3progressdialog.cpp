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
    QCOMPARE(false, obj1.autoReset());
    obj1.setAutoReset(true);
    QCOMPARE(true, obj1.autoReset());

    // bool Q3ProgressDialog::autoClose()
    // void Q3ProgressDialog::setAutoClose(bool)
    obj1.setAutoClose(false);
    QCOMPARE(false, obj1.autoClose());
    obj1.setAutoClose(true);
    QCOMPARE(true, obj1.autoClose());

    // int Q3ProgressDialog::totalSteps()
    // void Q3ProgressDialog::setTotalSteps(int)
    obj1.setTotalSteps(0);
    QCOMPARE(0, obj1.totalSteps());
    obj1.setTotalSteps(INT_MIN);
    QCOMPARE(INT_MIN, obj1.totalSteps());
    obj1.setTotalSteps(INT_MAX);
    QCOMPARE(INT_MAX, obj1.totalSteps());

    // int Q3ProgressDialog::minimumDuration()
    // void Q3ProgressDialog::setMinimumDuration(int)
    obj1.setMinimumDuration(0);
    QCOMPARE(0, obj1.minimumDuration());
    obj1.setMinimumDuration(INT_MIN);
    QCOMPARE(0, obj1.minimumDuration()); // Makes no sense with negative duration
    obj1.setMinimumDuration(INT_MAX);
    QCOMPARE(INT_MAX, obj1.minimumDuration());
}

QTEST_MAIN(tst_Q3ProgressDialog)
#include "tst_q3progressdialog.moc"
