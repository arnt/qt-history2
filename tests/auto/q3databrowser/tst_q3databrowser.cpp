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
#include <q3databrowser.h>


//TESTED_CLASS=
//TESTED_FILES=q3databrowser.h

class tst_Q3DataBrowser : public QObject
{
Q_OBJECT

public:
    tst_Q3DataBrowser();
    virtual ~tst_Q3DataBrowser();

private slots:
    void getSetCheck();
};

tst_Q3DataBrowser::tst_Q3DataBrowser()
{
}

tst_Q3DataBrowser::~tst_Q3DataBrowser()
{
}

// Testing get/set functions
void tst_Q3DataBrowser::getSetCheck()
{
    Q3DataBrowser obj1;
    // bool Q3DataBrowser::boundaryChecking()
    // void Q3DataBrowser::setBoundaryChecking(bool)
    obj1.setBoundaryChecking(false);
    QCOMPARE(false, obj1.boundaryChecking());
    obj1.setBoundaryChecking(true);
    QCOMPARE(true, obj1.boundaryChecking());
}

QTEST_MAIN(tst_Q3DataBrowser)
#include "tst_q3databrowser.moc"
