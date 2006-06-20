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
#include <q3progressbar.h>


//TESTED_CLASS=
//TESTED_FILES=q3progressbar.h

class tst_Q3ProgressBar : public QObject
{
Q_OBJECT

public:
    tst_Q3ProgressBar();
    virtual ~tst_Q3ProgressBar();

private slots:
    void getSetCheck();
};

tst_Q3ProgressBar::tst_Q3ProgressBar()
{
}

tst_Q3ProgressBar::~tst_Q3ProgressBar()
{
}

// Testing get/set functions
void tst_Q3ProgressBar::getSetCheck()
{
    Q3ProgressBar obj1;
    // bool Q3ProgressBar::centerIndicator()
    // void Q3ProgressBar::setCenterIndicator(bool)
    obj1.setCenterIndicator(false);
    QCOMPARE(false, obj1.centerIndicator());
    obj1.setCenterIndicator(true);
    QCOMPARE(true, obj1.centerIndicator());
}

QTEST_MAIN(tst_Q3ProgressBar)
#include "tst_q3progressbar.moc"
