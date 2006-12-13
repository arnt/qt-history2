/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qtabwidget.h>
#include <qdebug.h>
#include <qapplication.h>

//TESTED_CLASS=QTabWidget
//TESTED_FILES=gui/widgets/qtabwidget.h gui/widgets/qtabwidget.cpp

class QTabWidgetChild:public QTabWidget {
  public:
    QTabWidgetChild():tabCount(0) {
        QVERIFY(tabBar() != NULL);
        QWidget *w = new QWidget;
        int index = addTab(w, "test");
          QCOMPARE(tabCount, 1);
          removeTab(index);
          QCOMPARE(tabCount, 0);

          // Test bad arguments
          // This will assert, so don't do it :)
          //setTabBar(NULL);
    };

  protected:
    virtual void tabInserted(int /*index */ ) {
        tabCount++;
    };
    virtual void tabRemoved(int /*index */ ) {
        tabCount--;
    };
    int tabCount;
};

class tst_QTabWidget:public QObject {
  Q_OBJECT
  public:
    tst_QTabWidget();

  public slots:
    void init();
    void cleanup();
  private slots:
    void getSetCheck();
    void testChild();
    void addRemoveTab();
    void tabPosition();
    void tabEnabled();
    void tabText();
    void tabShape();
    void tabTooltip();
    void tabIcon();
    void indexOf();
    void currentWidget();
    void currentIndex();
    void cornerWidget();
    void removeTab();
    void keyboardNavigation();

  private:
    int addPage();
    void removePage(int index);
    QTabWidget *tw;
};

// Testing get/set functions
void tst_QTabWidget::getSetCheck()
{
    QTabWidget obj1;
    QWidget *w1 = new QWidget;
    QWidget *w2 = new QWidget;
    QWidget *w3 = new QWidget;
    QWidget *w4 = new QWidget;
    QWidget *w5 = new QWidget;
    
    obj1.addTab(w1, "Page 1");
    obj1.addTab(w2, "Page 2");
    obj1.addTab(w3, "Page 3");
    obj1.addTab(w4, "Page 4");
    obj1.addTab(w5, "Page 5");

    // TabShape QTabWidget::tabShape()
    // void QTabWidget::setTabShape(TabShape)
    obj1.setTabShape(QTabWidget::TabShape(QTabWidget::Rounded));
    QCOMPARE(QTabWidget::TabShape(QTabWidget::Rounded), obj1.tabShape());
    obj1.setTabShape(QTabWidget::TabShape(QTabWidget::Triangular));
    QCOMPARE(QTabWidget::TabShape(QTabWidget::Triangular), obj1.tabShape());

    // int QTabWidget::currentIndex()
    // void QTabWidget::setCurrentIndex(int)
    obj1.setCurrentIndex(0);
    QCOMPARE(0, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MIN);
    QCOMPARE(0, obj1.currentIndex());
    obj1.setCurrentIndex(INT_MAX);
    QCOMPARE(0, obj1.currentIndex());
    obj1.setCurrentIndex(4);
    QCOMPARE(4, obj1.currentIndex());

    // QWidget * QTabWidget::currentWidget()
    // void QTabWidget::setCurrentWidget(QWidget *)
    obj1.setCurrentWidget(w1);
    QCOMPARE(w1, obj1.currentWidget());
    obj1.setCurrentWidget(w5);
    QCOMPARE(w5, obj1.currentWidget());
    obj1.setCurrentWidget((QWidget *)0);
    QCOMPARE(w5, obj1.currentWidget()); // current not changed
}

tst_QTabWidget::tst_QTabWidget()
{
}

void tst_QTabWidget::init()
{
    tw = new QTabWidget(0);
    QCOMPARE(tw->count(), 0);
    QCOMPARE(tw->currentIndex(), -1);
    QVERIFY(tw->currentWidget() == NULL);
}

void tst_QTabWidget::cleanup()
{
    delete tw;
    tw = 0;
}

void tst_QTabWidget::testChild()
{
    QTabWidgetChild t;
}

#define LABEL "TEST"
#define TIP "TIP"
int tst_QTabWidget::addPage()
{
    QWidget *w = new QWidget();
    return tw->addTab(w, LABEL);
}

void tst_QTabWidget::removePage(int index)
{
    QWidget *w = tw->widget(index);
    tw->removeTab(index);
    delete w;
}

/**
 * Tests:
 * addTab(...) which really calls -> insertTab(...)
 * widget(...)
 * removeTab(...);
 * If this fails then many others probably will too.
 */
void tst_QTabWidget::addRemoveTab()
{
    // Test bad arguments
    tw->addTab(NULL, LABEL);
    QCOMPARE(tw->count(), 0);
    tw->removeTab(-1);
    QCOMPARE(tw->count(), 0);
    QVERIFY(tw->widget(-1) == 0);

    QWidget *w = new QWidget();
    int index = tw->addTab(w, LABEL);
    // return value
    QCOMPARE(tw->indexOf(w), index);

    QCOMPARE(tw->count(), 1);
    QVERIFY(tw->widget(index) == w);
    QCOMPARE(tw->tabText(index), QString(LABEL));

    removePage(index);
    QCOMPARE(tw->count(), 0);
}

void tst_QTabWidget::tabPosition()
{
    tw->setTabPosition(QTabWidget::North);
    QCOMPARE(tw->tabPosition(), QTabWidget::North);
    tw->setTabPosition(QTabWidget::South);
    QCOMPARE(tw->tabPosition(), QTabWidget::South);
    tw->setTabPosition(QTabWidget::East);
    QCOMPARE(tw->tabPosition(), QTabWidget::East);
    tw->setTabPosition(QTabWidget::West);
    QCOMPARE(tw->tabPosition(), QTabWidget::West);
}

void tst_QTabWidget::tabEnabled()
{
    // Test bad arguments
    QVERIFY(tw->isTabEnabled(-1) == false);
    tw->setTabEnabled(-1, false);

    int index = addPage();

    tw->setTabEnabled(index, true);
    QVERIFY(tw->isTabEnabled(index) == true);
    tw->setTabEnabled(index, false);
    QVERIFY(tw->isTabEnabled(index) == false);
    tw->setTabEnabled(index, true);
    QVERIFY(tw->isTabEnabled(index) == true);

    removePage(index);
}

void tst_QTabWidget::tabText()
{
    // Test bad arguments
    QCOMPARE(tw->tabText(-1), QString(""));
    tw->setTabText(-1, LABEL);

    int index = addPage();

    tw->setTabText(index, "new");
    QCOMPARE(tw->tabText(index), QString("new"));
    tw->setTabText(index, LABEL);
    QCOMPARE(tw->tabText(index), QString(LABEL));

    removePage(index);
}

void tst_QTabWidget::tabShape()
{
    int index = addPage();

    tw->setTabShape(QTabWidget::Rounded);
    QCOMPARE(tw->tabShape(), QTabWidget::Rounded);
    tw->setTabShape(QTabWidget::Triangular);
    QCOMPARE(tw->tabShape(), QTabWidget::Triangular);
    tw->setTabShape(QTabWidget::Rounded);
    QCOMPARE(tw->tabShape(), QTabWidget::Rounded);

    removePage(index);
}

void tst_QTabWidget::tabTooltip()
{
    // Test bad arguments
    QCOMPARE(tw->tabToolTip(-1), QString(""));
    tw->setTabText(-1, TIP);

    int index = addPage();

    tw->setTabToolTip(index, "tip");
    QCOMPARE(tw->tabToolTip(index), QString("tip"));
    tw->setTabToolTip(index, TIP);
    QCOMPARE(tw->tabToolTip(index), QString(TIP));

    removePage(index);
}

void tst_QTabWidget::tabIcon()
{
    // Test bad arguments
    QVERIFY(tw->tabToolTip(-1).isNull());
    tw->setTabIcon(-1, QIcon());

    int index = addPage();

    QIcon icon;
    tw->setTabIcon(index, icon);
    QVERIFY(tw->tabIcon(index).isNull());

    removePage(index);
}

void tst_QTabWidget::indexOf()
{
    // Test bad arguments
    QCOMPARE(tw->indexOf(NULL), -1);

    int index = addPage();
    QWidget *w = tw->widget(index);
    QCOMPARE(tw->indexOf(w), index);

    removePage(index);
}

void tst_QTabWidget::currentWidget()
{
    // Test bad arguments
    tw->setCurrentWidget(NULL);
    QVERIFY(tw->currentWidget() == NULL);

    int index = addPage();
    QWidget *w = tw->widget(index);
    QVERIFY(tw->currentWidget() == w);
    QCOMPARE(tw->currentIndex(), index);

    tw->setCurrentWidget(NULL);
    QVERIFY(tw->currentWidget() == w);
    QCOMPARE(tw->currentIndex(), index);

    int index2 = addPage();
    QWidget *w2 = tw->widget(index2);
    QVERIFY(tw->currentWidget() == w);
    QCOMPARE(tw->currentIndex(), index);

    removePage(index2);
    removePage(index);
}

/**
 * setCurrentWidget(..) calls setCurrentIndex(..)
 * currentChanged(..) SIGNAL
 */
void tst_QTabWidget::currentIndex()
{
    // Test bad arguments
    QSignalSpy spy(tw, SIGNAL(currentChanged(int)));
    tw->setCurrentIndex(-1);
    QCOMPARE(tw->currentIndex(), -1);
    QCOMPARE(spy.count(), 0);

    int firstIndex = addPage();
    int index = addPage();
    tw->setCurrentIndex(firstIndex);
    QCOMPARE(tw->currentIndex(), firstIndex);
    tw->setCurrentIndex(index);
    QCOMPARE(tw->currentIndex(), index);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toInt() == index);

    removePage(index);
    removePage(firstIndex);
}

void tst_QTabWidget::cornerWidget()
{
    // Test bad arguments
    tw->setCornerWidget(NULL, Qt::TopRightCorner);

    QVERIFY(tw->cornerWidget(Qt::TopLeftCorner) == 0);
    QVERIFY(tw->cornerWidget(Qt::TopRightCorner) == 0);
    QVERIFY(tw->cornerWidget(Qt::BottomLeftCorner) == 0);
    QVERIFY(tw->cornerWidget(Qt::BottomRightCorner) == 0);

    QWidget *w = new QWidget(0);
    tw->setCornerWidget(w, Qt::TopLeftCorner);
    QCOMPARE(w->parent(), tw);
    QVERIFY(tw->cornerWidget(Qt::TopLeftCorner) == w);
    tw->setCornerWidget(w, Qt::TopRightCorner);
    QVERIFY(tw->cornerWidget(Qt::TopRightCorner) == w);
    tw->setCornerWidget(w, Qt::BottomLeftCorner);
    QVERIFY(tw->cornerWidget(Qt::BottomLeftCorner) == w);
    tw->setCornerWidget(w, Qt::BottomRightCorner);
    QVERIFY(tw->cornerWidget(Qt::BottomRightCorner) == w);

    tw->setCornerWidget(0, Qt::TopRightCorner);
    QVERIFY(tw->cornerWidget(Qt::TopRightCorner) == 0);
    QCOMPARE(w->isHidden(), true);
}

void tst_QTabWidget::removeTab()
{
#if QT_VERSION < 0x040100
    QSKIP("Fixed in 4.1.", SkipSingle);
#endif
    tw->addTab(new QWidget, "1");
    tw->addTab(new QWidget, "2");
    tw->addTab(new QWidget, "3");
    tw->addTab(new QWidget, "4");
    tw->addTab(new QWidget, "5");
    tw->setCurrentIndex(4);
    tw->removeTab(4);
    QCOMPARE(tw->currentIndex(), 3);

    tw->setCurrentIndex(1);
    tw->removeTab(1);
    QCOMPARE(tw->currentIndex(), 1);
}

void tst_QTabWidget::keyboardNavigation()
{
    int firstIndex = addPage();
    int index = addPage();
    addPage();
    tw->setCurrentIndex(firstIndex);
    QCOMPARE(tw->currentIndex(), firstIndex);

    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier);
    QCOMPARE(tw->currentIndex(), 1);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier);
    QCOMPARE(tw->currentIndex(), 2);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier);
    QCOMPARE(tw->currentIndex(), 0);
    tw->setTabEnabled(1, false);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier);
    QCOMPARE(tw->currentIndex(), 2);

    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier | Qt::ShiftModifier);
    QCOMPARE(tw->currentIndex(), 0);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier | Qt::ShiftModifier);
    QCOMPARE(tw->currentIndex(), 2);
    tw->setTabEnabled(1, true);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier | Qt::ShiftModifier);
    QCOMPARE(tw->currentIndex(), 1);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier | Qt::ShiftModifier);
    QCOMPARE(tw->currentIndex(), 0);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier | Qt::ShiftModifier);
    QCOMPARE(tw->currentIndex(), 2);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier);
    QCOMPARE(tw->currentIndex(), 0);
    
    // Disable all and try to go to the next. It should not move anywhere, and more importantly
    // it should not loop forever. (a naive "search for the first enabled tabbar") implementation
    // might do that)
    tw->setTabEnabled(0, false);
    tw->setTabEnabled(1, false);
    tw->setTabEnabled(2, false);
    QTest::keyClick(tw, Qt::Key_Tab, Qt::ControlModifier);
    // Disabling the current tab will move current tab to the next,
    // but what if next tab is also disabled.. Seems that we haven't thought about that...
    QVERIFY(tw->currentIndex() < 3 && tw->currentIndex() >= 0);
}

QTEST_MAIN(tst_QTabWidget)
#include "tst_qtabwidget.moc"
