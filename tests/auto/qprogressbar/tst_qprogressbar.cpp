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
    void text();
    void format();
    void setValueRepaint();
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

void tst_QProgressBar::text()
{
    QProgressBar bar;
    bar.setRange(10, 10);
    bar.setValue(10);
    QCOMPARE(bar.text(), QString("100%"));
    bar.setRange(0, 10);
    QCOMPARE(bar.text(), QString("100%"));
    bar.setValue(5);
    QCOMPARE(bar.text(), QString("50%"));
    bar.setRange(0, 5);
    bar.setValue(0);
    bar.setRange(5, 5);
    QCOMPARE(bar.text(), QString());
}

class ProgressBar : public QProgressBar
{
    void paintEvent(QPaintEvent *event)
    {
        repainted = true;
        QProgressBar::paintEvent(event);
    }
public:
    bool repainted;
};

void tst_QProgressBar::format()
{
    ProgressBar bar;
    bar.setRange(0, 10);
    bar.setValue(1);
    bar.show();

    qApp->processEvents();
    bar.repainted = false;
    bar.setFormat("%v of %m (%p%)");
    qApp->processEvents();
    QVERIFY(bar.repainted);
    bar.repainted = false;
    bar.setFormat("%v of %m (%p%)");
    qApp->processEvents();
    QVERIFY(!bar.repainted);

    QCOMPARE(bar.text(), QString("1 of 10 (10%)"));
    bar.setRange(5, 5);
    bar.setValue(5);
    QCOMPARE(bar.text(), QString("5 of 0 (100%)"));
    bar.setRange(0, 5);
    bar.setValue(0);
    bar.setRange(5, 5);
    QCOMPARE(bar.text(), QString());
}

void tst_QProgressBar::setValueRepaint()
{
    ProgressBar pbar;
    pbar.setMinimum(0);
    pbar.setMaximum(1000);
    pbar.setFormat("%v");
    pbar.show();
    QApplication::processEvents();
    for (int i = pbar.minimum(); i < pbar.maximum(); ++i) {
        pbar.repainted = false;
        pbar.setValue(i);
        QApplication::processEvents();
        QVERIFY(pbar.repainted);
    }
}

QTEST_MAIN(tst_QProgressBar)
#include "tst_qprogressbar.moc"
