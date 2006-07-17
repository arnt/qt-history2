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
#include <qtextformat.h>


//TESTED_CLASS=
//TESTED_FILES=qtextformat.h

class tst_QTextFormat : public QObject
{
Q_OBJECT

public:
    tst_QTextFormat();
    virtual ~tst_QTextFormat();

private slots:
    void getSetCheck();
    void defaultAlignment();
};

tst_QTextFormat::tst_QTextFormat()
{
}

tst_QTextFormat::~tst_QTextFormat()
{
}

// Testing get/set functions
void tst_QTextFormat::getSetCheck()
{
    QTextFormat obj1;
    // int QTextFormat::objectIndex()
    // void QTextFormat::setObjectIndex(int)
    obj1.setObjectIndex(0);
    QCOMPARE(0, obj1.objectIndex());
    obj1.setObjectIndex(INT_MIN);
    QCOMPARE(INT_MIN, obj1.objectIndex());
    obj1.setObjectIndex(INT_MAX);
    QCOMPARE(INT_MAX, obj1.objectIndex());
}

void tst_QTextFormat::defaultAlignment()
{
    QTextBlockFormat fmt;
    QVERIFY(!fmt.hasProperty(QTextFormat::BlockAlignment));
    QCOMPARE(fmt.intProperty(QTextFormat::BlockAlignment), 0);
    QVERIFY(fmt.alignment() == Qt::AlignLeft);
}

QTEST_MAIN(tst_QTextFormat)
#include "tst_qtextformat.moc"
