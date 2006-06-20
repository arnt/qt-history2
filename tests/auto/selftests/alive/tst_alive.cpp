/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <QtTest/QtTest>

#include "qtestalive.cpp"

class tst_Alive: public QObject
{
    Q_OBJECT

private slots:
    void alive();
};

void tst_Alive::alive()
{
    QTestAlive a;
    a.start();

    sleep(5);
    QCoreApplication::processEvents();
    qDebug("CUT");
    sleep(5);
}

QTEST_MAIN(tst_Alive)
#include "tst_alive.moc"
