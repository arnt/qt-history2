/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqldriver.h>
#include <q3sqlcursor.h>
#include <qsqlrecord.h>
#include <qregexp.h>
#include <qvariant.h>
#include <q3cstring.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <q3sqlrecordinfo.h>

#define NODATABASE_SKIP "No database drivers are available in this Qt configuration"

#include "tst_databases.h"

//TESTED_FILES=

QT_DECLARE_CLASS(QSqlDatabase)
struct FieldDef;

class tst_QSqlDatabase : public QObject
{
    Q_OBJECT

public:
    tst_QSqlDatabase();
    virtual ~tst_QSqlDatabase();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void record_data() { generic_data(); }
    void record();
    void open_data() { generic_data(); }
    void open();
    void tables_data() { generic_data(); }
    void tables();
    void transaction_data() { generic_data(); }
    void transaction();
    void eventNotification_data() { generic_data(); }
    void eventNotification();
    void addDatabase();

    //database specific tests
    void recordMySQL_data();
    void recordMySQL();
    void recordPSQL_data();
    void recordPSQL();
    void recordOCI_data();
    void recordOCI();
    void recordTDS_data();
    void recordTDS();
    void recordDB2_data();
    void recordDB2();
    void recordSQLite_data();
    void recordSQLite();
    void recordAccess_data();
    void recordAccess();
    void recordSQLServer_data();
    void recordSQLServer();
    void recordIBase_data();
    void recordIBase();

    void eventNotificationIBase_data() { generic_data(); }
    void eventNotificationIBase();
    void eventNotificationPSQL_data() { generic_data(); }
    void eventNotificationPSQL();

    //database specific 64 bit integer test
    void bigIntField_data() { generic_data(); }
    void bigIntField();

    // general tests
    void getConnectionName_data() { generic_data(); }
    void getConnectionName(); // For task 129992

    //problem specific tests
    void alterTable_data() { generic_data(); }
    void alterTable();
    void recordNonSelect_data() { generic_data(); }
    void recordNonSelect();
    void caseSensivity_data() { generic_data(); }
    void caseSensivity();
    void noEscapedFieldNamesInRecord_data() { generic_data(); }
    void noEscapedFieldNamesInRecord();
    void whitespaceInIdentifiers_data() { generic_data(); }
    void whitespaceInIdentifiers();
    void formatValueTrimStrings_data() { generic_data(); }
    void formatValueTrimStrings();

    void psql_schemas_data();
    void psql_schemas();
    void psql_escapedIdentifiers_data(){ psql_schemas_data(); }
    void psql_escapedIdentifiers();
    void psql_escapeBytea_data() { generic_data(); }
    void psql_escapeBytea();
    void psql_precisionPolicy_data() { generic_data(); }
    void psql_precisionPolicy();

    void mysqlOdbc_unsignedIntegers_data() { generic_data(); }
    void mysqlOdbc_unsignedIntegers();
    void mysql_multiselect_data() { generic_data(); }
    void mysql_multiselect();  // For task 144331

    void accessOdbc_strings_data() { generic_data(); }
    void accessOdbc_strings();

    void ibase_numericFields_data() { generic_data(); }
    void ibase_numericFields(); // For task 125053
    void ibase_fetchBlobs_data() { generic_data(); }
    void ibase_fetchBlobs(); // For task 143471
    void ibase_useCustomCharset_data() { generic_data(); }
    void ibase_useCustomCharset(); // For task 134608
    void ibase_procWithoutReturnValues_data() { generic_data(); } // For task 165423
    void ibase_procWithoutReturnValues();
    void ibase_procWithReturnValues_data() { generic_data(); } // For task 177530
    void ibase_procWithReturnValues();

    void odbc_reopenDatabase_data() { generic_data(); }
    void odbc_reopenDatabase();
    void odbc_uniqueidentifier_data() { generic_data(); }
    void odbc_uniqueidentifier(); // For task 141822
    void odbc_uintfield_data() { generic_data(); }
    void odbc_uintfield();
    void odbc_bindBoolean_data() { generic_data(); }
    void odbc_bindBoolean();

    void oci_serverDetach_data() { generic_data(); }
    void oci_serverDetach(); // For task 154518
    void oci_xmltypeSupport_data() { generic_data(); }
    void oci_xmltypeSupport();

    void sqlite_bindAndFetchUInt_data() { generic_data(); }
    void sqlite_bindAndFetchUInt();

private:
    void createTestTables(QSqlDatabase db);
    void dropTestTables(QSqlDatabase db);
    void populateTestTables(QSqlDatabase db);
    void generic_data();

    void testRecordInfo(const FieldDef fieldDefs[], const Q3SqlRecordInfo& inf);
    void testRecord(const FieldDef fieldDefs[], const QSqlRecord& inf);
    void commonFieldTest(const FieldDef fieldDefs[], QSqlDatabase, const int);
    void checkValues(const FieldDef fieldDefs[], QSqlDatabase db);
    void checkNullValues(const FieldDef fieldDefs[], QSqlDatabase db);

    tst_Databases dbs;
};

// number of records to be inserted per testfunction
static const int ITERATION_COUNT = 2;
static int pkey = 1;

static bool qContainsTable(const QString& str, const QStringList& list)
{
    QString tbl = qTableName(str).upper();
    for (int i = 0; i < (int)list.count(); ++i) {
	if (list[i].upper() == tbl)
	    return TRUE;
    }
    return FALSE;
}

//helper class for database specific tests
struct FieldDef {
    FieldDef(QString tn,
	      QVariant::Type t,
	      QVariant v = QVariant(),
	      bool nl = TRUE):
	typeName(tn), type(t), val(v), nullable(nl) {}

    QString fieldName() const
    {
	QString rt = typeName;
	rt.replace(QRegExp("\\s"), QString("_"));
	int i = rt.find("(");
	if (i == -1)
	    i = rt.length();
	if (i > 20)
	    i = 20;
	return "t_" + rt.left(i);
    }
    QString typeName;
    QVariant::Type type;
    QVariant val;
    bool nullable;
};

// creates a table out of the FieldDefs and returns the number of fields
// excluding the primary key field
static int createFieldTable(const FieldDef fieldDefs[], QSqlDatabase db)
{
    tst_Databases::safeDropTable(db, qTableName("qtestfields"));

    QSqlQuery q(QString::null, db);
    // construct a create table statement consisting of all fieldtypes
    QString qs = "create table " + qTableName("qtestfields");

    if (tst_Databases::isMSAccess(db))
        qs.append(" (id int not null");
    else
        qs.append(" (id integer not null primary key");
    
    int i = 0;
    for (i = 0; !fieldDefs[ i ].typeName.isNull(); ++i) {
	qs += QString(",\n %1 %2").arg(fieldDefs[ i ].fieldName()).arg(fieldDefs[ i ].typeName);
	if ((db.driverName().startsWith("QTDS") || tst_Databases::isSqlServer(db)) && fieldDefs[ i ].nullable) {
	    qs += " null";
	}
    }

    if (tst_Databases::isMSAccess(db))
        qs.append(",\n primary key (id)");

    qs += ")";
    if (!q.exec(qs)) {
	qDebug("Creation of Table failed: \n" + q.lastError().driverText() + "\n" +
		q.lastError().databaseText());
	qDebug("Query: " + qs);
	return -1;
    }
    return i;
}

tst_QSqlDatabase::tst_QSqlDatabase()
{
}

tst_QSqlDatabase::~tst_QSqlDatabase()
{
}

void tst_QSqlDatabase::createTestTables(QSqlDatabase db)
{
    if (!db.isValid())
	return;
    QSqlQuery q(QString::null, db);
    if (db.driverName().startsWith("QMYSQL"))
	// ### stupid workaround until we find a way to hardcode this
	// in the MySQL server startup script
	q.exec("set table_type=innodb");

    // please never ever change this table; otherwise fix all tests ;)
    if (tst_Databases::isMSAccess(db)) {
	QVERIFY2(q.exec("create table " + qTableName("qtest") +
		       " (id int not null, t_varchar varchar(40) not null, t_char char(40), "
		       "t_numeric number, primary key (id, t_varchar))"),
		 tst_Databases::printError(q.lastError()));
    } else {
	QVERIFY2(q.exec("create table " + qTableName("qtest") +
		       " (id integer not null, t_varchar varchar(40) not null, "
		       "t_char char(40), t_numeric numeric(6, 3), primary key (id, t_varchar))"),
		tst_Databases::printError(q.lastError()));
    }

    if (testWhiteSpaceNames(db.driverName())) {
        QString qry = "create table \"" + qTableName("qtest") + " test\" (\"test test\" int primary key)";
        QVERIFY2(q.exec(qry), q.lastError().text());
    }
}

void tst_QSqlDatabase::dropTestTables(QSqlDatabase db)
{
    if (!db.isValid())
	return;
    // drop the view first, otherwise we'll get dependency problems
    tst_Databases::safeDropView(db, qTableName("qtest_view"));
    tst_Databases::safeDropView(db, qTableName("qtest_view2"));
    tst_Databases::safeDropTable(db, qTableName("qtest"));
    tst_Databases::safeDropTable(db, qTableName("qtestfields"));
    tst_Databases::safeDropTable(db, qTableName("qtestalter"));
    tst_Databases::safeDropTable(db, qTableName("qtest_temp"));
    tst_Databases::safeDropTable(db, qTableName("qtest_bigint"));
    tst_Databases::safeDropTable(db, qTableName("qtest_xmltype"));
    
    QSqlQuery q(0, db);
    if (db.driverName().startsWith("QPSQL")) {
        q.exec("drop schema " + qTableName("qtestschema") + " cascade");
        q.exec("drop table \"" + qTableName("qtest") + " test\"");
    }

    if (testWhiteSpaceNames(db.driverName())) {
        QString qry = "drop table \"" + qTableName("qtest") + " test\"";
        q.exec(qry), q.lastError().text();
    }
}

void tst_QSqlDatabase::populateTestTables(QSqlDatabase db)
{
    if (!db.isValid())
	return;
    QSqlQuery q(QString::null, db);

    q.exec("delete from " + qTableName("qtest")); //non-fatal
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " (id, t_varchar, t_char, t_numeric) values (0, 'VarChar0', 'Char0', 1.1)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " (id, t_varchar, t_char, t_numeric) values (1, 'VarChar1', 'Char1', 2.2)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " (id, t_varchar, t_char, t_numeric) values (2, 'VarChar2', 'Char2', 3.3)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " (id, t_varchar, t_char, t_numeric) values (3, 'VarChar3', 'Char3', 4.4)"),
	    tst_Databases::printError(q.lastError()));
}

void tst_QSqlDatabase::initTestCase()
{
    dbs.open();

    for (QStringList::ConstIterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it) {
	QSqlDatabase db = QSqlDatabase::database((*it));
	CHECK_DATABASE(db);
	dropTestTables(db); //in case of leftovers
	createTestTables(db);
	populateTestTables(db);
    }
}

void tst_QSqlDatabase::cleanupTestCase()
{
    for (QStringList::ConstIterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it) {
	QSqlDatabase db = QSqlDatabase::database((*it));
	CHECK_DATABASE(db);
	dropTestTables(db);
    }

    dbs.close();
}

void tst_QSqlDatabase::init()
{
}

void tst_QSqlDatabase::cleanup()
{
}

void tst_QSqlDatabase::recordOCI_data()
{
    if (dbs.fillTestTable("QOCI") == 0)
	QSKIP("No Oracle database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordPSQL_data()
{
    if (dbs.fillTestTable("QPSQL") == 0)
	QSKIP("No PostgreSQL database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordMySQL_data()
{
    if (dbs.fillTestTable("QMYSQL") == 0)
	QSKIP("No MySQL database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordTDS_data()
{
    if (dbs.fillTestTable("QTDS") == 0)
	QSKIP("No TDS database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordDB2_data()
{
    if (dbs.fillTestTable("QDB2") == 0)
	QSKIP("No DB2 database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordSQLite_data()
{
    if (dbs.fillTestTable("QSQLITE") == 0)
	QSKIP("No SQLite database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordAccess_data()
{
    if (dbs.fillTestTable("QODBC") == 0)
	QSKIP("No ODBC database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordSQLServer_data()
{
    if (dbs.fillTestTable("QODBC") == 0)
	QSKIP("No ODBC database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::recordIBase_data()
{
    if (dbs.fillTestTable("QIBASE") == 0)
	QSKIP("No Interbase database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::psql_schemas_data()
{
    if (dbs.fillTestTable("QPSQL") == 0)
        QSKIP("No Postgres database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::generic_data()
{
    if (dbs.fillTestTable() == 0)
        QSKIP("No database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlDatabase::addDatabase()
{
    QTest::ignoreMessage(QtWarningMsg, "QSqlDatabase: BLAH_FOO_NONEXISTENT_DRIVER driver not loaded");
    QTest::ignoreMessage(QtWarningMsg, "QSqlDatabase: available drivers: " + QSqlDatabase::drivers().join(QLatin1String(" ")));
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("BLAH_FOO_NONEXISTENT_DRIVER",
                                                    "INVALID_CONNECTION");
        QVERIFY(!db.isValid());
    }
    QVERIFY(QSqlDatabase::contains("INVALID_CONNECTION"));
    QSqlDatabase::removeDatabase("INVALID_CONNECTION");
    QVERIFY(!QSqlDatabase::contains("INVALID_CONNECTION"));
}

void tst_QSqlDatabase::open()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    int i;
    for (i = 0; i < 10; ++i) {
	db.close();
	QVERIFY(!db.isOpen());
	QVERIFY2(db.open(), tst_Databases::printError(db.lastError()));
	QVERIFY(db.isOpen());
	QVERIFY(!db.isOpenError());
    }

    if (db.driverName().startsWith("QSQLITE") && db.databaseName() == ":memory:") {
        // tables in in-memory databases don't survive an open/close
        createTestTables(db);
        populateTestTables(db);
    }
}

void tst_QSqlDatabase::recordNonSelect()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);

    // nothing should happen on an empty query
    QSqlRecord rec = db.record(q);
    QVERIFY(rec.isEmpty());
    Q3SqlRecordInfo rInf = db.recordInfo(q);
    QVERIFY(rInf.isEmpty());

    QVERIFY2(q.exec("create table " + qTableName("qtest_temp") + " (id int)"),
	     tst_Databases::printError(q.lastError()));

    // query without result set should return empty record
    rec = db.record(q);
    QVERIFY(rec.isEmpty());
    rInf = db.recordInfo(q);
    QVERIFY(rInf.isEmpty());

    tst_Databases::safeDropTable(db, qTableName("qtest_temp"));
}

void tst_QSqlDatabase::tables()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    bool views = true;
    bool tempTables = false;

    QSqlQuery q(db);
    if (!q.exec("CREATE VIEW " + qTableName("qtest_view") + " as select * from " + qTableName("qtest"))) {
        qDebug(QString("DBMS '%1' cannot handle VIEWs: %2").arg(
                tst_Databases::dbToString(db)).arg(QString(tst_Databases::printError(q.lastError()))).toLatin1());
        views = false;
    }

    if (db.driverName().startsWith("QSQLITE")) {
        QVERIFY_SQL(q, q.exec("CREATE TEMPORARY TABLE " + qTableName("temp_tab") + " (id int)"));
        tempTables = true;
    }

    QStringList tables = db.tables(QSql::Tables);
    QVERIFY(qContainsTable("qtest", tables));
    QVERIFY(!qContainsTable("sql_features", tables)); //check for postgres 7.4 internal tables
    if (views)
	if (db.driverName().startsWith("QMYSQL"))
            // MySQL doesn't differentiate between tables and views when calling QSqlDatabase::tables()
            // May be fixable by doing a select on informational_schema.tables instead of using the client library api
	    QEXPECT_FAIL("", "MySQL driver thinks that views are tables", Continue);
	QVERIFY(!qContainsTable("qtest_view", tables));
    if (tempTables)
        QVERIFY(qContainsTable("temp_tab", tables));

    tables = db.tables(QSql::Views);
    if (views)
	if (db.driverName().startsWith("QMYSQL"))
            // MySQL doesn't give back anything when calling QSqlDatabase::tables() with QSql::Views
            // May be fixable by doing a select on informational_schema.views instead of using the client library api
	    QEXPECT_FAIL("", "MySQL driver thinks that views are tables", Continue);
	QVERIFY(qContainsTable("qtest_view", tables));
    if (tempTables)
        QVERIFY(!qContainsTable("temp_tab", tables));
    QVERIFY(!qContainsTable("qtest", tables));

    tables = db.tables(QSql::SystemTables);
    QVERIFY(!qContainsTable("qtest", tables));
    QVERIFY(!qContainsTable("qtest_view", tables));
    QVERIFY(!qContainsTable("temp_tab", tables));

    tables = db.tables(QSql::AllTables);
    if (views)
	QVERIFY(qContainsTable("qtest_view", tables));
    if (tempTables)
        QVERIFY(qContainsTable("temp_tab", tables));
    QVERIFY(qContainsTable("qtest", tables));

    if (tst_Databases::isMSAccess(db))
        QSqlQuery("drop table " + qTableName("qtest_view"), db);
    else
        QSqlQuery("drop view " + qTableName("qtest_view"), db);

    if (db.driverName().startsWith("QPSQL")) {
        QVERIFY(tables.contains(qTableName("qtest") + " test"));
    }
}

void tst_QSqlDatabase::whitespaceInIdentifiers()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (testWhiteSpaceNames(db.driverName())) {
        QString tableName = qTableName("qtest") + " test";
        QVERIFY(db.tables().contains(tableName, Qt::CaseInsensitive));

        QSqlRecord rec = db.record(tableName);
        QCOMPARE(rec.count(), 1);
        QCOMPARE(rec.fieldName(0), QString("test test"));
        QCOMPARE(rec.field(0).type(), QVariant::Int);

        QSqlIndex idx = db.primaryIndex(tableName);
        QCOMPARE(idx.count(), 1);
        QCOMPARE(idx.fieldName(0), QString("test test"));
        QCOMPARE(idx.field(0).type(), QVariant::Int);
    } else {
        QSKIP("DBMS does not support whitespaces in identifiers", SkipSingle);
    }
}

void tst_QSqlDatabase::alterTable()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);

    QVERIFY2(q.exec("create table " + qTableName("qtestalter") + " (F1 char(20), F2 char(20), F3 char(20))"),
	     tst_Databases::printError(q.lastError()));
    QSqlRecord rec = db.record(qTableName("qtestalter"));
    Q3SqlRecordInfo rinf = db.recordInfo(qTableName("qtestalter"));

    QCOMPARE((int)rec.count(), 3);
    QCOMPARE((int)rinf.count(), 3);

    int i;
    for (i = 0; i < 3; ++i) {
	QCOMPARE(rec.field(i).name().upper(), QString("F%1").arg(i + 1));
	QCOMPARE(rinf[ i ].name().upper(), QString("F%1").arg(i + 1));
    }

    if (!q.exec("alter table " + qTableName("qtestalter") + " drop column F2")) {
	QSKIP("DBMS doesn't support dropping columns in ALTER TABLE statement", SkipSingle);
    }

    rec = db.record(qTableName("qtestalter"));
    rinf = db.recordInfo(qTableName("qtestalter"));

    QCOMPARE((int)rec.count(), 2);
    QCOMPARE((int)rinf.count(), 2);

    QCOMPARE(rec.field(0).name().upper(), QString("F1"));
    QCOMPARE(rec.field(1).name().upper(), QString("F3"));
    QCOMPARE(rinf[ 0 ].name().upper(), QString("F1"));
    QCOMPARE(rinf[ 1 ].name().upper(), QString("F3"));

    q.exec("select * from " + qTableName("qtestalter"));

    rec = db.record(q);
    rinf = db.recordInfo(q);

    QCOMPARE((int)rec.count(), 2);
    QCOMPARE((int)rinf.count(), 2);

    QCOMPARE(rec.field(0).name().upper(), QString("F1"));
    QCOMPARE(rec.field(1).name().upper(), QString("F3"));
    QCOMPARE(rinf[ 0 ].name().upper(), QString("F1"));
    QCOMPARE(rinf[ 1 ].name().upper(), QString("F3"));

    tst_Databases::safeDropTable(db, qTableName("qtestalter"));
}

// this is the general test that should work on all databases.
// unfortunately no DBMS supports SQL 92/ 99 so the general
// test is more or less a joke. Please write a test for each
// database plugin (see recordOCI and so on). Use this test
// as a template.
void tst_QSqlDatabase::record()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    static const FieldDef fieldDefs[] = {
	FieldDef("char(20)", QVariant::String,         QString("blah1"), FALSE),
	FieldDef("varchar(20)", QVariant::String,      QString("blah2"), FALSE),

	FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

// doesn't work with oracle:   checkNullValues(fieldDefs, db);
    commonFieldTest(fieldDefs, db, fieldCount);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
	checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::testRecordInfo(const FieldDef fieldDefs[], const Q3SqlRecordInfo& inf)
{
    int i = 0;
    for (i = 0; !fieldDefs[ i ].typeName.isNull(); ++i) {
	QCOMPARE(inf[i+1].name().upper(), fieldDefs[ i ].fieldName().upper());
	if (inf[i+1].type() != fieldDefs[ i ].type) {
	    QFAIL(QString(" Expected: '%1' Received: '%2' for field %3 in testRecordInfo").arg(
		  QVariant::typeToName(fieldDefs[ i ].type)).arg(
		  QVariant::typeToName(inf[i+1].type())).arg(
		  fieldDefs[ i ].fieldName()));
	}
    }
}

void tst_QSqlDatabase::testRecord(const FieldDef fieldDefs[], const QSqlRecord& inf)
{
    int i = 0;
    for (i = 0; !fieldDefs[ i ].typeName.isNull(); ++i) {
	QCOMPARE(inf.field(i+1).name().upper(), fieldDefs[ i ].fieldName().upper());
	if (inf.field(i+1).type() != fieldDefs[ i ].type) {
	    QFAIL(QString(" Expected: '%1' Received: '%2' for field %3 in testRecord").arg(
		  QVariant::typeToName(fieldDefs[ i ].type)).arg(
		  QVariant::typeToName(inf.field(i+1).type())).arg(
		  fieldDefs[ i ].fieldName()));
	}

//	qDebug(QString(" field: %1 type: %2 variant type: %3").arg(fieldDefs[ i ].fieldName()).arg(QVariant::typeToName(inf.field(i+1)->type())).arg(QVariant::typeToName(inf.field(i+1)->value().type())));
    }
}

// non-dbms specific tests
void tst_QSqlDatabase::commonFieldTest(const FieldDef fieldDefs[], QSqlDatabase db, const int fieldCount)
{
    CHECK_DATABASE(db);

    // check whether recordInfo returns the right types
    Q3SqlRecordInfo inf = db.recordInfo(qTableName("qtestfields"));
    QCOMPARE((int)inf.count(), fieldCount+1);
    testRecordInfo(fieldDefs, inf);

    QSqlRecord rec = db.record(qTableName("qtestfields"));
    QCOMPARE((int)rec.count(), fieldCount+1);
    testRecord(fieldDefs, rec);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("select * from " + qTableName("qtestfields")), tst_Databases::printError(q.lastError()));

    inf = db.recordInfo(q);
    QCOMPARE((int)inf.count(), fieldCount+1);
    testRecordInfo(fieldDefs, inf);

    rec = db.record(q);
    QCOMPARE((int)rec.count(), fieldCount+1);
    testRecord(fieldDefs, rec);
}

// inserts testdata into the testtable, fetches and compares them
void tst_QSqlDatabase::checkValues(const FieldDef fieldDefs[], QSqlDatabase db)
{
    CHECK_DATABASE(db);

    Q3SqlCursor cur(qTableName("qtestfields"), TRUE, db);
    QVERIFY2(cur.select(), tst_Databases::printError(cur.lastError()));
    QSqlRecord* rec = cur.primeInsert();
    Q_ASSERT(rec);
    rec->setValue("id", pkey++);
    int i = 0;
    for (i = 0; !fieldDefs[ i ].typeName.isNull(); ++i) {
	rec->setValue(fieldDefs[ i ].fieldName(), fieldDefs[ i ].val);
//     qDebug(QString("inserting %1 into %2").arg(fieldDefs[ i ].val.toString()).arg(fieldDefs[ i ].fieldName()));
    }
    if (!cur.insert()) {
	QFAIL(QString("Couldn't insert record: %1 %2").arg(cur.lastError().databaseText()).arg(cur.lastError().driverText()));
    }
    cur.setForwardOnly(TRUE);
    QVERIFY2(cur.select("id = " + QString::number(pkey - 1)),
	    tst_Databases::printError(cur.lastError()));
    QVERIFY2(cur.next(),
	    tst_Databases::printError(cur.lastError()));

    for (i = 0; !fieldDefs[ i ].typeName.isNull(); ++i) {
	bool ok = FALSE;
	QVariant val1 = cur.value(fieldDefs[ i ].fieldName());
	QVariant val2 = fieldDefs[ i ].val;
	if (val1.type() == QVariant::String)
	    //TDS Workaround
	    val1 = val1.toString().stripWhiteSpace();
	if (fieldDefs[ i ].fieldName() == "t_real") {
	    // strip precision
	    val1 = (float)val1.toDouble();
	    val2 = (float)val2.toDouble();
	}
	if (val1.canCast(QVariant::Double) && val2.type() == QVariant::Double) {
	    // we don't care about precision here, we just want to know whether
	    // we can insert/fetch the right values
	    ok = (val1.toDouble() - val2.toDouble() < 0.00001);
	} else if (val1.type() == val2.type()) {
            ok = (val1 == val2);
	} else {
	    ok = (val1.toString() == val2.toString());
	}
	if (!ok) {
	    if (val2.type() == QVariant::DateTime || val2.type() == QVariant::Time)
		qDebug("Expected Time: " + val2.toTime().toString("hh:mm:ss.zzz"));
	    if (val1.type() == QVariant::DateTime || val1.type() == QVariant::Time)
		qDebug("Received Time: " + val1.toTime().toString("hh:mm:ss.zzz"));
	    QFAIL(QString(" Expected: '%1' Received: '%2' for field %3 (etype %4 rtype %5) in checkValues").arg(
		val2.toString()).arg(
		val1.toString()).arg(
		fieldDefs[ i ].fieldName()).arg(
		val2.typeName()).arg(
		val1.typeName())
	   );

	}
    }
}

// inserts a NULL value for each nullable field in testdata, fetches and checks whether
// we get back NULL
void tst_QSqlDatabase::checkNullValues(const FieldDef fieldDefs[], QSqlDatabase db)
{
    CHECK_DATABASE(db);

    Q3SqlCursor cur(qTableName("qtestfields"), TRUE, db);
    QVERIFY2(cur.select(), tst_Databases::printError(cur.lastError()));
    QSqlRecord* rec = cur.primeInsert();
    Q_ASSERT(rec);
    rec->setValue("id", pkey++);
    int i = 0;
    for (i = 0; !fieldDefs[ i ].typeName.isNull(); ++i) {
	if (fieldDefs[ i ].fieldName(), fieldDefs[ i ].nullable)
	    rec->setNull(fieldDefs[ i ].fieldName());
	else
	    rec->setValue(fieldDefs[ i ].fieldName(), fieldDefs[ i ].val);
    }
    if (!cur.insert()) {
	QFAIL(QString("Couldn't insert record: %1 %2").arg(cur.lastError().databaseText()).arg(cur.lastError().driverText()));
    }
    cur.setForwardOnly(TRUE);
    QVERIFY2(cur.select("id = " + QString::number(pkey - 1)),
	    tst_Databases::printError(cur.lastError()));
    QVERIFY2(cur.next(),
	    tst_Databases::printError(cur.lastError()));

    for (i = 0; !fieldDefs[ i ].typeName.isNull(); ++i) {
	if (fieldDefs[ i ].nullable == FALSE)
	    continue;
	// multiple inheritance sucks so much
	QVERIFY2(((QSqlQuery)cur).isNull(i + 1), "Check whether '" + fieldDefs[ i ].fieldName() + "' is null in QSqlQuery");
	QVERIFY2(((QSqlRecord)cur).isNull(fieldDefs[ i ].fieldName()), "Check whether '" + fieldDefs[ i ].fieldName() + "' is null in QSqlRecord");
	if (!cur.value(fieldDefs[ i ].fieldName()).isNull())
	    qDebug(QString("QVariant is not null for NULL-Value in Field '%1'").arg(fieldDefs[ i ].fieldName()));
    }
}

void tst_QSqlDatabase::recordTDS()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QTDS");

    static const FieldDef fieldDefs[] = {
	FieldDef("tinyint", QVariant::Int,		255),
	FieldDef("smallint", QVariant::Int,		32767),
	FieldDef("int", QVariant::Int,			2147483647),
	FieldDef("numeric(10,9)", QVariant::Double,	1.23456789),
	FieldDef("decimal(10,9)", QVariant::Double,	1.23456789),
	FieldDef("float(4)", QVariant::Double,		1.23456789),
	FieldDef("double precision", QVariant::Double,	1.23456789),
	FieldDef("real", QVariant::Double,		1.23456789),
	FieldDef("smallmoney", QVariant::Double,	100.42),
	FieldDef("money", QVariant::Double,		200.42),
	// accuracy is that of a minute
	FieldDef("smalldatetime", QVariant::DateTime,	QDateTime(QDate::currentDate(), QTime(1, 2, 0, 0))),
	// accuracy is that of a second
	FieldDef("datetime", QVariant::DateTime,	QDateTime(QDate::currentDate(), QTime(1, 2, 3, 0))),
	FieldDef("char(20)", QVariant::String,		"blah1"),
	FieldDef("varchar(20)", QVariant::String,	"blah2"),
	FieldDef("nchar(20)", QVariant::String,	"blah3"),
	FieldDef("nvarchar(20)", QVariant::String,	"blah4"),
	FieldDef("text", QVariant::String,		"blah5"),
	FieldDef("binary(20)", QVariant::ByteArray,	Q3CString("blah6")),
	FieldDef("varbinary(20)", QVariant::ByteArray, Q3CString("blah7")),
	FieldDef("image", QVariant::ByteArray,		Q3CString("blah8")),
	FieldDef("bit", QVariant::Int,			1, FALSE),

	FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
	checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::recordOCI()
{
    bool hasTimeStamp = FALSE;

    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QOIC");

    // runtime check for Oracle version since V8 doesn't support TIMESTAMPs
    if (tst_Databases::getOraVersion(db) >= 9) {
	qDebug("Detected Oracle >= 9, TIMESTAMP test enabled");
	hasTimeStamp = TRUE;
    } else {
	qDebug("Detected Oracle < 9, TIMESTAMP test disabled");
    }

    FieldDef tsdef(QString::null, QVariant::Invalid);
    FieldDef tstzdef(QString::null, QVariant::Invalid);
    FieldDef tsltzdef(QString::null, QVariant::Invalid);
    FieldDef intytm(QString::null, QVariant::Invalid);
    FieldDef intdts(QString::null, QVariant::Invalid);

    static const QDateTime dt(QDate::currentDate(), QTime(1, 2, 3, 0));

    if (hasTimeStamp) {
	tsdef = FieldDef("timestamp", QVariant::DateTime,  dt);
	tstzdef = FieldDef("timestamp with time zone", QVariant::DateTime, dt);
	tsltzdef = FieldDef("timestamp with local time zone", QVariant::DateTime, dt);
	intytm = FieldDef("interval year to month", QVariant::String, QString("+01-01"));
	intdts = FieldDef("interval day to second", QVariant::String, QString("+01 00:00:01.000000"));
    }

    const FieldDef fieldDefs[] = {
        FieldDef("char(20)", QVariant::String,          QString("blah1")),
        FieldDef("varchar(20)", QVariant::String,       QString("blah2")),
        FieldDef("nchar(20)", QVariant::String,         QString("blah3")),
        FieldDef("nvarchar2(20)", QVariant::String,     QString("blah4")),
        FieldDef("number(10,5)", QVariant::Double,      1.1234567),
        FieldDef("date", QVariant::DateTime,            dt),
//X?    FieldDef("long raw", QVariant::ByteArray,       QByteArray(Q3CString("blah5"))),
        FieldDef("raw(2000)", QVariant::ByteArray,      QByteArray(Q3CString("blah6")), FALSE),
        FieldDef("blob", QVariant::ByteArray,           QByteArray(Q3CString("blah7"))),
//FIXME FieldDef("clob", QVariant::CString,             Q3CString("blah8")),
//FIXME FieldDef("nclob", QVariant::CString,            Q3CString("blah9")),
//X     FieldDef("bfile", QVariant::ByteArray,          QByteArray(Q3CString("blah10"))),

	intytm,
	intdts,
	tsdef,
	tstzdef,
	tsltzdef,
	FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
	checkValues(fieldDefs, db);
    }

    // some additional tests
    QSqlRecord rec = db.record(qTableName("qtestfields"));
    QCOMPARE(rec.field("T_NUMBER").length(), 10);
    QCOMPARE(rec.field("T_NUMBER").precision(), 5);

    QSqlQuery q(db);
    QVERIFY2(q.exec("SELECT * FROM " + qTableName("qtestfields")),
            q.lastError().text().toLocal8Bit());
    rec = q.record();
    QCOMPARE(rec.field("T_NUMBER").length(), 10);
    QCOMPARE(rec.field("T_NUMBER").precision(), 5);
}

void tst_QSqlDatabase::recordPSQL()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QPSQL");

    FieldDef byteadef(QString::null, QVariant::Invalid);
    if (db.driver()->hasFeature(QSqlDriver::BLOB))
	byteadef = FieldDef("bytea", QVariant::ByteArray, QByteArray(Q3CString("bl\\ah")));
    static FieldDef fieldDefs[] = {
	FieldDef("bigint", QVariant::LongLong,	Q_INT64_C(9223372036854775807)),
	FieldDef("bigserial", QVariant::LongLong, 100, FALSE),
	FieldDef("bit", QVariant::String,	1),
	FieldDef("boolean", QVariant::Bool,	QVariant(bool(TRUE), 0)),
	FieldDef("box", QVariant::String,	"(5,6),(1,2)"),
	FieldDef("char(20)", QVariant::String, "blah5678901234567890"),
	FieldDef("varchar(20)", QVariant::String, "blah5678901234567890"),
	FieldDef("cidr", QVariant::String,	"12.123.0.0/24"),
	FieldDef("circle", QVariant::String,	"<(1,2),3>"),
	FieldDef("date", QVariant::Date,	QDate::currentDate()),
	FieldDef("float8", QVariant::Double,	1.12345678912),
	FieldDef("inet", QVariant::String,	"12.123.12.23"),
	FieldDef("integer", QVariant::Int,	2147483647),
	FieldDef("interval", QVariant::String, "1 day 12:59:10"),
//	LOL... you can create a "line" datatype in PostgreSQL <= 7.2.x but
//	as soon as you want to insert data you get a "not implemented yet" error
//	FieldDef("line", QVariant::Polygon, QPolygon(QRect(1, 2, 3, 4))),
	FieldDef("lseg", QVariant::String,     "[(1,1),(2,2)]"),
	FieldDef("macaddr", QVariant::String, "08:00:2b:01:02:03"),
	FieldDef("money", QVariant::String,	"$12.23"),
	FieldDef("numeric", QVariant::Double,  1.2345678912),
	FieldDef("path", QVariant::String,	"((1,2),(3,2),(3,5),(1,5))"),
	FieldDef("point", QVariant::String,	"(1,2)"),
	FieldDef("polygon", QVariant::String,	"((1,2),(3,2),(3,5),(1,5))"),
	FieldDef("real", QVariant::Double,	1.1234),
	FieldDef("smallint", QVariant::Int,	32767),
	FieldDef("serial", QVariant::Int,	100, FALSE),
	FieldDef("text", QVariant::String,	"blah"),
	FieldDef("time(6)", QVariant::Time,	QTime(1, 2, 3)),
	FieldDef("timetz", QVariant::Time,	QTime(1, 2, 3)),
	FieldDef("timestamp(6)", QVariant::DateTime, QDateTime::currentDateTime()),
	FieldDef("timestamptz", QVariant::DateTime, QDateTime::currentDateTime()),
	byteadef,

	FieldDef(QString::null, QVariant::Invalid)
    };

    QSqlQuery q(QString::null, db);
    q.exec("drop sequence " + qTableName("qtestfields") + "_t_bigserial_seq");
    q.exec("drop sequence " + qTableName("qtestfields") + "_t_serial_seq");
    // older psql cut off the table name
    q.exec("drop sequence " + qTableName("qtestfields").left(15) + "_t_bigserial_seq");
    q.exec("drop sequence " + qTableName("qtestfields").left(18) + "_t_serial_seq");

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
	// increase serial values
	for (int i2 = 0; !fieldDefs[ i2 ].typeName.isNull(); ++i2) {
	    if (fieldDefs[ i2 ].typeName == "serial" ||
		 fieldDefs[ i2 ].typeName == "bigserial") {

		FieldDef def = fieldDefs[ i2 ];
		def.val = def.val.asInt() + 1;
		fieldDefs[ i2 ] = def;
	    }
	}
	checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::recordMySQL()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QMYSQL");

    static QDateTime dt(QDate::currentDate(), QTime(1, 2, 3, 0));
    static const FieldDef fieldDefs[] = {
	FieldDef("tinyint", QVariant::Int,	    127),
	FieldDef("tinyint unsigned", QVariant::UInt, 255),
	FieldDef("smallint", QVariant::Int,	    32767),
	FieldDef("smallint unsigned", QVariant::UInt, 65535),
	FieldDef("mediumint", QVariant::Int,	    8388607),
	FieldDef("mediumint unsigned", QVariant::UInt, 16777215),
	FieldDef("integer", QVariant::Int,	    2147483647),
	FieldDef("integer unsigned", QVariant::UInt, 4294967295u),
	FieldDef("bigint", QVariant::LongLong,	    Q_INT64_C(9223372036854775807)),
	FieldDef("bigint unsigned", QVariant::ULongLong, Q_UINT64_C(18446744073709551615)),
	FieldDef("float", QVariant::Double,	    1.12345),
	FieldDef("double", QVariant::Double,	    1.123456789),
	FieldDef("decimal(10, 9)", QVariant::String,1.123456789),
        FieldDef("numeric(5, 2)", QVariant::String, 123.67),
	FieldDef("date", QVariant::Date,	    QDate::currentDate()),
	FieldDef("datetime", QVariant::DateTime,   dt),
	FieldDef("timestamp", QVariant::DateTime,  dt, FALSE),
	FieldDef("time", QVariant::Time,	    dt.time()),
	FieldDef("year", QVariant::Int,	    2003),
	FieldDef("char(20)", QVariant::String,	    "Blah"),
	FieldDef("varchar(20)", QVariant::String,  "BlahBlah"),
	FieldDef("tinyblob", QVariant::ByteArray,  QByteArray(Q3CString("blah1"))),
	FieldDef("blob", QVariant::ByteArray,	    QByteArray(Q3CString("blah2"))),
	FieldDef("mediumblob", QVariant::ByteArray,QByteArray(Q3CString("blah3"))),
	FieldDef("longblob", QVariant::ByteArray,  QByteArray(Q3CString("blah4"))),
	FieldDef("tinytext", QVariant::String,    QString("blah5")),
	FieldDef("text", QVariant::String,	    QString("blah6")),
	FieldDef("mediumtext", QVariant::String,  QString("blah7")),
	FieldDef("longtext", QVariant::String,    QString("blah8")),
        FieldDef("binary(10)", QVariant::ByteArray, QByteArray(Q3CString("123abc    "))),
        FieldDef("varbinary(10)", QVariant::ByteArray, QByteArray(Q3CString("123abc    "))),
	// SET OF?

	FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
	checkValues(fieldDefs, db);
    }

    QSqlQuery q(db);
    QVERIFY2(q.exec("SELECT DATE_SUB(CURDATE(), INTERVAL 2 DAY)"),
            tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toDateTime().date(), QDate::currentDate().addDays(-2));
}

void tst_QSqlDatabase::recordDB2()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QDB2");

    static const FieldDef fieldDefs[] = {
	FieldDef("char(20)", QVariant::String,		QString("Blah1")),
	FieldDef("varchar(20)", QVariant::String,	QString("Blah2")),
	FieldDef("long varchar", QVariant::String,	QString("Blah3")),
	// using BOOLEAN results in "SQL0486N  The BOOLEAN data type is currently only supported internally."
//X	FieldDef("boolean" , QVariant::Bool,		QVariant(TRUE, 1)),
	FieldDef("smallint", QVariant::Int,		32767),
	FieldDef("integer", QVariant::Int,		2147483647),
	FieldDef("bigint", QVariant::LongLong,		Q_INT64_C(9223372036854775807)),
	FieldDef("real", QVariant::Double,		1.12345),
	FieldDef("double", QVariant::Double,		1.23456789),
	FieldDef("float", QVariant::Double,		1.23456789),
	FieldDef("decimal(10,9)", QVariant::Double,    1.234567891),
	FieldDef("numeric(10,9)", QVariant::Double,    1.234567891),
	FieldDef("date", QVariant::Date,		QDate::currentDate()),
	FieldDef("time", QVariant::Time,		QTime(1, 2, 3)),
	FieldDef("timestamp", QVariant::DateTime,	QDateTime::currentDateTime()),
	FieldDef("graphic(20)", QVariant::String,	QString("Blah4")),
	FieldDef("vargraphic(20)", QVariant::String,	QString("Blah5")),
	FieldDef("long vargraphic", QVariant::String,	QString("Blah6")),
	FieldDef("clob(20)", QVariant::CString,	QString("Blah7")),
	FieldDef("dbclob(20)", QVariant::CString,	QString("Blah8")),
	FieldDef("blob(20)", QVariant::ByteArray,	QByteArray(Q3CString("Blah9"))),
//X	FieldDef("datalink", QVariant::String,		QString("DLVALUE('Blah10')")),
	FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
	checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::recordIBase()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QIBASE");

    static const FieldDef fieldDefs[] = {
	FieldDef("char(20)", QVariant::String, QString("Blah1"), FALSE),
	FieldDef("varchar(20)", QVariant::String, QString("Blah2")),
	FieldDef("smallint", QVariant::Int, 32767),
	FieldDef("float", QVariant::Double, 1.2345),
	FieldDef("double precision", QVariant::Double, 1.2345678),
	FieldDef("timestamp", QVariant::DateTime, QDateTime::currentDateTime()),
	FieldDef("time", QVariant::Time, QTime::currentTime()),
	FieldDef("decimal(18)", QVariant::LongLong, Q_INT64_C(9223372036854775807)),
        FieldDef("numeric(5,2)", QVariant::Double, 123.45),

	FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
        checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::recordSQLite()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QSQLITE");

    static const FieldDef fieldDefs[] = {
        // The affinity of these fields are TEXT so SQLite should give us strings, not ints or doubles.
        FieldDef("char(20)", QVariant::String,          QString("123")),
        FieldDef("varchar(20)", QVariant::String,       QString("123.4")),
        FieldDef("clob", QVariant::String,              QString("123.45")),
        FieldDef("text", QVariant::String,              QString("123.456")),

        FieldDef("integer", QVariant::Int,              QVariant(13)),
        FieldDef("int", QVariant::Int,                  QVariant(12)),

        FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
        checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::recordSQLServer()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!tst_Databases::isSqlServer(db)) {
	QSKIP("SQL server specific test", SkipSingle);
	return;
    }

    // ### TODO: Add the rest of the fields
    static const FieldDef fieldDefs[] = {
        FieldDef("varchar(20)", QVariant::String, QString("Blah1")),
        FieldDef("bigint", QVariant::LongLong, 12345),
        FieldDef("int", QVariant::Int, 123456),
        FieldDef("tinyint", QVariant::Int, 255),
        FieldDef("image", QVariant::ByteArray, Q3CString("Blah1")),
        FieldDef("float", QVariant::Double, 1.12345),
        FieldDef("numeric(5,2)", QVariant::Double, 123.45),
        FieldDef("uniqueidentifier", QVariant::String,
            QString("AA7DF450-F119-11CD-8465-00AA00425D90")),

        FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
        checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::recordAccess()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!tst_Databases::isMSAccess(db)) {
	QSKIP("MS Access specific test", SkipSingle);
	return;
    }

    QString memo;
    for (int i = 0; i < 32; i++)
        memo.append("ABCDEFGH12345678abcdefgh12345678");

    // ### TODO: Add the rest of the fields
    static const FieldDef fieldDefs[] = {
	FieldDef("varchar(20)", QVariant::String, QString("Blah1")),
	FieldDef("single", QVariant::Double, 1.12345),
	FieldDef("double", QVariant::Double, 1.123456),
        FieldDef("byte", QVariant::Int, 255),
	FieldDef("binary", QVariant::ByteArray, Q3CString("Blah2")),
	FieldDef("long", QVariant::Int, 2147483647),
        FieldDef("memo", QVariant::String, memo),
	FieldDef(QString::null, QVariant::Invalid)
    };

    const uint fieldCount = createFieldTable(fieldDefs, db);
    QVERIFY(fieldCount > 0);

    commonFieldTest(fieldDefs, db, fieldCount);
    checkNullValues(fieldDefs, db);
    for (int i = 0; i < ITERATION_COUNT; ++i) {
        checkValues(fieldDefs, db);
    }
}

void tst_QSqlDatabase::transaction()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::Transactions)) {
	QSKIP("DBMS not transaction capable", SkipSingle);
    }

    QVERIFY(db.transaction());

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (40, 'VarChar40', 'Char40', 40.40)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from " + qTableName("qtest") + " where id = 40"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 40);
    q.clear();

    QVERIFY(db.commit());

    QVERIFY(db.transaction());
    QVERIFY2(q.exec("select * from " + qTableName("qtest") + " where id = 40"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 40);
    q.clear();
    QVERIFY(db.commit());

    QVERIFY(db.transaction());
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (41, 'VarChar41', 'Char41', 41.41)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from " + qTableName("qtest") + " where id = 41"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 41);
    q.clear(); // for SQLite which does not allow any references on rows that shall be rolled back
    if (!db.rollback()) {
	if (db.driverName().startsWith("QMYSQL")) {
	    qDebug("MySQL: " + tst_Databases::printError(db.lastError()));
	    QSKIP("MySQL transaction failed ", SkipSingle); //non-fatal
	} else {
	    QFAIL("Could not rollback transaction: " + tst_Databases::printError(db.lastError()));
        }
    }

    QVERIFY2(q.exec("select * from " + qTableName("qtest") + " where id = 41"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(!q.next());

    populateTestTables(db);
}

void tst_QSqlDatabase::bigIntField()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    q.setForwardOnly(TRUE);

    QString drvName = db.driverName();
    if (drvName.startsWith("QMYSQL")) {
	QVERIFY2(q.exec("create table " + qTableName("qtest_bigint") + " (id int, t_s64bit bigint, t_u64bit bigint unsigned)"),
		 q.lastError().text());
    } else if (drvName.startsWith("QPSQL")
		|| drvName.startsWith("QDB2")
		|| tst_Databases::isSqlServer(db)) {
	QVERIFY2(q.exec("create table " + qTableName("qtest_bigint") + "(id int, t_s64bit bigint, t_u64bit bigint)"),
		 q.lastError().text());
    } else if (drvName.startsWith("QOCI")) {
	QVERIFY2(q.exec("create table " + qTableName("qtest_bigint") + " (id int, t_s64bit int, t_u64bit int)"),
		 q.lastError().text());
    } else {
	QSKIP("no 64 bit integer support", SkipAll);
    }
    QVERIFY(q.prepare("insert into " + qTableName("qtest_bigint") + " values (?, ?, ?)"));
    qlonglong ll = Q_INT64_C(9223372036854775807);
    qulonglong ull = Q_UINT64_C(18446744073709551615);

    if (drvName.startsWith("QMYSQL") || drvName.startsWith("QOCI")) {
	q.bindValue(0, 0);
	q.bindValue(1, ll);
	q.bindValue(2, ull);
	QVERIFY2(q.exec(), q.lastError().text());
	q.bindValue(0, 1);
	q.bindValue(1, -ll);
	q.bindValue(2, ull);
	QVERIFY2(q.exec(), q.lastError().text());
    } else {
	// usinged bigint fields not supported - a cast is necessary
	q.bindValue(0, 0);
	q.bindValue(1, ll);
	q.bindValue(2, (qlonglong) ull);
	QVERIFY2(q.exec(), q.lastError().text());
	q.bindValue(0, 1);
	q.bindValue(1, -ll);
	q.bindValue(2,  (qlonglong) ull);
	QVERIFY2(q.exec(), q.lastError().text());
    }
    QVERIFY(q.exec("select * from " + qTableName("qtest_bigint") + " order by id"));
    QVERIFY(q.next());
    QCOMPARE(q.value(1).toLongLong(), ll);
    QCOMPARE(q.value(2).toULongLong(), ull);
    QVERIFY(q.next());
    QCOMPARE(q.value(1).toLongLong(), -ll);
    QCOMPARE(q.value(2).toULongLong(), ull);
    tst_Databases::safeDropTable(db, qTableName("qtest_bigint"));
}

void tst_QSqlDatabase::caseSensivity()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    bool cs = FALSE;
    if (db.driverName().startsWith("QMYSQL")
	 || db.driverName().startsWith("QSQLITE")
	 || db.driverName().startsWith("QTDS"))
	cs = TRUE;

    QSqlRecord rec = db.record(qTableName("qtest"));
    QVERIFY((int)rec.count() > 0);
    if (!cs) {
	rec = db.record(qTableName("QTEST").upper());
	QVERIFY((int)rec.count() > 0);
	rec = db.record(qTableName("qTesT"));
	QVERIFY((int)rec.count() > 0);
    }

    Q3SqlRecordInfo rInf = db.recordInfo(qTableName("qtest"));
    QVERIFY((int)rInf.count() > 0);
    if (!cs) {
	rInf = db.recordInfo(qTableName("QTEST").upper());
	QVERIFY((int)rInf.count() > 0);
	rInf = db.recordInfo(qTableName("qTesT"));
	QVERIFY((int)rInf.count() > 0);
    }

    rec = db.primaryIndex(qTableName("qtest"));
    QVERIFY((int)rec.count() > 0);
    if (!cs) {
	rec = db.primaryIndex(qTableName("QTEST").upper());
	QVERIFY((int)rec.count() > 0);
	rec = db.primaryIndex(qTableName("qTesT"));
	QVERIFY((int)rec.count() > 0);
    }
}

void tst_QSqlDatabase::noEscapedFieldNamesInRecord()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QString fieldname("t_varchar");
    if (db.driverName().startsWith("QOCI") || db.driverName().startsWith("QIBASE"))
        fieldname = fieldname.toUpper();

    QSqlQuery q(db);
    QVERIFY2(q.exec("CREATE VIEW " + qTableName("qtest_view2") + " as select * from " + qTableName("qtest")),
        q.lastError().text());
    QVERIFY2(q.exec("SELECT \"" + fieldname + "\" FROM " + qTableName("qtest_view2")), 
        q.lastError().text());
    QCOMPARE(q.record().fieldName(0), fieldname);
    tst_Databases::safeDropView(db, qTableName("qtest_view2"));
}

void tst_QSqlDatabase::psql_schemas()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.tables(QSql::SystemTables).contains("pg_namespace"))
        QSKIP("server does not support schemas", SkipSingle);

    QSqlQuery q(db);
    QVERIFY2(q.exec("CREATE SCHEMA " + qTableName("qtestschema")), q.lastError().text());

    QString table = qTableName("qtestschema") + "." + qTableName("qtesttable");
    QVERIFY2(q.exec("CREATE TABLE " + table + " (id int primary key, name varchar(20))"),
            q.lastError().text());

    QVERIFY(db.tables().contains(table));

    QSqlRecord rec = db.record(table);
    QCOMPARE(rec.count(), 2);
    QCOMPARE(rec.fieldName(0), QString("id"));
    QCOMPARE(rec.fieldName(1), QString("name"));

    rec = db.record(QSqlQuery("select * from " + table, db));
    QCOMPARE(rec.count(), 2);
    QCOMPARE(rec.fieldName(0), QString("id"));
    QCOMPARE(rec.fieldName(1), QString("name"));

    QSqlIndex idx = db.primaryIndex(table);
    QCOMPARE(idx.count(), 1);
    QCOMPARE(idx.fieldName(0), QString("id"));
}

void tst_QSqlDatabase::psql_escapedIdentifiers()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    QSqlDriver* drv = db.driver();
    CHECK_DATABASE(db);

    if (!db.tables(QSql::SystemTables).contains("pg_namespace"))
        QSKIP("server does not support schemas", SkipSingle);

    QSqlQuery q(db);

    QString schemaName = qTableName("qtestScHeMa");
    QString tableName = qTableName("qtestTaBlE");
    QString field1Name = QString("fIeLdNaMe");
    QString field2Name = QString("ZuLu");

    QString createSchema = QString("CREATE SCHEMA \"%1\"").arg(schemaName);
    QVERIFY2(q.exec(createSchema), q.lastError().text());
    QString createTable = QString("CREATE TABLE \"%1\".\"%2\" (\"%3\" int PRIMARY KEY, \"%4\" varchar(20))").arg(schemaName).arg(tableName).arg(field1Name).arg(field2Name);
    QVERIFY2(q.exec(createTable), q.lastError().text());

    QVERIFY(db.tables().contains(schemaName + "." + tableName, Qt::CaseSensitive));

    QSqlField fld1(field1Name, QVariant::Int);
    QSqlField fld2(field2Name, QVariant::String);
    QSqlRecord rec;
    rec.append(fld1);
    rec.append(fld2);

    QVERIFY2(q.exec(drv->sqlStatement(QSqlDriver::SelectStatement, schemaName + "." + tableName, rec, false).toLatin1().data()), q.lastError().text());

    rec = q.record();
    QCOMPARE(rec.count(), 2);
    QCOMPARE(rec.fieldName(0), field1Name);
    QCOMPARE(rec.fieldName(1), field2Name);
    QCOMPARE(rec.field(0).type(), QVariant::Int);

    q.exec(QString("DROP SCHEMA \"%1\" CASCADE").arg(schemaName));
}


void tst_QSqlDatabase::psql_escapeBytea()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QPSQL");

    const char dta[4] = {'\x71', '\x14', '\x32', '\x81'};
    QByteArray ba(dta, 4);

    QSqlQuery q(db);
    QString tableName = qTableName("batable");
    q.exec(QString("DROP TABLE %1").arg(tableName));
    QVERIFY2(q.exec(QString("CREATE TABLE %1 (ba bytea)").arg(tableName)), q.lastError().text());

    QSqlQuery iq(db);
    QVERIFY2(iq.prepare(QString("INSERT INTO %1 VALUES (?)").arg(tableName)), q.lastError().text());
    iq.bindValue(0, QVariant(ba));
    QVERIFY(iq.exec());

    QVERIFY2(q.exec(QString("SELECT ba FROM %1").arg(tableName)), q.lastError().text());
    QVERIFY(q.next());

    QByteArray res = q.value(0).toByteArray();
    int i = 0;
    for (; i < ba.size(); ++i){
	if (ba[i] != res[i])
	    break;
    }

    QCOMPARE(i, 4);

    QVERIFY2(q.exec(QString("DROP TABLE %1").arg(tableName)), q.lastError().text());
}

// This test should be rewritten to work with Oracle as well - or the Oracle driver
// should be fixed to make this test pass (handle overflows)
void tst_QSqlDatabase::psql_precisionPolicy()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QPSQL");

    QSqlQuery q(db);
    QString tableName = qTableName("qtest_precisionpolicy");
    QVERIFY(db.driver()->hasFeature(QSqlDriver::LowPrecisionNumbers));

    // Create a test table with some data
    q.exec(QString("DROP TABLE %1").arg(tableName));
    QVERIFY2(q.exec(QString("CREATE TABLE %1 (id smallint, num numeric(20,0))").arg(tableName)), q.lastError().text());
    QVERIFY2(q.prepare(QString("INSERT INTO %1 VALUES (?, ?)").arg(tableName)), q.lastError().text());
    q.bindValue(0, 1);
    q.bindValue(1, 123);
    QVERIFY2(q.exec(), q.lastError().text());
    q.bindValue(0, 2);
    q.bindValue(1, QString("18500000000000000000"));
    QVERIFY2(q.exec(), q.lastError().text());

    // These are expected to pass
    QString query = QString("SELECT num FROM %1 WHERE id = 1").arg(tableName);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::String);
    
    q.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::LongLong);
    QCOMPARE(q.value(0).toLongLong(), (qlonglong)123);

    q.setNumericalPrecisionPolicy(QSql::LowPrecisionInt32);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::Int);
    QCOMPARE(q.value(0).toInt(), 123);

    q.setNumericalPrecisionPolicy(QSql::LowPrecisionDouble);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::Double);
    QCOMPARE(q.value(0).toDouble(), (double)123);

    query = QString("SELECT num FROM %1 WHERE id = 2").arg(tableName);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::Double);
    QCOMPARE(q.value(0).toDouble(), QString("18500000000000000000").toDouble());

    // Postgres returns invalid QVariants on overflow
    q.setNumericalPrecisionPolicy(QSql::HighPrecision);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::String);
    
    q.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::Invalid);

    q.setNumericalPrecisionPolicy(QSql::LowPrecisionInt32);
    QVERIFY2(q.exec(query), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).type(), QVariant::Invalid);

    q.exec(QString("DROP TABLE %1").arg(tableName));
}

// This test needs a ODBC data source containing MYSQL in it's name
void tst_QSqlDatabase::mysqlOdbc_unsignedIntegers()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QODBC") || !dbName.toUpper().contains("MYSQL")) {
       QSKIP("MySQL through ODBC-driver specific test", SkipSingle);
       return;
    }

    QSqlQuery q(db);
    QString tableName = qTableName("uint");
    q.exec(QString("DROP TABLE %1").arg(tableName));
    QVERIFY2(q.exec(QString("CREATE TABLE %1 (foo integer(10) unsigned, bar integer(10))").arg(tableName)), q.lastError().text());
    QVERIFY2(q.exec(QString("INSERT INTO %1 VALUES (-4000000000, -4000000000)").arg(tableName)), q.lastError().text());
    QVERIFY2(q.exec(QString("INSERT INTO %1 VALUES (4000000000, 4000000000)").arg(tableName)), q.lastError().text());

    QVERIFY2(q.exec(QString("SELECT foo, bar FROM %1").arg(tableName)), q.lastError().text());
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), QString("0"));
    QCOMPARE(q.value(1).toString(), QString("-2147483648"));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), QString("4000000000"));
    QCOMPARE(q.value(1).toString(), QString("2147483647"));

    q.exec(QString("DROP TABLE %1").arg(tableName)), q.lastError().text();
}

void tst_QSqlDatabase::accessOdbc_strings()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!tst_Databases::isMSAccess(db)) {
	QSKIP("MS Access specific test", SkipSingle);
	return;
    }

    QSqlQuery q(db);
    QString tableName = qTableName("strings");
    q.exec(QString("DROP TABLE %1").arg(tableName));
    QVERIFY2(q.exec(QString("CREATE TABLE %1 (aStr memo, bStr memo, cStr memo, dStr memo, eStr memo, fStr memo, gStr memo, hStr memo)").arg(tableName)), q.lastError().text());
    
    QVERIFY2(q.prepare(QString("INSERT INTO %1 VALUES (?, ?, ?, ?, ?, ?, ?, ?)").arg(tableName)), q.lastError().text());
    QString aStr, bStr, cStr, dStr, eStr, fStr, gStr, hStr;

    q.bindValue(0, aStr.fill('A', 32));
    q.bindValue(1, bStr.fill('B', 127));
    q.bindValue(2, cStr.fill('C', 128));
    q.bindValue(3, dStr.fill('D', 129));
    q.bindValue(4, eStr.fill('E', 254));
    q.bindValue(5, fStr.fill('F', 255));
    q.bindValue(6, gStr.fill('G', 256));
    q.bindValue(7, hStr.fill('H', 512));
    
    QVERIFY2(q.exec(), q.lastError().text());

    QVERIFY2(q.exec(QString("SELECT aStr, bStr, cStr, dStr, eStr, fStr, gStr, hStr FROM %1").arg(tableName)), q.lastError().text());
    q.next();
    QCOMPARE(q.value(0).toString(), aStr);
    QCOMPARE(q.value(1).toString(), bStr);
    QCOMPARE(q.value(2).toString(), cStr);
    QCOMPARE(q.value(3).toString(), dStr);
    QCOMPARE(q.value(4).toString(), eStr);
    QCOMPARE(q.value(5).toString(), fStr);
    QCOMPARE(q.value(6).toString(), gStr);
    QCOMPARE(q.value(7).toString(), hStr);

    q.exec(QString("DROP TABLE %1").arg(tableName));

}

// For task 125053
void tst_QSqlDatabase::ibase_numericFields()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QIBASE");

    QSqlQuery q(db);
    QString tableName = qTableName("numericfields");
    q.exec(QString("DROP TABLE %1").arg(tableName));
    QVERIFY2(q.exec(QString("CREATE TABLE %1 (id int not null, num1 NUMERIC(2,1), "
        "num2 NUMERIC(5,2), num3 NUMERIC(10,3), "
        "num4 NUMERIC(18,4))").arg(tableName)), q.lastError().text());

    QVERIFY2(q.exec(QString("INSERT INTO %1 VALUES (1, 1.1, 123.45, 1234567.123, 10203040506070.8090)").arg(tableName)), 
        q.lastError().text());

    QVERIFY2(q.prepare(QString("INSERT INTO %1 VALUES (?, ?, ?, ?, ?)").arg(tableName)),
        q.lastError().text());

    double num1 = 1.1;
    double num2 = 123.45;
    double num3 = 1234567.123;
    double num4 = 10203040506070.8090;

    q.bindValue(0, 2);
    q.bindValue(1, QVariant(num1));
    q.bindValue(2, QVariant(num2));
    q.bindValue(3, QVariant(num3));
    q.bindValue(4, QVariant(num4)); 
    QVERIFY2(q.exec(), q.lastError().text());

    QVERIFY2(q.exec(QString("SELECT id, num1, num2, num3, num4 FROM %1").arg(tableName)),
        q.lastError().text());

    int id = 0;
    while (q.next()) {
        QCOMPARE(q.value(0).toInt(), ++id);
        QCOMPARE(q.value(1).toString(), QString("%1").arg(num1));
        QCOMPARE(q.value(2).toString(), QString("%1").arg(num2));
        QCOMPARE(QString("%1").arg(q.value(3).toDouble()), QString("%1").arg(num3));
        QCOMPARE(QString("%1").arg(q.value(4).toDouble()), QString("%1").arg(num4));
        QVERIFY(q.value(0).type() == QVariant::Int);
        QVERIFY(q.value(1).type() == QVariant::Double);
        QVERIFY(q.value(2).type() == QVariant::Double);
        QVERIFY(q.value(3).type() == QVariant::Double);
        QVERIFY(q.value(4).type() == QVariant::Double);
        
        /*  There are some meta-data we can't get using query.record().field()
            These are available using database.record("tablename").field(), 
            see test after while-loop
             
            Fixable with InterBase 7.5 API 
        */
        QEXPECT_FAIL("", "We dont' know the precision! Fixable with IBase 7.5 API.", Continue);
        QCOMPARE(q.record().field(1).precision(), 2);
        QEXPECT_FAIL("", "We dont' know the precision! Fixable with IBase 7.5 API.", Continue);
        QCOMPARE(q.record().field(2).precision(), 5);
        QEXPECT_FAIL("", "We dont' know the precision! Fixable with IBase 7.5 API.", Continue);
        QCOMPARE(q.record().field(3).precision(), 10);
        QEXPECT_FAIL("", "We dont' know the precision! Fixable with IBase 7.5 API.", Continue);
        QCOMPARE(q.record().field(4).precision(), 18);
        QEXPECT_FAIL("", "Don't know the requiredStatus! Maybe fixable with IBase 7.5 API.", Continue);
        QVERIFY(q.record().field(0).requiredStatus() == QSqlField::Required);
        QEXPECT_FAIL("", "Don't know the requiredStatus! Maybe fixable with IBase 7.5 API.", Continue);
        QVERIFY(q.record().field(1).requiredStatus() == QSqlField::Optional);
    }

    QSqlRecord r = db.record(tableName);
    QVERIFY(r.field(0).type() == QVariant::Int);
    QVERIFY(r.field(1).type() == QVariant::Double);
    QVERIFY(r.field(2).type() == QVariant::Double);
    QVERIFY(r.field(3).type() == QVariant::Double);
    QVERIFY(r.field(4).type() == QVariant::Double);
    QCOMPARE(r.field(1).precision(), 2);
    QCOMPARE(r.field(2).precision(), 5);
    QCOMPARE(r.field(3).precision(), 10);
    QCOMPARE(r.field(4).precision(), 18);
    QVERIFY(r.field(0).requiredStatus() == QSqlField::Required);
    QVERIFY(r.field(1).requiredStatus() == QSqlField::Optional);

    q.exec(QString("DROP TABLE %1").arg(tableName));

}

void tst_QSqlDatabase::ibase_fetchBlobs()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QIBASE");

    QString tableName = qTableName("qtest_ibaseblobs");
    QSqlQuery q(db);
    q.exec(QString("DROP TABLE %1").arg(tableName));
    QVERIFY2(q.exec(QString("CREATE TABLE %1 (blob1 BLOB segment size 256)").arg(tableName)), q.lastError().text());
    
    QVERIFY2(q.prepare(QString("INSERT INTO %1 VALUES (?)").arg(tableName)), q.lastError().text());
    q.bindValue(0, QByteArray().fill('x', 1024));
    QVERIFY2(q.exec(), q.lastError().text());

    QVERIFY2(q.prepare(QString("INSERT INTO %1 VALUES (?)").arg(tableName)), q.lastError().text());
    q.bindValue(0, QByteArray().fill('x', 16383));
    QVERIFY2(q.exec(), q.lastError().text());

    QVERIFY2(q.prepare(QString("INSERT INTO %1 VALUES (?)").arg(tableName)), q.lastError().text());
    q.bindValue(0, QByteArray().fill('x', 17408));
    QVERIFY2(q.exec(), q.lastError().text());

    QVERIFY2(q.exec(QString("SELECT * FROM %1").arg(tableName)), q.lastError().text());

    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toByteArray().size(), 1024);
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toByteArray().size(), 16383);
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toByteArray().size(), 17408);

    q.exec(QString("DROP TABLE %1").arg(tableName));

}

void tst_QSqlDatabase::ibase_procWithoutReturnValues()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QIBASE");

    QSqlQuery q(db);
    QString procName = qTableName("qtest_proc1");
    q.exec(QString("drop procedure %1").arg(procName));
    QVERIFY2(q.exec("CREATE PROCEDURE " + procName + " (str VARCHAR(10))\nAS BEGIN\nstr='test';\nEND;"), q.lastError().text());
    QVERIFY2(q.exec(QString("execute procedure %1('qtest')").arg(procName)), q.lastError().text());
    q.exec(QString("drop procedure %1").arg(procName));
}

void tst_QSqlDatabase::ibase_procWithReturnValues()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QIBASE")) {
       QSKIP("InterBase specific test", SkipSingle);
       return;
    }

    QString procName = qTableName("qtest_proc2");

    QSqlQuery q(db);
    q.exec(QString("drop procedure %1").arg(procName));
    QVERIFY2(q.exec("CREATE PROCEDURE " + procName + " ("
                        "\nABC INTEGER)"
                        "\nRETURNS ("
                        "\nRESULT INTEGER)"
                        "\nAS"
                        "\nbegin"
                        "\nRESULT = 10;"
                        "\nsuspend;"
                        "\nend"), q.lastError().text());

    // Interbase procedures can be executed in two ways: EXECUTE PROCEDURE or SELECT
    QVERIFY2(q.exec(QString("execute procedure %1(123)").arg(procName)), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toInt(), 10);
    QVERIFY2(q.exec(QString("select result from %1(456)").arg(procName)), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toInt(), 10);

    q.exec(QString("drop procedure %1").arg(procName));
}

void tst_QSqlDatabase::formatValueTrimStrings()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(db);

    QVERIFY2(q.exec(QString("INSERT INTO %1 (id, t_varchar, t_char) values (50, 'Trim Test ', 'Trim Test 2   ')").arg(qTableName("qtest"))), q.lastError().text());
    QVERIFY2(q.exec(QString("INSERT INTO %1 (id, t_varchar, t_char) values (51, 'TrimTest', 'Trim Test 2')").arg(qTableName("qtest"))), q.lastError().text());
    QVERIFY2(q.exec(QString("INSERT INTO %1 (id, t_varchar, t_char) values (52, ' ', '    ')").arg(qTableName("qtest"))), q.lastError().text());
    
    QVERIFY2(q.exec(QString("SELECT t_varchar, t_char FROM %1 WHERE id >= 50 AND id <= 52 ORDER BY id").arg(qTableName("qtest"))), q.lastError().text());
    
    QVERIFY2(q.next(), q.lastError().text());

    QCOMPARE(db.driver()->formatValue(q.record().field(0), true), QString("'Trim Test'"));
    QCOMPARE(db.driver()->formatValue(q.record().field(1), true), QString("'Trim Test 2'"));

    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(db.driver()->formatValue(q.record().field(0), true), QString("'TrimTest'"));
    QCOMPARE(db.driver()->formatValue(q.record().field(1), true), QString("'Trim Test 2'"));

    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(db.driver()->formatValue(q.record().field(0), true), QString("''"));
    QCOMPARE(db.driver()->formatValue(q.record().field(1), true), QString("''"));

}

void tst_QSqlDatabase::odbc_reopenDatabase()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!tst_Databases::isSqlServer(db)) {
        QSKIP("SQL Server (ODBC) specific test", SkipSingle);
        return;
    }

    QSqlQuery q(db);
    QVERIFY2(q.exec("SELECT * from " + qTableName("qtest")), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    db.open();
    QVERIFY2(q.exec("SELECT * from " + qTableName("qtest")), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    db.open();
}

void tst_QSqlDatabase::odbc_bindBoolean()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QODBC");

    QSqlQuery q(db);
    q.exec("DROP TABLE " + qTableName("qtestBindBool"));
    QVERIFY2(q.exec("CREATE TABLE " + qTableName("qtestBindBool") + "(id int, boolvalue bit)"), q.lastError().text());

    // Bind and insert
    QVERIFY2(q.prepare("INSERT INTO " + qTableName("qtestBindBool") + " VALUES(?, ?)"), q.lastError().text());
    q.bindValue(0, 1);
    q.bindValue(1, true);
    QVERIFY2(q.exec(), q.lastError().text());
    q.bindValue(0, 2);
    q.bindValue(1, false);
    QVERIFY2(q.exec(), q.lastError().text());

    // Retrive
    QVERIFY2(q.exec("SELECT id, boolvalue FROM " + qTableName("qtestBindBool") + " ORDER BY id"), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toBool(), true);
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toInt(), 2);
    QCOMPARE(q.value(1).toBool(), false);

    q.exec("DROP TABLE" + qTableName("qtestBindBool"));
}

void tst_QSqlDatabase::mysql_multiselect()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QMYSQL");

    QSqlQuery q(db);
    QVERIFY2(q.exec("SELECT * FROM " + qTableName("qtest") + "; SELECT * FROM " + qTableName("qtest")), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QVERIFY2(q.exec("SELECT * FROM " + qTableName("qtest") + "; SELECT * FROM " + qTableName("qtest")), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QVERIFY2(q.exec("SELECT * FROM " + qTableName("qtest")), q.lastError().text());
}

void tst_QSqlDatabase::ibase_useCustomCharset()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QIBASE");

    db.close();
    db.setConnectOptions("ISC_DPB_LC_CTYPE=Latin1");
    db.open();

    QString tableName = qTableName("latin1table");

    QSqlQuery q(db);
    tst_Databases::safeDropTable(db, tableName);
    QVERIFY2(q.exec(QString("CREATE TABLE %1(text VARCHAR(5) CHARACTER SET Latin1)").arg(tableName)), q.lastError().text());
    QVERIFY2(q.exec(QString("INSERT INTO %1 VALUES('�����')").arg(tableName)), q.lastError().text());
    QVERIFY2(q.exec(QString("SELECT text FROM %1").arg(tableName)), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toString(), QString("�����"));
    q.exec(QString("DROP TABLE %1").arg(tableName));
}

void tst_QSqlDatabase::oci_serverDetach()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QOCI");

    for (int i = 0; i < 200; i++) {
        db.close();
        if (db.open()) {
            QSqlQuery query(db);
            query.exec("SELECT 1 FROM DUAL");
            db.close();
        } else {
            QFAIL(QString("Can't open database: " + db.lastError().text()));
        }
    }
}

void tst_QSqlDatabase::oci_xmltypeSupport()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QOCI");

    QString tableName = qTableName("qtest_xmltype");
    QString xml("<?xml version=\"1.0\"?><TABLE_NAME>MY_TABLE</TABLE_NAME>");
    QSqlQuery q(db);

    // Embedding the XML in the statement
    q.exec(QString("CREATE TABLE %1(xmldata xmltype)").arg(tableName));
    QVERIFY2(q.exec(QString("INSERT INTO %1 values('%2')").arg(tableName).arg(xml)), q.lastError().text());
    QVERIFY2(q.exec(QString("SELECT a.xmldata.getStringVal() FROM %1 a").arg(tableName)), q.lastError().text());
    QVERIFY2(q.last(), q.lastError().text());
    QCOMPARE(q.value(0).toString(), xml);

    // Binding the XML with a prepared statement
    QVERIFY2(q.prepare(QString("INSERT INTO %1 values(?)").arg(tableName)), q.lastError().text());
    q.addBindValue(xml);
    QVERIFY2(q.exec(), q.lastError().text());
    QVERIFY2(q.exec(QString("SELECT a.xmldata.getStringVal() FROM %1 a").arg(tableName)), q.lastError().text());
    QVERIFY2(q.last(), q.lastError().text());
    QCOMPARE(q.value(0).toString(), xml);
}

// This test isn't really necessary as SQL_GUID / uniqueidentifier is
// already tested in recordSQLServer().
void tst_QSqlDatabase::odbc_uniqueidentifier()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    if (!tst_Databases::isSqlServer(db)) {
        QSKIP("SQL Server (ODBC) specific test", SkipSingle);
        return;
    }

    QString tableName = qTableName("qtest_sqlguid");
    QString guid = QString("AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE");
    QString invalidGuid = QString("GAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE");

    QSqlQuery q(db);
    tst_Databases::safeDropTable(db, tableName);
    QVERIFY2(q.exec(QString("CREATE TABLE %1(id uniqueidentifier)").arg(tableName)),
        q.lastError().text());

    q.prepare(QString("INSERT INTO %1 VALUES(?)").arg(tableName));;
    q.addBindValue(guid);
    QVERIFY2(q.exec(), q.lastError().text());

    q.addBindValue(invalidGuid);
    QEXPECT_FAIL("", "The GUID string is required to be correctly formated!",
        Continue);
    QVERIFY2(q.exec(), q.lastError().text());

    QVERIFY2(q.exec(QString("SELECT id FROM %1").arg(tableName)),
        q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());
    QCOMPARE(q.value(0).toString(), guid);

    q.exec(QString("DROP TABLE %1").arg(tableName));
}

void tst_QSqlDatabase::getConnectionName()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QCOMPARE(db.connectionName(), dbName);
    QSqlDatabase clone = QSqlDatabase::cloneDatabase(db, "clonedDatabase");
    QCOMPARE(clone.connectionName(), QString("clonedDatabase"));
    QTest::ignoreMessage(QtWarningMsg, "QSqlDatabasePrivate::removeDatabase: "
        "connection 'clonedDatabase' is still in use, all queries will cease to work.");
    QSqlDatabase::removeDatabase("clonedDatabase");
    QCOMPARE(clone.connectionName(), QString());
    QCOMPARE(db.connectionName(), dbName);
}

void tst_QSqlDatabase::odbc_uintfield()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QODBC");

    QString tableName = qTableName("uint_table");
    unsigned int val = 4294967295;

    QSqlQuery q(db);
    q.exec(QString("CREATE TABLE %1(num numeric(10))").arg(tableName));
    q.prepare(QString("INSERT INTO %1 VALUES(?)").arg(tableName));
    q.addBindValue(val);
    QVERIFY2(q.exec(), q.lastError().text());

    q.exec(QString("SELECT num FROM %1").arg(tableName));
    if (q.next())
        QCOMPARE(q.value(0).toUInt(), val);
    q.exec(QString("DROP TABLE %1").arg(tableName));
}

void tst_QSqlDatabase::eventNotification()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlDriver *driver = db.driver();
    if (!driver->hasFeature(QSqlDriver::EventNotifications))
        QSKIP("DBMS doesn't support event notifications", SkipSingle);

    // Not subscribed to any events yet
    QCOMPARE(driver->subscribedToNotifications().size(), 0);

    // Subscribe to "event_foo"
    QVERIFY2(driver->subscribeToNotification(QLatin1String("event_foo")), qPrintable(driver->lastError().text()));
    QCOMPARE(driver->subscribedToNotifications().size(), 1);
    QVERIFY(driver->subscribedToNotifications().contains("event_foo"));

    // Can't subscribe to the same event multiple times
    QVERIFY2(!driver->subscribeToNotification(QLatin1String("event_foo")), "Shouldn't be able to subscribe to event_foo twice");
    QCOMPARE(driver->subscribedToNotifications().size(), 1);

    // Unsubscribe from "event_foo"
    QVERIFY2(driver->unsubscribeFromNotification(QLatin1String("event_foo")), driver->lastError().text());
    QCOMPARE(driver->subscribedToNotifications().size(), 0);

    // Re-subscribing to "event_foo" now is allowed
    QVERIFY2(driver->subscribeToNotification(QLatin1String("event_foo")), "Couldn't subscribe to event_foo");
    QCOMPARE(driver->subscribedToNotifications().size(), 1);

    // closing the connection causes automatically unsubscription from all events
    db.close();
    QCOMPARE(driver->subscribedToNotifications().size(), 0);

    // Can't subscribe to anything while database is closed
    QVERIFY2(!driver->subscribeToNotification(QLatin1String("event_foo")), "Shouldn't be able to subscribe to event_foo");
    QCOMPARE(driver->subscribedToNotifications().size(), 0);

    db.open();
}

void tst_QSqlDatabase::eventNotificationIBase()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QIBASE");

    QString procedureName = qTableName("posteventProc");
    QVERIFY2(db.driver()->subscribeToNotification(procedureName), qPrintable(db.driver()->lastError().text()));
    QTest::qWait(300);  // Interbase needs some time to call the driver callback.
    
    db.transaction();   // InterBase events are posted from within transactions.
    QSqlQuery q(db);
    q.exec(QString("DROP PROCEDURE %1").arg(procedureName));
    q.exec(QString("CREATE PROCEDURE %1\nAS BEGIN\nPOST_EVENT '%1';\nEND;").arg(procedureName));
    q.exec(QString("EXECUTE PROCEDURE %1").arg(procedureName));
    QSignalSpy spy(db.driver(), SIGNAL(notification(const QString&)));
    db.commit();        // No notifications are posted until the transaction is commited.
    QTest::qWait(300);  // Interbase needs some time to post the notification and call the driver callback.
                        // This happends from another thread, and we have to process events in order for the
                        // event handler in the driver to be executed and emit the notification signal.

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toString() == procedureName);
    QVERIFY2(db.driver()->unsubscribeFromNotification(procedureName), qPrintable(db.driver()->lastError().text()));
    q.exec(QString("DROP PROCEDURE %1").arg(procedureName));
}

void tst_QSqlDatabase::eventNotificationPSQL()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QPSQL");

    QSqlQuery query(db);
    QString procedureName = qTableName("posteventProc");

    QVERIFY2(db.driver()->subscribeToNotification(procedureName), qPrintable(db.driver()->lastError().text()));
    QSignalSpy spy(db.driver(), SIGNAL(notification(const QString&)));
    query.exec(QString("NOTIFY \"%1\"").arg(procedureName));
    QCoreApplication::processEvents();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toString() == procedureName);
    QVERIFY2(db.driver()->unsubscribeFromNotification(procedureName), qPrintable(db.driver()->lastError().text()));
}

void tst_QSqlDatabase::sqlite_bindAndFetchUInt()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    DBMS_SPECIFIC(db, "QSQLITE");

    QSqlQuery q(db);
    QString tableName = qTableName("uint_test");
    QVERIFY2(q.exec(QString("CREATE TABLE %1(uint_field UNSIGNED INTEGER)").arg(tableName)), q.lastError().text());
    QVERIFY2(q.prepare(QString("INSERT INTO %1 VALUES(?)").arg(tableName)), q.lastError().text());
    q.addBindValue((uint)4000000000);
    QVERIFY2(q.exec(), q.lastError().text());
    QVERIFY2(q.exec(QString("SELECT uint_field FROM %1").arg(tableName)), q.lastError().text());
    QVERIFY2(q.next(), q.lastError().text());

    // All integers in SQLite are signed, so even though we bound the value 
    // as an UInt it will come back as a LongLong
    QCOMPARE(q.value(0).type(), QVariant::LongLong);
    QCOMPARE(q.value(0).toUInt(), (uint)4000000000);
    
    q.exec(QString("DROP TABLE %1").arg(tableName));
}

QTEST_MAIN(tst_QSqlDatabase)
#include "tst_qsqldatabase.moc"
