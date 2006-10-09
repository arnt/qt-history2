/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qapplication.h>
#include <qtabbar.h>
#include <qstyle.h>

class tst_QTabBar : public QObject
{
    Q_OBJECT

public:
    tst_QTabBar();
    virtual ~tst_QTabBar();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

private slots:
    void getSetCheck();
    void setIconSize();
    void setIconSize_data();

    void testCurrentChanged_data();
    void testCurrentChanged();

    void insertAtCurrentIndex();

    void removeTab_data();
    void removeTab();

    // New functianality in Qt 4.
#if QT_VERSION >= 0x040200
    void setElideMode_data();
    void setElideMode();

    void setUsesScrollButtons_data();
    void setUsesScrollButtons();
#endif
};

// Testing get/set functions
void tst_QTabBar::getSetCheck()
{
    QTabBar obj1;
    obj1.addTab("Tab1");
    obj1.addTab("Tab2");
    obj1.addTab("Tab3");
    obj1.addTab("Tab4");
    obj1.addTab("Tab5");
    // Shape QTabBar::shape()
    // void QTabBar::setShape(Shape)
    obj1.setShape(QTabBar::Shape(QTabBar::RoundedNorth));
    QCOMPARE(QTabBar::Shape(QTabBar::RoundedNorth), obj1.shape());
    obj1.setShape(QTabBar::Shape(QTabBar::RoundedSouth));
    QCOMPARE(QTabBar::Shape(QTabBar::RoundedSouth), obj1.shape());
    obj1.setShape(QTabBar::Shape(QTabBar::RoundedWest));
    QCOMPARE(QTabBar::Shape(QTabBar::RoundedWest), obj1.shape());
    obj1.setShape(QTabBar::Shape(QTabBar::RoundedEast));
    QCOMPARE(QTabBar::Shape(QTabBar::RoundedEast), obj1.shape());
    obj1.setShape(QTabBar::Shape(QTabBar::TriangularNorth));
    QCOMPARE(QTabBar::Shape(QTabBar::TriangularNorth), obj1.shape());
    obj1.setShape(QTabBar::Shape(QTabBar::TriangularSouth));
    QCOMPARE(QTabBar::Shape(QTabBar::TriangularSouth), obj1.shape());
    obj1.setShape(QTabBar::Shape(QTabBar::TriangularWest));
    QCOMPARE(QTabBar::Shape(QTabBar::TriangularWest), obj1.shape());
    obj1.setShape(QTabBar::Shape(QTabBar::TriangularEast));
    QCOMPARE(QTabBar::Shape(QTabBar::TriangularEast), obj1.shape());

    // bool QTabBar::drawBase()
    // void QTabBar::setDrawBase(bool)
    obj1.setDrawBase(false);
    QCOMPARE(false, obj1.drawBase());
    obj1.setDrawBase(true);
    QCOMPARE(true, obj1.drawBase());

    // int QTabBar::currentIndex()
    // void QTabBar::setCurrentIndex(int)
    obj1.setCurrentIndex(0);
    QCOMPARE(0, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MIN);
    QCOMPARE(0, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MAX);
    QCOMPARE(0, obj1.currentIndex());
    obj1.setCurrentIndex(4);
    QCOMPARE(4, obj1.currentIndex());
}

tst_QTabBar::tst_QTabBar()
{
}

tst_QTabBar::~tst_QTabBar()
{
}

void tst_QTabBar::initTestCase()
{
}

void tst_QTabBar::cleanupTestCase()
{
}

void tst_QTabBar::init()
{
}

void tst_QTabBar::setIconSize_data()
{
    QTest::addColumn<int>("sizeToSet");
    QTest::addColumn<int>("expectedWidth");

    int iconDefault = qApp->style()->pixelMetric(QStyle::PM_TabBarIconSize);
    int small = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
    int large = qApp->style()->pixelMetric(QStyle::PM_LargeIconSize);
    QTest::newRow("default") << -1 << iconDefault;
    QTest::newRow("zero") << 0 << 0;
    QTest::newRow("same as default") << iconDefault << iconDefault;
    QTest::newRow("large") << large << large;
    QTest::newRow("small") << small << small;
}

void tst_QTabBar::setIconSize()
{
    QFETCH(int, sizeToSet);
    QFETCH(int, expectedWidth);
    QTabBar tabBar;
    tabBar.setIconSize(QSize(sizeToSet, sizeToSet));
    QCOMPARE(tabBar.iconSize().width(), expectedWidth);
}

void tst_QTabBar::testCurrentChanged_data()
{
    QTest::addColumn<int>("tabToSet");
    QTest::addColumn<int>("expectedCount");

    QTest::newRow("pressAntotherTab") << 1 << 2;
    QTest::newRow("pressTheSameTab") << 0 << 1;
}

void tst_QTabBar::testCurrentChanged()
{
    QFETCH(int, tabToSet);
    QFETCH(int, expectedCount);
    QTabBar tabBar;
    QSignalSpy spy(&tabBar, SIGNAL(currentChanged(int)));
    tabBar.addTab("Tab1");
    tabBar.addTab("Tab2");
    QCOMPARE(tabBar.currentIndex(), 0);
    tabBar.setCurrentIndex(tabToSet);
    QCOMPARE(tabBar.currentIndex(), tabToSet);
    QCOMPARE(spy.count(), expectedCount);
}

void tst_QTabBar::insertAtCurrentIndex()
{
#if QT_VERSION >= 0x040103
    QTabBar tabBar;
    tabBar.addTab("Tab1");
    QCOMPARE(tabBar.currentIndex(), 0);
    tabBar.insertTab(0, "Tab2");
    QCOMPARE(tabBar.currentIndex(), 1);
    tabBar.insertTab(0, "Tab3");
    QCOMPARE(tabBar.currentIndex(), 2);
    tabBar.insertTab(2, "Tab4");
    QCOMPARE(tabBar.currentIndex(), 3);
#else
    QSKIP("This behavior is not well-defined in Qt < 4.1.3", SkipAll);
#endif
}

void tst_QTabBar::removeTab_data()
{
    QTest::addColumn<int>("currentIndex");
    QTest::addColumn<int>("deleteIndex");
    QTest::addColumn<int>("spyCount");
    QTest::addColumn<int>("finalIndex");

    QTest::newRow("deleteEnd") << 0 << 2 << 0 << 0;
    QTest::newRow("deleteEndWithIndexOnEnd") << 2 << 2 << 1 << 1;
    QTest::newRow("deleteMiddle") << 2 << 1 << 0 << 1;
    QTest::newRow("deleteMiddleOnMiddle") << 1 << 1 << 1 << 1;
}
void tst_QTabBar::removeTab()
{
    QTabBar tabbar;

    QFETCH(int, currentIndex);
    QFETCH(int, deleteIndex);
    tabbar.addTab("foo");
    tabbar.addTab("bar");
    tabbar.addTab("baz");
    tabbar.setCurrentIndex(currentIndex);
    QSignalSpy spy(&tabbar, SIGNAL(currentChanged(int)));
    tabbar.removeTab(deleteIndex);
    QTEST(spy.count(), "spyCount");
    QTEST(tabbar.currentIndex(), "finalIndex");
}

#if QT_VERSION >= 0x040200
void tst_QTabBar::setElideMode_data()
{
    QTest::addColumn<int>("tabElideMode");
    QTest::addColumn<int>("expectedMode");

    QTest::newRow("default") << -128 << qApp->style()->styleHint(QStyle::SH_TabBar_ElideMode);
    QTest::newRow("explicit default") << qApp->style()->styleHint(QStyle::SH_TabBar_ElideMode)
                                      << qApp->style()->styleHint(QStyle::SH_TabBar_ElideMode);
    QTest::newRow("None") << int(Qt::ElideNone) << int(Qt::ElideNone);
    QTest::newRow("Left") << int(Qt::ElideLeft) << int(Qt::ElideLeft);
    QTest::newRow("Center") << int(Qt::ElideMiddle) << int(Qt::ElideMiddle);
    QTest::newRow("Right") << int(Qt::ElideRight) << int(Qt::ElideRight);
}

void tst_QTabBar::setElideMode()
{
    QFETCH(int, tabElideMode);
    QTabBar tabBar;
    if (tabElideMode != -128)
        tabBar.setElideMode(Qt::TextElideMode(tabElideMode));
    QTEST(int(tabBar.elideMode()), "expectedMode");
}

void tst_QTabBar::setUsesScrollButtons_data()
{
    QTest::addColumn<int>("usesArrows");
    QTest::addColumn<bool>("expectedArrows");

    QTest::newRow("default") << -128 << !qApp->style()->styleHint(QStyle::SH_TabBar_PreferNoArrows);
    QTest::newRow("explicit default")
                        << int(!qApp->style()->styleHint(QStyle::SH_TabBar_PreferNoArrows))
                        << !qApp->style()->styleHint(QStyle::SH_TabBar_PreferNoArrows);
    QTest::newRow("No") << int(false) << false;
    QTest::newRow("Yes") << int(true) << true;
}

void tst_QTabBar::setUsesScrollButtons()
{
    QFETCH(int, usesArrows);
    QTabBar tabBar;
    if (usesArrows != -128)
        tabBar.setUsesScrollButtons(usesArrows);
    QTEST(tabBar.usesScrollButtons(), "expectedArrows");
}

#endif
QTEST_MAIN(tst_QTabBar)
#include "tst_qtabbar.moc"
