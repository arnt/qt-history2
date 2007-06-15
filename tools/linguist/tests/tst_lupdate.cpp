
/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include <QtTest/QtTest>

#include <QtCore>

//TESTED_CLASS=
//TESTED_FILES=

class tst_lupdate : public QObject
{
    Q_OBJECT

public:
    tst_lupdate() {}
    ~tst_lupdate() {}

private slots:
    void runCheck();
};


void tst_lupdate::runCheck()
{
}

QTEST_MAIN(tst_lupdate)
#include "tst_lupdate.moc"

