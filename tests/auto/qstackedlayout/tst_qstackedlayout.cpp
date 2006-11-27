/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QStackedLayout>
#include <qapplication.h>
#include <qwidget.h>
#include <QPushButton>

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qlayout.cpp gui/kernel/qlayout.h

class tst_QStackedLayout : public QObject
{
    Q_OBJECT

public:
    tst_QStackedLayout();
    virtual ~tst_QStackedLayout();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void getSetCheck();
    void testCase();
    void deleteCurrent();
    void removeWidget();

private:
    QWidget *testWidget;
};

// Testing get/set functions
void tst_QStackedLayout::getSetCheck()
{
    QStackedLayout obj1;
    // int QStackedLayout::currentIndex()
    // void QStackedLayout::setCurrentIndex(int)
    obj1.setCurrentIndex(0);
    QCOMPARE(-1, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MIN);
    QCOMPARE(-1, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MAX);
    QCOMPARE(-1, obj1.currentIndex());

    // QWidget * QStackedLayout::currentWidget()
    // void QStackedLayout::setCurrentWidget(QWidget *)
    QWidget *var2 = new QWidget();
    obj1.addWidget(var2);
    obj1.setCurrentWidget(var2);
    QCOMPARE(var2, obj1.currentWidget());

    obj1.setCurrentWidget((QWidget *)0);
    QCOMPARE(obj1.currentWidget(), var2);

    delete var2;
}


tst_QStackedLayout::tst_QStackedLayout()
{
}

tst_QStackedLayout::~tst_QStackedLayout()
{
}

void tst_QStackedLayout::initTestCase()
{
    testWidget = new QWidget(0);
    testWidget->resize( 200, 200 );
    testWidget->show();
}

void tst_QStackedLayout::cleanupTestCase()
{
    delete testWidget;
    testWidget = 0;
}

void tst_QStackedLayout::init()
{
}

void tst_QStackedLayout::cleanup()
{
}


void tst_QStackedLayout::testCase()
{
    QStackedLayout onStack(testWidget);
    QStackedLayout *testLayout = &onStack;
    testWidget->setLayout(testLayout);

    // Nothing in layout
    QCOMPARE(testLayout->currentIndex(), -1);
    QCOMPARE(testLayout->currentWidget(), static_cast<QWidget*>(0));
    QCOMPARE(testLayout->count(), 0);

    // One widget added to layout
    QWidget *w1 = new QWidget(testWidget);
    testLayout->addWidget(w1);
    QCOMPARE(testLayout->currentIndex(), 0);
    QCOMPARE(testLayout->currentWidget(), w1);
    QCOMPARE(testLayout->count(), 1);

    // Another widget added to layout
    QWidget *w2 = new QWidget(testWidget);
    testLayout->addWidget(w2);
    QCOMPARE(testLayout->currentIndex(), 0);
    QCOMPARE(testLayout->currentWidget(), w1);
    QCOMPARE(testLayout->indexOf(w2), 1);
    QCOMPARE(testLayout->count(), 2);

    // Change the current index
    testLayout->setCurrentIndex(1);
    QCOMPARE(testLayout->currentIndex(), 1);
    QCOMPARE(testLayout->currentWidget(), w2);

    // First widget removed from layout
    testLayout->removeWidget(w1);
    QCOMPARE(testLayout->currentIndex(), 0);
    QCOMPARE(testLayout->currentWidget(), w2);
    QCOMPARE(testLayout->count(), 1);

    // Second widget removed from layout; back to nothing
    testLayout->removeWidget(w2);
    QCOMPARE(testLayout->currentIndex(), -1);
    QCOMPARE(testLayout->currentWidget(), static_cast<QWidget*>(0));
    QCOMPARE(testLayout->count(), 0);

    // Another widget inserted at current index.
    // Current index should become current index + 1, but the
    // current widget should stay the same
    testLayout->addWidget(w1);
    QCOMPARE(testLayout->currentIndex(), 0);
    QCOMPARE(testLayout->currentWidget(), w1);
    testLayout->insertWidget(0, w2);
    QCOMPARE(testLayout->currentIndex(), 1);
    QCOMPARE(testLayout->currentWidget(), w1);
    QVERIFY(w1->isVisible());
    QVERIFY(!w2->isVisible());

    testLayout->setCurrentWidget(w2);
    // Another widget added, so we have: w2, w1, w3 with w2 current
    QWidget *w3 = new QWidget(testWidget);
    testLayout->addWidget(w3);
    QCOMPARE(testLayout->indexOf(w2), 0);
    QCOMPARE(testLayout->indexOf(w1), 1);
    QCOMPARE(testLayout->indexOf(w3), 2);

    // Set index to 1 and remove that widget (w1).
    // Then, current index should still be 1, but w3
    // should be the new current widget.
    testLayout->setCurrentIndex(1);
    testLayout->removeWidget(w1);
    QCOMPARE(testLayout->currentIndex(), 1);
    QCOMPARE(testLayout->currentWidget(), w3);
    QVERIFY(w3->isVisible());

    // Remove the current widget (w3).
    // Since it's the last one in the list, current index should now
    // become 0 and w2 becomes the current widget.
    testLayout->removeWidget(w3);
    QCOMPARE(testLayout->currentIndex(), 0);
    QCOMPARE(testLayout->currentWidget(), w2);
    QVERIFY(w2->isVisible());

    // Make sure index is decremented when we remove a widget at index < current index
    testLayout->addWidget(w1);
    testLayout->addWidget(w3);
    testLayout->setCurrentIndex(2);
    testLayout->removeWidget(w2); // At index 0
    QCOMPARE(testLayout->currentIndex(), 1);
    QCOMPARE(testLayout->currentWidget(), w3);
    QVERIFY(w3->isVisible());
    testLayout->removeWidget(w1); // At index 0
    QCOMPARE(testLayout->currentIndex(), 0);
    QCOMPARE(testLayout->currentWidget(), w3);
    QVERIFY(w3->isVisible());
    testLayout->removeWidget(w3);
    QCOMPARE(testLayout->currentIndex(), -1);
    QCOMPARE(testLayout->currentWidget(), static_cast<QWidget*>(0));
}

void tst_QStackedLayout::deleteCurrent()
{
    QStackedLayout *testLayout = new QStackedLayout(testWidget);

    QWidget *w1 = new QWidget;
    testLayout->addWidget(w1);
    QWidget *w2 = new QWidget;
    testLayout->addWidget(w2);
    QCOMPARE(testLayout->currentWidget(), w1);
    delete testLayout->currentWidget();
    QCOMPARE(testLayout->currentWidget(), w2);
}

void tst_QStackedLayout::removeWidget()
{
    if (testWidget->layout()) delete testWidget->layout();
    QVBoxLayout *vbox = new QVBoxLayout(testWidget);

    QPushButton *top = new QPushButton("top", testWidget);  //add another widget that can receive focus
    top->setObjectName("top");
    vbox->addWidget(top);

    QStackedLayout *testLayout = new QStackedLayout();
    QPushButton *w1 = new QPushButton("1st", testWidget);
    w1->setObjectName("1st");
    testLayout->addWidget(w1);
    QPushButton *w2 = new QPushButton("2nd", testWidget);
    w2->setObjectName("2nd");
    testLayout->addWidget(w2);
    vbox->addLayout(testLayout);
    top->setFocus();
    int i =0;
    for (;;) {
        if (QApplication::focusWidget() == top)
            break;
        else if (i >= 5)
            QSKIP("Can't get focus", SkipSingle);
        QTest::qWait(100);
        ++i;
    }
    QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(top));

    // focus should stay at the 'top' widget
    testLayout->removeWidget(w1);

    QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(top));
}

QTEST_MAIN(tst_QStackedLayout)
#include "tst_qstackedlayout.moc"

