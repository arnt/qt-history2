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
