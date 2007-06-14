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
#include <qtextobject.h>
#include <qtextdocument.h>
#include <qtextedit.h>
#include <qtextcursor.h>

//TESTED_CLASS=
//TESTED_FILES=qtextobject.h

class tst_QTextObject : public QObject
{
Q_OBJECT

public:
    tst_QTextObject();
    virtual ~tst_QTextObject();

private slots:
    void getSetCheck();
};

tst_QTextObject::tst_QTextObject()
{
}

tst_QTextObject::~tst_QTextObject()
{
}

// Testing get/set functions
void tst_QTextObject::getSetCheck()
{
    QTextEdit edit;
    QTextFrame obj1(edit.document());
    // QTextFrameLayoutData * QTextFrame::layoutData()
    // void QTextFrame::setLayoutData(QTextFrameLayoutData *)
    QTextFrameLayoutData *var1 = new QTextFrameLayoutData;
    obj1.setLayoutData(var1);
    QCOMPARE(var1, obj1.layoutData());
    obj1.setLayoutData((QTextFrameLayoutData *)0);
    QCOMPARE((QTextFrameLayoutData *)0, obj1.layoutData());
    // delete var1; // No delete, since QTextFrame takes ownership

    QTextBlock obj2 = edit.textCursor().block();
    // QTextBlockUserData * QTextBlock::userData()
    // void QTextBlock::setUserData(QTextBlockUserData *)
    QTextBlockUserData *var2 = new QTextBlockUserData;
    obj2.setUserData(var2);
    QCOMPARE(var2, obj2.userData());
    obj2.setUserData((QTextBlockUserData *)0);
    QCOMPARE((QTextBlockUserData *)0, obj2.userData());

    // int QTextBlock::userState()
    // void QTextBlock::setUserState(int)
    obj2.setUserState(0);
    QCOMPARE(0, obj2.userState());
    obj2.setUserState(INT_MIN);
    QCOMPARE(INT_MIN, obj2.userState());
    obj2.setUserState(INT_MAX);
    QCOMPARE(INT_MAX, obj2.userState());
}

QTEST_MAIN(tst_QTextObject)
#include "tst_qtextobject.moc"
