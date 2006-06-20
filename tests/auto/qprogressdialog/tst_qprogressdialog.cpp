/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qprogressdialog.h>


//TESTED_CLASS=
//TESTED_FILES=qprogressdialog.h

class tst_QProgressDialog : public QObject
{
Q_OBJECT

public:
    tst_QProgressDialog();
    virtual ~tst_QProgressDialog();

private slots:
    void getSetCheck();
};

tst_QProgressDialog::tst_QProgressDialog()
{
}

tst_QProgressDialog::~tst_QProgressDialog()
{
}

// Testing get/set functions
void tst_QProgressDialog::getSetCheck()
{
    QProgressDialog obj1;
    // bool QProgressDialog::autoReset()
    // void QProgressDialog::setAutoReset(bool)
    obj1.setAutoReset(false);
    QCOMPARE(false, obj1.autoReset());
    obj1.setAutoReset(true);
    QCOMPARE(true, obj1.autoReset());

    // bool QProgressDialog::autoClose()
    // void QProgressDialog::setAutoClose(bool)
    obj1.setAutoClose(false);
    QCOMPARE(false, obj1.autoClose());
    obj1.setAutoClose(true);
    QCOMPARE(true, obj1.autoClose());

    // int QProgressDialog::maximum()
    // void QProgressDialog::setMaximum(int)
    obj1.setMaximum(0);
    QCOMPARE(0, obj1.maximum());
    obj1.setMaximum(INT_MIN);
    QCOMPARE(INT_MIN, obj1.maximum());
    obj1.setMaximum(INT_MAX);
    QCOMPARE(INT_MAX, obj1.maximum());

    // int QProgressDialog::minimum()
    // void QProgressDialog::setMinimum(int)
    obj1.setMinimum(0);
    QCOMPARE(0, obj1.minimum());
    obj1.setMinimum(INT_MIN);
    QCOMPARE(INT_MIN, obj1.minimum());
    obj1.setMinimum(INT_MAX);
    QCOMPARE(INT_MAX, obj1.minimum());

    // int QProgressDialog::value()
    // void QProgressDialog::setValue(int)
    obj1.setMaximum(INT_MAX);
    obj1.setMinimum(INT_MIN);
    obj1.setValue(0);
    QCOMPARE(0, obj1.value());
    obj1.setValue(INT_MIN+1);
    QCOMPARE(INT_MIN+1, obj1.value());
    obj1.setValue(INT_MIN);
    QCOMPARE(INT_MIN, obj1.value());
    obj1.setValue(INT_MAX-1);
    QCOMPARE(INT_MAX-1, obj1.value());
    obj1.setValue(INT_MAX);
    QCOMPARE(INT_MAX, obj1.value());

    // int QProgressDialog::minimumDuration()
    // void QProgressDialog::setMinimumDuration(int)
    obj1.setMinimumDuration(0);
    QCOMPARE(0, obj1.minimumDuration());
    obj1.setMinimumDuration(INT_MIN);
    QCOMPARE(INT_MIN, obj1.minimumDuration());
    obj1.setMinimumDuration(INT_MAX);
    QCOMPARE(INT_MAX, obj1.minimumDuration());
}

QTEST_MAIN(tst_QProgressDialog)
#include "tst_qprogressdialog.moc"
