/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qvalidator.h>

//TESTED_CLASS=
//TESTED_FILES=gui/image/qimage.h gui/image/qimage.cpp

class tst_QIntValidator : public QObject
{
    Q_OBJECT

public:
    tst_QIntValidator();
    virtual ~tst_QIntValidator();


    // I can think of no other way to do this for the moment
    enum State { Invalid=0, Intermediate=1, Acceptable=2 };
public slots:
    void init();
    void cleanup();
private slots:
    void validate_data();
    void validate();
};

tst_QIntValidator::tst_QIntValidator()
{
}

tst_QIntValidator::~tst_QIntValidator()
{

}

void tst_QIntValidator::init()
{
}

void tst_QIntValidator::cleanup()
{
}

void tst_QIntValidator::validate_data()
{
    QTest::addColumn<int>("minimum");
    QTest::addColumn<int>("maximum");
    QTest::addColumn<QString>("value");
    QTest::addColumn<int>("state");

    const int Invalid = 0;
    const int Intermediate = 1;
    const int Acceptable = 2;

    QTest::newRow( "data0" ) << 0 << 100 << QString("50") << Acceptable;
    QTest::newRow( "data1" ) << 0 << 100 << QString("500") << Invalid;
    QTest::newRow( "data2" ) << 0 << 100 << QString("-35") << Invalid;
    QTest::newRow( "data3" ) << 0 << 100 << QString("a") << Invalid;
    QTest::newRow( "data4" ) << 0 << 100 << QString("-") << Invalid;
    QTest::newRow( "data5" ) << 0 << 100 << QString("100") << Acceptable;
    QTest::newRow( "data6" ) << -100 << 100 << QString("-") << Intermediate;
    QTest::newRow( "data7" ) << -100 << 100 << QString("-500") << Invalid;
    QTest::newRow( "data8" ) << -100 << 100 << QString("-100") << Acceptable;
    QTest::newRow( "data9" ) << -100 << -10 << QString("10") << Invalid;

    QTest::newRow( "data10" ) << 100 << 999 << QString("") << Intermediate;
    QTest::newRow( "data11" ) << 100 << 999 << QString("5") << Intermediate;
    QTest::newRow( "data12" ) << 100 << 999 << QString("50") << Intermediate;
    QTest::newRow( "data13" ) << 100 << 999 << QString("99") << Intermediate;
    QTest::newRow( "data14" ) << 100 << 999 << QString("100") << Acceptable;
    QTest::newRow( "data15" ) << 100 << 999 << QString("101") << Acceptable;
    QTest::newRow( "data16" ) << 100 << 999 << QString("998") << Acceptable;
    QTest::newRow( "data17" ) << 100 << 999 << QString("999") << Acceptable;
    QTest::newRow( "data18" ) << 100 << 999 << QString("1000") << Invalid;
    QTest::newRow( "data19" ) << 100 << 999 << QString("-10") << Invalid;

    QTest::newRow( "data20" ) << -999 << -100 << QString("50") << Invalid;
    QTest::newRow( "data21" ) << -999 << -100 << QString("-") << Intermediate;
    QTest::newRow( "data22" ) << -999 << -100 << QString("-1") << Intermediate;
    QTest::newRow( "data23" ) << -999 << -100 << QString("-10") << Intermediate;
    QTest::newRow( "data24" ) << -999 << -100 << QString("-100") << Acceptable;
    QTest::newRow( "data25" ) << -999 << -100 << QString("-500") << Acceptable;
    QTest::newRow( "data26" ) << -999 << -100 << QString("-998") << Acceptable;
    QTest::newRow( "data27" ) << -999 << -100 << QString("-999") << Acceptable;
    QTest::newRow( "data28" ) << -999 << -100 << QString("-1000") << Invalid;
    QTest::newRow( "data29" ) << -999 << -100 << QString("-2000") << Invalid;
}

void tst_QIntValidator::validate()
{
    QFETCH( int, minimum );
    QFETCH( int, maximum );
    QFETCH( QString, value );
    QFETCH( int, state );

    QIntValidator iv( minimum, maximum, 0 );
    int dummy;
    QCOMPARE( (int)iv.validate( value, dummy ), state );
}

QTEST_MAIN(tst_QIntValidator)
#include "tst_qintvalidator.moc"
