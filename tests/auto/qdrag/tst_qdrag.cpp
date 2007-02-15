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
#include <qdrag.h>


//TESTED_CLASS=
//TESTED_FILES=qdrag.h

class tst_QDrag : public QObject
{
Q_OBJECT

public:
    tst_QDrag();
    virtual ~tst_QDrag();

private slots:
    void getSetCheck();
};

tst_QDrag::tst_QDrag()
{
}

tst_QDrag::~tst_QDrag()
{
}

// Testing get/set functions
void tst_QDrag::getSetCheck()
{
    QDrag obj1(0);
    // QMimeData * QDrag::mimeData()
    // void QDrag::setMimeData(QMimeData *)
    QMimeData *var1 = new QMimeData;
    obj1.setMimeData(var1);
    QCOMPARE(var1, obj1.mimeData());
    obj1.setMimeData(var1);
    QCOMPARE(var1, obj1.mimeData());
    obj1.setMimeData((QMimeData *)0);
    QCOMPARE((QMimeData *)0, obj1.mimeData());
    // delete var1; // No delete, since QDrag takes ownership

    Qt::DropAction result = obj1.start();
    QCOMPARE(result, Qt::IgnoreAction);
    result = obj1.start(Qt::MoveAction | Qt::LinkAction);
    QCOMPARE(result, Qt::IgnoreAction);
}

QTEST_MAIN(tst_QDrag)
#include "tst_qdrag.moc"
