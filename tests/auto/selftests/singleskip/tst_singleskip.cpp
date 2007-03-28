/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtTest/QtTest>

class tst_SingleSkip: public QObject
{
    Q_OBJECT

private slots:
    void myTest() const;
};

void tst_SingleSkip::myTest() const
{
    QSKIP("skipping test", SkipAll);
}

QTEST_MAIN(tst_SingleSkip)

#include "tst_singleskip.moc"
