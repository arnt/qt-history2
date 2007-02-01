/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcalendarwidget.h>
#include <qtoolbutton.h>
#include <qspinbox.h>
#include <qmenu.h>
#include <qdebug.h>


//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qcalendarwidget.h gui/widgets/qcalendarwidget.cpp

class tst_QCalendarWidget : public QObject
{
    Q_OBJECT

public:
    tst_QCalendarWidget();
    virtual ~tst_QCalendarWidget();
public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void getSetCheck();
    void buttonClickCheck();

};

// Testing get/set functions
void tst_QCalendarWidget::getSetCheck()
{
    QCalendarWidget object;

    //horizontal header formats
    object.setHorizontalHeaderFormat(QCalendarWidget::NoHorizontalHeader);
    QCOMPARE(QCalendarWidget::NoHorizontalHeader, object.horizontalHeaderFormat());
    object.setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    QCOMPARE(QCalendarWidget::SingleLetterDayNames, object.horizontalHeaderFormat());
    object.setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    QCOMPARE(QCalendarWidget::ShortDayNames, object.horizontalHeaderFormat());
    object.setHorizontalHeaderFormat(QCalendarWidget::LongDayNames);
    QCOMPARE(QCalendarWidget::LongDayNames, object.horizontalHeaderFormat());
    //vertical header formats
    object.setVerticalHeaderFormat(QCalendarWidget::ISOWeekNumbers);
    QCOMPARE(QCalendarWidget::ISOWeekNumbers, object.verticalHeaderFormat());
    object.setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    QCOMPARE(QCalendarWidget::NoVerticalHeader, object.verticalHeaderFormat());
    //maximum Date
    QDate maxDate(2006, 7, 3);
    object.setMaximumDate(maxDate);
    QCOMPARE(maxDate, object.maximumDate());
    //minimum date
    QDate minDate(2004, 7, 3);
    object.setMinimumDate(minDate);
    QCOMPARE(minDate, object.minimumDate());
    //day of week
    object.setFirstDayOfWeek(Qt::Thursday);
    QCOMPARE(Qt::Thursday, object.firstDayOfWeek());
    //grid visible
    object.setGridVisible(true);
    QVERIFY(object.isGridVisible());
    object.setGridVisible(false);
    QVERIFY(!object.isGridVisible());
    //header visible
    object.setHeaderVisible(true);
    QVERIFY(object.isHeaderVisible());
    object.setHeaderVisible(false);
    QVERIFY(!object.isHeaderVisible());
    //selection mode
    QCOMPARE(QCalendarWidget::SingleSelection, object.selectionMode());
    object.setSelectionMode(QCalendarWidget::NoSelection);
    QCOMPARE(QCalendarWidget::NoSelection, object.selectionMode());
    object.setSelectionMode(QCalendarWidget::SingleSelection);
    QCOMPARE(QCalendarWidget::SingleSelection, object.selectionMode());
   //selected date
    QDate selectedDate(2005, 7, 3);
    object.setSelectedDate(selectedDate);
    QCOMPARE(selectedDate, object.selectedDate());
    //month and year
    object.setCurrentPage(2004, 1);
    QCOMPARE(1, object.monthShown());
    QCOMPARE(2004, object.yearShown());
    object.showNextMonth();
    QCOMPARE(2, object.monthShown());
    object.showPreviousMonth();
    QCOMPARE(1, object.monthShown());
    object.showNextYear();
    QCOMPARE(2005, object.yearShown());
    object.showPreviousYear();
    QCOMPARE(2004, object.yearShown());
    //date range
    minDate = QDate(2006,1,1);
    maxDate = QDate(2010,12,31);
    object.setDateRange(minDate, maxDate);
    QCOMPARE(maxDate, object.maximumDate());
    QCOMPARE(minDate, object.minimumDate());

    //date should not go beyond the minimum.
    selectedDate = minDate.addDays(-10);
    object.setSelectedDate(selectedDate);
    QCOMPARE(minDate, object.selectedDate());
    QVERIFY(selectedDate != object.selectedDate());
    //date should not go beyond the maximum.
    selectedDate = maxDate.addDays(10);
    object.setSelectedDate(selectedDate);
    QCOMPARE(maxDate, object.selectedDate());
    QVERIFY(selectedDate != object.selectedDate());
    //show today
    QDate today = QDate::currentDate();
    object.showToday();
    QCOMPARE(today.month(), object.monthShown());
    QCOMPARE(today.year(), object.yearShown());
    //slect a different date and move.
    object.setSelectedDate(minDate);
    object.showSelectedDate();
    QCOMPARE(minDate.month(), object.monthShown());
    QCOMPARE(minDate.year(), object.yearShown());
}

void tst_QCalendarWidget::buttonClickCheck()
{
    QCalendarWidget object;
    QSize size = object.sizeHint();
    object.setGeometry(0,0,size.width(), size.height());
    object.show();

    QRect rect = object.geometry();
    QDate selectedDate(2005, 1, 1);
    //click on the month buttons
    int month = object.monthShown();
    QToolButton *button = qFindChild<QToolButton *>(&object, "qt_calendar_prevmonth");
    QTest::mouseClick(button, Qt::LeftButton);
	QCOMPARE(month > 1 ? month-1 : 12, object.monthShown());
    button = qFindChild<QToolButton *>(&object, "qt_calendar_nextmonth");
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(month, object.monthShown());

    button = qFindChild<QToolButton *>(&object, "qt_calendar_yearbutton");
    QTest::mouseClick(button, Qt::LeftButton);
    QVERIFY(!button->isVisible());
    QSpinBox *spinbox = qFindChild<QSpinBox *>(&object, "qt_calendar_yearedit");
    QTest::keyClick(spinbox, '2');
    QTest::keyClick(spinbox, '0');
    QTest::keyClick(spinbox, '0');
    QTest::keyClick(spinbox, '6');
    QTest::qWait(500);
    QWidget *widget = qFindChild<QWidget *>(&object, "qt_calendar_calendarview");
    QTest::mouseClick(widget, Qt::LeftButton);
    QCOMPARE(2006, object.yearShown());
    object.setSelectedDate(selectedDate);
    object.showSelectedDate();
    QTest::keyClick(widget, Qt::Key_Down);
    QVERIFY(selectedDate != object.selectedDate());

    object.setDateRange(QDate(2006,1,1), QDate(2006,2,28));
    object.setSelectedDate(QDate(2006,1,1));
    object.showSelectedDate();
    button = qFindChild<QToolButton *>(&object, "qt_calendar_prevmonth");
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(1, object.monthShown());

    button = qFindChild<QToolButton *>(&object, "qt_calendar_nextmonth");
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(2, object.monthShown());
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(2, object.monthShown());

}


tst_QCalendarWidget::tst_QCalendarWidget()
{
}

tst_QCalendarWidget::~tst_QCalendarWidget()
{
}

void tst_QCalendarWidget::initTestCase()
{
}

void tst_QCalendarWidget::cleanupTestCase()
{
}

void tst_QCalendarWidget::init()
{
}

void tst_QCalendarWidget::cleanup()
{
}

QTEST_MAIN(tst_QCalendarWidget)
#include "tst_qcalendarwidget.moc"
