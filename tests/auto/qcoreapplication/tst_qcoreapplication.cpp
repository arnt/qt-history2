/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

class tst_QCoreApplication: public QObject
{
    Q_OBJECT
private slots:
    void qAppName();
};

void tst_QCoreApplication::qAppName()
{
    QVERIFY(!::qAppName().isEmpty());
}

QTEST_MAIN(tst_QCoreApplication)
#include "tst_qcoreapplication.moc"
