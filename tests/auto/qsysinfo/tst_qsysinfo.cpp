/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_QSysInfo: public QObject
{
Q_OBJECT
private slots:
    void coreCount();
};

void tst_QSysInfo::coreCount()
{
    QVERIFY(QSysInfo::coreCount() > 0);
    qDebug() << "Available cpu cores:" << QSysInfo::coreCount();
}

QTEST_MAIN(tst_QSysInfo)
#include "tst_qsysinfo.moc"
