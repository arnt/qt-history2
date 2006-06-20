/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include "qprogressbar.h"
#include <qapplication.h>
#include <qdebug.h>
#include <qtimer.h>

//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qprogressbar.h gui/widgets/qprogressbar.cpp

class tst_QProgressBar : public QObject
{
Q_OBJECT
private slots:
    void getSetCheck();
    void minMaxSameValue();
    void destroyIndeterminate();
};

// Testing get/set functions
void tst_QProgressBar::getSetCheck()
{
    QProgressBar obj1;
    // bool QProgressBar::invertedAppearance()
    // void QProgressBar::setInvertedAppearance(bool)
    obj1.setInvertedAppearance(false);
    QCOMPARE(false, obj1.invertedAppearance());
    obj1.setInvertedAppearance(true);
    QCOMPARE(true, obj1.invertedAppearance());

    // int QProgressBar::minimum()
    // void QProgressBar::setMinimum(int)
    obj1.setMinimum(0);
    QCOMPARE(0, obj1.minimum());
    obj1.setMinimum(INT_MAX);
    QCOMPARE(INT_MAX, obj1.minimum());
    obj1.setMinimum(INT_MIN);
    QCOMPARE(INT_MIN, obj1.minimum());

    // int QProgressBar::maximum()
    // void QProgressBar::setMaximum(int)
    obj1.setMaximum(0);
    QCOMPARE(0, obj1.maximum());
    obj1.setMaximum(INT_MIN);
    QCOMPARE(INT_MIN, obj1.maximum());
    obj1.setMaximum(INT_MAX);
    QCOMPARE(INT_MAX, obj1.maximum());

    // int QProgressBar::value()
    // void QProgressBar::setValue(int)
    obj1.setValue(0);
    QCOMPARE(0, obj1.value());
    obj1.setValue(INT_MIN);
    QCOMPARE(INT_MIN, obj1.value());
    obj1.setValue(INT_MAX);
    QCOMPARE(INT_MAX, obj1.value());
}

void tst_QProgressBar::minMaxSameValue()
{
    QProgressBar bar;
    bar.setRange(10, 10);
    bar.setValue(10);
    bar.show();
}

void tst_QProgressBar::destroyIndeterminate()
{
    // This test crashes on styles that animate indeterminate / busy
    // progressbars, and forget to remove the bars from internal logics when
    // it's deleted.
    QPointer<QProgressBar> bar = new QProgressBar;
    bar->setMaximum(0);
    bar->show();

    QEventLoop loop;
    QTimer::singleShot(500, bar, SLOT(deleteLater()));
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY(!bar);
}

QTEST_MAIN(tst_QProgressBar)
#include "tst_qprogressbar.moc"
