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
#include <qabstractspinbox.h>
#include <qlineedit.h>


//TESTED_CLASS=
//TESTED_FILES=qabstractspinbox.h

class tst_QAbstractSpinBox : public QObject
{
Q_OBJECT

public:
    tst_QAbstractSpinBox();
    virtual ~tst_QAbstractSpinBox();

private slots:
    void getSetCheck();
};

tst_QAbstractSpinBox::tst_QAbstractSpinBox()
{
}

tst_QAbstractSpinBox::~tst_QAbstractSpinBox()
{
}

class MyAbstractSpinBox : public QAbstractSpinBox
{
public:
    MyAbstractSpinBox() : QAbstractSpinBox() {}
    QLineEdit *lineEdit() { return QAbstractSpinBox::lineEdit(); }
    void setLineEdit(QLineEdit *le) { QAbstractSpinBox::setLineEdit(le); }
};

// Testing get/set functions
void tst_QAbstractSpinBox::getSetCheck()
{
    MyAbstractSpinBox obj1;
    // ButtonSymbols QAbstractSpinBox::buttonSymbols()
    // void QAbstractSpinBox::setButtonSymbols(ButtonSymbols)
    obj1.setButtonSymbols(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::UpDownArrows));
    QCOMPARE(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::UpDownArrows), obj1.buttonSymbols());
    obj1.setButtonSymbols(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::PlusMinus));
    QCOMPARE(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::PlusMinus), obj1.buttonSymbols());

    // bool QAbstractSpinBox::wrapping()
    // void QAbstractSpinBox::setWrapping(bool)
    obj1.setWrapping(false);
    QCOMPARE(false, obj1.wrapping());
    obj1.setWrapping(true);
    QCOMPARE(true, obj1.wrapping());

    // QLineEdit * QAbstractSpinBox::lineEdit()
    // void QAbstractSpinBox::setLineEdit(QLineEdit *)
    QLineEdit *var3 = new QLineEdit(0);
    obj1.setLineEdit(var3);
    QCOMPARE(var3, obj1.lineEdit());
#ifndef QT_DEBUG
    obj1.setLineEdit((QLineEdit *)0); // Will assert in debug, so only test in release
    QCOMPARE(var3, obj1.lineEdit()); // Setting 0 should keep the current editor
#endif
    // delete var3; // No delete, since QAbstractSpinBox takes ownership
}

QTEST_MAIN(tst_QAbstractSpinBox)
#include "tst_qabstractspinbox.moc"
