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
    void validateThouSep_data();
    void validateThouSep();
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

void tst_QDoubleValidator::validateThouSep_data()
{
    QTest::addColumn<QString>("localeName");
    QTest::addColumn<QString>("value");
    QTest::addColumn<int>("result");

    QTest::newRow("1,000") << "C" << QString("1,000") << (int)QValidator::Acceptable;
    QTest::newRow("1,000.0") << "C" << QString("1,000.0") << (int)QValidator::Acceptable;
    QTest::newRow("1,000E") << "C" << QString("1,000E") << (int)QValidator::Intermediate;
    QTest::newRow("1,00") << "C" << QString("1,00") << (int)QValidator::Intermediate;
    QTest::newRow("1,00,") << "C" << QString("1,00,") << (int)QValidator::Invalid;
    QTest::newRow("1,00.") << "C" << QString("1,00.") << (int)QValidator::Invalid;
    QTest::newRow("1,00E") << "C" << QString("1,00E") << (int)QValidator::Invalid;

    QTest::newRow("1.000") << "de" << QString("1.000") << (int)QValidator::Acceptable;
    QTest::newRow("1.000,0") << "de" << QString("1.000,0") << (int)QValidator::Acceptable;
    QTest::newRow("1.000E") << "de" << QString("1.000E") << (int)QValidator::Intermediate;
    QTest::newRow("1.00") << "de" << QString("1.00") << (int)QValidator::Intermediate;
    QTest::newRow("1.00.") << "de" << QString("1.00.") << (int)QValidator::Invalid;
    QTest::newRow("1.00,") << "de" << QString("1.00,") << (int)QValidator::Invalid;
    QTest::newRow("1.00E") << "de" << QString("1.00E") << (int)QValidator::Invalid;
}

void tst_QDoubleValidator::validateThouSep()
{
    QFETCH( QString, localeName);
    QFETCH( QString, value );
    QFETCH( int, result );
    int dummy = 0;
    
    QDoubleValidator iv( -10000, 10000, 2, 0);
    iv.setNotation(QDoubleValidator::ScientificNotation);
    iv.setLocale(QLocale(localeName));

    QCOMPARE((int)iv.validate(value, dummy), result);
}

void tst_QDoubleValidator::validate_data()
{
    QTest::addColumn<QString>("localeName");
    QTest::addColumn<double>("minimum");
    QTest::addColumn<double>("maximum");
    QTest::addColumn<int>("decimals");
    QTest::addColumn<QString>("value");
    QTest::addColumn<int>("scientific_state");
    QTest::addColumn<int>("standard_state");

    // Invalid=0, Intermediate=1, Acceptable=2

    QTest::newRow( "data0" )  << "C" << 0.0 << 100.0 << 1 << QString("50.0") << 2 << 2;
    QTest::newRow( "data1" )  << "C" << 00.0 << 100.0 << 1 << QString("500.0") << 1 << 0;
    QTest::newRow( "data2" )  << "C" << 00.0 << 100.0 << 1 << QString("-35.0") << 0 << 0;
    QTest::newRow( "data3" )  << "C" << 00.0 << 100.0 << 1 << QString("a") << 0 << 0;
    QTest::newRow( "data4" )  << "C" << 0.0 << 100.0 << 1 << QString("-") << 0 << 0;
    QTest::newRow( "data5" )  << "C" << 0.0 << 100.0 << 1 << QString("100.0") << 2 << 2;
    QTest::newRow( "data6" )  << "C" << -100.0 << 100.0 << 1 << QString("-") << 1 << 1;
    QTest::newRow( "data7" )  << "C" << -100.0 << 100.0 << 1 << QString("-500.0") << 1 << 0;
    QTest::newRow( "data8" )  << "C" << -100.0 << 100.0 << 1 << QString("-100") << 2 << 2;
    QTest::newRow( "data9" )  << "C" << -100.0 << -10.0 << 1 << QString("10") << 1 << 0;
    QTest::newRow( "data10" ) << "C" << 0.3 << 0.5 << 5 << QString("0.34567") << 2 << 2;
    QTest::newRow( "data11" ) << "C" << -0.3 << -0.5 << 5 << QString("-0.345678") << 1 << 0;
    QTest::newRow( "data12" ) << "C" << -0.32 << 0.32 << 1 << QString("0") << 2 << 2;
    QTest::newRow( "data13" ) << "C" << 0.0 << 100.0 << 1 << QString("3456a") << 0 << 0;
    QTest::newRow( "data14" ) << "C" << -100.0 << 100.0 << 1 << QString("-3456a") << 0 << 0;
    QTest::newRow( "data15" ) << "C" << -100.0 << 100.0 << 1 << QString("a-3456") << 0 << 0;
    QTest::newRow( "data16" ) << "C" << -100.0 << 100.0 << 1 << QString("a-3456a") << 0 << 0;
    QTest::newRow( "data17" ) << "C" << 1229.0 << 1231.0 << 0 << QString("123e") << 1 << 0;
    QTest::newRow( "data18" ) << "C" << 1229.0 << 1231.0 << 0 << QString("123e+") << 1 << 0;
    QTest::newRow( "data19" ) << "C" << 1229.0 << 1231.0 << 0 << QString("123e+1") << 2 << 0;
    QTest::newRow( "data20" ) << "C" << 12290.0 << 12310.0 << 0 << QString("123e+2") << 2 << 0;
    QTest::newRow( "data21" ) << "C" << 12.290 << 12.310 << 2 << QString("123e-") << 1 << 0;
    QTest::newRow( "data22" ) << "C" << 12.290 << 12.310 << 2 << QString("123e-1") << 2 << 0;
    QTest::newRow( "data23" ) << "C" << 1.2290 << 1.2310 << 3 << QString("123e-2") << 2 << 0;
    QTest::newRow( "data24" ) << "C" << 1229.0 << 1231.0 << 0 << QString("123E") << 1 << 0;
    QTest::newRow( "data25" ) << "C" << 1229.0 << 1231.0 << 0 << QString("123E+") << 1 << 0;
    QTest::newRow( "data26" ) << "C" << 1229.0 << 1231.0 << 0 << QString("123E+1") << 2 << 0;
    QTest::newRow( "data27" ) << "C" << 12290.0 << 12310.0 << 0 << QString("123E+2") << 2 << 0;
    QTest::newRow( "data28" ) << "C" << 12.290 << 12.310 << 2 << QString("123E-") << 1 << 0;
    QTest::newRow( "data29" ) << "C" << 12.290 << 12.310 << 2 << QString("123E-1") << 2 << 0;
    QTest::newRow( "data30" ) << "C" << 1.2290 << 1.2310 << 3 << QString("123E-2") << 2 << 0;
    QTest::newRow( "data31" ) << "C" << 1.2290 << 1.2310 << 3 << QString("e") << 1 << 0;
    QTest::newRow( "data32" ) << "C" << 1.2290 << 1.2310 << 3 << QString("e+") << 1 << 0;
    QTest::newRow( "data33" ) << "C" << 1.2290 << 1.2310 << 3 << QString("e+1") << 1 << 0;
    QTest::newRow( "data34" ) << "C" << 1.2290 << 1.2310 << 3 << QString("e-") << 1 << 0;
    QTest::newRow( "data35" ) << "C" << 1.2290 << 1.2310 << 3 << QString("e-1") << 1 << 0;
    QTest::newRow( "data36" ) << "C" << 1.2290 << 1.2310 << 3 << QString("E") << 1 << 0;
    QTest::newRow( "data37" ) << "C" << 1.2290 << 1.2310 << 3 << QString("E+") << 1 << 0;
    QTest::newRow( "data38" ) << "C" << 1.2290 << 1.2310 << 3 << QString("E+1") << 1 << 0;
    QTest::newRow( "data39" ) << "C" << 1.2290 << 1.2310 << 3 << QString("E-") << 1 << 0;
    QTest::newRow( "data40" ) << "C" << 1.2290 << 1.2310 << 3 << QString("E-1") << 1 << 0;
    QTest::newRow( "data41" ) << "C" << -100.0 << 100.0 << 0 << QString("10e") << 1 << 0;
    QTest::newRow( "data42" ) << "C" << -100.0 << 100.0 << 0 << QString("10e+") << 1 << 0;

    QTest::newRow( "data_de0" )  << "de" << 0.0 << 100.0 << 1 << QString("50,0") << 2 << 2;
    QTest::newRow( "data_de1" )  << "de" << 00.0 << 100.0 << 1 << QString("500,0") << 1 << 0;
    QTest::newRow( "data_de2" )  << "de" << 00.0 << 100.0 << 1 << QString("-35,0") << 0 << 0;
    QTest::newRow( "data_de3" )  << "de" << 00.0 << 100.0 << 1 << QString("a") << 0 << 0;
    QTest::newRow( "data_de4" )  << "de" << 0.0 << 100.0 << 1 << QString("-") << 0 << 0;
    QTest::newRow( "data_de5" )  << "de" << 0.0 << 100.0 << 1 << QString("100,0") << 2 << 2;
    QTest::newRow( "data_de6" )  << "de" << -100.0 << 100.0 << 1 << QString("-") << 1 << 1;
    QTest::newRow( "data_de7" )  << "de" << -100.0 << 100.0 << 1 << QString("-500,0") << 1 << 0;
    QTest::newRow( "data_de8" )  << "de" << -100.0 << 100.0 << 1 << QString("-100") << 2 << 2;
    QTest::newRow( "data_de9" )  << "de" << -100.0 << -10.0 << 1 << QString("10") << 1 << 0;
    QTest::newRow( "data_de10" ) << "de" << 0.3 << 0.5 << 5 << QString("0,34567") << 2 << 2;
    QTest::newRow( "data_de11" ) << "de" << -0.3 << -0.5 << 5 << QString("-0,345678") << 1 << 0;
    QTest::newRow( "data_de12" ) << "de" << -0.32 << 0.32 << 1 << QString("0") << 2 << 2;
    QTest::newRow( "data_de13" ) << "de" << 0.0 << 100.0 << 1 << QString("3456a") << 0 << 0;
    QTest::newRow( "data_de14" ) << "de" << -100.0 << 100.0 << 1 << QString("-3456a") << 0 << 0;
    QTest::newRow( "data_de15" ) << "de" << -100.0 << 100.0 << 1 << QString("a-3456") << 0 << 0;
    QTest::newRow( "data_de16" ) << "de" << -100.0 << 100.0 << 1 << QString("a-3456a") << 0 << 0;
    QTest::newRow( "data_de17" ) << "de" << 1229.0 << 1231.0 << 0 << QString("123e") << 1 << 0;
    QTest::newRow( "data_de18" ) << "de" << 1229.0 << 1231.0 << 0 << QString("123e+") << 1 << 0;
    QTest::newRow( "data_de19" ) << "de" << 1229.0 << 1231.0 << 0 << QString("123e+1") << 2 << 0;
    QTest::newRow( "data_de20" ) << "de" << 12290.0 << 12310.0 << 0 << QString("123e+2") << 2 << 0;
    QTest::newRow( "data_de21" ) << "de" << 12.290 << 12.310 << 2 << QString("123e-") << 1 << 0;
    QTest::newRow( "data_de22" ) << "de" << 12.290 << 12.310 << 2 << QString("123e-1") << 2 << 0;
    QTest::newRow( "data_de23" ) << "de" << 1.2290 << 1.2310 << 3 << QString("123e-2") << 2 << 0;
    QTest::newRow( "data_de24" ) << "de" << 1229.0 << 1231.0 << 0 << QString("123E") << 1 << 0;
    QTest::newRow( "data_de25" ) << "de" << 1229.0 << 1231.0 << 0 << QString("123E+") << 1 << 0;
    QTest::newRow( "data_de26" ) << "de" << 1229.0 << 1231.0 << 0 << QString("123E+1") << 2 << 0;
    QTest::newRow( "data_de27" ) << "de" << 12290.0 << 12310.0 << 0 << QString("123E+2") << 2 << 0;
    QTest::newRow( "data_de28" ) << "de" << 12.290 << 12.310 << 2 << QString("123E-") << 1 << 0;
    QTest::newRow( "data_de29" ) << "de" << 12.290 << 12.310 << 2 << QString("123E-1") << 2 << 0;
    QTest::newRow( "data_de30" ) << "de" << 1.2290 << 1.2310 << 3 << QString("123E-2") << 2 << 0;
    QTest::newRow( "data_de31" ) << "de" << 1.2290 << 1.2310 << 3 << QString("e") << 1 << 0;
    QTest::newRow( "data_de32" ) << "de" << 1.2290 << 1.2310 << 3 << QString("e+") << 1 << 0;
    QTest::newRow( "data_de33" ) << "de" << 1.2290 << 1.2310 << 3 << QString("e+1") << 1 << 0;
    QTest::newRow( "data_de34" ) << "de" << 1.2290 << 1.2310 << 3 << QString("e-") << 1 << 0;
    QTest::newRow( "data_de35" ) << "de" << 1.2290 << 1.2310 << 3 << QString("e-1") << 1 << 0;
    QTest::newRow( "data_de36" ) << "de" << 1.2290 << 1.2310 << 3 << QString("E") << 1 << 0;
    QTest::newRow( "data_de37" ) << "de" << 1.2290 << 1.2310 << 3 << QString("E+") << 1 << 0;
    QTest::newRow( "data_de38" ) << "de" << 1.2290 << 1.2310 << 3 << QString("E+1") << 1 << 0;
    QTest::newRow( "data_de39" ) << "de" << 1.2290 << 1.2310 << 3 << QString("E-") << 1 << 0;
    QTest::newRow( "data_de40" ) << "de" << 1.2290 << 1.2310 << 3 << QString("E-1") << 1 << 0;
    QTest::newRow( "data_de41" ) << "de" << -100.0 << 100.0 << 0 << QString("10e") << 1 << 0;
    QTest::newRow( "data_de42" ) << "de" << -100.0 << 100.0 << 0 << QString("10e+") << 1 << 0;
}

void tst_QDoubleValidator::validate()
{
    QFETCH( QString, localeName);
    QFETCH( double, minimum );
    QFETCH( double, maximum );
    QFETCH( int, decimals );
    QFETCH( QString, value );
    QFETCH( int, scientific_state );
    QFETCH( int, standard_state );

    QLocale::setDefault(QLocale(localeName));

    QDoubleValidator iv( minimum, maximum, decimals, 0 );
    int dummy;
    QCOMPARE((int)iv.validate( value, dummy ), scientific_state);
    iv.setNotation(QDoubleValidator::StandardNotation);
    QCOMPARE((int)iv.validate(value, dummy), standard_state);
}

QTEST_APPLESS_MAIN(tst_QDoubleValidator)
#include "tst_qdoublevalidator.moc"
