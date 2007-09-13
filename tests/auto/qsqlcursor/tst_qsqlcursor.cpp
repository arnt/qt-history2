/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <q3sqlcursor.h>
#include <qsqlfield.h>
#include <qsqldriver.h>


#include "../qsqldatabase/tst_databases.h"


//TESTED_FILES=

QT_DECLARE_CLASS(QSqlDatabase)

class tst_Q3SqlCursor : public QObject
{
Q_OBJECT

public:
    tst_Q3SqlCursor();
    virtual ~tst_Q3SqlCursor();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void copyConstructor_data() { generic_data(); }
    void copyConstructor();

    void value_data() { generic_data(); }
    void value();
    void primaryIndex_data() { generic_data(); }
    void primaryIndex();
    void insert_data() { generic_data(); }
    void insert();
    void select_data() { generic_data(); }
    void select();
    void setFilter_data() { generic_data(); }
    void setFilter();
    void setName_data() { generic_data(); }
    void setName();

    // problem specific tests
    void unicode_data() { generic_data(); }
    void unicode();
    void precision_data() { generic_data(); }
    void precision();
    void insertORA_data();
    void insertORA();
    void batchInsert_data() { generic_data(); }
    void batchInsert();
    void insertSpecial_data() { generic_data(); }
    void insertSpecial();
    void updateNoPK_data() { generic_data(); }
    void updateNoPK();
    void insertFieldNameContainsWS_data() { generic_data(); }
    void insertFieldNameContainsWS(); // For task 117996

private:
    void generic_data();
    void createTestTables( QSqlDatabase db );
    void dropTestTables( QSqlDatabase db );
    void populateTestTables( QSqlDatabase db );

    tst_Databases dbs;
};

tst_Q3SqlCursor::tst_Q3SqlCursor()
{
}

tst_Q3SqlCursor::~tst_Q3SqlCursor()
{
}

void tst_Q3SqlCursor::generic_data()
{
    if ( dbs.fillTestTable() == 0 )
	QSKIP( "No database drivers are available in this Qt configuration", SkipAll );
}

void tst_Q3SqlCursor::createTestTables( QSqlDatabase db )
{
    if ( !db.isValid() )
	return;
    QSqlQuery q( QString::null, db );
    // please never ever change this table; otherwise fix all tests ;)
    if ( tst_Databases::isMSAccess( db ) ) {
	QVERIFY2( q.exec( "create table " + qTableName( "qtest" ) + " ( id int not null, t_varchar varchar(40) not null,"
			 "t_char char(40), t_numeric number, primary key (id, t_varchar) )" ),
		 tst_Databases::printError( q.lastError() ) );
    } else {
	QVERIFY2( q.exec( "create table " + qTableName( "qtest" ) + " ( id int not null, t_varchar varchar(40) not null,"
			 "t_char char(40), t_numeric numeric(6, 3), primary key (id, t_varchar) )" ),
		 tst_Databases::printError( q.lastError() ) );
    }

    if ( tst_Databases::isSqlServer( db ) ) {
	//workaround for SQL SERVER since he can store unicode only in nvarchar fields
	QVERIFY2(q.exec("create table " + qTableName("qtest_unicode") + " (id int not null, "
		       "t_varchar nvarchar(40) not null, t_char nchar(40) )" ),
		tst_Databases::printError(q.lastError()));
    } else {
	QVERIFY2(q.exec("create table " + qTableName("qtest_unicode") + " (id int not null, "
		       "t_varchar varchar(40) not null," "t_char char(40))" ),
		tst_Databases::printError(q.lastError()));
    }

    if (tst_Databases::isMSAccess(db)) {
	QVERIFY2(q.exec("create table " + qTableName("qtest_precision") + " (col1 number)"),
		tst_Databases::printError(q.lastError()));
    } else if (db.driverName().startsWith("QIBASE")) {
	QVERIFY2(q.exec("create table " + qTableName("qtest_precision") + " (col1 numeric(15, 14))"),
		tst_Databases::printError(q.lastError()));
    } else {
	QVERIFY2(q.exec("create table " + qTableName("qtest_precision") + " (col1 numeric(15, 14))"),
		tst_Databases::printError(q.lastError()));
    }
}

void tst_Q3SqlCursor::dropTestTables( QSqlDatabase db )
{
    if ( !db.isValid() )
	return;
    tst_Databases::safeDropTable( db, qTableName( "qtest" ) );
    tst_Databases::safeDropTable( db, qTableName( "qtest_unicode" ) );
    tst_Databases::safeDropTable( db, qTableName( "qtest_precision" ) );
    tst_Databases::safeDropTable( db, qTableName( "qtest_ora" ) );
    tst_Databases::safeDropTable( db, qTableName( "qtestPK" ) );
}

void tst_Q3SqlCursor::populateTestTables( QSqlDatabase db )
{
    if (!db.isValid())
	return;
    QSqlQuery q( QString::null, db );

    q.exec( "delete from " + qTableName( "qtest" ) ); //not fatal
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 0, 'VarChar0', 'Char0', 1.1 )" ),
	    tst_Databases::printError( q.lastError() ) );
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 1, 'VarChar1', 'Char1', 2.2 )" ),
	    tst_Databases::printError( q.lastError() ) );
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 2, 'VarChar2', 'Char2', 3.3 )" ),
	    tst_Databases::printError( q.lastError() ) );
    QVERIFY2( q.exec( "insert into " + qTableName( "qtest" ) + " (id, t_varchar, t_char, t_numeric) values ( 3, 'VarChar3', 'Char3', 4.4 )" ),
	    tst_Databases::printError( q.lastError() ) );
}

void tst_Q3SqlCursor::initTestCase()
{
    dbs.open();

    for ( QStringList::ConstIterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it ) {
	QSqlDatabase db = QSqlDatabase::database( (*it) );
	CHECK_DATABASE( db );

	dropTestTables( db ); //in case of leftovers
	createTestTables( db );
	populateTestTables( db );
    }
}

void tst_Q3SqlCursor::cleanupTestCase()
{
    for ( QStringList::ConstIterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it ) {
	QSqlDatabase db = QSqlDatabase::database( (*it) );
	CHECK_DATABASE( db );
	dropTestTables( db );
    }

    dbs.close();
}

void tst_Q3SqlCursor::init()
{
}

void tst_Q3SqlCursor::cleanup()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );
    if ( QTest::currentTestFailed() ) {
	//since Oracle ODBC totally craps out on error, we init again
	db.close();
	db.open();
    }
}

void tst_Q3SqlCursor::copyConstructor()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur2;
    {
	Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );
	QVERIFY2( cur.select( cur.index( QString("id") ) ),
		 tst_Databases::printError( cur.lastError() ) );
	cur2 = Q3SqlCursor( cur );
	// let "cur" run out of scope...
    }

    QSqlRecord* rec = cur2.primeUpdate();
    Q_ASSERT( rec );
    QCOMPARE( (int)rec->count(), 4 );

    int i = 0;
    while ( cur2.next() ) {
	QVERIFY( cur2.value("id").toInt() == i );
	i++;
    }
}

void tst_Q3SqlCursor::value()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );
    QVERIFY2( cur.select( cur.index( QString("id") ) ),
	     tst_Databases::printError( cur.lastError() ) );
    int i = 0;
    while ( cur.next() ) {
	QCOMPARE(cur.value("id").toInt(), i);
	i++;
    }
}

void tst_Q3SqlCursor::primaryIndex()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );
    QSqlIndex index = cur.primaryIndex();
    if ( tst_Databases::isMSAccess( db ) ) {
	QCOMPARE( index.fieldName(1).upper(), QString( "ID" ) );
	QCOMPARE( index.fieldName(0).upper(), QString( "T_VARCHAR" ) );
    } else {
	QCOMPARE( index.fieldName(0).upper(), QString( "ID" ) );
	QCOMPARE( index.fieldName(1).upper(), QString( "T_VARCHAR" ) );
    }
    QVERIFY(!index.isDescending(0));
    QVERIFY(!index.isDescending(1));
}

void tst_Q3SqlCursor::insert()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );
    QSqlRecord* irec = cur.primeInsert();
    QVERIFY( irec != 0 );

    // check that primeInsert returns a valid QSqlRecord
    QCOMPARE( (int)irec->count(), 4 );
    if ( ( irec->field( 0 ).type() != QVariant::Int ) &&
	 ( irec->field( 0 ).type() != QVariant::Double ) ) {
	QFAIL( QString( "Wrong datatype %1 for field 'ID'"
	    " (expected Int or Double)" ).arg( QVariant::typeToName( irec->field( 0 ).type() ) ) );
    }
    QCOMPARE( QVariant::typeToName( irec->field( 1 ).type() ), QVariant::typeToName( QVariant::String ) );
    QCOMPARE( QVariant::typeToName( irec->field( 2 ).type() ), QVariant::typeToName( QVariant::String ) );
    if (db.driverName().startsWith("QIBASE")) {
	QCOMPARE(QVariant::typeToName(irec->field(3).type()), QVariant::typeToName(QVariant::Int));
    } else {
	QCOMPARE(QVariant::typeToName(irec->field(3).type()), QVariant::typeToName(QVariant::Double));
    }
    QCOMPARE( irec->field( 0 ).name().upper(), QString( "ID" ) );
    QCOMPARE( irec->field( 1 ).name().upper(), QString( "T_VARCHAR" ) );
    QCOMPARE( irec->field( 2 ).name().upper(), QString( "T_CHAR" ) );
    QCOMPARE( irec->field( 3 ).name().upper(), QString( "T_NUMERIC" ) );

    irec->setValue( "id", 400 );
    irec->setValue( "t_varchar", "SomeVarChar" );
    irec->setValue( "t_char", "SomeChar" );
    irec->setValue( "t_numeric", 400.400 );

    QCOMPARE( cur.insert(), 1 );

    // restore old test-tables
    populateTestTables( db );
}

void tst_Q3SqlCursor::insertSpecial()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );
    QSqlRecord* irec = cur.primeInsert();
    QVERIFY( irec != 0 );

    QStringList strings;
    strings << "StringWith'ATick" << "StringWith\"Doublequote" << "StringWith\\Backslash" << "StringWith~Tilde";
    strings << "StringWith%Percent" << "StringWith_Underscore" << "StringWith[SquareBracket" << "StringWith{Brace";
    strings << "StringWith''DoubleTick" << "StringWith\\Lot\\of\\Backslash" << "StringWith\"lot\"of\"quotes\"";
    strings << "'StartsAndEndsWithTick'" << "\"StartsAndEndsWithQuote\"";
    strings << "StringWith\nCR" << "StringWith\n\rCRLF";

    int i = 800;

    // INSERT the strings
    QStringList::Iterator it;
    for ( it = strings.begin(); it != strings.end(); ++it ) {
	QSqlRecord* irec = cur.primeInsert();
	QVERIFY( irec != 0 );
	irec->setValue( "id", i );
	irec->setValue( "t_varchar", (*it) );
	irec->setValue( "t_char", (*it) );
	irec->setValue( "t_numeric", (double)i );
	++i;
	QCOMPARE( cur.insert(), 1 );
    }

    QVERIFY( cur.select( "id >= 800 and id < 900" ) );

    int i2 = 800;
    while( cur.next() ) {
	QCOMPARE( cur.value( "id" ).toInt(), i2 );
	QCOMPARE( cur.value( "t_varchar" ).toString().stripWhiteSpace(), strings.at( i2 - 800 ) );
	QCOMPARE( cur.value( "t_char" ).toString().stripWhiteSpace(), strings.at( i2 - 800 ) );
	QCOMPARE( cur.value( "t_numeric" ).toDouble(), (double)i2 );
	++i2;
    }
    QCOMPARE( i, i2 );

    populateTestTables( db );
}

void tst_Q3SqlCursor::batchInsert()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    QSqlQuery q( QString::null, db );
    q.exec( "delete from " + qTableName( "qtest" ) );

    Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );

    int i = 0;
    for ( ; i < 100; ++i ) {
	QSqlRecord* irec = cur.primeInsert();
	Q_ASSERT( irec );
	irec->setValue( "id", i );
	irec->setValue( "t_varchar", "blah" );
	irec->setValue( "t_char", "blah" );
	irec->setValue( "t_numeric", 1.1 );
	if ( db.driverName().startsWith( "QSQLITE" ) ) {
	    QVERIFY( cur.insert( TRUE ) );
	} else {
	    QCOMPARE( cur.insert( TRUE ), 1 );
        }
    }

    for ( ; i < 200; ++i ) {
        QSqlRecord* irec = cur.primeInsert();
        Q_ASSERT( irec );
        irec->setValue( "id", i );
        irec->setValue( "t_varchar", "blah" );
        irec->setValue( "t_char", "blah" );
        irec->setValue( "t_numeric", 1.1 );
	if ( db.driverName().startsWith( "QSQLITE" ) ) {
	    QVERIFY( cur.insert( FALSE ) );
	} else {
	    QCOMPARE( cur.insert( FALSE ), 1 );
        }
    }

    i = 0;
    QVERIFY2( q.exec( "select * from " + qTableName( "qtest" ) + " order by id" ),
	    tst_Databases::printError( q.lastError() ) );
    while ( q.next() ) {
	QCOMPARE( q.value( 0 ).toInt(), i );
	i++;
    }

    QCOMPARE( i, 200 );

    populateTestTables( db );
}

static QString dumpUtf8( const QString& str )
{
    QString res;
    for ( int i = 0; i < (int)str.length(); ++i ) {
	res += "0x" + QString::number( str[ i ].unicode(), 16 ) + " ";
    }
    return res;
}

void tst_Q3SqlCursor::insertORA_data()
{
    if ( dbs.fillTestTable( "QOCI" ) == 0 )
	QSKIP( "No Oracle database drivers are available in this Qt configuration", SkipAll );
}

void tst_Q3SqlCursor::insertORA()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    if (tst_Databases::getOraVersion(db) < 9)
	QSKIP("Need Oracle >= 9", SkipSingle);

    /****** CHARSET TEST ******/

    QSqlQuery q( QString::null, db );
    QVERIFY2( q.exec( "create table " + qTableName( "qtest_ora" ) + " ( id int primary key, t_char varchar(40) )" ),
	    tst_Databases::printError( q.lastError() ) );

    static const QString val1( "blah1" );

    Q3SqlCursor cur ( qTableName( "qtest_ora" ), TRUE, db );
    QSqlRecord* irec = cur.primeInsert();
    irec->setValue( "id", 1 );
    irec->setValue( "t_char", val1 );
    QVERIFY( cur.insert() );

    QVERIFY2( cur.select(),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );
    if ( cur.value( "t_char" ).toString() != val1 )
	qDebug( QString( "Wrong value for t_char: expected '%1', got '%2'" ).arg( val1 ).arg(
		cur.value( "t_char" ).toString() ) );

    static const unsigned short utf8arr[] = { 0xd792,0xd79c,0xd792,0xd79c,0xd799,0x00 };
    static const QString utf8str = QString::fromUcs2( utf8arr );

    irec = cur.primeInsert();
    irec->setValue( "id", 2 );
    irec->setValue( "t_char", utf8str );
    QVERIFY( cur.insert() );

    QVERIFY2( cur.select( "id=2" ),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );

    // until qtest knows non-fatal errors we use qDebug instead
    if ( cur.value( "t_char" ).toString() != utf8str )
        qDebug( QString( "Wrong value for t_char: expected '%1', got '%2'" ).arg( dumpUtf8 ( utf8str ) ).arg(
                dumpUtf8( cur.value( "t_char" ).toString() ) ) );

    tst_Databases::safeDropTable( db, qTableName( "qtest_ora" ) );



    /****** NCHARSET TEST ********/

    QVERIFY2( q.exec( "create table " + qTableName( "qtest_ora" ) + " ( id int primary key, t_nchar nvarchar2(40) )" ),
	    tst_Databases::printError( q.lastError() ) );

    Q3SqlCursor cur2 ( qTableName( "qtest_ora" ), TRUE, db );
    irec = cur2.primeInsert();
    irec->setValue( "id", 1 );
    irec->setValue( "t_nchar", val1 );
    QVERIFY( cur2.insert() );

    QVERIFY2( cur2.select(),
	    tst_Databases::printError( cur2.lastError() ) );
    QVERIFY( cur2.next() );
    if ( cur2.value( "t_nchar" ).toString() != val1 )
        qDebug( QString( "Wrong value for t_nchar: expected '%1', got '%2'" ).arg( val1 ).arg(
                cur2.value( "t_nchar" ).toString() ) );

    irec = cur2.primeInsert();
    irec->setValue( "id", 2 );
    irec->setValue( "t_nchar", utf8str );
    QVERIFY( cur2.insert() );

    QVERIFY2( cur2.select( "id=2" ),
	    tst_Databases::printError( cur2.lastError() ) );
    QVERIFY( cur2.next() );

    // until qtest knows non-fatal errors we use qDebug instead
    if ( cur2.value( "t_nchar" ).toString() != utf8str )
        qDebug( QString( "Wrong value for t_nchar: expected '%1', got '%2'" ).arg( dumpUtf8( utf8str ) ).arg(
                dumpUtf8( cur2.value( "t_nchar" ).toString() ) ) );

    tst_Databases::safeDropTable( db, qTableName( "qtest_ora" ) );
}

void tst_Q3SqlCursor::unicode()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    static const unsigned short utf8arr[] = { 0xd792,0xd79c,0xd792,0xd79c,0xd799,0x20,0xd7a9,0xd799,0x00 };
    static const QString utf8str = QString::fromUcs2( utf8arr );
    if ( !db.driver()->hasFeature( QSqlDriver::Unicode ) ) {
	 QSKIP( "DBMS not Unicode capable", SkipSingle );
    }

    Q3SqlCursor cur( qTableName( "qtest_unicode" ), TRUE, db );
    QSqlRecord* irec = cur.primeInsert();
    irec->setValue( 0, 500 );
    irec->setValue( 1, utf8str );
    irec->setValue( 2, utf8str );
    QVERIFY2( cur.insert(),
	     tst_Databases::printError( cur.lastError() ) );
    QVERIFY2( cur.select( "id=500" ),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY2( cur.next(),
	    tst_Databases::printError( cur.lastError() ) );
    QString res = cur.value( 1 ).asString();
    cur.primeDelete();
    cur.del();

    if ( res != utf8str ) {
	int i;
	for ( i = 0; i < (int)res.length(); ++i ) {
	    if ( res[ i ] != utf8str[ i ] )
		break;
	    }
	QFAIL( QString( "Strings differ at position %1: orig: %2, db: %3" ).arg( i ).arg( utf8str[ i ].unicode() ).arg( res[ i ].unicode() ) );
    }
    QVERIFY( res == utf8str );
}

void tst_Q3SqlCursor::precision()
{
    static const QString precStr = "1.23456789012345";
    static const double precDbl = 2.23456789012345;

    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur( qTableName( "qtest_precision" ), TRUE, db );
    cur.setTrimmed( "col1", TRUE );
    QSqlRecord* irec = cur.primeInsert();
    irec->setValue( 0, precStr );
    QVERIFY( cur.insert() );

    irec = cur.primeInsert();
    irec->setValue( 0, precDbl );
    QVERIFY( cur.insert() );

    QVERIFY2( cur.select(),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );
    QCOMPARE( cur.value( 0 ).asString(), QString( precStr ) );
    QVERIFY( cur.next() );
    QCOMPARE( cur.value( 0 ).asDouble(), precDbl );
}

void tst_Q3SqlCursor::setFilter()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );
    cur.setFilter( "id = 2" );

    QVERIFY2( cur.select(),
	     tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );
    QCOMPARE( cur.value( "id" ).toInt(), 2 );
    QVERIFY( !cur.next() );

    QVERIFY2( cur.select(),
	     tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );
    QCOMPARE( cur.value( "id" ).toInt(), 2 );
    QVERIFY( !cur.next() );

    QVERIFY2( cur.select( "id = 3" ),
             tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );
    QCOMPARE( cur.value( "id" ).toInt(), 3 );
    QVERIFY( !cur.next() );
    
    QVERIFY2( cur.select(),
             tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );
    QCOMPARE( cur.value( "id" ).toInt(), 3 );
    QVERIFY( !cur.next() );
}

void tst_Q3SqlCursor::select()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor cur( qTableName( "qtest" ), TRUE, db );
    QVERIFY2( cur.select(),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur.next() );
    QVERIFY( cur.next() );

    Q3SqlCursor cur2( qTableName( "qtest" ), TRUE, db );
    QVERIFY2( cur2.select( "id = 1" ),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur2.next() );
    QCOMPARE( cur2.value( 0 ).toInt(), 1 );

    Q3SqlCursor cur3( qTableName( "qtest" ), TRUE, db );
    QVERIFY2( cur3.select( cur3.primaryIndex( false ) ),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur3.next() );
    QVERIFY( cur3.next() );
    QCOMPARE( cur3.value( 0 ).toInt(), 1 );

    Q3SqlCursor cur4( qTableName( "qtest" ), TRUE, db );
    QSqlIndex idx = cur4.primaryIndex( false );
    QCOMPARE( (int)idx.count(), 2 );
    if ( tst_Databases::isMSAccess( db ) ) {
	QCOMPARE( idx.field( 1 ).name().upper(), QString("ID") );
	QCOMPARE( idx.field( 0 ).name().upper(), QString("T_VARCHAR") );
    } else {
	QCOMPARE( idx.field( 0 ).name().upper(), QString("ID") );
	QCOMPARE( idx.field( 1 ).name().upper(), QString("T_VARCHAR") );
    }

#ifdef QT_DEBUG
    // for people too stupid to read docs we had to insert this debugging message.
    QTest::ignoreMessage(QtDebugMsg, "Q3SqlCursor::setValue(): This will not affect actual database values. Use primeInsert(), primeUpdate() or primeDelete().");
#endif
    cur4.setValue( "id", 1 );
#ifdef QT_DEBUG
    QTest::ignoreMessage(QtDebugMsg, "Q3SqlCursor::setValue(): This will not affect actual database values. Use primeInsert(), primeUpdate() or primeDelete().");
#endif
    cur4.setValue( "t_varchar", "VarChar1" );

    QVERIFY2( cur4.select( idx, cur4.primaryIndex( false ) ),
	    tst_Databases::printError( cur.lastError() ) );
    QVERIFY( cur4.next() );
    QCOMPARE( cur4.value( 0 ).toInt(), 1 );
}

void tst_Q3SqlCursor::setName()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    Q3SqlCursor c2( qTableName( "qtest" ), TRUE, db );
    QCOMPARE( c2.name(), qTableName( "qtest" ) );
    QCOMPARE( c2.fieldName( 0 ).lower(), QString( "id" ) );

    Q3SqlCursor c( QString::null, TRUE, db );
    c.setName( qTableName( "qtest" ) );
    QCOMPARE( c.name(), qTableName( "qtest" ) );
    QCOMPARE( c.fieldName( 0 ).lower(), QString( "id" ) );

    c.setName( qTableName( "qtest_precision" ) );
    QCOMPARE( c.name(), qTableName( "qtest_precision" ) );
    QCOMPARE( c.fieldName( 0 ).lower(), QString( "col1" ) );
}

/* Database independent test */
void tst_Q3SqlCursor::updateNoPK()
{
    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );
    
    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("create table " + qTableName( "qtestPK" ) + " (id int, name varchar(20), num numeric)"), q.lastError().text());
    
    Q3SqlCursor cur(qTableName("qtestPK"), TRUE, db);
    QSqlRecord* rec = cur.primeInsert();
    Q_ASSERT(rec);
    rec->setNull(0);
    rec->setNull(1);
    rec->setNull(2);
    QVERIFY2(cur.insert() == 1,
	    tst_Databases::printError(cur.lastError()));
    if (!db.driver()->hasFeature(QSqlDriver::PreparedQueries)) {

        // Only QPSQL, QODBC and QOCI drivers currently use escape identifiers for column names 
        if (db.driverName().startsWith("QPSQL") || 
                db.driverName().startsWith("QODBC") ||  
                db.driverName().startsWith("QOCI")) {
            QCOMPARE(cur.lastQuery(), QString::fromLatin1("insert into " + qTableName("qtestPK") +
						         " (\"id\",\"name\",\"num\")"
                                                         " values (NULL,NULL,NULL)"));
        } else {
	    QCOMPARE(cur.lastQuery(), QString::fromLatin1("insert into " + qTableName("qtestPK") +
						         " (id,name,num) values (NULL,NULL,NULL)"));
        }
    }

    rec = cur.primeUpdate();
    Q_ASSERT(rec);
    rec->setValue(0, 1);
    rec->setNull(1);
    rec->setNull(2);
    // Sqlite returns 2, don't ask why.
    QVERIFY(cur.update() != 0);
    QString expect = "update " + qTableName("qtestPK") +
            " set id = 1 , name = NULL , num = NULL  where " + qTableName("qtestPK") + ".id"
            " IS NULL and " + qTableName("qtestPK") + ".name IS NULL and " +
            qTableName("qtestPK") + ".num IS NULL";
    if (!db.driver()->hasFeature(QSqlDriver::PreparedQueries)) {
        if (!db.driverName().startsWith("QSQLITE")) {
	    QCOMPARE(cur.lastQuery(), expect);
        }
    }
    QVERIFY(cur.select(cur.index(QString("id"))));
    QVERIFY(cur.next());
    QCOMPARE(cur.value("id").toInt(), 1);
    QVERIFY(cur.isNull("name"));
    QVERIFY(cur.isNull("num")); 
}

// For task 117996: Q3SqlCursor::insert() should not fail even if field names 
// contain white spaces.
void tst_Q3SqlCursor::insertFieldNameContainsWS() {

    QFETCH( QString, dbName );
    QSqlDatabase db = QSqlDatabase::database( dbName );
    CHECK_DATABASE( db );

    // The bugfix (and this test) depends on QSqlDriver::escapeIdentifier(...) 
    // to be implemented, which is currently only the case for the 
    // QPSQL, QODBC and QOCI drivers.
    if (!db.driverName().startsWith("QPSQL") && 
            !db.driverName().startsWith("QODBC") &&  
            !db.driverName().startsWith("QOCI")) {
       QSKIP("PSQL, QODBC or QOCI specific test", SkipSingle);
       return;
    }

    QString tableName = qTableName("qtestws");

    QSqlQuery q(QString::null, db);
    q.exec(QString("DROP TABLE %1").arg(tableName));
    QVERIFY2(q.exec(QString("CREATE TABLE %1 (id int, \"first Name\" varchar(20), "
        "lastName varchar(20))").arg(tableName)), q.lastError().text());

    Q3SqlCursor cur(QString("%1").arg(tableName), true, db);
    cur.select();

    QSqlRecord *r = cur.primeInsert();
    r->setValue("id", 1);
    r->setValue("firsT NaMe", "Kong");
    r->setValue("lastNaMe", "Harald");

    QVERIFY(cur.insert() == 1);

    cur.select();
    cur.next();

    QVERIFY(cur.value(0) == 1);
    QCOMPARE(cur.value(1).toString(), QString("Kong"));
    QCOMPARE(cur.value(2).toString(), QString("Harald"));
    
    q.exec(QString("DROP TABLE %1").arg(tableName));

}

QTEST_MAIN(tst_Q3SqlCursor)
#include "tst_qsqlcursor.moc"
