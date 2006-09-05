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
#include <qimageiohandler.h>
#include <qfile.h>

//TESTED_CLASS=
//TESTED_FILES=qimageiohandler.h

class tst_QImageIOHandler : public QObject
{
Q_OBJECT

public:
    tst_QImageIOHandler();
    virtual ~tst_QImageIOHandler();

private slots:
    void getSetCheck();
};

class MyImageIOHandler : public QImageIOHandler
{
public:
    MyImageIOHandler() : QImageIOHandler() { }
    bool canRead() const { return true; }    
    bool read(QImage *) { return true; }
};

tst_QImageIOHandler::tst_QImageIOHandler()
{
}

tst_QImageIOHandler::~tst_QImageIOHandler()
{
}

// Testing get/set functions
void tst_QImageIOHandler::getSetCheck()
{
    MyImageIOHandler obj1;
    // QIODevice * QImageIOHandler::device()
    // void QImageIOHandler::setDevice(QIODevice *)
    QFile *var1 = new QFile;
    obj1.setDevice(var1);
    QCOMPARE(obj1.device(), (QIODevice *)var1);
    obj1.setDevice((QIODevice *)0);
    QCOMPARE(obj1.device(), (QIODevice *)0);
    delete var1;
}

QTEST_MAIN(tst_QImageIOHandler)
#include "tst_qimageiohandler.moc"
