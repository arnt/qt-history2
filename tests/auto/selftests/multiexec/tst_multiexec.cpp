/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_Nothing: public QObject
{
Q_OBJECT
private slots:
    void nothing() { QVERIFY(true); }
};

int main(int argc, char *argv[])
{
    tst_Nothing nada;
    for (int i = 0; i < 5; ++i)
        QTest::qExec(&nada, argc, argv);
    return 0;
}

#include "tst_multiexec.moc"
