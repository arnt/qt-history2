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
//TESTED_FILES=xml/qdom.h xml/qdom.cpp

class tst_QDoubleValidator : public QObject
{
    Q_OBJECT

public:
    tst_QDoubleValidator();
    virtual ~tst_QDoubleValidator();


    // I can think of no other way to do this for the moment
    enum State { Invalid=0, Intermediate=1, Acceptable=2 };
public slots:
    void init();
    void cleanup();
private slots:
    void validate_data();
    void validate();
};

tst_QDoubleValidator::tst_QDoubleValidator()
{
}

tst_QDoubleValidator::~tst_QDoubleValidator()
{

}

void tst_QDoubleValidator::init()
{
}

void tst_QDoubleValidator::cleanup()
{
}

void tst_QDoubleValidator::validate_data()
{
    QTest::addColumn<double>("minimum");
    QTest::addColumn<double>("maximum");
    QTest::addColumn<int>("decimals");
    QTest::addColumn<QString>("value");
    QTest::addColumn<int>("scientific_state");
    QTest::addColumn<int>("standard_state");

    // Invalid=0, Intermediate=1, Acceptable=2

    QTest::newRow( "data0" ) << 0.0 << 100.0 << 1 << QString("50.0") << 2 << 2;
    QTest::newRow( "data1" ) << 0.0 << 100.0 << 1 << QString("500.0") << 1 << 0;
    QTest::newRow( "data2" ) << 0.0 << 100.0 << 1 << QString("-35.0") << 0 << 0;
    QTest::newRow( "data3" ) << 0.0 << 100.0 << 1 << QString("a") << 0 << 0;
    QTest::newRow( "data4" ) << 0.0 << 100.0 << 1 << QString("-") << 0 << 0;
    QTest::newRow( "data5" ) << 0.0 << 100.0 << 1 << QString("100.0") << 2 << 2;
    QTest::newRow( "data6" ) << -100.0 << 100.0 << 1 << QString("-") << 1 << 1;
    QTest::newRow( "data7" ) << -100.0 << 100.0 << 1 << QString("-500.0") << 1 << 0;
    QTest::newRow( "data8" ) << -100.0 << 100.0 << 1 << QString("-100") << 2 << 2;
    QTest::newRow( "data9" ) << -100.0 << -10.0 << 1 << QString("10") << 1 << 0;
    QTest::newRow( "data10" ) << 0.3 << 0.5 << 5 << QString("0.34567") << 2 << 2;
    QTest::newRow( "data11" ) << -0.3 << -0.5 << 5 << QString("-0.345678") << 1 << 0;
    QTest::newRow( "data12" ) << -0.32 << 0.32 << 1 << QString("0") << 2 << 2;
    QTest::newRow( "data13" ) << 0.0 << 100.0 << 1 << QString("3456a") << 0 << 0;
    QTest::newRow( "data14" ) << -100.0 << 100.0 << 1 << QString("-3456a") << 0 << 0;
    QTest::newRow( "data15" ) << -100.0 << 100.0 << 1 << QString("a-3456") << 0 << 0;
    QTest::newRow( "data16" ) << -100.0 << 100.0 << 1 << QString("a-3456a") << 0 << 0;
    QTest::newRow( "data17" ) << 1229.0 << 1231.0 << 0 << QString("123e") << 1 << 0;
    QTest::newRow( "data18" ) << 1229.0 << 1231.0 << 0 << QString("123e+") << 1 << 0;
    QTest::newRow( "data19" ) << 1229.0 << 1231.0 << 0 << QString("123e+1") << 2 << 0;
    QTest::newRow( "data20" ) << 12290.0 << 12310.0 << 0 << QString("123e+2") << 2 << 0;
    QTest::newRow( "data21" ) << 12.290 << 12.310 << 2 << QString("123e-") << 1 << 0;
    QTest::newRow( "data22" ) << 12.290 << 12.310 << 2 << QString("123e-1") << 2 << 0;
    QTest::newRow( "data23" ) << 1.2290 << 1.2310 << 3 << QString("123e-2") << 2 << 0;
    QTest::newRow( "data24" ) << 1229.0 << 1231.0 << 0 << QString("123E") << 1 << 0;
    QTest::newRow( "data25" ) << 1229.0 << 1231.0 << 0 << QString("123E+") << 1 << 0;
    QTest::newRow( "data26" ) << 1229.0 << 1231.0 << 0 << QString("123E+1") << 2 << 0;
    QTest::newRow( "data27" ) << 12290.0 << 12310.0 << 0 << QString("123E+2") << 2 << 0;
    QTest::newRow( "data28" ) << 12.290 << 12.310 << 2 << QString("123E-") << 1 << 0;
    QTest::newRow( "data29" ) << 12.290 << 12.310 << 2 << QString("123E-1") << 2 << 0;
    QTest::newRow( "data30" ) << 1.2290 << 1.2310 << 3 << QString("123E-2") << 2 << 0;
    QTest::newRow( "data31" ) << 1.2290 << 1.2310 << 3 << QString("e") << 1 << 0;
    QTest::newRow( "data32" ) << 1.2290 << 1.2310 << 3 << QString("e+") << 1 << 0;
    QTest::newRow( "data33" ) << 1.2290 << 1.2310 << 3 << QString("e+1") << 1 << 0;
    QTest::newRow( "data34" ) << 1.2290 << 1.2310 << 3 << QString("e-") << 1 << 0;
    QTest::newRow( "data35" ) << 1.2290 << 1.2310 << 3 << QString("e-1") << 1 << 0;
    QTest::newRow( "data36" ) << 1.2290 << 1.2310 << 3 << QString("E") << 1 << 0;
    QTest::newRow( "data37" ) << 1.2290 << 1.2310 << 3 << QString("E+") << 1 << 0;
    QTest::newRow( "data38" ) << 1.2290 << 1.2310 << 3 << QString("E+1") << 1 << 0;
    QTest::newRow( "data39" ) << 1.2290 << 1.2310 << 3 << QString("E-") << 1 << 0;
    QTest::newRow( "data40" ) << 1.2290 << 1.2310 << 3 << QString("E-1") << 1 << 0;
    QTest::newRow( "data41" ) << -100.0 << 100.0 << 0 << QString("10e") << 1 << 0;
    QTest::newRow( "data42" ) << -100.0 << 100.0 << 0 << QString("10e+") << 1 << 0;
}

void tst_QDoubleValidator::validate()
{
    QFETCH( double, minimum );
    QFETCH( double, maximum );
    QFETCH( int, decimals );
    QFETCH( QString, value );
    QFETCH( int, scientific_state );
    QFETCH( int, standard_state );

    QDoubleValidator iv( minimum, maximum, decimals, 0 );
    int dummy;
    QCOMPARE((int)iv.validate( value, dummy ), scientific_state);
    iv.setNotation(QDoubleValidator::StandardNotation);
    QCOMPARE((int)iv.validate(value, dummy), standard_state);
}

QTEST_APPLESS_MAIN(tst_QDoubleValidator)
#include "tst_qdoublevalidator.moc"
