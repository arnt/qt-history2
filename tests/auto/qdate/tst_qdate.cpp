/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qdatetime.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qdatetime.h corelib/tools/qdatetime.cpp

class tst_QDate : public QObject
{
Q_OBJECT

public:
    tst_QDate();
    virtual ~tst_QDate();

public slots:
    void init();
    void cleanup();
private slots:
    void toString();
    void isValid_data();
    void isValid();
    void julianDay_data();
    void julianDay();
    void weekNumber_invalid_data();
    void weekNumber_invalid();
    void weekNumber_data();
    void weekNumber();
    void addDays_data();
    void addDays();
    void addMonths_data();
    void addMonths();
    void addYears_data();
    void addYears();
    void operator_eq_eq();
    void operator_not_eq();
    void operator_lt();
    void operator_gt();
    void operator_lt_eq();
    void operator_gt_eq();
    void fromString_data();
    void fromString();
    void toString_format_data();
    void toString_format();
    void isLeapYear();
    void yearsZeroToNinetyNine();
};

Q_DECLARE_METATYPE(QDate)

tst_QDate::tst_QDate()
{
}

tst_QDate::~tst_QDate()
{

}

void tst_QDate::init()
{
// This will be executed immediately before each test is run.
// TODO: Add initialization code here.
}

void tst_QDate::cleanup()
{
// This will be executed immediately after each test is run.
// TODO: Add cleanup code here.
}

void tst_QDate::isValid_data()
{
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");
    QTest::addColumn<uint>("jd");
    QTest::addColumn<bool>("valid");

    QTest::newRow("0-0-0") << 0 << 0 << 0 << 0U << false;
    QTest::newRow("month 0") << 2000 << 0 << 1 << 0U << false;
    QTest::newRow("day 0") << 2000 << 1 << 0 << 0U << false;

    // the month of cazember!
    QTest::newRow("month 13") << 2000 << 13 << 1 << 0U << false;

    // test leap years
    QTest::newRow("non-leap") << 2006 << 2 << 29 << 0U << false;
    QTest::newRow("normal leap") << 2004 << 2 << 29 << 2453065U << true;
    QTest::newRow("century leap") << 1900 << 2 << 29 << 0U << false;
    QTest::newRow("century leap") << 2100 << 2 << 29 << 0U << false;
    QTest::newRow("400-years leap") << 2000 << 2 << 29 << 2451604U << true;
    QTest::newRow("400-years leap 2") << 2400 << 2 << 29 << 2597701U << true;
#if QT_VERSION >= 0x040200
    QTest::newRow("400-years leap 3") << 1600 << 2 << 29 << 2305507U << true;
    QTest::newRow("year 0") << 0 << 2 << 27 << 0U << false;
#endif

    // test the number of days in months:
    QTest::newRow("jan") << 2000 << 1 << 31 << 2451575U << true;
    QTest::newRow("feb") << 2000 << 2 << 29 << 2451604U << true; // same data as 400-years leap
    QTest::newRow("mar") << 2000 << 3 << 31 << 2451635U << true;
    QTest::newRow("apr") << 2000 << 4 << 30 << 2451665U << true;
    QTest::newRow("may") << 2000 << 5 << 31 << 2451696U << true;
    QTest::newRow("jun") << 2000 << 6 << 30 << 2451726U << true;
    QTest::newRow("jul") << 2000 << 7 << 31 << 2451757U << true;
    QTest::newRow("aug") << 2000 << 8 << 31 << 2451788U << true;
    QTest::newRow("sep") << 2000 << 9 << 30 << 2451818U << true;
    QTest::newRow("oct") << 2000 << 10 << 31 << 2451849U << true;
    QTest::newRow("nov") << 2000 << 11 << 30 << 2451879U << true;
    QTest::newRow("dec") << 2000 << 12 << 31 << 2451910U << true;

    // and invalid dates:
    QTest::newRow("ijan") << 2000 << 1 << 32 << 0U << false;
    QTest::newRow("ifeb") << 2000 << 2 << 30 << 0U << false;
    QTest::newRow("imar") << 2000 << 3 << 32 << 0U << false;
    QTest::newRow("iapr") << 2000 << 4 << 31 << 0U << false;
    QTest::newRow("imay") << 2000 << 5 << 32 << 0U << false;
    QTest::newRow("ijun") << 2000 << 6 << 31 << 0U << false;
    QTest::newRow("ijul") << 2000 << 7 << 32 << 0U << false;
    QTest::newRow("iaug") << 2000 << 8 << 32 << 0U << false;
    QTest::newRow("isep") << 2000 << 9 << 31 << 0U << false;
    QTest::newRow("ioct") << 2000 << 10 << 32 << 0U << false;
    QTest::newRow("inov") << 2000 << 11 << 31 << 0U << false;
    QTest::newRow("idec") << 2000 << 12 << 32 << 0U << false;

    // the beginning of the Julian Day calendar:
#if QT_VERSION >= 0x040200
    QTest::newRow("jd negative1") << -4714 << 1 << 1 << 0U << false;
    QTest::newRow("jd negative2") << -4713 << 1 << 1 << 0U << false;
    QTest::newRow("jd negative3") << -4713 << 1 << 2 << 1U << true;
    QTest::newRow("jd negative4") << -4713 << 1 << 3 << 2U << true;
    QTest::newRow("jd 0") << -4713 << 1 << 1 << 0U << false;
    QTest::newRow("jd 1") << -4713 << 1 << 2 << 1U << true;
    QTest::newRow("imminent overflow") << 11754508 << 12 << 13 << 4294967295U << true;
#endif
}

void tst_QDate::isValid()
{
    QFETCH(int, year);
    QFETCH(int, month);
    QFETCH(int, day);

    QTEST(QDate::isValid(year, month, day), "valid");

    QDate d;
    d.setDate(year, month, day);
    QTEST(d.isValid(), "valid");
}

void tst_QDate::julianDay_data()
{
    isValid_data();
}

void tst_QDate::julianDay()
{
    QFETCH(int, year);
    QFETCH(int, month);
    QFETCH(int, day);
    QFETCH(uint, jd);

    {
        QDate d;
        d.setDate(year, month, day);
        QCOMPARE(uint(d.toJulianDay()), jd);
    }

    if (jd) {
        QDate d = QDate::fromJulianDay(jd);
        QCOMPARE(d.year(), year);
        QCOMPARE(d.month(), month);
        QCOMPARE(d.day(), day);
    }
}

void tst_QDate::weekNumber_data()
{
    QTest::addColumn<int>("expectedWeekNum");
    QTest::addColumn<int>("expectedYearNum");
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");

    //next we fill it with data
    QTest::newRow( "data0" )  << 10 << 2002 << 2002 << 3 << 8;
    QTest::newRow( "data1" )  << 10 << 2002 << 2002 << 3 << 8;
    QTest::newRow( "data2" )  << 52 << 1999 << 2000 << 1 << 1;
    QTest::newRow( "data3" )  << 52 << 1999 << 1999 << 12 << 31;
    QTest::newRow( "data4" )  << 1 << 2001 << 2001 << 1 << 1;
    QTest::newRow( "data5" )  << 53 << 1998 << 1998 << 12 << 31;
    QTest::newRow( "data6" )  << 1 << 1985 << 1984 << 12 << 31;
#if QT_VERSION >= 0x030300
    // This is a bug that was fixed for 3.3
    QTest::newRow( "data7" )  << 52 << 2006 << 2006 << 12 << 31;
#endif
}

void tst_QDate::weekNumber()
{
#if QT_VERSION >= 0x030100
    int yearNumber;
//    int weekNumber;
    QFETCH( int, year );
    QFETCH( int, month );
    QFETCH( int, day );
    QFETCH( int, expectedWeekNum );
    QFETCH( int, expectedYearNum );
    QDate dt1( year, month, day );
    QCOMPARE( dt1.weekNumber( &yearNumber ), expectedWeekNum );
    QCOMPARE( yearNumber, expectedYearNum );
#else
    QSKIP( "Not tested with Qt versions < 3.1", SkipAll);
#endif
}

void tst_QDate::weekNumber_invalid_data()
{
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");

    //next we fill it with data
    QTest::newRow( "data0" )  << 0 << 0 << 0;
    QTest::newRow( "data1" )  << 2001 << 1 << 32;
    QTest::newRow( "data2" )  << 1999 << 2 << 29;
}

void tst_QDate::weekNumber_invalid()
{
#if QT_VERSION >= 0x030100
    QDate dt;
    int yearNumber;
//    int weekNumber;
    QCOMPARE( dt.weekNumber( &yearNumber ), 0 );
#else
    QSKIP( "Not tested with Qt versions < 3.1", SkipAll);
#endif
}

void tst_QDate::addDays()
{
    QFETCH( int, year );
    QFETCH( int, month );
    QFETCH( int, day );
    QFETCH( int, amountToAdd );
    QFETCH( int, expectedYear );
    QFETCH( int, expectedMonth );
    QFETCH( int, expectedDay );

    QDate dt( year, month, day );
    dt = dt.addDays( amountToAdd );

    QCOMPARE( dt.year(), expectedYear );
    QCOMPARE( dt.month(), expectedMonth );
    QCOMPARE( dt.day(), expectedDay );
}

void tst_QDate::addDays_data()
{
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");
    QTest::addColumn<int>("amountToAdd");
    QTest::addColumn<int>("expectedYear");
    QTest::addColumn<int>("expectedMonth");
    QTest::addColumn<int>("expectedDay");

    QTest::newRow( "data0" ) << 2000 << 1 << 1 << 1 << 2000 << 1 << 2;
    QTest::newRow( "data1" ) << 2000 << 1 << 31 << 1 << 2000 << 2 << 1;
    QTest::newRow( "data2" ) << 2000 << 2 << 28 << 1 << 2000 << 2 << 29;
    QTest::newRow( "data3" ) << 2000 << 2 << 29 << 1 << 2000 << 3 << 1;
    QTest::newRow( "data4" ) << 2000 << 12 << 31 << 1 << 2001 << 1 << 1;
    QTest::newRow( "data5" ) << 2001 << 2 << 28 << 1 << 2001 << 3 << 1;
    QTest::newRow( "data6" ) << 2001 << 2 << 28 << 30 << 2001 << 3 << 30;
    QTest::newRow( "data7" ) << 2001 << 3 << 30 << 5 << 2001 << 4 << 4;

    QTest::newRow( "data8" ) << 2000 << 1 << 1 << -1 << 1999 << 12 << 31;
    QTest::newRow( "data9" ) << 2000 << 1 << 31 << -1 << 2000 << 1 << 30;
    QTest::newRow( "data10" ) << 2000 << 2 << 28 << -1 << 2000 << 2 << 27;
    QTest::newRow( "data11" ) << 2001 << 2 << 28 << -30 << 2001 << 1 << 29;

}

void tst_QDate::addMonths()
{
    QFETCH( int, year );
    QFETCH( int, month );
    QFETCH( int, day );
    QFETCH( int, amountToAdd );
    QFETCH( int, expectedYear );
    QFETCH( int, expectedMonth );
    QFETCH( int, expectedDay );

    QDate dt( year, month, day );
    dt = dt.addMonths( amountToAdd );

    QCOMPARE( dt.year(), expectedYear );
    QCOMPARE( dt.month(), expectedMonth );
    QCOMPARE( dt.day(), expectedDay );
}

void tst_QDate::addMonths_data()
{
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");
    QTest::addColumn<int>("amountToAdd");
    QTest::addColumn<int>("expectedYear");
    QTest::addColumn<int>("expectedMonth");
    QTest::addColumn<int>("expectedDay");

    QTest::newRow( "data0" ) << 2000 << 1 << 1 << 1 << 2000 << 2 << 1;
    QTest::newRow( "data1" ) << 2000 << 1 << 31 << 1 << 2000 << 2 << 29;
    QTest::newRow( "data2" ) << 2000 << 2 << 28 << 1 << 2000 << 3 << 28;
    QTest::newRow( "data3" ) << 2000 << 2 << 29 << 1 << 2000 << 3 << 29;
    QTest::newRow( "data4" ) << 2000 << 12 << 31 << 1 << 2001 << 1 << 31;
    QTest::newRow( "data5" ) << 2001 << 2 << 28 << 1 << 2001 << 3 << 28;
    QTest::newRow( "data6" ) << 2001 << 2 << 28 << 12 << 2002 << 2 << 28;
    QTest::newRow( "data7" ) << 2000 << 2 << 29 << 12 << 2001 << 2 << 28;
    QTest::newRow( "data8" ) << 2000 << 10 << 15 << 4 << 2001 << 2 << 15;

    QTest::newRow( "data9" ) << 2000 << 1 << 1 << -1 << 1999 << 12 << 1;
    QTest::newRow( "data10" ) << 2000 << 1 << 31 << -1 << 1999 << 12 << 31;
    QTest::newRow( "data11" ) << 2000 << 12 << 31 << -1 << 2000 << 11 << 30;
    QTest::newRow( "data12" ) << 2001 << 2 << 28 << -12 << 2000 << 2 << 28;
    QTest::newRow( "data13" ) << 2000 << 1 << 31 << -7 << 1999 << 6 << 30;
    QTest::newRow( "data14" ) << 2000 << 2 << 29 << -12 << 1999 << 2 << 28;

}

void tst_QDate::addYears()
{
    QFETCH( int, year );
    QFETCH( int, month );
    QFETCH( int, day );
    QFETCH( int, amountToAdd );
    QFETCH( int, expectedYear );
    QFETCH( int, expectedMonth );
    QFETCH( int, expectedDay );

    QDate dt( year, month, day );
    dt = dt.addYears( amountToAdd );

    QCOMPARE( dt.year(), expectedYear );
    QCOMPARE( dt.month(), expectedMonth );
    QCOMPARE( dt.day(), expectedDay );
}

void tst_QDate::addYears_data()
{
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");
    QTest::addColumn<int>("amountToAdd");
    QTest::addColumn<int>("expectedYear");
    QTest::addColumn<int>("expectedMonth");
    QTest::addColumn<int>("expectedDay");

    QTest::newRow( "data0" ) << 2000 << 1 << 1 << 1 << 2001 << 1 << 1;
    QTest::newRow( "data1" ) << 2000 << 1 << 31 << 1 << 2001 << 1 << 31;
    QTest::newRow( "data2" ) << 2000 << 2 << 28 << 1 << 2001 << 2 << 28;
    QTest::newRow( "data3" ) << 2000 << 2 << 29 << 1 << 2001 << 2 << 28;
    QTest::newRow( "data4" ) << 2000 << 12 << 31 << 1 << 2001 << 12 << 31;
    QTest::newRow( "data5" ) << 2001 << 2 << 28 << 3 << 2004 << 2 << 28;
    QTest::newRow( "data6" ) << 2000 << 2 << 29 << 4 << 2004 << 2 << 29;

    QTest::newRow( "data7" ) << 2000 << 1 << 31 << -1 << 1999 << 1 << 31;
    QTest::newRow( "data9" ) << 2000 << 2 << 29 << -1 << 1999 << 2 << 28;
    QTest::newRow( "data10" ) << 2000 << 12 << 31 << -1 << 1999 << 12 << 31;
    QTest::newRow( "data11" ) << 2001 << 2 << 28 << -3 << 1998 << 2 << 28;
    QTest::newRow( "data12" ) << 2000 << 2 << 29 << -4 << 1996 << 2 << 29;
    QTest::newRow( "data13" ) << 2000 << 2 << 29 << -5 << 1995 << 2 << 28;
}

void tst_QDate::operator_eq_eq()
{
    QDate d1(2000,1,2);
    QDate d2(2000,1,2);
    QVERIFY( d1 == d2 );

    d1 = QDate(2001,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 == d2 );

    d2 = QDate(2002,12,5);
    QVERIFY( !(d1 == d2) );
}

void tst_QDate::operator_not_eq()
{
    QDate d1(2000,1,2);
    QDate d2(2000,1,2);
    QVERIFY( !(d1 != d2) );

    d1 = QDate(2001,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 != d2) );

    d2 = QDate(2002,12,5);
    QVERIFY( d1 != d2 );
}

void tst_QDate::operator_lt()
{
    QDate d1(2000,1,2);
    QDate d2(2000,1,2);
    QVERIFY( !(d1 < d2) );

    d1 = QDate(2001,12,4);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 < d2 );

    d1 = QDate(2001,11,5);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 < d2 );

    d1 = QDate(2000,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 < d2 );

    d1 = QDate(2002,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 < d2) );

    d1 = QDate(2001,12,5);
    d2 = QDate(2001,11,5);
    QVERIFY( !(d1 < d2) );

    d1 = QDate(2001,12,6);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 < d2) );
}

void tst_QDate::operator_gt()
{
    QDate d1(2000,1,2);
    QDate d2(2000,1,2);
    QVERIFY( !(d1 > d2) );

    d1 = QDate(2001,12,4);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 > d2) );

    d1 = QDate(2001,11,5);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 > d2) );

    d1 = QDate(2000,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 > d2) );

    d1 = QDate(2002,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 > d2 );

    d1 = QDate(2001,12,5);
    d2 = QDate(2001,11,5);
    QVERIFY( d1 > d2 );

    d1 = QDate(2001,12,6);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 > d2 );
}

void tst_QDate::operator_lt_eq()
{
    QDate d1(2000,1,2);
    QDate d2(2000,1,2);
    QVERIFY( d1 <= d2 );

    d1 = QDate(2001,12,4);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 <= d2 );

    d1 = QDate(2001,11,5);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 <= d2 );

    d1 = QDate(2000,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 <= d2 );

    d1 = QDate(2002,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 <= d2) );

    d1 = QDate(2001,12,5);
    d2 = QDate(2001,11,5);
    QVERIFY( !(d1 <= d2) );

    d1 = QDate(2001,12,6);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 <= d2) );
}

void tst_QDate::operator_gt_eq()
{
    QDate d1(2000,1,2);
    QDate d2(2000,1,2);
    QVERIFY( d1 >= d2 );

    d1 = QDate(2001,12,4);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 >= d2) );

    d1 = QDate(2001,11,5);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 >= d2) );

    d1 = QDate(2000,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( !(d1 >= d2) );

    d1 = QDate(2002,12,5);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 >= d2 );

    d1 = QDate(2001,12,5);
    d2 = QDate(2001,11,5);
    QVERIFY( d1 >= d2 );

    d1 = QDate(2001,12,6);
    d2 = QDate(2001,12,5);
    QVERIFY( d1 >= d2 );
}

void tst_QDate::fromString_data()
{
    // Since we can't define an element of Qt::DateFormat, d1 will be the date
    // expected when we have a TextDate, and d2 will be the date expected when
    // we have an ISODate.

    QTest::addColumn<QString>("str1");
    QTest::addColumn<QString>("str2");
    QTest::addColumn<QDate>("d1");

    QTest::newRow( "data0" ) << QString("Sat May 20 1995") << QString("1995-05-20") << QDate(1995,5,20);
    QTest::newRow( "data1" ) << QString("Tue Dec 17 2002") << QString("2002-12-17") << QDate(2002,12,17);
    QDate d( 1999, 11, 14 );
    QTest::newRow( "data2" ) << d.toString( Qt::TextDate ) << d.toString( Qt::ISODate ) << d;

#if QT_VERSION >= 0x040200
    QTest::newRow( "data3" ) << QString("xxx Jan 1 0999") << QString("0999-01-01") << QDate(999, 1, 1);
    QTest::newRow( "data3b" ) << QString("xxx Jan 1 999") << QString("0999-01-01") << QDate(999, 1, 1);
    QTest::newRow( "data4" ) << QString("xxx Jan 1 12345") << QString() << QDate(12345, 1, 1);
    QTest::newRow( "data5" ) << QString("xxx Jan 1 -0001") << QString() << QDate(-1, 1, 1);
    QTest::newRow( "data6" ) << QString("xxx Jan 1 -4712") << QString() << QDate(-4712, 1, 1);
    QTest::newRow( "data7" ) << QString("xxx Nov 25 -4713") << QString() << QDate(-4713, 11, 25);
#endif
}

void tst_QDate::fromString()
{
    QFETCH( QString, str1 );
    QFETCH( QString, str2 );
    QFETCH( QDate, d1 );

    QCOMPARE( QDate::fromString( str1, Qt::TextDate ), d1 );
    if (!str2.isEmpty())
        QCOMPARE( QDate::fromString( str2, Qt::ISODate ), d1 );
}

void tst_QDate::toString_format_data()
{
    QTest::addColumn<QDate>("t");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QString>("str");

    QTest::newRow( "data0" ) << QDate(1995,5,20) << QString("d-M-yy") << QString("20-5-95");
    QTest::newRow( "data1" ) << QDate(2002,12,17) << QString("dd-MM-yyyy") << QString("17-12-2002");
    QTest::newRow( "data2" ) << QDate(1995,5,20) << QString("M-yy") << QString("5-95");
    QTest::newRow( "data3" ) << QDate(2002,12,17) << QString("dd") << QString("17");
#if (QT_VERSION-0 >= 0x030200)
    QTest::newRow( "data4" ) << QDate() << QString("dd-mm-yyyy") << QString();
#endif
}

void tst_QDate::toString_format()
{
    QFETCH( QDate, t );
    QFETCH( QString, format );
    QFETCH( QString, str );

    QCOMPARE( t.toString( format ), str );
}

void tst_QDate::isLeapYear()
{
    QVERIFY(QDate::isLeapYear(-4444));
    QVERIFY(!QDate::isLeapYear(-4443));
    QVERIFY(QDate::isLeapYear(-8));
    QVERIFY(!QDate::isLeapYear(-7));
    QVERIFY(QDate::isLeapYear(-4));
    QVERIFY(!QDate::isLeapYear(-3));
    QVERIFY(!QDate::isLeapYear(-2));
    QVERIFY(!QDate::isLeapYear(-1));
    QVERIFY(!QDate::isLeapYear(1));
    QVERIFY(!QDate::isLeapYear(1));
    QVERIFY(!QDate::isLeapYear(1));
    QVERIFY(QDate::isLeapYear(4));
    QVERIFY(!QDate::isLeapYear(7));
    QVERIFY(QDate::isLeapYear(8));
    QVERIFY(QDate::isLeapYear(100));
    QVERIFY(QDate::isLeapYear(400));
    QVERIFY(QDate::isLeapYear(700));
    QVERIFY(QDate::isLeapYear(1500));
    QVERIFY(QDate::isLeapYear(1600));
    QVERIFY(!QDate::isLeapYear(1700));
    QVERIFY(!QDate::isLeapYear(1800));
    QVERIFY(!QDate::isLeapYear(1900));
    QVERIFY(QDate::isLeapYear(2000));
    QVERIFY(!QDate::isLeapYear(2100));
    QVERIFY(!QDate::isLeapYear(2200));
    QVERIFY(!QDate::isLeapYear(2300));
    QVERIFY(QDate::isLeapYear(2400));
    QVERIFY(!QDate::isLeapYear(2500));
    QVERIFY(!QDate::isLeapYear(2600));
    QVERIFY(!QDate::isLeapYear(2700));
    QVERIFY(QDate::isLeapYear(2800));

    for (int i = -4713; i <= 10000; ++i) {
        if (i == 0)
            continue;
        QVERIFY(!QDate(i, 2, 29).isValid() == !QDate::isLeapYear(i));
    }
}

void tst_QDate::yearsZeroToNinetyNine()
{
    {
        QDate dt(-1, 2, 3);
        QCOMPARE(dt.year(), -1);
        QCOMPARE(dt.month(), 2);
        QCOMPARE(dt.day(), 3);
    }

    {
        QDate dt(1, 2, 3);
        QCOMPARE(dt.year(), 1);
        QCOMPARE(dt.month(), 2);
        QCOMPARE(dt.day(), 3);
    }

    {
        QDate dt(99, 2, 3);
        QCOMPARE(dt.year(), 99);
        QCOMPARE(dt.month(), 2);
        QCOMPARE(dt.day(), 3);
    }

    QVERIFY(!QDate::isValid(0, 2, 3));
    QVERIFY(QDate::isValid(1, 2, 3));
    QVERIFY(QDate::isValid(-1, 2, 3));

    {
        QDate dt;
        dt.setYMD(1, 2, 3);
        QCOMPARE(dt.year(), 1901);
        QCOMPARE(dt.month(), 2);
        QCOMPARE(dt.day(), 3);
    }

    {
        QDate dt;
        dt.setDate(1, 2, 3);
        QCOMPARE(dt.year(), 1);
        QCOMPARE(dt.month(), 2);
        QCOMPARE(dt.day(), 3);

        dt.setDate(0, 2, 3);
        QVERIFY(!dt.isValid());
    }
}

void tst_QDate::toString()
{
    QDate date(1974,12,1);
    QCOMPARE(date.toString(Qt::SystemLocaleDate),
                QLocale::system().toString(date, QLocale::ShortFormat));
    QCOMPARE(date.toString(Qt::LocaleDate),
                QLocale().toString(date, QLocale::ShortFormat));
    QLocale::setDefault(QLocale::German);
    QCOMPARE(date.toString(Qt::SystemLocaleDate),
                QLocale::system().toString(date, QLocale::ShortFormat));
    QCOMPARE(date.toString(Qt::LocaleDate),
                QLocale().toString(date, QLocale::ShortFormat));
}

QTEST_APPLESS_MAIN(tst_QDate)
#include "tst_qdate.moc"
