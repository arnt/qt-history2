/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qsqlfield.h>
#include <qvariant.h>
#include <qsqlfield.h>


//TESTED_CLASS=
//TESTED_FILES=sql/kernel/qsqlfield.h sql/kernel/qsqlfield.cpp

class tst_QSqlField : public QObject
{
Q_OBJECT

public:
    tst_QSqlField();
    virtual ~tst_QSqlField();


public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void value();
    void type();
    void setValue_data();
    void setValue();
    void setReadOnly();
    void setNull();
    void setName_data();
    void setName();
    void operator_Equal();
    void operator_Assign();
    void name_data();
    void name();
    void isReadOnly();
    void isNull();
    void clear_data();
    void clear();
};

// Testing get/set functions
void tst_QSqlField::getSetCheck()
{
    QSqlField obj1;
    // RequiredStatus QSqlField::requiredStatus()
    // void QSqlField::setRequiredStatus(RequiredStatus)
    obj1.setRequiredStatus(QSqlField::RequiredStatus(QSqlField::Unknown));
    QCOMPARE(QSqlField::RequiredStatus(QSqlField::Unknown), obj1.requiredStatus());
    obj1.setRequiredStatus(QSqlField::RequiredStatus(QSqlField::Optional));
    QCOMPARE(QSqlField::RequiredStatus(QSqlField::Optional), obj1.requiredStatus());
    obj1.setRequiredStatus(QSqlField::RequiredStatus(QSqlField::Required));
    QCOMPARE(QSqlField::RequiredStatus(QSqlField::Required), obj1.requiredStatus());

    // int QSqlField::length()
    // void QSqlField::setLength(int)
    obj1.setLength(0);
    QCOMPARE(0, obj1.length());
    obj1.setLength(INT_MIN);
    QCOMPARE(INT_MIN, obj1.length());
    obj1.setLength(INT_MAX);
    QCOMPARE(INT_MAX, obj1.length());

    // int QSqlField::precision()
    // void QSqlField::setPrecision(int)
    obj1.setPrecision(0);
    QCOMPARE(0, obj1.precision());
    obj1.setPrecision(INT_MIN);
    QCOMPARE(INT_MIN, obj1.precision());
    obj1.setPrecision(INT_MAX);
    QCOMPARE(INT_MAX, obj1.precision());
}

tst_QSqlField::tst_QSqlField()
{
}

tst_QSqlField::~tst_QSqlField()
{

}

void tst_QSqlField::init()
{
// TODO: Add initialization code here.
// This will be executed immediately before each test is run.
}

void tst_QSqlField::cleanup()
{
// TODO: Add cleanup code here.
// This will be executed immediately after each test is run.
}

void tst_QSqlField::clear_data()
{
    QTest::addColumn<int>("val");
    QTest::addColumn<bool>("bval");
    QTest::addColumn<QString>("strVal");
    QTest::addColumn<double>("fval");

    //next we fill it with data
    QTest::newRow( "data0" ) << (int)5 << (bool)TRUE << QString("Hallo") << (double)0;
    QTest::newRow( "data1" )  << -5 << (bool)FALSE << QString("NULL") << (double)-4;
    QTest::newRow( "data2" )  << 0 << (bool)FALSE << QString("0") << (double)0;
}

void tst_QSqlField::clear()
{
    QSqlField field( "Testfield", QVariant::Int );
    QFETCH( int, val );
    field.setValue( val );
    field.setReadOnly(true);
    field.clear();
    QVERIFY( field.value() == val );
    QVERIFY( !field.isNull() );

    QSqlField bfield( "Testfield", QVariant::Bool );
    QFETCH( bool, bval );
    bfield.setValue( QVariant(bval) );
    bfield.setReadOnly(true);
    bfield.clear();

    QVERIFY( bfield.value() == QVariant(bval) );
    QVERIFY( !bfield.isNull() );

    QSqlField ffield( "Testfield", QVariant::Double );
    QFETCH( double, fval );
    ffield.setValue( fval );
    ffield.setReadOnly(true);
    ffield.clear();
    QVERIFY( ffield.value() == fval );
    QVERIFY( !ffield.isNull() );

    QSqlField sfield( "Testfield", QVariant::String );
    QFETCH( QString, strVal );
    sfield.setValue( strVal );
    sfield.setReadOnly(true);
    sfield.clear();
    QVERIFY( sfield.value() == strVal );
    QVERIFY( !sfield.isNull() );
}

void tst_QSqlField::isNull()
{
    QSqlField field( "test", QVariant::String );
    QVERIFY( field.isNull() );
}

void tst_QSqlField::isReadOnly()
{
    QSqlField field( "test", QVariant::String );
    QVERIFY( !field.isReadOnly() );
    field.setReadOnly( TRUE );
    QVERIFY( field.isReadOnly() );
    field.setReadOnly( FALSE );
    QVERIFY( !field.isReadOnly() );
}

void tst_QSqlField::name_data()
{
    QTest::addColumn<QString>("val");

    //next we fill it with data
    QTest::newRow( "data0" )  << QString("test");
    QTest::newRow( "data1" )  << QString("Harry");
    QTest::newRow( "data2" )  << QString("");
}

void tst_QSqlField::name()
{
    QSqlField field( "test", QVariant::String );
    QFETCH( QString, val );
    QVERIFY( field.name() == "test" );
    field.setName( val );
    QVERIFY( field.name() == val );
}

void tst_QSqlField::operator_Assign()
{
    QSqlField field1( "test", QVariant::String );
    field1.setValue( "Harry" );
    field1.setReadOnly( TRUE );
    QSqlField field2 = field1;
    QVERIFY( field1 == field2 );
    QSqlField field3( "test", QVariant::Double );
    field3.clear();
    field1 = field3;
    QVERIFY( field1 == field3 );
}

void tst_QSqlField::operator_Equal()
{
    QSqlField field1( "test", QVariant::String );
    QSqlField field2( "test2", QVariant::String );
    QSqlField field3( "test", QVariant::Int );
    QVERIFY( !(field1 == field2) );
    QVERIFY( !(field1 == field3) );
    field2.setName( "test" );
    QVERIFY( field1 == field2 );
    QVERIFY( field1 == field2 );
    field1.setValue( "Harry" );
    QVERIFY( !(field1 == field2) );
    field2.setValue( "Harry" );
    QVERIFY( field1 == field2 );
    field1.setReadOnly( TRUE );
    QVERIFY( !(field1 == field2) );
    field2.setReadOnly( TRUE );
    QVERIFY( field1 == field2 );
}

void tst_QSqlField::setName_data()
{
    QTest::addColumn<QString>("val");

    //next we fill it with data
    QTest::newRow( "data0" )  << QString("test");
    QTest::newRow( "data1" )  << QString("Harry");
    QTest::newRow( "data2" )  << QString("");
}

void tst_QSqlField::setName()
{
    QSqlField field( "test", QVariant::String );
    QFETCH( QString, val );
    QVERIFY( field.name() == "test" );
    field.setName( val );
    QVERIFY( field.name() == val );
}

void tst_QSqlField::setNull()
{
    QSqlField field( "test", QVariant::String );
    field.setValue( "test" );
    field.clear();
    QVERIFY( field.value() == QVariant().toString() );
    QVERIFY( field.isNull() );
}

void tst_QSqlField::setReadOnly()
{
    QSqlField field( "test", QVariant::String );
    field.setValue( "test" );
    field.setReadOnly( TRUE );
    field.setValue( "Harry" );
    QVERIFY( field.value() == "test" );
    field.clear();
    QVERIFY( field.value() == "test" );
    QVERIFY( !field.isNull() );
    field.clear();
    QVERIFY( field.value() == "test" );
    QVERIFY( !field.isNull() );
    field.setReadOnly( FALSE );
    field.setValue( "Harry" );
    QVERIFY( field.value() == "Harry" );
    field.clear();
    QVERIFY( field.value() == QVariant().toString() );
    QVERIFY( field.isNull() );
}

void tst_QSqlField::setValue_data()
{
    QTest::addColumn<int>("ival");
    QTest::addColumn<bool>("bval");
    QTest::addColumn<double>("dval");
    QTest::addColumn<QString>("sval");

    //next we fill it with data
    QTest::newRow( "data0" )  << 0 << (bool)FALSE << (double)223.232 << QString("");
    QTest::newRow( "data1" )  << 123 << (bool)TRUE << (double)-232.232 << QString("Harry");
    QTest::newRow( "data2" )  << -123 << (bool)FALSE << (double)232222.323223233338 << QString("Woipertinger");
}

void tst_QSqlField::setValue()
{
    QSqlField field1 ( "test", QVariant::Int );
    QSqlField field2 ( "test", QVariant::String );
    QSqlField field3 ( "test", QVariant::Bool );
    QSqlField field4 ( "test", QVariant::Double );
    field1.clear();
    QFETCH( int, ival );
    QFETCH( QString, sval );
    QFETCH( double, dval );
    QFETCH( bool, bval );
    field1.setValue( ival );
    QCOMPARE( field1.value().toInt(), ival );
    // setValue should also reset the NULL flag
    QVERIFY( !field1.isNull() );

    field2.setValue( sval );
    QCOMPARE( field2.value().toString(), sval );
    field3.setValue( QVariant( bval) );
    QVERIFY( field3.value().toBool() == bval );
    field4.setValue( dval );
    QCOMPARE( field4.value().toDouble(), dval );
    field4.setReadOnly( TRUE );
    field4.setValue( "Something_that's_not_a_double" );
    QCOMPARE( field4.value().toDouble(), dval );
}

void tst_QSqlField::type()
{
    QSqlField field1( "string", QVariant::String );
    QSqlField field2( "string", QVariant::Bool );
    QSqlField field3( "string", QVariant::Double );
    QVERIFY( field1.type() == QVariant::String );
    QVERIFY( field2.type() == QVariant::Bool );
    QVERIFY( field3.type() == QVariant::Double );
}

void tst_QSqlField::value()
{
    DEPENDS_ON( "setValue" );
}

QTEST_MAIN(tst_QSqlField)
#include "tst_qsqlfield.moc"
