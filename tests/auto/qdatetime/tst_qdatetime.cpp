/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#ifndef Q_OS_TEMP
#include <time.h>
#endif

#include <qdatetime.h>
#include <private/qdatetime_p.h>

#ifdef Q_OS_WIN
# include <windows.h>
#endif

//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qdatetime.h corelib/tools/qdatetime.cpp corelib/tools/qdatetime_p.h

class tst_QDateTime : public QObject
{
    Q_OBJECT

public:
    tst_QDateTime();
    virtual ~tst_QDateTime();


    static QString str( int y, int month, int d, int h, int min, int s );
    static QDateTime dt( const QString& str );
public slots:
    void init();
    void cleanup();
private slots:
    void ctor();
    void operator_eq();
    void isNull();
    void isValid();
    void date();
    void time();
    void timeSpec();
    void toTime_t_data();
    void toTime_t();
    void setDate();
    void setTime();
    void setTimeSpec();
    void setTime_t();
    void toString_enumformat();
    void toString_strformat_data();
    void toString_strformat();
    void addDays();
    void addMonths();
    void addMonths_data();
    void addYears();
    void addYears_data();
    void addSecs_data();
    void addSecs();
    void addMSecs_data();
    void addMSecs();
    void toTimeSpec_data();
    void toTimeSpec();
    void toLocalTime_data();
    void toLocalTime();
    void toUTC_data();
    void toUTC();
    void daysTo();
    void secsTo_data();
    void secsTo();
    void operator_eqeq();
    void currentDateTime();
    void fromStringTextDate_data();
    void fromStringTextDate();

    void dateFromStringFormat_data();
    void dateFromStringFormat();
    void timeFromStringFormat_data();
    void timeFromStringFormat();
    void dateTimeFromStringFormat_data();
    void dateTimeFromStringFormat();
private:
    bool europeanTimeZone;
    QDate defDate() const { return QDate(1900, 1, 1); }
    QTime defTime() const { return QTime(0, 0, 0); }
    QDateTime defDateTime() const { return QDateTime(defDate(), defTime()); }
    QDateTime invalidDateTime() const { return QDateTime(invalidDate(), invalidTime()); }
    QDate invalidDate() const { return QDate(); }
    QTime invalidTime() const { return QTime(-1, -1, -1); }
};

Q_DECLARE_METATYPE(QDateTime)
Q_DECLARE_METATYPE(QDate)
Q_DECLARE_METATYPE(QTime)


tst_QDateTime::tst_QDateTime()
{
    uint x1 = QDateTime(QDate(1990, 1, 1), QTime()).toTime_t();
    uint x2 = QDateTime(QDate(1990, 6, 1), QTime()).toTime_t();
    europeanTimeZone = (x1 == 631148400 && x2 == 644191200);
}

tst_QDateTime::~tst_QDateTime()
{

}

void tst_QDateTime::init()
{
#ifdef Q_OS_WIN
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
#endif

}

void tst_QDateTime::cleanup()
{
}

QString tst_QDateTime::str( int y, int month, int d, int h, int min, int s )
{
    return QDateTime( QDate(y, month, d), QTime(h, min, s) ).toString( Qt::ISODate );
}

QDateTime tst_QDateTime::dt( const QString& str )
{
    if ( str == "INVALID" ) {
        return QDateTime();
    } else {
        return QDateTime::fromString( str, Qt::ISODate );
    }
}

void tst_QDateTime::ctor()
{
    QDateTime dt1(QDate(2004, 1, 2), QTime(1, 2, 3));
    QDateTime dt2(QDate(2004, 1, 2), QTime(1, 2, 3), Qt::LocalTime);
    QDateTime dt3(QDate(2004, 1, 2), QTime(1, 2, 3), Qt::UTC);

    QVERIFY(dt1 == dt2);
    QVERIFY(dt1 != dt3);
    if (europeanTimeZone) {
        QVERIFY(dt1 < dt3);
        QVERIFY(dt1.addSecs(3600).toUTC() == dt3);
    }
}

void tst_QDateTime::operator_eq()
{
    QDateTime dt1(QDate(2004, 3, 24), QTime(23, 45, 57), Qt::UTC);
    QDateTime dt2(QDate(2005, 3, 11), QTime(), Qt::UTC);
    dt2 = dt1;
    QVERIFY(dt1 == dt2);
}

void tst_QDateTime::isNull()
{
    QDateTime dt1;
    QVERIFY(dt1.isNull());
    dt1.setDate(QDate());
    QVERIFY(dt1.isNull());
    dt1.setTime(QTime());
    QVERIFY(dt1.isNull());
    dt1.setTimeSpec(Qt::UTC);
    QVERIFY(dt1.isNull());   // maybe it should return false?

    dt1.setDate(QDate(2004, 1, 2));
    QVERIFY(!dt1.isNull());
#if QT_VERSION < 0x040100
    dt1.setDate(QDate());
    QVERIFY(dt1.isNull());
#endif
    dt1.setTime(QTime(12, 34, 56));
    QVERIFY(!dt1.isNull());
    dt1.setTime(QTime());
#if QT_VERSION >= 0x040100
    QVERIFY(!dt1.isNull());
#else
    QVERIFY(dt1.isNull());
#endif
}

void tst_QDateTime::isValid()
{
    QDateTime dt1;
    QVERIFY(!dt1.isValid());
    dt1.setDate(QDate());
    QVERIFY(!dt1.isValid());
    dt1.setTime(QTime());
    QVERIFY(!dt1.isValid());
    dt1.setTimeSpec(Qt::UTC);
    QVERIFY(!dt1.isValid());

    dt1.setDate(QDate(2004, 1, 2));
    QVERIFY(dt1.isValid());
    dt1.setDate(QDate());
    QVERIFY(!dt1.isValid());
    dt1.setTime(QTime(12, 34, 56));
    QVERIFY(!dt1.isValid());
    dt1.setTime(QTime());
    QVERIFY(!dt1.isValid());
}

void tst_QDateTime::date()
{
    QDateTime dt1(QDate(2004, 3, 24), QTime(23, 45, 57), Qt::LocalTime);
    QCOMPARE(dt1.date(), QDate(2004, 3, 24));

    QDateTime dt2(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    QCOMPARE(dt2.date(), QDate(2004, 3, 25));

    QDateTime dt3(QDate(2004, 3, 24), QTime(23, 45, 57), Qt::UTC);
    QCOMPARE(dt3.date(), QDate(2004, 3, 24));

    QDateTime dt4(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::UTC);
    QCOMPARE(dt4.date(), QDate(2004, 3, 25));
}

void tst_QDateTime::time()
{
    QDateTime dt1(QDate(2004, 3, 24), QTime(23, 45, 57), Qt::LocalTime);
    QCOMPARE(dt1.time(), QTime(23, 45, 57));

    QDateTime dt2(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    QCOMPARE(dt2.time(), QTime(0, 45, 57));

    QDateTime dt3(QDate(2004, 3, 24), QTime(23, 45, 57), Qt::UTC);
    QCOMPARE(dt3.time(), QTime(23, 45, 57));

    QDateTime dt4(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::UTC);
    QCOMPARE(dt4.time(), QTime(0, 45, 57));
}

void tst_QDateTime::timeSpec()
{
    QDateTime dt1(QDate(2004, 1, 24), QTime(23, 45, 57));
    QCOMPARE(dt1.timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.addDays(0).timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.addMonths(0).timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.addMonths(6).timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.addYears(0).timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.addSecs(0).timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.addSecs(86400 * 185).timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.toTimeSpec(Qt::LocalTime).timeSpec(), Qt::LocalTime);
    QCOMPARE(dt1.toTimeSpec(Qt::UTC).timeSpec(), Qt::UTC);

    QDateTime dt2(QDate(2004, 1, 24), QTime(23, 45, 57), Qt::LocalTime);
    QCOMPARE(dt2.timeSpec(), Qt::LocalTime);

    QDateTime dt3(QDate(2004, 1, 25), QTime(0, 45, 57), Qt::UTC);
    QCOMPARE(dt3.timeSpec(), Qt::UTC);

    QDateTime dt4 = QDateTime::currentDateTime();
    QCOMPARE(dt4.timeSpec(), Qt::LocalTime);
}

void tst_QDateTime::setDate()
{
    QDateTime dt1(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::UTC);
    dt1.setDate(QDate(2004, 6, 25));
    QCOMPARE(dt1.date(), QDate(2004, 6, 25));
    QCOMPARE(dt1.time(), QTime(0, 45, 57));
    QCOMPARE(dt1.timeSpec(), Qt::UTC);

    QDateTime dt2(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    dt2.setDate(QDate(2004, 6, 25));
    QCOMPARE(dt2.date(), QDate(2004, 6, 25));
    QCOMPARE(dt2.time(), QTime(0, 45, 57));
    QCOMPARE(dt2.timeSpec(), Qt::LocalTime);

    QDateTime dt3(QDate(4004, 3, 25), QTime(0, 45, 57), Qt::UTC);
    dt3.setDate(QDate(4004, 6, 25));
    QCOMPARE(dt3.date(), QDate(4004, 6, 25));
    QCOMPARE(dt3.time(), QTime(0, 45, 57));
    QCOMPARE(dt3.timeSpec(), Qt::UTC);

    QDateTime dt4(QDate(4004, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    dt4.setDate(QDate(4004, 6, 25));
    QCOMPARE(dt4.date(), QDate(4004, 6, 25));
    QCOMPARE(dt4.time(), QTime(0, 45, 57));
    QCOMPARE(dt4.timeSpec(), Qt::LocalTime);

    QDateTime dt5(QDate(1760, 3, 25), QTime(0, 45, 57), Qt::UTC);
    dt5.setDate(QDate(1760, 6, 25));
    QCOMPARE(dt5.date(), QDate(1760, 6, 25));
    QCOMPARE(dt5.time(), QTime(0, 45, 57));
    QCOMPARE(dt5.timeSpec(), Qt::UTC);

    QDateTime dt6(QDate(1760, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    dt6.setDate(QDate(1760, 6, 25));
    QCOMPARE(dt6.date(), QDate(1760, 6, 25));
    QCOMPARE(dt6.time(), QTime(0, 45, 57));
    QCOMPARE(dt6.timeSpec(), Qt::LocalTime);
}

void tst_QDateTime::setTime()
{
    QDateTime dt1(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::UTC);
    dt1.setTime(QTime(23, 11, 22));
    QCOMPARE(dt1.date(), QDate(2004, 3, 25));
    QCOMPARE(dt1.time(), QTime(23, 11, 22));
    QCOMPARE(dt1.timeSpec(), Qt::UTC);

    QDateTime dt2(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    dt2.setTime(QTime(23, 11, 22));
    QCOMPARE(dt2.date(), QDate(2004, 3, 25));
    QCOMPARE(dt2.time(), QTime(23, 11, 22));
    QCOMPARE(dt2.timeSpec(), Qt::LocalTime);

    QDateTime dt3(QDate(4004, 3, 25), QTime(0, 45, 57), Qt::UTC);
    dt3.setTime(QTime(23, 11, 22));
    QCOMPARE(dt3.date(), QDate(4004, 3, 25));
    QCOMPARE(dt3.time(), QTime(23, 11, 22));
    QCOMPARE(dt3.timeSpec(), Qt::UTC);

    QDateTime dt4(QDate(4004, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    dt4.setTime(QTime(23, 11, 22));
    QCOMPARE(dt4.date(), QDate(4004, 3, 25));
    QCOMPARE(dt4.time(), QTime(23, 11, 22));
    QCOMPARE(dt4.timeSpec(), Qt::LocalTime);

    QDateTime dt5(QDate(1760, 3, 25), QTime(0, 45, 57), Qt::UTC);
    dt5.setTime(QTime(23, 11, 22));
    QCOMPARE(dt5.date(), QDate(1760, 3, 25));
    QCOMPARE(dt5.time(), QTime(23, 11, 22));
    QCOMPARE(dt5.timeSpec(), Qt::UTC);

    QDateTime dt6(QDate(1760, 3, 25), QTime(0, 45, 57), Qt::LocalTime);
    dt6.setTime(QTime(23, 11, 22));
    QCOMPARE(dt6.date(), QDate(1760, 3, 25));
    QCOMPARE(dt6.time(), QTime(23, 11, 22));
    QCOMPARE(dt6.timeSpec(), Qt::LocalTime);
}

void tst_QDateTime::setTimeSpec()
{
    QDateTime dt1(QDate(2004, 3, 25), QTime(0, 45, 57), Qt::UTC);
    dt1.setTimeSpec(Qt::UTC);
    QCOMPARE(dt1.date(), QDate(2004, 3, 25));
    QCOMPARE(dt1.time(), QTime(0, 45, 57));
    QCOMPARE(dt1.timeSpec(), Qt::UTC);

    dt1.setTimeSpec(Qt::LocalTime);
    QCOMPARE(dt1.date(), QDate(2004, 3, 25));
    QCOMPARE(dt1.time(), QTime(0, 45, 57));
    QCOMPARE(dt1.timeSpec(), Qt::LocalTime);
}

void tst_QDateTime::setTime_t()
{
    QDateTime dt1;
    dt1.setTime_t(0);
    QCOMPARE(dt1.toUTC(), QDateTime(QDate(1970, 1, 1), QTime(), Qt::UTC));

    dt1.setTimeSpec(Qt::UTC);
    dt1.setTime_t(0);
    QCOMPARE(dt1, QDateTime(QDate(1970, 1, 1), QTime(), Qt::UTC));

    dt1.setTime_t(123456);
    QCOMPARE(dt1, QDateTime(QDate(1970, 1, 2), QTime(10, 17, 36), Qt::UTC));
    if (europeanTimeZone) {
        QDateTime dt2;
        dt2.setTime_t(123456);
        QCOMPARE(dt2, QDateTime(QDate(1970, 1, 2), QTime(11, 17, 36), Qt::LocalTime));
    }

    dt1.setTime_t((uint)(quint32)-123456);
    QCOMPARE(dt1, QDateTime(QDate(2106, 2, 5), QTime(20, 10, 40), Qt::UTC));
    if (europeanTimeZone) {
        QDateTime dt2;
        dt2.setTime_t((uint)(quint32)-123456);
        QCOMPARE(dt2, QDateTime(QDate(2106, 2, 5), QTime(21, 10, 40), Qt::LocalTime));
    }

    dt1.setTime_t(1214567890);
    QCOMPARE(dt1, QDateTime(QDate(2008, 6, 27), QTime(11, 58, 10), Qt::UTC));
    if (europeanTimeZone) {
        QDateTime dt2;
        dt2.setTime_t(1214567890);
        QCOMPARE(dt2, QDateTime(QDate(2008, 6, 27), QTime(13, 58, 10), Qt::LocalTime));
    }

    dt1.setTime_t(0x7FFFFFFF);
    QCOMPARE(dt1, QDateTime(QDate(2038, 1, 19), QTime(3, 14, 7), Qt::UTC));
    if (europeanTimeZone) {
        QDateTime dt2;
        dt2.setTime_t(0x7FFFFFFF);
        QCOMPARE(dt2, QDateTime(QDate(2038, 1, 19), QTime(4, 14, 7), Qt::LocalTime));
    }
}

void tst_QDateTime::toString_enumformat()
{
    QDateTime dt1(QDate(1995, 5, 20), QTime(12, 34, 56));


    QString str1 = dt1.toString(Qt::TextDate);
#ifndef Q_WS_WIN
    QCOMPARE(str1, QString("Sat May 20 12:34:56 1995"));
#else
    QVERIFY(!str1.isEmpty()); // It's locale dependent on Windows
#endif

    QString str2 = dt1.toString(Qt::ISODate);
    QCOMPARE(str2, QString("1995-05-20T12:34:56"));

    QString str3 = dt1.toString(Qt::LocalDate);
    QVERIFY(!str3.isEmpty());
}

void tst_QDateTime::addDays()
{
    for (int pass = 0; pass < 2; ++pass) {
        QDateTime dt(QDate(2004, 1, 1), QTime(12, 34, 56), pass == 0 ? Qt::LocalTime : Qt::UTC);
        dt = dt.addDays(185);
        QVERIFY(dt.date().year() == 2004 && dt.date().month() == 7 && dt.date().day() == 4);
        QVERIFY(dt.time().hour() == 12 && dt.time().minute() == 34 && dt.time().second() == 56
               && dt.time().msec() == 0);
        QCOMPARE(dt.timeSpec(), (pass == 0 ? Qt::LocalTime : Qt::UTC));

        dt = dt.addDays(-185);
        QCOMPARE(dt.date(), QDate(2004, 1, 1));
        QCOMPARE(dt.time(), QTime(12, 34, 56));
    }

    QDateTime dt(QDate(1752, 9, 14), QTime(12, 34, 56));
    while (dt.date().year() < 8000) {
        int year = dt.date().year();
        if (QDate::isLeapYear(year + 1))
            dt = dt.addDays(366);
        else
            dt = dt.addDays(365);
        QCOMPARE(dt.date(), QDate(year + 1, 9, 14));
        QCOMPARE(dt.time(), QTime(12, 34, 56));
    }

    // ### test invalid QDateTime()
}


void tst_QDateTime::addMonths_data()
{
    QTest::addColumn<int>("months");
    QTest::addColumn<QDate>("dt");

    QTest::newRow("-15") << -15 << QDate(2002, 10, 31);
    QTest::newRow("-14") << -14 << QDate(2002, 11, 30);
    QTest::newRow("-13") << -13 << QDate(2002, 12, 31);
    QTest::newRow("-12") << -12 << QDate(2003, 1, 31);

    QTest::newRow("-11") << -11 << QDate(2003, 2, 28);
    QTest::newRow("-10") << -10 << QDate(2003, 3, 31);
    QTest::newRow("-9") << -9 << QDate(2003, 4, 30);
    QTest::newRow("-8") << -8 << QDate(2003, 5, 31);
    QTest::newRow("-7") << -7 << QDate(2003, 6, 30);
    QTest::newRow("-6") << -6 << QDate(2003, 7, 31);
    QTest::newRow("-5") << -5 << QDate(2003, 8, 31);
    QTest::newRow("-4") << -4 << QDate(2003, 9, 30);
    QTest::newRow("-3") << -3 << QDate(2003, 10, 31);
    QTest::newRow("-2") << -2 << QDate(2003, 11, 30);
    QTest::newRow("-1") << -1 << QDate(2003, 12, 31);
    QTest::newRow("0") << 0 << QDate(2004, 1, 31);
    QTest::newRow("1") << 1 << QDate(2004, 2, 29);
    QTest::newRow("2") << 2 << QDate(2004, 3, 31);
    QTest::newRow("3") << 3 << QDate(2004, 4, 30);
    QTest::newRow("4") << 4 << QDate(2004, 5, 31);
    QTest::newRow("5") << 5 << QDate(2004, 6, 30);
    QTest::newRow("6") << 6 << QDate(2004, 7, 31);
    QTest::newRow("7") << 7 << QDate(2004, 8, 31);
    QTest::newRow("8") << 8 << QDate(2004, 9, 30);
    QTest::newRow("9") << 9 << QDate(2004, 10, 31);
    QTest::newRow("10") << 10 << QDate(2004, 11, 30);
    QTest::newRow("11") << 11 << QDate(2004, 12, 31);
    QTest::newRow("12") << 12 << QDate(2005, 1, 31);
    QTest::newRow("13") << 13 << QDate(2005, 2, 28);
    QTest::newRow("14") << 14 << QDate(2005, 3, 31);
    QTest::newRow("15") << 15 << QDate(2005, 4, 30);
}

void tst_QDateTime::addMonths()
{
    QFETCH(QDate, dt);
    QFETCH(int, months);

    QDateTime start(QDate(2004, 1, 31), QTime(12, 34, 56));
    QCOMPARE(start.addMonths(months).date(), dt);
    QCOMPARE(start.addMonths(months).time(), QTime(12, 34, 56));
}

void tst_QDateTime::addYears_data()
{
    QTest::addColumn<int>("years1");
    QTest::addColumn<int>("years2");
    QTest::addColumn<QDate>("start");
    QTest::addColumn<QDate>("dt");

    QTest::newRow("0") << 0 << 0 << QDate(1752, 9, 14) << QDate(1752, 9, 14);
    QTest::newRow("4000 - 4000") << 4000 << -4000 << QDate(1752, 9, 14) << QDate(1752, 9, 14);
    QTest::newRow("10") << 10 << 0 << QDate(1752, 9, 14) << QDate(1762, 9, 14);
    QTest::newRow("0 leap year") << 0 << 0 << QDate(1760, 2, 29) << QDate(1760, 2, 29);
    QTest::newRow("1 leap year") << 1 << 0 << QDate(1760, 2, 29) << QDate(1761, 2, 28);
    QTest::newRow("2 leap year") << 2 << 0 << QDate(1760, 2, 29) << QDate(1762, 2, 28);
    QTest::newRow("3 leap year") << 3 << 0 << QDate(1760, 2, 29) << QDate(1763, 2, 28);
    QTest::newRow("4 leap year") << 4 << 0 << QDate(1760, 2, 29) << QDate(1764, 2, 29);
}

void tst_QDateTime::addYears()
{
    QFETCH(int, years1);
    QFETCH(int, years2);
    QFETCH(QDate, start);
    QFETCH(QDate, dt);

    QDateTime startdt(start, QTime(14, 25, 36));
    QCOMPARE(startdt.addYears(years1).addYears(years2).date(), dt);
    QCOMPARE(startdt.addYears(years1).addYears(years2).time(), QTime(14, 25, 36));
}

void tst_QDateTime::addSecs_data()
{
    QTest::addColumn<QDateTime>("dt");
    QTest::addColumn<int>("nsecs");
    QTest::addColumn<QDateTime>("result");

    QTime standardTime(12, 34, 56);
    QTime daylightTime(13, 34, 56);

    QTest::newRow("utc0") << QDateTime(QDate(2004, 1, 1), standardTime, Qt::UTC) << 86400
                       << QDateTime(QDate(2004, 1, 2), standardTime, Qt::UTC);
    QTest::newRow("utc1") << QDateTime(QDate(2004, 1, 1), standardTime, Qt::UTC) << (86400 * 185)
                       << QDateTime(QDate(2004, 7, 4), standardTime, Qt::UTC);
    QTest::newRow("utc2") << QDateTime(QDate(2004, 1, 1), standardTime, Qt::UTC) << (86400 * 366)
                       << QDateTime(QDate(2005, 1, 1), standardTime, Qt::UTC);
    QTest::newRow("utc3") << QDateTime(QDate(1760, 1, 1), standardTime, Qt::UTC) << 86400
                       << QDateTime(QDate(1760, 1, 2), standardTime, Qt::UTC);
    QTest::newRow("utc4") << QDateTime(QDate(1760, 1, 1), standardTime, Qt::UTC) << (86400 * 185)
                       << QDateTime(QDate(1760, 7, 4), standardTime, Qt::UTC);
    QTest::newRow("utc5") << QDateTime(QDate(1760, 1, 1), standardTime, Qt::UTC) << (86400 * 366)
                       << QDateTime(QDate(1761, 1, 1), standardTime, Qt::UTC);
    QTest::newRow("utc6") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::UTC) << 86400
                       << QDateTime(QDate(4000, 1, 2), standardTime, Qt::UTC);
    QTest::newRow("utc7") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::UTC) << (86400 * 185)
                       << QDateTime(QDate(4000, 7, 4), standardTime, Qt::UTC);
    QTest::newRow("utc8") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::UTC) << (86400 * 366)
                       << QDateTime(QDate(4001, 1, 1), standardTime, Qt::UTC);
    QTest::newRow("utc9") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::UTC) << 0
                       << QDateTime(QDate(4000, 1, 1), standardTime, Qt::UTC);

    if (europeanTimeZone) {
        QTest::newRow("cet0") << QDateTime(QDate(2004, 1, 1), standardTime, Qt::LocalTime) << 86400
                           << QDateTime(QDate(2004, 1, 2), standardTime, Qt::LocalTime);
        QTest::newRow("cet1") << QDateTime(QDate(2004, 1, 1), standardTime, Qt::LocalTime) << (86400 * 185)
                           << QDateTime(QDate(2004, 7, 4), daylightTime, Qt::LocalTime);
        QTest::newRow("cet2") << QDateTime(QDate(2004, 1, 1), standardTime, Qt::LocalTime) << (86400 * 366)
                           << QDateTime(QDate(2005, 1, 1), standardTime, Qt::LocalTime);
        QTest::newRow("cet3") << QDateTime(QDate(1760, 1, 1), standardTime, Qt::LocalTime) << 86400
                           << QDateTime(QDate(1760, 1, 2), standardTime, Qt::LocalTime);
        QTest::newRow("cet4") << QDateTime(QDate(1760, 1, 1), standardTime, Qt::LocalTime) << (86400 * 185)
                           << QDateTime(QDate(1760, 7, 4), standardTime, Qt::LocalTime);
        QTest::newRow("cet5") << QDateTime(QDate(1760, 1, 1), standardTime, Qt::LocalTime) << (86400 * 366)
                           << QDateTime(QDate(1761, 1, 1), standardTime, Qt::LocalTime);
        QTest::newRow("cet6") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::LocalTime) << 86400
                           << QDateTime(QDate(4000, 1, 2), standardTime, Qt::LocalTime);
        QTest::newRow("cet7") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::LocalTime) << (86400 * 185)
                           << QDateTime(QDate(4000, 7, 4), standardTime, Qt::LocalTime);
        QTest::newRow("cet8") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::LocalTime) << (86400 * 366)
                           << QDateTime(QDate(4001, 1, 1), standardTime, Qt::LocalTime);
        QTest::newRow("cet9") << QDateTime(QDate(4000, 1, 1), standardTime, Qt::LocalTime) << 0
                           << QDateTime(QDate(4000, 1, 1), standardTime, Qt::LocalTime);
    }
}

void tst_QDateTime::addSecs()
{
    QFETCH(QDateTime, dt);
    QFETCH(int, nsecs);
    QFETCH(QDateTime, result);

    QCOMPARE(dt.addSecs(nsecs), result);
    QCOMPARE(result.addSecs(-nsecs), dt);
}

void tst_QDateTime::addMSecs_data()
{
    addSecs_data();
}

void tst_QDateTime::addMSecs()
{
    QFETCH(QDateTime, dt);
    QFETCH(int, nsecs);
    QFETCH(QDateTime, result);

    QCOMPARE(dt.addMSecs(qint64(nsecs) * 1000), result);
    QCOMPARE(result.addMSecs(qint64(-nsecs) * 1000), dt);
}

void tst_QDateTime::toTimeSpec_data()
{
    QTest::addColumn<QDateTime>("utc");
    QTest::addColumn<QDateTime>("local");

    QTime utcTime(4, 20, 30);
    QTime localStandardTime(5, 20, 30);
    QTime localDaylightTime(6, 20, 30);

    QTest::newRow("winter1") << QDateTime(QDate(2004, 1, 1), utcTime, Qt::UTC)
                          << QDateTime(QDate(2004, 1, 1), localStandardTime, Qt::LocalTime);
    QTest::newRow("winter2") << QDateTime(QDate(2004, 2, 29), utcTime, Qt::UTC)
                          << QDateTime(QDate(2004, 2, 29), localStandardTime, Qt::LocalTime);
    QTest::newRow("winter3") << QDateTime(QDate(1760, 2, 29), utcTime, Qt::UTC)
                          << QDateTime(QDate(1760, 2, 29), localStandardTime, Qt::LocalTime);
    QTest::newRow("winter4") << QDateTime(QDate(6000, 2, 29), utcTime, Qt::UTC)
                          << QDateTime(QDate(6000, 2, 29), localStandardTime, Qt::LocalTime);
    QTest::newRow("summer1") << QDateTime(QDate(2004, 6, 30), utcTime, Qt::UTC)
                          << QDateTime(QDate(2004, 6, 30), localDaylightTime, Qt::LocalTime);
    QTest::newRow("summer2") << QDateTime(QDate(1760, 6, 30), utcTime, Qt::UTC)
                          << QDateTime(QDate(1760, 6, 30), localStandardTime, Qt::LocalTime);
    QTest::newRow("summer3") << QDateTime(QDate(4000, 6, 30), utcTime, Qt::UTC)
                          << QDateTime(QDate(4000, 6, 30), localStandardTime, Qt::LocalTime);
    QTest::newRow("msec") << QDateTime(QDate(4000, 6, 30), utcTime.addMSecs(1), Qt::UTC)
                       << QDateTime(QDate(4000, 6, 30), localStandardTime.addMSecs(1), Qt::LocalTime);
}

void tst_QDateTime::toTimeSpec()
{
    if (europeanTimeZone) {
        QFETCH(QDateTime, utc);
        QFETCH(QDateTime, local);

        QCOMPARE(utc.toTimeSpec(Qt::UTC), utc);
        QCOMPARE(local.toTimeSpec(Qt::LocalTime), local);
        QCOMPARE(utc.toTimeSpec(Qt::LocalTime), local);
        QCOMPARE(local.toTimeSpec(Qt::UTC), utc);
        QCOMPARE(utc.toTimeSpec(Qt::UTC), local.toTimeSpec(Qt::UTC));
        QCOMPARE(utc.toTimeSpec(Qt::LocalTime), local.toTimeSpec(Qt::LocalTime));
    } else {
        QSKIP("Not tested with timezone other than Central European (CET/CST)", SkipAll);
    }
}

void tst_QDateTime::toLocalTime_data()
{
    toTimeSpec_data();
}

void tst_QDateTime::toLocalTime()
{
    if (europeanTimeZone) {
        QFETCH(QDateTime, utc);
        QFETCH(QDateTime, local);

        QCOMPARE(local.toLocalTime(), local);
        QCOMPARE(utc.toLocalTime(), local);
        QCOMPARE(utc.toLocalTime(), local.toLocalTime());
    } else {
        QSKIP("Not tested with timezone other than Central European (CET/CST)", SkipAll);
    }
}

void tst_QDateTime::toUTC_data()
{
    toTimeSpec_data();
}

void tst_QDateTime::toUTC()
{
    if (europeanTimeZone) {
        QFETCH(QDateTime, utc);
        QFETCH(QDateTime, local);

        QCOMPARE(utc.toUTC(), utc);
        QCOMPARE(local.toUTC(), utc);
        QCOMPARE(utc.toUTC(), local.toUTC());
    } else {
        QSKIP("Not tested with timezone other than Central European (CET/CST)", SkipAll);
    }

    // To make sure bug 72713 never happens again
    QDateTime dt = QDateTime::currentDateTime();
    if(dt.time().msec() == 0){
        dt.setTime(dt.time().addMSecs(1));
    }
    QString s = dt.toString("zzz");
    QString t = dt.toUTC().toString("zzz");
    QCOMPARE(s, t);
}

void tst_QDateTime::daysTo()
{
    QDateTime dt1(QDate(1760, 1, 2), QTime());
    QDateTime dt2(QDate(1760, 2, 2), QTime());
    QDateTime dt3(QDate(1760, 3, 2), QTime());

    QCOMPARE(dt1.daysTo(dt2), 31);
    QCOMPARE(dt1.addDays(31), dt2);

    QCOMPARE(dt2.daysTo(dt3), 29);
    QCOMPARE(dt2.addDays(29), dt3);

    QCOMPARE(dt1.daysTo(dt3), 60);
    QCOMPARE(dt1.addDays(60), dt3);

    QCOMPARE(dt2.daysTo(dt1), -31);
    QCOMPARE(dt2.addDays(-31), dt1);

    QCOMPARE(dt3.daysTo(dt2), -29);
    QCOMPARE(dt3.addDays(-29), dt2);

    QCOMPARE(dt3.daysTo(dt1), -60);
    QCOMPARE(dt3.addDays(-60), dt1);
}

void tst_QDateTime::secsTo_data()
{
    addSecs_data();
}

void tst_QDateTime::secsTo()
{
    QFETCH(QDateTime, dt);
    QFETCH(int, nsecs);
    QFETCH(QDateTime, result);

    QCOMPARE(dt.secsTo(result), nsecs);
    QCOMPARE(result.secsTo(dt), -nsecs);
    QVERIFY((dt == result) == (0 == nsecs));
    QVERIFY((dt != result) == (0 != nsecs));
    QVERIFY((dt < result) == (0 < nsecs));
    QVERIFY((dt <= result) == (0 <= nsecs));
    QVERIFY((dt > result) == (0 > nsecs));
    QVERIFY((dt >= result) == (0 >= nsecs));
}

void tst_QDateTime::currentDateTime()
{
    time_t buf1, buf2;
    ::time(&buf1);
    QDateTime lowerBound;
    lowerBound.setTime_t(buf1);

    QDateTime dt1 = QDateTime::currentDateTime();
    QDateTime dt2 = QDateTime::currentDateTime().toLocalTime();
    QDateTime dt3 = QDateTime::currentDateTime().toUTC();

    ::time(&buf2);
    QDateTime upperBound;
    upperBound.setTime_t(buf2);
    upperBound = upperBound.addSecs(1);

    QVERIFY(lowerBound < upperBound);

    QVERIFY(lowerBound <= dt1);
    QVERIFY(dt1 < upperBound);
    QVERIFY(lowerBound <= dt2);
    QVERIFY(dt2 < upperBound);
    QVERIFY(lowerBound <= dt3);
    QVERIFY(dt3 < upperBound);

    QVERIFY(dt1.timeSpec() == Qt::LocalTime);
    QVERIFY(dt2.timeSpec() == Qt::LocalTime);
    QVERIFY(dt3.timeSpec() == Qt::UTC);
}

void tst_QDateTime::toTime_t_data()
{
    QTest::addColumn<QString>("dateTimeStr");
    QTest::addColumn<bool>("res");

    QTest::newRow( "data1" ) << str( 1800, 1, 1, 12, 0, 0 ) << FALSE;
    QTest::newRow( "data2" ) << str( 1969, 1, 1, 12, 0, 0 ) << FALSE;
    QTest::newRow( "data3" ) << str( 2002, 1, 1, 12, 0, 0 ) << TRUE;
    QTest::newRow( "data4" ) << str( 2002, 6, 1, 12, 0, 0 ) << TRUE;
    QTest::newRow( "data5" ) << QString("INVALID") << FALSE;
    QTest::newRow( "data6" ) << str( 2038, 1, 1, 12, 0, 0 ) << TRUE;
#if QT_VERSION - 0 >= 0x40200
    QTest::newRow( "data7" ) << str( 2063, 4, 5, 12, 0, 0 ) << TRUE; // the day of First Contact
#endif
    QTest::newRow( "data8" ) << str( 2107, 1, 1, 12, 0, 0 )
                          << bool( sizeof(uint) > 32 && sizeof(time_t) > 32 );
}

void tst_QDateTime::toTime_t()
{
#if QT_VERSION >= 0x030100
    QFETCH( QString, dateTimeStr );
    QDateTime datetime = dt( dateTimeStr );

    uint asTime_t = datetime.toTime_t();
    QFETCH( bool, res );
    if (res) {
        QVERIFY( asTime_t != (uint)-1 );
    } else {
        QVERIFY( asTime_t == (uint)-1 );
    }

    if ( asTime_t != (uint) -1 ) {
#if QT_VERSION < 0x040200
        QDateTime datetime2;
        datetime2.setTime_t( asTime_t );
#else
        QDateTime datetime2 = QDateTime::fromTime_t( asTime_t );
#endif
        QCOMPARE(datetime, datetime2);
    }
#else
    QSKIP( "Not tested with Qt versions < 3.1", SkipAll);
#endif
}

void tst_QDateTime::operator_eqeq()
{
    QDateTime dt1(QDate(2004, 1, 2), QTime(2, 2, 3), Qt::LocalTime);
    QDateTime dt2(QDate(2004, 1, 2), QTime(1, 2, 3), Qt::UTC);

    QVERIFY(dt1 == dt1);
    QVERIFY(!(dt1 != dt1));

    QVERIFY(dt2 == dt2);
    QVERIFY(!(dt2 != dt2));

    QVERIFY(dt1 != dt2);
    QVERIFY(!(dt1 == dt2));

    QVERIFY(dt1 != QDateTime::currentDateTime());
    QVERIFY(dt2 != QDateTime::currentDateTime());

    QVERIFY(dt1.toUTC() == dt1.toUTC());

    if (europeanTimeZone) {
        QVERIFY(dt1.toUTC() == dt2);
        QVERIFY(dt1 == dt2.toLocalTime());
    }
}

void tst_QDateTime::toString_strformat_data()
{
    QTest::addColumn<QDateTime>("dt");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QString>("str");

    QTest::newRow( "datetime0" ) << QDateTime() << QString("dd-MM-yyyy hh:mm:ss") << QString();
    QTest::newRow( "datetime1" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("dd-'mmddyy'MM-yyyy hh:mm:ss.zzz")
                                 << QString("31-mmddyy12-1999 23:59:59.999");
    QTest::newRow( "datetime2" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("dd-'apAP'MM-yyyy hh:mm:ss.zzz")
                                 << QString("31-apAP12-1999 23:59:59.999");
    QTest::newRow( "datetime3" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("Apdd-MM-yyyy hh:mm:ss.zzz")
                                 << QString("PMp31-12-1999 11:59:59.999");
    QTest::newRow( "datetime4" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("'ap'apdd-MM-yyyy 'AP'hh:mm:ss.zzz")
                                 << QString("appm31-12-1999 AP11:59:59.999");
    QTest::newRow( "datetime5" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("'''") << QString();
    QTest::newRow( "datetime6" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("'ap") << QString("ap");
    QTest::newRow( "datetime7" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("' ' 'hh' hh") << QString("  hh 23");
    QTest::newRow( "datetime8" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("d'foobar'") << QString("31foobar");
    QTest::newRow( "datetime9" ) << QDateTime(QDate(1999, 12, 31), QTime(3, 59, 59, 999))
                                 << QString("hhhhh") << QString("03033");
    QTest::newRow( "datetime10" ) << QDateTime(QDate(1999, 12, 31), QTime(3, 59, 59, 999))
                                 << QString("hhhhhaA") << QString("03033amAM");
    QTest::newRow( "datetime11" ) << QDateTime(QDate(1999, 12, 31), QTime(23, 59, 59, 999))
                                 << QString("HHHhhhAaAPap") << QString("23231111PMpmPMpm");
    QTest::newRow( "datetime12" ) << QDateTime(QDate(1999, 12, 31), QTime(3, 59, 59, 999))
                                 << QString("HHHhhhAaAPap") << QString("033033AMamAMam");
}

void tst_QDateTime::toString_strformat()
{
#if (QT_VERSION-0 >= 0x030200)
    QFETCH( QDateTime, dt );
    QFETCH( QString, format );
    QFETCH( QString, str );
    QCOMPARE( dt.toString( format ), str );
#else
    QSKIP( "No test implemented for < 3.2 yet", SkipAll);
#endif

}

void tst_QDateTime::fromStringTextDate_data()
{
    QTest::addColumn<QString>("dateTime");
    QTest::addColumn<int>("dateFormat");
    QTest::addColumn<int>("day");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("hour");
    QTest::addColumn<int>("minute");
    QTest::addColumn<int>("second");
    QTest::addColumn<int>("msec");
    QTest::addColumn<int>("timeSpec");

    QTest::newRow("task27910") << QString("Tue Jun 17 08:00:10 2003")
                            << int(Qt::TextDate)
                            << 17 << 6 << 2003 << 8 << 0 << 10 << 0
                            << int(Qt::LocalTime);
    QTest::newRow("task77042") << QString("2005-06-28T07:57:30.0010000000Z")
                            << int(Qt::ISODate)
                            << 28 << 6 << 2005 << 7 << 57 << 30 << 1
                            << int(Qt::UTC);

    QTest::newRow("task77042-2") << QString("2005-06-28T07:57:30,0040000000Z")
                              << int(Qt::ISODate)
                              << 28 << 6 << 2005 << 7 << 57 << 30 << 4
                              << int(Qt::UTC);

    QTest::newRow("task77042-3") << QString("2005-06-28T07:57:30,0015Z")
                              << int(Qt::ISODate)
                              << 28 << 6 << 2005 << 7 << 57 << 30 << 2
                              << int(Qt::UTC);

    QTest::newRow("task77042-4") << QString("2005-06-28T07:57:30,0014Z")
                              << int(Qt::ISODate)
                              << 28 << 6 << 2005 << 7 << 57 << 30 << 1
                              << int(Qt::UTC);

    QTest::newRow("task77042-5") << QString("2005-06-28T07:57:30,1Z")
                              << int(Qt::ISODate)
                              << 28 << 6 << 2005 << 7 << 57 << 30 << 100
                              << int(Qt::UTC);

    QTest::newRow("task77042-6") << QString("2005-06-28T07:57:30,11")
                              << int(Qt::ISODate)
                              << 28 << 6 << 2005 << 7 << 57 << 30 << 110
                              << int(Qt::LocalTime);

#if QT_VERSION >= 0x040200
    QTest::newRow("Year 0999") << QString("Tue Jun 17 08:00:10 0999")
                            << int(Qt::TextDate)
                            << 17 << 6 << 999 << 8 << 0 << 10 << 0
                            << int(Qt::LocalTime);

    QTest::newRow("Year 999") << QString("Tue Jun 17 08:00:10 999")
                            << int(Qt::TextDate)
                            << 17 << 6 << 999 << 8 << 0 << 10 << 0
                            << int(Qt::LocalTime);

    QTest::newRow("Year 12345") << QString("Tue Jun 17 08:00:10 12345")
                            << int(Qt::TextDate)
                            << 17 << 6 << 12345 << 8 << 0 << 10 << 0
                            << int(Qt::LocalTime);

    QTest::newRow("Year -4712") << QString("Tue Jan 1 00:01:02 -4712")
                            << int(Qt::TextDate)
                            << 1 << 1 << -4712 << 0 << 01 << 02 << 0
                            << int(Qt::LocalTime);
#endif
}

void tst_QDateTime::fromStringTextDate()
{
    QFETCH(QString, dateTime);
    QFETCH(int, dateFormat);
    QFETCH(int, day);
    QFETCH(int, month);
    QFETCH(int, year);
    QFETCH(int, hour);
    QFETCH(int, minute);
    QFETCH(int, second);
    QFETCH(int, msec);
    QFETCH(int, timeSpec);

    QDateTime dt = QDateTime::fromString(dateTime, (Qt::DateFormat)dateFormat);
    QCOMPARE(dt.date().day(), day);
    QCOMPARE(dt.date().month(), month);
    QCOMPARE(dt.date().year(), year);
    QCOMPARE(dt.time().hour(), hour);
    QCOMPARE(dt.time().minute(), minute);
    QCOMPARE(dt.time().second(), second);
    QCOMPARE(dt.time().msec(), msec);
    QCOMPARE(int(dt.timeSpec()), timeSpec);
}

void tst_QDateTime::dateFromStringFormat_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QDate>("expected");

    //get localized names
    QString january = QDate::longMonthName(1);
    QString february = QDate::longMonthName(2);
    QString august = QDate::longMonthName(8);
    QString mon = QDate::shortDayName(1);

    QTest::newRow("data0") << QString("") << QString("") << defDate();
    QTest::newRow("data1") << QString(" ") << QString("") << invalidDate();
    QTest::newRow("data2") << QString(" ") << QString(" ") << defDate();
    QTest::newRow("data3") << QString("-%$%#") << QString("$*(#@") << invalidDate();
    QTest::newRow("data4") << QString("d") << QString("'d'") << defDate();
    QTest::newRow("data5") << QString("101010") << QString("dMyy") << QDate(1910, 10, 10);
    QTest::newRow("data6") << QString("101010b") << QString("dMyy") << invalidDate();
    QTest::newRow("data7") << january << QString("MMMM") << defDate();
    QTest::newRow("data8") << QString("ball") << QString("balle") << invalidDate();
    QTest::newRow("data9") << QString("balleh") << QString("balleh") << defDate();
    QTest::newRow("data10") << QString("10.01.1") << QString("M.dd.d") << QDate(defDate().year(), 10, 1);
    QTest::newRow("data11") << QString("-1.01.1") << QString("M.dd.d") << invalidDate();
    QTest::newRow("data12") << QString("11010") << QString("dMMyy") << invalidDate();
    QTest::newRow("data13") << QString("-2") << QString("d") << invalidDate();
    QTest::newRow("data14") << QString("132") << QString("Md") << invalidDate();
    QTest::newRow("data15") << february << QString("MMMM") << QDate(defDate().year(), 2, 1);
    QString date = mon + " " + august + " 8 2005";
    QTest::newRow("data16") << 	date << QString("ddd MMMM d yyyy")		<< QDate(2005, 8, 8);
    QTest::newRow("data17") << QString("2000:00") << QString("yyyy:yy") << QDate(2000, 1, 1);
    QTest::newRow("data18") << QString("1999:99") << QString("yyyy:yy") << QDate(1999, 1, 1);
    QTest::newRow("data19") << QString("2099:99") << QString("yyyy:yy") << QDate(2099, 1, 1);
    QTest::newRow("data20") << QString("2001:01") << QString("yyyy:yy") << QDate(2001, 1, 1);
    QTest::newRow("data21") << QString("99") << QString("yy") << QDate(1999, 1, 1);
    QTest::newRow("data22") << QString("01") << QString("yy") << QDate(1901, 1, 1);

}


void tst_QDateTime::dateFromStringFormat()
{
    QFETCH(QString, string);
    QFETCH(QString, format);
    QFETCH(QDate, expected);

    QDate dt = QDate::fromString(string, format);
    QCOMPARE(dt, expected);
}

void tst_QDateTime::timeFromStringFormat_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QTime>("expected");

    QTest::newRow("data0") << QString("1010") << QString("mmm") << QTime(0, 10, 0);
    QTest::newRow("data1") << QString("00") << QString("hm") << invalidTime();
    QTest::newRow("data2") << QString("10am") << QString("hap") << QTime(10, 0, 0);
    QTest::newRow("data3") << QString("10pm") << QString("hap") << QTime(22, 0, 0);
    QTest::newRow("data4") << QString("10pmam") << QString("hapap") << invalidTime();
    QTest::newRow("data5") << QString("1070") << QString("hhm") << invalidTime();
    QTest::newRow("data6") << QString("1011") << QString("hh") << invalidTime();
    QTest::newRow("data7") << QString("25") << QString("hh") << invalidTime();
    QTest::newRow("data8") << QString("22pm") << QString("Hap") << QTime(22, 0, 0);
    QTest::newRow("data9") << QString("2221") << QString("hhhh") << invalidTime();
}


void tst_QDateTime::timeFromStringFormat()
{
    QFETCH(QString, string);
    QFETCH(QString, format);
    QFETCH(QTime, expected);

    QTime dt = QTime::fromString(string, format);
    QCOMPARE(dt, expected);
}


void tst_QDateTime::dateTimeFromStringFormat_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QDateTime>("expected");

	QString january = QDate::longMonthName(1);
	QString oct = QDate::shortMonthName(10);
	QString december = QDate::longMonthName(12);
	QString thu = QDate::shortDayName(4);
	QString fri = QDate::shortDayName(5);
	QString date = "10 " + oct + " 10";

    QTest::newRow("data0") << QString("101010") << QString("dMyy") << QDateTime(QDate(1910, 10, 10), QTime());
    QTest::newRow("data1") << QString("1020") << QString("sss") << invalidDateTime();
    QTest::newRow("data2") << QString("1010") << QString("sss") << QDateTime(defDate(), QTime(0, 0, 10));
    QTest::newRow("data3") << QString("10hello20") << QString("ss'hello'ss") << invalidDateTime();
    QTest::newRow("data4") << QString("10") << QString("''") << invalidDateTime();
    QTest::newRow("data5") << QString("10") << QString("'") << invalidDateTime();
    QTest::newRow("data6") << QString("pm") << QString("ap") << QDateTime(defDate(), QTime(12, 0, 0));
    QTest::newRow("data7") << QString("foo") << QString("ap") << invalidDateTime();
    QTest::newRow("data8") << QString("101010") << QString("dMyy") << QDateTime(QDate(1910, 10, 10), QTime());
    QTest::newRow("data9") << QString("101010") << QString("dMyy") << QDateTime(QDate(1910, 10, 10), QTime());
    QTest::newRow("data10") << QString("101010") << QString("dMyy") << QDateTime(QDate(1910, 10, 10), QTime());
    QTest::newRow("data11") << date << QString("dd MMM yy") << QDateTime(QDate(1910, 10, 10), QTime());
    date = fri + " " + december + " 3 2004";
	QTest::newRow("data12") << date << QString("ddd MMMM d yyyy") << QDateTime(QDate(2004, 12, 3), QTime());
    QTest::newRow("data13") << QString("30.02.2004") << QString("dd.MM.yyyy") << invalidDateTime();
    QTest::newRow("data14") << QString("32.01.2004") << QString("dd.MM.yyyy") << invalidDateTime();
    date = thu + " " + january + " 2004";
	QTest::newRow("data15") << date << QString("ddd MMMM yyyy") << QDateTime(QDate(2004, 1, 1), QTime());
    QTest::newRow("data16") << QString("2005-06-28T07:57:30.001Z")
                         << QString("yyyy-MM-ddThh:mm:ss.zZ")
                         << QDateTime(QDate(2005, 06, 28), QTime(07, 57, 30, 1));

}

void tst_QDateTime::dateTimeFromStringFormat()
{
    QFETCH(QString, string);
    QFETCH(QString, format);
    QFETCH(QDateTime, expected);

    QDateTime dt = QDateTime::fromString(string, format);

    QCOMPARE(dt, expected);
}


QTEST_APPLESS_MAIN(tst_QDateTime)
#include "tst_qdatetime.moc"
