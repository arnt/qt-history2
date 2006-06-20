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
#include <q3datetimeedit.h>


//TESTED_CLASS=
//TESTED_FILES=q3datetimeedit.h

class tst_Q3DateTimeEdit : public QObject
{
Q_OBJECT

public:
    tst_Q3DateTimeEdit();
    virtual ~tst_Q3DateTimeEdit();

private slots:
    void getSetCheck();
};

tst_Q3DateTimeEdit::tst_Q3DateTimeEdit()
{
}

tst_Q3DateTimeEdit::~tst_Q3DateTimeEdit()
{
}

// Testing get/set functions
void tst_Q3DateTimeEdit::getSetCheck()
{
    Q3TimeEdit obj1;
    // uint Q3TimeEdit::display()
    // void Q3TimeEdit::setDisplay(uint)
    obj1.setDisplay(Q3TimeEdit::Hours);
    QCOMPARE(uint(Q3TimeEdit::Hours), obj1.display());
    obj1.setDisplay(Q3TimeEdit::Minutes);
    QCOMPARE(uint(Q3TimeEdit::Minutes), obj1.display());
    obj1.setDisplay(Q3TimeEdit::Seconds);
    QCOMPARE(uint(Q3TimeEdit::Seconds), obj1.display());
    obj1.setDisplay(Q3TimeEdit::AMPM);
    QCOMPARE(uint(Q3TimeEdit::AMPM), obj1.display());
}

QTEST_MAIN(tst_Q3DateTimeEdit)
#include "tst_q3datetimeedit.moc"
