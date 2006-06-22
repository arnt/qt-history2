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
#include <qfocusframe.h>


//TESTED_CLASS=
//TESTED_FILES=qfocusframe.h

class tst_QFocusFrame : public QObject
{
Q_OBJECT

public:
    tst_QFocusFrame();
    virtual ~tst_QFocusFrame();

private slots:
    void getSetCheck();
};

tst_QFocusFrame::tst_QFocusFrame()
{
}

tst_QFocusFrame::~tst_QFocusFrame()
{
}

// Testing get/set functions
void tst_QFocusFrame::getSetCheck()
{
    QFocusFrame *obj1 = new QFocusFrame();
    // QWidget * QFocusFrame::widget()
    // void QFocusFrame::setWidget(QWidget *)
    QWidget var1;
    QWidget *var2 = new QWidget(&var1);
    obj1->setWidget(var2);
    QCOMPARE(var2, obj1->widget());
    obj1->setWidget((QWidget *)0);
    QCOMPARE((QWidget *)0, obj1->widget());
    delete obj1;
}

QTEST_MAIN(tst_QFocusFrame)
#include "tst_qfocusframe.moc"
