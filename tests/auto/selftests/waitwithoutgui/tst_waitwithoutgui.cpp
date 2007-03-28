/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QPushButton>

class tst_WaitWithoutGUI: public QObject
{
Q_OBJECT
private slots:
    void callQWait() const;
};

void tst_WaitWithoutGUI::callQWait() const
{
    /* Simply call qWait(). */
    QTest::qWait(100);
}

QTEST_MAIN(tst_WaitWithoutGUI)

#include "tst_waitwithoutgui.moc"
