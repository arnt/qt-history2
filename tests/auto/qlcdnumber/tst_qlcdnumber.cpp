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
#include <qlcdnumber.h>


//TESTED_CLASS=
//TESTED_FILES=qlcdnumber.h

class tst_QLCDNumber : public QObject
{
Q_OBJECT

public:
    tst_QLCDNumber();
    virtual ~tst_QLCDNumber();

private slots:
    void getSetCheck();
};

tst_QLCDNumber::tst_QLCDNumber()
{
}

tst_QLCDNumber::~tst_QLCDNumber()
{
}

// Testing get/set functions
void tst_QLCDNumber::getSetCheck()
{
    QLCDNumber obj1;
    // int QLCDNumber::numDigits()
    // void QLCDNumber::setNumDigits(int)
    obj1.setNumDigits(0);
    QCOMPARE(0, obj1.numDigits());
    obj1.setNumDigits(INT_MIN);
    QCOMPARE(0, obj1.numDigits()); // Range<0, 99>
    obj1.setNumDigits(INT_MAX);
    QCOMPARE(99, obj1.numDigits()); // Range<0, 99>
}

QTEST_MAIN(tst_QLCDNumber)
#include "tst_qlcdnumber.moc"
