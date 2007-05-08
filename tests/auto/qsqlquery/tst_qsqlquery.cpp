/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtSql/QtSql>

#include "../qsqldatabase/tst_databases.h"


//TESTED_FILES=

class tst_QSqlQuery : public QObject
{
Q_OBJECT

public:
    tst_QSqlQuery();
    virtual ~tst_QSqlQuery();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void value_data() { generic_data(); }
    void value();
    void isValid_data() { generic_data(); }
    void isValid();
    void isActive_data() { generic_data(); }
    void isActive();
    void isSelect_data() { generic_data(); }
    void isSelect();
    void numRowsAffected_data() { generic_data(); }
    void numRowsAffected();
    void size_data() { generic_data(); }
    void size();
    void isNull_data() { generic_data(); }
    void isNull();
    void query_exec_data() { generic_data(); }
    void query_exec();
    void execErrorRecovery_data() { generic_data(); }
    void execErrorRecovery();
    void first_data() { generic_data(); }
    void first();
    void next_data() { generic_data(); }
    void next();
    void prev_data() { generic_data(); }
    void prev();
    void last_data() { generic_data(); }
    void last();
    void seek_data() { generic_data(); }
    void seek();
    void transaction_data() { generic_data(); }
    void transaction();
    void record_data() { generic_data(); }
    void record();

    void record_sqlite_data() { generic_data(); }
    void record_sqlite();

    // forwardOnly mode need special treatment
    void forwardOnly_data() { generic_data(); }
    void forwardOnly();

    // bug specific tests
    void bitField_data();
    void bitField();
    void nullBlob_data();
    void nullBlob();
    void blob_data() { generic_data(); }
    void blob();
    void rawField_data();
    void rawField();
    void precision_data() { generic_data(); }
    void precision();
    void nullResult_data() { generic_data(); }
    void nullResult();
    void joins_data() { generic_data(); }
    void joins();
    void outValues_data() { generic_data(); }
    void outValues();
    void char1Select_data() { generic_data(); }
    void char1Select();
    void char1SelectUnicode_data() { generic_data(); }
    void char1SelectUnicode();
    void synonyms_data() { generic_data(); }
    void synonyms();
    void oraOutValues_data();
    void oraOutValues();
    void mysqlOutValues_data();
    void mysqlOutValues();
    void oraClob_data() { oraOutValues_data(); }
    void oraClob();
    void oraLong_data() { oraOutValues_data(); }
    void oraLong();
    void outValuesDB2_data();
    void outValuesDB2();
    void storedProceduresIBase_data();
    void storedProceduresIBase();
    void oraRowId_data();
    void oraRowId();
    void oraXmlType_data();
    void oraXmlType();
    void prepare_bind_exec_data() { generic_data(); }
    void prepare_bind_exec();
    void prepared_select_data() { generic_data(); }
    void prepared_select();
    void sqlServerLongStrings_data() { generic_data(); }
    void sqlServerLongStrings();
    void invalidQuery_data() { generic_data(); }
    void invalidQuery();
    void batchExec_data() { generic_data(); }
    void batchExec();
    void oraArrayBind_data() { generic_data(); }
    void oraArrayBind();
    void lastInsertId_data() { generic_data(); }
    void lastInsertId();
    void lastQuery_data() { generic_data(); }
    void lastQuery();
    void bindWithDoubleColonCastOperator_data() { generic_data(); }
    void bindWithDoubleColonCastOperator();
    void queryOnInvalidDatabase_data() { generic_data(); }
    void queryOnInvalidDatabase();
    void createQueryOnClosedDatabase_data() { generic_data(); }
    void createQueryOnClosedDatabase();
    void seekForwardOnlyQuery_data() { generic_data(); }
    void seekForwardOnlyQuery();

private:
    // returns all database connections
    void generic_data();
    void dropTestTables(QSqlDatabase db);
    void createTestTables(QSqlDatabase db);
    void populateTestTables(QSqlDatabase db);

    tst_Databases dbs;
};

tst_QSqlQuery::tst_QSqlQuery()
{
}

tst_QSqlQuery::~tst_QSqlQuery()
{
}

void tst_QSqlQuery::initTestCase()
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

void tst_QSqlQuery::cleanupTestCase()
{
    for (QStringList::ConstIterator it = dbs.dbNames.begin(); it != dbs.dbNames.end(); ++it) {
	QSqlDatabase db = QSqlDatabase::database((*it));
	CHECK_DATABASE(db);
	dropTestTables(db);
    }
    dbs.close();
}

void tst_QSqlQuery::init()
{
}

void tst_QSqlQuery::cleanup()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    if (QTest::currentTestFunction() == QLatin1String("numRowsAffected")
	 || QTest::currentTestFunction() == QLatin1String("transactions")
	 || QTest::currentTestFunction() == QLatin1String("size")
	 || QTest::currentTestFunction() == QLatin1String("isActive")
         || QTest::currentTestFunction() == QLatin1String("lastInsertId")) {
	populateTestTables(db);
    }
    if (QString(QTest::currentTestFunction()).startsWith("char1Select"))
	 tst_Databases::safeDropTable(db, qTableName("qtest_char1"));
    if (QString(QTest::currentTestFunction()).startsWith("oraClob"))
	 tst_Databases::safeDropTable(db, qTableName("clobby"));
    if (QTest::currentTestFailed() && (db.driverName().startsWith("QOCI")
	 || db.driverName().startsWith("QODBC"))) {
	//since Oracle ODBC totally craps out on error, we init again
	db.close();
	db.open();
    }
}

void tst_QSqlQuery::bitField_data()
{
    if (dbs.fillTestTable("QTDS") == 0)
	QSKIP("No TDS database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::nullBlob_data()
{
    if (dbs.fillTestTable("QOCI") == 0)
	QSKIP("No Oracle database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::oraXmlType_data()
{
    if (dbs.fillTestTable("QOCI") == 0)
	QSKIP("No Oracle database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::rawField_data()
{
    if (dbs.fillTestTable("QOCI") == 0)
	QSKIP("No Oracle database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::generic_data()
{
    if (dbs.fillTestTable() == 0)
	QSKIP("No database drivers are available in this Qt configuration", SkipAll);
}


void tst_QSqlQuery::dropTestTables(QSqlDatabase db)
{
    // drop all the table in case a testcase failed
    tst_Databases::safeDropTable(db, qTableName("qtest"));
    tst_Databases::safeDropTable(db, qTableName("qtest_null"));
    tst_Databases::safeDropTable(db, qTableName("qtest_blob"));
    tst_Databases::safeDropTable(db, qTableName("qtest_bittest"));
    tst_Databases::safeDropTable(db, qTableName("qtest_nullblob"));
    tst_Databases::safeDropTable(db, qTableName("qtest_rawtest"));
    tst_Databases::safeDropTable(db, qTableName("qtest_precision"));
    tst_Databases::safeDropTable(db, qTableName("qtest_prepare"));
    tst_Databases::safeDropTable(db, qTableName("qtestj1"));
    tst_Databases::safeDropTable(db, qTableName("qtestj2"));
    tst_Databases::safeDropTable(db, qTableName("qtest_char1"));
    tst_Databases::safeDropTable(db, qTableName("qxmltest"));
    tst_Databases::safeDropTable(db, qTableName("qtest_exerr"));
//    tst_Databases::safeDropTable(db, qTableName("qtest_batch"));
    if (tst_Databases::isSqlServer(db) || db.driverName().startsWith("QOCI"))
        tst_Databases::safeDropTable(db, qTableName("qtest_longstr"));
}

void tst_QSqlQuery::createTestTables(QSqlDatabase db)
{
    QSqlQuery q(QString::null, db);

    if (db.driverName().startsWith("QMYSQL"))
	// ### stupid workaround until we find a way to hardcode this
	// in the MySQL server startup script
	q.exec("set table_type=innodb");

    QVERIFY2(q.exec("create table " + qTableName("qtest") + " (id int, t_varchar varchar(20),"
	    "t_char char(20))"), tst_Databases::printError(q.lastError()));
    if (tst_Databases::isSqlServer(db) || db.driverName().startsWith("QTDS")) {
	QVERIFY2(q.exec("create table " + qTableName("qtest_null") + " (id int null, t_varchar varchar(20) null)"), tst_Databases::printError(q.lastError()));
    } else {
	QVERIFY2(q.exec("create table " + qTableName("qtest_null") + " (id int, t_varchar varchar(20))"), tst_Databases::printError(q.lastError()));
    }
}

void tst_QSqlQuery::populateTestTables(QSqlDatabase db)
{
    QSqlQuery q(QString::null, db);
    q.exec("delete from " + qTableName("qtest"));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (0, 'VarChar0', 'Char0')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (1, 'VarChar1', 'Char1')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (2, 'VarChar2', 'Char2')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (3, 'VarChar3', 'Char3')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (4, 'VarChar4', 'Char4')"),
	    tst_Databases::printError(q.lastError()));

    q.exec("delete from " + qTableName("qtest_null"));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_null") + " values (0, NULL)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_null") + " values (1, 'n')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_null") + " values (2, 'i')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_null") + " values (3, NULL)"),
	    tst_Databases::printError(q.lastError()));
}

// There were problems with char fields of size 1
void tst_QSqlQuery::char1Select()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    {
    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("create table " + qTableName("qtest_char1") + " (id char(1))"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_char1") + " values ('a')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from " + qTableName("qtest_char1")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    if (db.driverName().startsWith("QIBASE")) {
	QCOMPARE(q.value(0).toString().left(1), QString("a"));
    } else {
	QCOMPARE(q.value(0).toString(), QString("a"));
    }
    QVERIFY(!q.next());
    }

    tst_Databases::safeDropTable(db, qTableName("qtest_char1"));
}

void tst_QSqlQuery::char1SelectUnicode()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.driver()->hasFeature(QSqlDriver::Unicode)) {
	QString uniStr(QChar(0xfb50));
	QSqlQuery q(QString::null, db);

        QString createQuery;
	if (tst_Databases::isSqlServer(db)) {
            createQuery = "create table " + qTableName("qtest_char1") + "(id nchar(1))";
	} else if (db.driverName().startsWith("QDB2")
		    || db.driverName().startsWith("QOCI")
		    || db.driverName().startsWith("QPSQL")) {
            createQuery = "create table " + qTableName("qtest_char1") + " (id char(3))";
	} else if (db.driverName().startsWith("QIBASE")) {
            createQuery = "create table " + qTableName("qtest_char1") +
			   " (id char(1) character set unicode_fss)";
        } else if (db.driverName().startsWith("QMYSQL")) {
            createQuery = "create table " + qTableName("qtest_char1") + " (id char(1)) "
                          "default character set utf8";
	} else {
            createQuery = "create table " + qTableName("qtest_char1") + " (id char(1))";
	}
        QVERIFY_SQL(q, q.exec(createQuery));
	QVERIFY2(q.prepare("insert into " + qTableName("qtest_char1") + " values(?)"),
		tst_Databases::printError(q.lastError()));
	q.bindValue(0, uniStr);
	QVERIFY2(q.exec(),
		tst_Databases::printError(q.lastError()));
	QVERIFY2(q.exec("select * from " + qTableName("qtest_char1")),
		tst_Databases::printError(q.lastError()));
	QVERIFY(q.next());
	if (!q.value(0).toString().isEmpty())
	    QCOMPARE(q.value(0).toString()[ 0 ].unicode(), uniStr[0].unicode());
	QCOMPARE(q.value(0).toString().trimmed(), uniStr);
	QVERIFY(!q.next());

	tst_Databases::safeDropTable(db, qTableName("qtest_char1"));

    } else {
	QSKIP("Database not unicode capable", SkipSingle);
    }
}

void tst_QSqlQuery::oraRowId_data()
{
    if (dbs.fillTestTable("QOCI") == 0)
        QSKIP("No Oracle database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::oraRowId()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("select rowid from " + qTableName("qtest")),
            tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).type(), QVariant::String);
    QVERIFY(!q.value(0).toString().isEmpty());

    tst_Databases::safeDropTable(db, qTableName("qtest_char1"));
    QVERIFY2(q.exec("create table " + qTableName("qtest_char1") + " (id char(1))"),
            tst_Databases::printError(q.lastError()));

    QVERIFY2(q.exec("insert into " + qTableName("qtest_char1") + " values('a')"),
            tst_Databases::printError(q.lastError()));
    QVariant v1 = q.lastInsertId();
    QVERIFY(v1.isValid());

    QVERIFY2(q.exec("insert into " + qTableName("qtest_char1") + " values('b')"),
            tst_Databases::printError(q.lastError()));
    QVariant v2 = q.lastInsertId();
    QVERIFY(v2.isValid());

    QVERIFY2(q.prepare("select * from " + qTableName("qtest_char1") + " where rowid = ?"),
            tst_Databases::printError(q.lastError()));
    q.addBindValue(v1);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), QString("a"));

    q.addBindValue(v2);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), QString("b"));

    tst_Databases::safeDropTable(db, qTableName("qtest_char1"));
}

void tst_QSqlQuery::oraXmlType()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QString xmlStr = "<foo>" + QString().fill('a', 5000) + "</foo>";

    QSqlQuery q(db);

    QVERIFY2(q.exec("create table " + qTableName("qxmltest") + " (col1 SYS.XMLTYPE)"),
            tst_Databases::printError(q.lastError()));
    QVERIFY2(q.prepare("insert into " + qTableName("qxmltest") + " values (?)"),
            tst_Databases::printError(q.lastError()));
    q.addBindValue(xmlStr, QSql::Binary);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
}

void tst_QSqlQuery::mysqlOutValues_data()
{
    if (dbs.fillTestTable("QMYSQL") == 0)
        QSKIP("No MySQL database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::mysqlOutValues()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(db);

    QVERIFY_SQL(q, q.exec("select version()"));
    QVERIFY_SQL(q, q.next());
    if (q.value(0).toString().startsWith("4."))
        QSKIP("Test requires MySQL >5.0", SkipSingle);

    q.exec("drop function " + qTableName("hello"));
    QVERIFY_SQL(q, q.exec("create function " + qTableName("hello") + " (s char(20)) returns varchar(50) return concat('Hello ', s)"));

    QVERIFY_SQL(q, q.exec("select " + qTableName("hello") + "('world')"));
    QVERIFY_SQL(q, q.next());

    QCOMPARE(q.value(0).toString(), QString("Hello world"));

    QVERIFY_SQL(q, q.prepare("select " + qTableName("hello") + "('harald')"));
    QVERIFY_SQL(q, q.exec());
    QVERIFY_SQL(q, q.next());

    QCOMPARE(q.value(0).toString(), QString("Hello harald"));

    QVERIFY_SQL(q, q.exec("drop function " + qTableName("hello")));

    q.exec("drop procedure " + qTableName("qtestproc"));

    QVERIFY_SQL(q, q.exec("create procedure " + qTableName("qtestproc") + " () "
                "BEGIN select * from " + qTableName("qtest") + " order by id; END"));
    QVERIFY_SQL(q, q.exec("call " + qTableName("qtestproc") + "()"));
    QVERIFY_SQL(q, q.next());
    QCOMPARE(q.value(1).toString(), QString("VarChar0"));

#if 0 // this is not supported by MySQL, see task 101954
    QVERIFY_SQL(q, q.prepare("call " + qTableName("qtestproc") + "()"));
    QVERIFY_SQL(q, q.exec());
    QVERIFY_SQL(q, q.next());
    QCOMPARE(q.value(1).toString(), QString("VarChar0"));
#endif

    QVERIFY_SQL(q, q.exec("drop procedure " + qTableName("qtestproc")));

    QVERIFY_SQL(q, q.exec("create procedure " + qTableName("qtestproc") + " (OUT param1 INT) "
                "BEGIN set param1 = 42; END"));

    QVERIFY_SQL(q, q.exec("call " + qTableName("qtestproc") + " (@out)"));
    QVERIFY_SQL(q, q.exec("select @out"));
    QCOMPARE(q.record().fieldName(0), QString("@out"));
    QVERIFY_SQL(q, q.next());
    QCOMPARE(q.value(0).toInt(), 42);

    QVERIFY_SQL(q, q.exec("drop procedure " + qTableName("qtestproc")));
}

void tst_QSqlQuery::oraOutValues_data()
{
    if (dbs.fillTestTable("QOCI") == 0)
        QSKIP("No Oracle database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::oraOutValues()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::PreparedQueries)) {
        QSKIP("Test requires prepared query support", SkipSingle);
        return;
    }
    QSqlQuery q(QString::null, db);
    q.setForwardOnly(TRUE);

    /*** outvalue int ***/
    QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x out int) is\n"
                     "begin\n"
                     "    x := 42;\n"
                     "end;\n"),
             tst_Databases::printError(q.lastError()));
    QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?)"));
    q.addBindValue(0, QSql::Out);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.boundValue(0).toInt(), 42);

    // bind a null value, make sure the OCI driver resets the null flag
    q.addBindValue(QVariant(QVariant::Int), QSql::Out);
    QVERIFY_SQL(q, q.exec());
    QCOMPARE(q.boundValue(0).toInt(), 42);
    QVERIFY(!q.boundValue(0).isNull());

    /*** outvalue varchar ***/
    QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x out varchar) is\n"
                     "begin\n"
                     "    x := 'blah';\n"
                     "end;\n"),
             tst_Databases::printError(q.lastError()));
    QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?)"));
    QString s1("12345");
    s1.reserve(512);
    q.addBindValue(s1, QSql::Out);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.boundValue(0).toString(), QString("blah"));

    /*** in/outvalue numeric ***/
    QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x in out numeric) is\n"
                     "begin\n"
                     "    x := x + 10;\n"
                     "end;\n"),
             tst_Databases::printError(q.lastError()));
    QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?)"));
    q.addBindValue(10, QSql::Out);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.boundValue(0).toInt(), 20);

    /*** in/outvalue varchar ***/
    QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x in out varchar) is\n"
                     "begin\n"
                     "    x := 'homer';\n"
                     "end;\n"),
             tst_Databases::printError(q.lastError()));
    QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?)"));
    q.addBindValue(QString("maggy"), QSql::Out);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.boundValue(0).toString(), QString("homer"));

    /*** in/outvalue varchar ***/
    QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x in out varchar) is\n"
                     "begin\n"
                     "    x := NULL;\n"
                     "end;\n"),
             tst_Databases::printError(q.lastError()));
    QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?)"));
    q.addBindValue(QString("maggy"), QSql::Out);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QVERIFY(q.boundValue(0).isNull());

    /*** in/outvalue int ***/
    QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x in out int) is\n"
                     "begin\n"
                     "    x := NULL;\n"
                     "end;\n"),
             tst_Databases::printError(q.lastError()));
    QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?)"));
    q.addBindValue(42, QSql::Out);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QVERIFY(q.boundValue(0).isNull());

    /*** in/outvalue varchar ***/
    QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x in varchar, y out varchar) is\n"
                     "begin\n"
                     "    y := x||'bubulalakikikokololo';\n"
                     "end;\n"),
             tst_Databases::printError(q.lastError()));
    QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?, ?)"));
    q.addBindValue(QString("fifi"), QSql::In);
    QString out;
    out.reserve(50);
    q.addBindValue(out, QSql::Out);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.boundValue(1).toString(), QString("fifibubulalakikikokololo"));
}

void tst_QSqlQuery::oraClob()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    tst_Databases::safeDropTable(db, qTableName("clobby"));
    QSqlQuery q(db);

    // simple short string
    QVERIFY2(q.exec("create table " + qTableName("clobby") + "(id int primary key, cl clob)"),
            q.lastError().text().toLocal8Bit());
    QVERIFY2(q.exec("insert into " + qTableName("clobby") + " values(1, 'bubu')"),
            q.lastError().text().toLocal8Bit());
    QVERIFY2(q.exec("select cl from " + qTableName("clobby")), q.lastError().text().toLocal8Bit());
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), QString("bubu"));

    // simple short string with binding
    QVERIFY2(q.prepare("insert into " + qTableName("clobby") + " (id, cl) values(?, ?)"),
            q.lastError().text().toLocal8Bit());
    q.addBindValue(2);
    q.addBindValue("lala", QSql::Binary);
    QVERIFY2(q.exec(), q.lastError().text().toLocal8Bit());

    QVERIFY2(q.exec("select cl from " + qTableName("clobby") + " where id = 2"),
            q.lastError().text().toLocal8Bit());
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), QString("lala"));

    // loooong string
    QString loong;
    loong.fill(QLatin1Char('A'), 25000);
    QVERIFY2(q.prepare("insert into " + qTableName("clobby") + " (id, cl) values(?, ?)"),
            q.lastError().text().toLocal8Bit());
    q.addBindValue(3);
    q.addBindValue(loong, QSql::Binary);
    QVERIFY2(q.exec(), q.lastError().text().toLocal8Bit());

    QVERIFY2(q.exec("select cl from " + qTableName("clobby") + " where id = 3"),
            q.lastError().text().toLocal8Bit());
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString().count(), loong.count());
    QVERIFY(q.value(0).toString() == loong);
}

void tst_QSqlQuery::storedProceduresIBase_data()
{
    if (dbs.fillTestTable("QIBASE") == 0)
        QSKIP("No Interbase database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::storedProceduresIBase()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QIBASE")) {
        QSKIP("Test requires Interbase", SkipSingle);
        return;
    }

    QSqlQuery q(db);
    q.exec("drop procedure " + qTableName("TESTPROC"));

    QVERIFY_SQL(q, q.exec("create procedure " + qTableName("TESTPROC") +
                       " RETURNS (x integer, y varchar(20)) "
                       "AS BEGIN "
                       "  x = 42; "
                       "  y = 'Hello Anders'; "
                       "END"));

    QVERIFY_SQL(q, q.prepare("execute procedure " + qTableName("TestProc")));
    QVERIFY_SQL(q, q.exec());

    // check for a valid result set
    QSqlRecord rec = q.record();
    QCOMPARE(rec.count(), 2);
    QCOMPARE(rec.fieldName(0).toUpper(), QString("X"));
    QCOMPARE(rec.fieldName(1).toUpper(), QString("Y"));

    // the first next shall suceed
    QVERIFY_SQL(q, q.next());
    QCOMPARE(q.value(0).toInt(), 42);
    QCOMPARE(q.value(1).toString(), QString("Hello Anders"));

    // the second next shall fail
    QVERIFY(!q.next());

    q.exec("drop procedure " + qTableName("TestProc"));
}

void tst_QSqlQuery::outValuesDB2_data()
{
    if (dbs.fillTestTable("QDB2") == 0)
        QSKIP("No DB2 database drivers are available in this Qt configuration", SkipAll);
}

void tst_QSqlQuery::outValuesDB2()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::PreparedQueries)) {
        QSKIP("Test requires prepared query support", SkipSingle);
        return;
    }

    QSqlQuery q(QString::null, db);
    q.setForwardOnly(TRUE);

    q.exec("drop procedure " + qTableName("tst_outValues")); //non-fatal
    QVERIFY2(q.exec("CREATE PROCEDURE " + qTableName("tst_outValues") +
		     " (OUT x int, OUT x2 double, OUT x3 char(20))\n"
                     "LANGUAGE SQL\n"
                     "P1: BEGIN\n"
                     " SET x = 42;\n"
		     " SET x2 = 4.2;\n"
		     " SET x3 = 'Homer';\n"
                     "END P1"),
             tst_Databases::printError(q.lastError()));

    QVERIFY2(q.prepare("call " + qTableName("tst_outValues") + "(?, ?, ?)"),
	    tst_Databases::printError(q.lastError()));

    q.addBindValue(0, QSql::Out);
    q.addBindValue(0.0, QSql::Out);
    q.addBindValue("Simpson", QSql::Out);

    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));

    QCOMPARE(q.boundValue(0).toInt(), 42);
    QCOMPARE(q.boundValue(1).toDouble(), 4.2);
    QCOMPARE(q.boundValue(2).toString().trimmed(), QString("Homer"));
}

void tst_QSqlQuery::outValues()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::PreparedQueries)) {
	QSKIP("Test requires prepared query support", SkipSingle);
	return;
    }

    QSqlQuery q(QString::null, db);
    q.setForwardOnly(TRUE);

    if (db.driverName().startsWith("QOCI")) {
	QVERIFY2(q.exec("create or replace procedure " + qTableName("tst_outValues") + "(x out int) is\n"
			"begin\n"
			"    x := 42;\n"
			"end;\n"),
		tst_Databases::printError(q.lastError()));
	QVERIFY(q.prepare("call " + qTableName("tst_outvalues") + "(?)"));
    } else if (db.driverName().startsWith("QDB2")) {
	q.exec("drop procedure " + qTableName("tst_outValues")); //non-fatal
	QVERIFY2(q.exec("CREATE PROCEDURE " + qTableName("tst_outValues") + " (OUT x int)\n"
			 "LANGUAGE SQL\n"
			 "P1: BEGIN\n"
			 " SET x = 42;\n"
			 "END P1"),
		tst_Databases::printError(q.lastError()));
	QVERIFY(q.prepare("call " + qTableName("tst_outValues") + "(?)"));
    } else if (tst_Databases::isSqlServer(db)) {
	q.exec("drop procedure " + qTableName("tst_outValues"));  //non-fatal
	QVERIFY2(q.exec("create procedure " + qTableName("tst_outValues") + " (@x int out) as\n"
			"begin\n"
			"    set @x = 42\n"
			"end\n"),
		tst_Databases::printError(q.lastError()));
	QVERIFY(q.prepare("{call " + qTableName("tst_outvalues") + "(?)}"));
    } else {
	QSKIP("Don't know how to create a stored procedure for this database server, please fix this test", SkipSingle);
	return;
    }

    q.addBindValue(0, QSql::Out);

    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));

    QCOMPARE(q.boundValue(0).toInt(), 42);
}

void tst_QSqlQuery::blob()
{
    static const int BLOBSIZE = 1024 * 10;
    static const int BLOBCOUNT = 2;

    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::BLOB))
	QSKIP("DBMS not BLOB capable", SkipSingle);

    //don' make it too big otherwise sybase and mysql will complain
    QByteArray ba(BLOBSIZE, 0);
    int i;
    for (i = 0; i < (int)ba.size(); ++i) {
	ba[i] = i % 256;
    }

    QSqlQuery q(QString::null, db);
    q.setForwardOnly(TRUE);

    QString queryString = QString("create table " + qTableName("qtest_blob") +
	" (id int not null primary key, t_blob %1)").arg(tst_Databases::blobTypeName(db, BLOBSIZE));
    QVERIFY2(q.exec(queryString), tst_Databases::printError(q.lastError()));

    QVERIFY2(q.prepare("insert into " + qTableName("qtest_blob") + " (id, t_blob) values (?, ?)"),
	    tst_Databases::printError(q.lastError()));
    for (i = 0; i < BLOBCOUNT; ++i) {
	q.addBindValue(i);
	q.addBindValue(ba);
	QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    }

    QVERIFY2(q.exec("select * from " + qTableName("qtest_blob")),
	    tst_Databases::printError(q.lastError()));
    for (i = 0; i < BLOBCOUNT; ++i) {
	QVERIFY(q.next());
	QByteArray res = q.value(1).toByteArray();
	QVERIFY2(res.size() >= ba.size(),
	        QString("array sizes differ, expected %1, got %2").arg(ba.size()).arg(res.size()).toLatin1());

	for (int i2 = 0; i2 < (int)ba.size(); ++i2) {
	    if (res[i2] != ba[i2])
		QFAIL(QString("ByteArrays differ at position %1, expected %2, got %3").arg(
		    i2).arg((int)(unsigned char)ba[i2]).arg((int)(unsigned char)res[i2]).toLatin1());
	}
    }

    tst_Databases::safeDropTable(db, qTableName("qtest_blob"));
}

void tst_QSqlQuery::value()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("select id, t_varchar, t_char from " + qTableName("qtest") + " order by id"),
	    tst_Databases::printError(q.lastError()));
    int i = 0;
    while (q.next()) {
	QCOMPARE(q.value(0).toInt(), i);
	if (db.driverName().startsWith("QIBASE")) {
	    QVERIFY(q.value(1).toString().startsWith("VarChar" + QString::number(i)));
	} else if (q.value(1).toString().right(1) == " ") {
	    QCOMPARE(q.value(1).toString(), ("VarChar" + QString::number(i) + "            "));
	} else {
	    QCOMPARE(q.value(1).toString(), ("VarChar" + QString::number(i)));
	}
        if (db.driverName().startsWith("QIBASE")) {
            QVERIFY(q.value(2).toString().startsWith("Char" + QString::number(i)));
	} else if (q.value(2).toString().right(1) != " ") {
	    QCOMPARE(q.value(2).toString(), ("Char" + QString::number(i)));
	} else {
	    QCOMPARE(q.value(2).toString(), ("Char" + QString::number(i) + "               "));
	}
	i++;
    }
}

void tst_QSqlQuery::record()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(db);
    QVERIFY(q.record().isEmpty());
    QVERIFY2(q.exec("select id, t_varchar, t_char from " + qTableName("qtest") + " order by id"),
	    tst_Databases::printError(q.lastError()));
    QSqlRecord rec = q.record();
    QCOMPARE(q.record().fieldName(0).toLower(), QString("id"));
    QCOMPARE(q.record().fieldName(1).toLower(), QString("t_varchar"));
    QCOMPARE(q.record().fieldName(2).toLower(), QString("t_char"));
    QVERIFY(!q.record().value(0).isValid());
    QVERIFY(!q.record().value(1).isValid());
    QVERIFY(!q.record().value(2).isValid());

    QVERIFY(q.next());
    QVERIFY(q.next());

    QCOMPARE(q.record().fieldName(0).toLower(), QString("id"));
    QCOMPARE(q.value(0).toInt(), 1);
}

void tst_QSqlQuery::isValid()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY(!q.isValid());
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.first());
    QVERIFY(q.isValid());
}

void tst_QSqlQuery::isActive()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY(!q.isActive());
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.isActive());
    QVERIFY(q.last());
    if (!tst_Databases::isMSAccess(db))
	// Access is stupid enough to let you scroll over boundaries
        QVERIFY(!q.next());
    QVERIFY(q.isActive());

    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (41, 'VarChar41', 'Char41')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.isActive());

    QVERIFY2(q.exec("update " + qTableName("qtest") + " set id = 42 where id = 41"),
            tst_Databases::printError(q.lastError()));
    QVERIFY(q.isActive());

    QVERIFY2(q.exec("delete from " + qTableName("qtest") + " where id = 42"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.isActive());
}

void tst_QSqlQuery::numRowsAffected()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QCOMPARE(q.numRowsAffected(), -1);

    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
            tst_Databases::printError(q.lastError()));
    int i = 0;
    while (q.next())
	++i;
    if (q.numRowsAffected() != -1 && q.numRowsAffected() != 0 && q.numRowsAffected() != i) {
	// the value is undefined for SELECT, this check is just here for curiosity
	qDebug("Expected numRowsAffected to be -1, 0 or %d, got %d", i, q.numRowsAffected());
    }

    QVERIFY2(q.exec("update " + qTableName("qtest") + " set id = 100 where id = 1"),
	    tst_Databases::printError(q.lastError()));
    QCOMPARE(q.numRowsAffected(), 1);
    QCOMPARE(q.numRowsAffected(), 1); // yes, we check twice

    QVERIFY2(q.exec("update " + qTableName("qtest") + " set id = id + 100"),
	    tst_Databases::printError(q.lastError()));
    QCOMPARE(q.numRowsAffected(), i);
    QCOMPARE(q.numRowsAffected(), i); // yes, we check twice

    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (42000, 'homer', 'marge')"),
	    tst_Databases::printError(q.lastError()));
    QCOMPARE(q.numRowsAffected(), 1);
    QCOMPARE(q.numRowsAffected(), 1); // yes, we check twice

    QSqlQuery q2(QString::null, db);
    QVERIFY2(q2.exec("insert into " + qTableName("qtest") + " values (42001, 'homer', 'marge')"),
            tst_Databases::printError(q.lastError()));
    if (!db.driverName().startsWith("QSQLITE2")) {
	// SQLite 2.x accumulates changed rows in nested queries. See task 33794
	QCOMPARE(q2.numRowsAffected(), 1);
	QCOMPARE(q2.numRowsAffected(), 1); // yes, we check twice
    }
}

void tst_QSqlQuery::size()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QCOMPARE(q.size(), -1);

    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
            tst_Databases::printError(q.lastError()));
    int i = 0;
    while (q.next())
	++i;
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
	QCOMPARE(q.size(), i);
	QCOMPARE(q.size(), i); // yes, twice
    } else {
	QCOMPARE(q.size(), -1);
	QCOMPARE(q.size(), -1); // yes, twice
    }

    QSqlQuery q2("select * from " + qTableName("qtest"), db);
    if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
	QCOMPARE(q.size(), i);
    } else {
	QCOMPARE(q.size(), -1);
    }
    q2.clear();    

    QVERIFY2(q.exec("update " + qTableName("qtest") + " set id = 100 where id = 1"),
	    tst_Databases::printError(q.lastError()));
    QCOMPARE(q.size(), -1);
    QCOMPARE(q.size(), -1); // yes, twice
}

void tst_QSqlQuery::isSelect()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.isSelect());

    QVERIFY2(q.exec("update " + qTableName("qtest") + " set id = 1 where id = 1"),
            tst_Databases::printError(q.lastError()));
    QVERIFY(q.isSelect() == FALSE);
}

void tst_QSqlQuery::first()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY(q.at() == QSql::BeforeFirstRow);
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.last());
    QVERIFY2(q.first(), tst_Databases::printError(q.lastError()));
    QVERIFY(q.at() == 0);
}

void tst_QSqlQuery::next()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY(q.at() == QSql::BeforeFirstRow);
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.first());
    QVERIFY(q.next());
    QVERIFY(q.at() == 1);
}

void tst_QSqlQuery::prev()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY(q.at() == QSql::BeforeFirstRow);
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.first());
    QVERIFY(q.next());
    QVERIFY(q.previous());
    QVERIFY(q.at() == 0);
}

void tst_QSqlQuery::last()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QCOMPARE(q.at(), int(QSql::BeforeFirstRow));
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    int i = 0;
    while (q.next()) {
	i++;
    }
    QCOMPARE(q.at(), int(QSql::AfterLastRow));
    QVERIFY(q.last());
    if (!tst_Databases::isMSAccess(db))
        // Access doesn't return the correct position
        QCOMPARE(q.at(), (i-1));

    QSqlQuery q2("select * from " + qTableName("qtest"), db);
    QVERIFY(q2.last());
    if (!tst_Databases::isMSAccess(db))
	// Access doesn't return the correct position
        QCOMPARE(q.at(), (i-1));
}

void tst_QSqlQuery::seek()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);
    QSqlQuery q(QString::null, db);
    QVERIFY(q.at() == QSql::BeforeFirstRow);
    QVERIFY2(q.exec(QString("select id from %1 order by id").arg(qTableName("qtest"))),
	     tst_Databases::printError(q.lastError()));

    // NB! The order of the calls below are important!
    QVERIFY(q.last());
    QVERIFY(!q.seek(QSql::BeforeFirstRow));
    QCOMPARE(q.at(), int(QSql::BeforeFirstRow));
    QVERIFY(q.seek(0));
    QCOMPARE(q.at(), 0);
    QCOMPARE(q.value(0).toInt(), 0);

    QVERIFY(q.seek(1));
    QCOMPARE(q.at(), 1);
    QCOMPARE(q.value(0).toInt(), 1);

    QVERIFY(q.seek(3));
    QCOMPARE(q.at(), 3);
    QCOMPARE(q.value(0).toInt(), 3);

    QVERIFY(q.seek(-2, TRUE));
    QCOMPARE(q.at(), 1);
    QVERIFY(q.seek(0));
    QCOMPARE(q.at(), 0);
    QCOMPARE(q.value(0).toInt(), 0);
}

void tst_QSqlQuery::seekForwardOnlyQuery()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    q.setForwardOnly(FALSE);
    QVERIFY(!q.isForwardOnly());

    QVERIFY(q.at() == QSql::BeforeFirstRow);
    QVERIFY2(q.exec(QString("select id from %1 order by id").arg(qTableName("qtest"))),
	     tst_Databases::printError(q.lastError()));

    QSqlRecord rec;

    // NB! The order of the calls below are important!
    QVERIFY(q.seek(0));
    QCOMPARE(q.at(), 0);
    rec = q.record();
    QCOMPARE(rec.value(0).toInt(), 0);

    QVERIFY(q.seek(1));
    QCOMPARE(q.at(), 1);
    rec = q.record();
    QCOMPARE(rec.value(0).toInt(), 1);

    // Make a jump!
    QVERIFY(q.seek(3));
    QCOMPARE(q.at(), 3);
    rec = q.record();
    QCOMPARE(rec.value(0).toInt(), 3);

    // Last record in result set
    QVERIFY(q.seek(4));
    QCOMPARE(q.at(), 4);
    rec = q.record();
    QCOMPARE(rec.value(0).toInt(), 4);
}

// tests the forward only mode;
void tst_QSqlQuery::forwardOnly()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    q.setForwardOnly(TRUE);
    QVERIFY(q.isForwardOnly());
    QVERIFY(q.at() == QSql::BeforeFirstRow);
    QVERIFY2(q.exec("select * from " + qTableName("qtest") + " order by id"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.at() == QSql::BeforeFirstRow);
    QVERIFY(q.first());
    QCOMPARE(q.at(), 0);
    QCOMPARE(q.value(0).toInt(), 0);
    QVERIFY(q.next());
    QCOMPARE(q.at(), 1);
    QCOMPARE(q.value(0).toInt(), 1);
    QVERIFY(q.next());
    QCOMPARE(q.at(), 2);
    QCOMPARE(q.value(0).toInt(), 2);

    // lets make some mistakes to see how robust it is
    QTest::ignoreMessage(QtWarningMsg, "QSqlQuery::seek: cannot seek backwards in a forward only query");
    QVERIFY(q.first() == FALSE);
    QCOMPARE(q.at(), 2);
    QCOMPARE(q.value(0).toInt(), 2);
    QTest::ignoreMessage(QtWarningMsg, "QSqlQuery::seek: cannot seek backwards in a forward only query");
    QVERIFY(q.previous() == FALSE);
    QCOMPARE(q.at(), 2);
    QCOMPARE(q.value(0).toInt(), 2);
    QVERIFY(q.next());
    QCOMPARE(q.at(), 3);
    QCOMPARE(q.value(0).toInt(), 3);

    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    int i = 0;
    while (q.next())
	i++;
    QVERIFY(q.at() == QSql::AfterLastRow);

    QSqlQuery q2 = q;
    QVERIFY(q2.isForwardOnly());

    QVERIFY2(q.exec("select * from " + qTableName("qtest") + " order by id"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.isForwardOnly());
    QVERIFY(q2.isForwardOnly());
    QCOMPARE(q.at(), int(QSql::BeforeFirstRow));
    QVERIFY2(q.seek(3), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.at(), 3);
    QCOMPARE(q.value(0).toInt(), 3);
    QTest::ignoreMessage(QtWarningMsg, "QSqlQuery::seek: cannot seek backwards in a forward only query");
    QVERIFY(q.seek(0) == FALSE);
    QCOMPARE(q.value(0).toInt(), 3);
    QCOMPARE(q.at(), 3);
    QVERIFY(q.last());
    QCOMPARE(q.at(), i-1);
    QTest::ignoreMessage(QtWarningMsg, "QSqlQuery::seek: cannot seek backwards in a forward only query");
    QVERIFY(q.first() == FALSE);
    QCOMPARE(q.at(), i-1);
    QVERIFY(q.next() == FALSE);
    QCOMPARE(q.at(), int(QSql::AfterLastRow));
}

void tst_QSqlQuery::query_exec()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY(!q.isValid());
    QVERIFY(!q.isActive());
    QVERIFY2(q.exec("select * from " + qTableName("qtest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.isActive());
    QVERIFY(q.next());
    QVERIFY(q.isValid());
}

void tst_QSqlQuery::isNull()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("select id, t_varchar from " + qTableName("qtest_null") + " order by id"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QVERIFY(!q.isNull(0));
    QVERIFY(q.isNull(1));
    QCOMPARE(q.value(0).toInt(), 0);
    QCOMPARE(q.value(1).toString(), QString());
    QVERIFY(!q.value(0).isNull());
    QVERIFY(q.value(1).isNull());

    QVERIFY(q.next());
    QVERIFY(!q.isNull(0));
    QVERIFY(!q.isNull(1));
}

/*! TDS specific BIT field test */
void tst_QSqlQuery::bitField()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QTDS"))
	QSKIP("TDS specific test", SkipSingle);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("create table " + qTableName("qtest_bittest") + " (bitty bit)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_bittest") + " values (0)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_bittest") + " values (1)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select bitty from " + qTableName("qtest_bittest")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QVERIFY(q.value(0).toInt() == 0);
    QVERIFY(q.next());
    QVERIFY(q.value(0).toInt() == 1);
    tst_Databases::safeDropTable(db, qTableName("qtest_bittest"));
}


/*! Oracle specific NULL BLOB test */
void tst_QSqlQuery::nullBlob()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QOCI"))
	QSKIP("OCI specific test", SkipSingle);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("create table " + qTableName("qtest_nullblob") + " (id int primary key, bb blob)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_nullblob") + " values (0, EMPTY_BLOB())"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_nullblob") + " values (1, NULL)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_nullblob") + " values (2, 'aabbcc00112233445566')"),
	    tst_Databases::printError(q.lastError()));
    // neccessary otherwise oracle will bombard you with internal errors
    q.setForwardOnly(TRUE);
    QVERIFY2(q.exec("select * from " + qTableName("qtest_nullblob") + " order by id"),
	    tst_Databases::printError(q.lastError()));

    QVERIFY(q.next());
    QCOMPARE((int)q.value(1).toByteArray().size(), 0);
    QVERIFY(!q.isNull(1));

    QVERIFY(q.next());
    QCOMPARE((int)q.value(1).toByteArray().size(), 0);
    QVERIFY(q.isNull(1));

    QVERIFY(q.next());
    QCOMPARE((int)q.value(1).toByteArray().size(), 10);
    QVERIFY(!q.isNull(1));

    tst_Databases::safeDropTable(db, qTableName("qtest_nullblob"));
}

/* Oracle specific RAW field test */
void tst_QSqlQuery::rawField()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QOCI"))
	QSKIP("OCI specific test", SkipSingle);

    QSqlQuery q(QString::null, db);
    q.setForwardOnly(TRUE);
    tst_Databases::safeDropTable(db, qTableName("qtest_rawtest"));
    QVERIFY2(q.exec("create table " + qTableName("qtest_rawtest") +
                    " (id int, col raw(20))"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_rawtest") + " values (0, NULL)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtest_rawtest") +
                    " values (1, '00aa1100ddeeff')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select col from " + qTableName("qtest_rawtest") + " order by id"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QVERIFY(q.isNull(0));
    QCOMPARE((int)q.value(0).toByteArray().size(), 0);
    QVERIFY(q.next());
    QVERIFY(!q.isNull(0));
    QCOMPARE((int)q.value(0).toByteArray().size(), 7);
    tst_Databases::safeDropTable(db, qTableName("qtest_rawtest"));
}

// test whether we can fetch values with more than DOUBLE precision
// note that MySQL's 3.x highest precision is that of a double, although
// you can define field with higher precision
void tst_QSqlQuery::precision()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    static const char* precStr = "1.2345678901234567891";

    if (db.driverName().startsWith("QIBASE"))
	QSKIP("DB unable to store high precision", SkipSingle);

    { // need a new scope for SQLITE
    QSqlQuery q(QString::null, db);
    if (tst_Databases::isMSAccess(db)) {
	QVERIFY2(q.exec("create table " + qTableName("qtest_precision") + " (col1 number)"),
	tst_Databases::printError(q.lastError()));
    } else {
	QVERIFY2(q.exec("create table " + qTableName("qtest_precision") + " (col1 numeric(21, 20))"),
		 tst_Databases::printError(q.lastError()));
    }
    QVERIFY2(q.exec("insert into " + qTableName("qtest_precision") + " (col1) values (1.2345678901234567891)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from " + qTableName("qtest_precision")),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());

    QString val = q.value(0).toString();
    if (!val.startsWith("1.2345678901234567891")) {
	int i = 0;
	while (precStr[i] != 0 && *(precStr + i) == val[i].toLatin1())
	    i++;
	// MySQL and TDS have crappy precisions by default
	if (db.driverName().startsWith("QMYSQL")) {
	    if (i < 17)
		QWARN("MySQL didn't return the right precision");
	} else if (db.driverName().startsWith("QTDS")) {
	    if (i < 18)
		QWARN("TDS didn't return the right precision");
	} else {
	    QWARN(QString(tst_Databases::dbToString(db) + " didn't return the right precision (" +
		  QString::number(i) + " out of 21), " + val).toLatin1());
	}
    }
    } // SQLITE scope
    tst_Databases::safeDropTable(db, qTableName("qtest_precision"));
}


void tst_QSqlQuery::nullResult()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(QString::null, db);
    QVERIFY2(q.exec("select * from " + qTableName("qtest") + " where id > 50000"),
	    tst_Databases::printError(q.lastError()));
    if (q.driver()->hasFeature(QSqlDriver::QuerySize)) {
	QCOMPARE(q.size(), 0);
    }
    QVERIFY(q.next() == FALSE);
    QVERIFY(q.first() == FALSE);
    QVERIFY(q.last() == FALSE);
    QVERIFY(q.previous() == FALSE);
    QVERIFY(q.seek(10) == FALSE);
    QVERIFY(q.seek(0) == FALSE);
}

// this test is just an experiment to see whether we can do query-based transactions
// the real transaction test is in tst_QSqlDatabase
void tst_QSqlQuery::transaction()
{
    // query based transaction is not really possible with Qt
    QSKIP("only tested manually by trained staff", SkipAll);

    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::Transactions)) {
	QSKIP("DBMS not transaction capable", SkipSingle);
    }

    // this is the standard SQL
    QString startTransactionStr("start transaction");
    if (db.driverName().startsWith("QMYSQL"))
	startTransactionStr = "begin work";

    QSqlQuery q(QString::null, db);
    QSqlQuery q2(QString::null, db);

    // test a working transaction
    q.exec(startTransactionStr);
    QVERIFY2(q.exec("insert into" + qTableName("qtest") + " values (40, 'VarChar40', 'Char40')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from" + qTableName("qtest") + " where id = 40"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 40);
    QVERIFY2(q.exec("commit"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from" + qTableName("qtest") + " where id = 40"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 40);

    // test a rollback
    q.exec(startTransactionStr);
    QVERIFY2(q.exec("insert into" + qTableName("qtest") + " values (41, 'VarChar41', 'Char41')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from" + qTableName("qtest") + " where id = 41"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 41);
    if (!q.exec("rollback")) {
	if (db.driverName().startsWith("QMYSQL")) {
	    qDebug("MySQL: " + tst_Databases::printError(q.lastError()));
	    QSKIP("MySQL transaction failed ", SkipSingle); //non-fatal
	} else
	    QFAIL("Could not rollback transaction: " + tst_Databases::printError(q.lastError()));
    }

    QVERIFY2(q.exec("select * from" + qTableName("qtest") + " where id = 41"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next() == FALSE);

    // test concurrent access
    q.exec(startTransactionStr);
    QVERIFY2(q.exec("insert into" + qTableName("qtest") + " values (42, 'VarChar42', 'Char42')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("select * from" + qTableName("qtest") + " where id = 42"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 42);

    QVERIFY2(q2.exec("select * from" + qTableName("qtest") + " where id = 42"),
	    tst_Databases::printError(q2.lastError()));
    if (q2.next())
	qDebug(QString("DBMS '%1' doesn't support query based transactions with concurrent access").arg(
	        tst_Databases::dbToString(db)).toLatin1());

    QVERIFY2(q.exec("commit"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q2.exec("select * from" + qTableName("qtest") + " where id = 42"),
	    tst_Databases::printError(q2.lastError()));
    QVERIFY(q2.next());
    QCOMPARE(q2.value(0).toInt(), 42);
}

void tst_QSqlQuery::joins()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.driverName().startsWith("QOCI")
	|| db.driverName().startsWith("QTDS")
	|| db.driverName().startsWith("QODBC")
	|| db.driverName().startsWith("QIBASE")) {
	// Oracle broken beyond recognition - cannot outer join on more than
	// one table.
	QSKIP("DBMS cannot understand standard SQL", SkipSingle);
	return;
    }

    QSqlQuery q(QString::null, db);
    tst_Databases::safeDropTable(db, qTableName("qtestj1"));
    tst_Databases::safeDropTable(db, qTableName("qtestj2"));
    QVERIFY2(q.exec("create table " + qTableName("qtestj1") + " (id1 int, id2 int)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("create table " + qTableName("qtestj2") + " (id int, name varchar(20))"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtestj1") + " values (1, 1)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtestj1") + " values (1, 2)"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtestj2") + " values(1, 'trenton')"),
	    tst_Databases::printError(q.lastError()));
    QVERIFY2(q.exec("insert into " + qTableName("qtestj2") + " values(2, 'marius')"),
	    tst_Databases::printError(q.lastError()));

    QVERIFY2(q.exec("select qtestj1.id1, qtestj1.id2, qtestj2.id, qtestj2.name, qtestj3.id, qtestj3.name "
		    "from " + qTableName("qtestj1") + " qtestj1 left outer join " + qTableName("qtestj2") +
		    " qtestj2 on (qtestj1.id1 = qtestj2.id) "
		    "left outer join " + qTableName("qtestj2") + " as qtestj3 on (qtestj1.id2 = qtestj3.id)"),
	    tst_Databases::printError(q.lastError()));

    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toInt(), 1);
    QCOMPARE(q.value(2).toInt(), 1);
    QCOMPARE(q.value(3).toString(), QString("trenton"));
    QCOMPARE(q.value(4).toInt(), 1);
    QCOMPARE(q.value(5).toString(), QString("trenton"));

    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toInt(), 2);
    QCOMPARE(q.value(2).toInt(), 1);
    QCOMPARE(q.value(3).toString(), QString("trenton"));
    QCOMPARE(q.value(4).toInt(), 2);
    QCOMPARE(q.value(5).toString(), QString("marius"));

    tst_Databases::safeDropTable(db, "qtestj1");
    tst_Databases::safeDropTable(db, "qtestj2");
}

void tst_QSqlQuery::synonyms()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q("select a.id, a.t_char, a.t_varchar from " + qTableName("qtest") + " a where a.id = 0", db);
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 0);
    QCOMPARE(q.value(1).toString().trimmed(), QString("Char0"));
    QCOMPARE(q.value(2).toString().trimmed(), QString("VarChar0"));

    QSqlRecord rec = q.record();
    QCOMPARE((int)rec.count(), 3);
    QCOMPARE(rec.field(0).name().toLower(), QString("id"));
    QCOMPARE(rec.field(1).name().toLower(), QString("t_char"));
    QCOMPARE(rec.field(2).name().toLower(), QString("t_varchar"));
}

// It doesn't make sense to split this into several tests
void tst_QSqlQuery::prepare_bind_exec()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    { // new scope for SQLITE
    static const unsigned short utf8arr[] = { 0xfb50,0xfb60,0xfb70,0xfb80,0xfbe0,0xfbf0,0x00 };
    static const QString utf8str = QString::fromUtf16(utf8arr);

    static const QString values[6] = { "Harry", "Trond", "Mark", "Ma?rk", "?", ":id" };

    bool useUnicode = db.driver()->hasFeature(QSqlDriver::Unicode);

    QSqlQuery q(QString::null, db);

    QString createQuery;
    if (tst_Databases::isSqlServer(db) || db.driverName().startsWith("QTDS")) {
        createQuery = "create table " + qTableName("qtest_prepare") + " (id int primary key, name nvarchar(20) null)";
    } else if (db.driverName().startsWith("QMYSQL")) {
        createQuery = "create table " + qTableName("qtest_prepare") + " (id int not null primary key, name varchar(20)) default character set utf8";
    } else {
        createQuery = "create table " + qTableName("qtest_prepare") + " (id int not null primary key, name varchar(20))";
    }
    QVERIFY_SQL(q, q.exec(createQuery));

    QVERIFY(q.prepare("insert into " + qTableName("qtest_prepare") + " (id, name) values (:id, :name)"));
    int i;
    for (i = 0; i < 6; ++i) {
	q.bindValue(":name", values[i]);
	q.bindValue(":id", i);
	QVERIFY2(q.exec(),
		tst_Databases::printError(q.lastError()));
	QMap<QString, QVariant> m = q.boundValues();
	QCOMPARE((int) m.count(), 2);
	QCOMPARE(m[":name"].toString(), values[i]);
	QCOMPARE(m[":id"].toInt(), i);
    }

    q.bindValue(":id", 8);
    QVERIFY_SQL(q, q.exec());

    if (useUnicode) {
	q.bindValue(":id", 7);
	q.bindValue(":name", utf8str);
	QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    }

    QVERIFY2(q.exec("SELECT * FROM " + qTableName("qtest_prepare") + " order by id"),
	    tst_Databases::printError(q.lastError()));

    for (i = 0; i < 6; ++i) {
	QVERIFY(q.next());
	QCOMPARE(q.value(0).toInt(), i);
	QCOMPARE(q.value(1).toString().trimmed(), values[ i ]);
    }

    if (useUnicode) {
	QVERIFY2(q.next(), qPrintable(q.lastError().text()));
	QCOMPARE(q.value(0).toInt(), 7);
	QCOMPARE(q.value(1).toString(), utf8str);
    }

    QVERIFY_SQL(q, q.next());
    QCOMPARE(q.value(0).toInt(), 8);
    QCOMPARE(q.value(1).toString(), values[5]);

    QVERIFY(q.prepare("insert into " + qTableName("qtest_prepare") + " (id, name) values (:id, 'Bart')"));
    q.bindValue(":id", 99);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.bindValue(":id", 100);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.exec("select * from " + qTableName("qtest_prepare") + " where id > 98 order by id"));
    for (i = 99; i <= 100; ++i){
	QVERIFY(q.next());
	QCOMPARE(q.value(0).toInt(), i);
	QCOMPARE(q.value(1).toString().trimmed(), QString("Bart"));
    }

    /*** SELECT stuff ***/
    QVERIFY(q.prepare("select * from " + qTableName("qtest_prepare") + " where id = :id"));
    for (i = 0; i < 6; ++i) {
	q.bindValue(":id", i);
	QVERIFY2(q.exec(),
	         tst_Databases::printError(q.lastError()));
	QVERIFY2(q.next(), tst_Databases::printError(q.lastError()));
	QCOMPARE(q.value(0).toInt(), i);
	QCOMPARE(q.value(1).toString().trimmed(), values[ i ]);
	QSqlRecord rInf = q.record();
	QCOMPARE((int)rInf.count(), 2);
	QCOMPARE(rInf.field(0).name().toUpper(), QString("ID"));
	QCOMPARE(rInf.field(1).name().toUpper(), QString("NAME"));
	QVERIFY(!q.next());
    }

    QVERIFY2(q.exec("DELETE FROM " + qTableName("qtest_prepare")),
	    tst_Databases::printError(q.lastError()));

    QVERIFY(q.prepare("insert into " + qTableName("qtest_prepare") + " (id, name) values (?, ?)"));
    q.bindValue(0, 0);
    q.bindValue(1, values[ 0 ]);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.addBindValue(1);
    q.addBindValue(values[ 1 ]);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.addBindValue(2);
    q.addBindValue(values[ 2 ]);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.addBindValue(3);
    q.addBindValue(values[ 3 ]);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.addBindValue(4);
    q.addBindValue(values[ 4 ]);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.bindValue(1, values[ 5 ]);
    q.bindValue(0, 5);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.bindValue(0, 6);
    q.bindValue(1, QVariant(QString::null));
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));

    if (db.driver()->hasFeature(QSqlDriver::Unicode)) {
	q.bindValue(0, 7);
	q.bindValue(1, utf8str);
	QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    }

    QVERIFY2(q.exec("SELECT * FROM " + qTableName("qtest_prepare") + " order by id"),
	    tst_Databases::printError(q.lastError()));
    for (i = 0; i < 6; ++i) {
	QVERIFY(q.next());
	QCOMPARE(q.value(0).toInt(), i);
	QCOMPARE(q.value(1).toString().trimmed(), values[ i ]);
    }

    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 6);
    QVERIFY(q.isNull(1));

    if (useUnicode) {
	QVERIFY(q.next());
	QCOMPARE(q.value(0).toInt(), 7);
	QCOMPARE(q.value(1).toString(), utf8str);
    }

    QVERIFY(q.prepare("insert into " + qTableName("qtest_prepare") + " (id, name) values (?, 'Bart')"));
    q.bindValue(0, 99);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    q.addBindValue(100);
    QVERIFY2(q.exec(),
	    tst_Databases::printError(q.lastError()));
    QVERIFY(q.exec("select * from " + qTableName("qtest_prepare") + " where id > 98 order by id"));
    for (i = 99; i <= 100; ++i){
	QVERIFY(q.next());
	QCOMPARE(q.value(0).toInt(), i);
	QCOMPARE(q.value(1).toString().trimmed(), QString("Bart"));
    }

    /* insert a duplicate id and make sure the db bails out */
    QVERIFY(q.prepare("insert into " + qTableName("qtest_prepare") + " (id, name) values (?, ?)"));
    q.addBindValue(99);
    q.addBindValue("something silly");
    QVERIFY(!q.exec());
    QVERIFY(q.lastError().isValid());
    QVERIFY(!q.isActive());

    } // end of SQLite scope

    tst_Databases::safeDropTable(db, qTableName("qtest_prepare"));
}

void tst_QSqlQuery::prepared_select()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(db);
    QVERIFY2(q.prepare("select a.id, a.t_char, a.t_varchar from " + qTableName("qtest") +
                      " a where a.id = ?"), tst_Databases::printError(q.lastError()));

    q.bindValue(0, 1);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.at(), (int)QSql::BeforeFirstRow);
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);

    q.bindValue(0, 2);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.at(), (int)QSql::BeforeFirstRow);
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 2);

    q.bindValue(0, 3);
    QVERIFY2(q.exec(), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.at(), (int)QSql::BeforeFirstRow);
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 3);

    QVERIFY2(q.prepare("select a.id, a.t_char, a.t_varchar from " + qTableName("qtest") +
                      " a where a.id = ?"), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.at(), (int)QSql::BeforeFirstRow);
    QVERIFY(!q.first());
}

void tst_QSqlQuery::sqlServerLongStrings()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!tst_Databases::isSqlServer(db))
        QSKIP("SQL Server specific test", SkipSingle);

    QSqlQuery q(db);

    QVERIFY2(q.exec("CREATE TABLE " + qTableName("qtest_longstr") +
                   " (id int primary key, longstring ntext)"), q.lastError().text().toLatin1());
    QVERIFY2(q.prepare("INSERT INTO " + qTableName("qtest_longstr") + " VALUES (?, ?)"),
            q.lastError().text().toLatin1());
    q.addBindValue(0);
    q.addBindValue(QString::fromLatin1("bubu"));
    QVERIFY2(q.exec(), q.lastError().text().toLatin1());

    QString testStr;
    testStr.fill(QLatin1Char('a'), 85000);
    q.addBindValue(1);
    q.addBindValue(testStr);
    QVERIFY2(q.exec(), q.lastError().text().toLatin1());

    QVERIFY2(q.exec("select * from " + qTableName("qtest_longstr")), q.lastError().text().toLatin1());

    QVERIFY2(q.next(), q.lastError().text().toLatin1());
    QCOMPARE(q.value(0).toInt(), 0);
    QCOMPARE(q.value(1).toString(), QString::fromLatin1("bubu"));

    QVERIFY2(q.next(), q.lastError().text().toLatin1());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toString(), testStr);
}

void tst_QSqlQuery::invalidQuery()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(db);

    QVERIFY(!q.exec());

    QVERIFY(!q.exec("blahfasel"));
    QVERIFY(q.lastError().type() != QSqlError::NoError);
    QVERIFY(!q.next());
    QVERIFY(!q.isActive());

    if (!db.driverName().startsWith("QOCI")) {
        // oracle just prepares everything without complaining
        if (db.driver()->hasFeature(QSqlDriver::PreparedQueries))
            QVERIFY(!q.prepare("blahfasel"));
    }
    QVERIFY(!q.exec());
    QVERIFY(!q.isActive());
    QVERIFY(!q.next());
}

class ResultHelper: public QSqlResult
{
public:
    ResultHelper(): QSqlResult(0) {} // don't call, it's only for stupid compilers

    bool execBatch(bool bindArray = false)
    { return QSqlResult::execBatch(bindArray); }
};

void tst_QSqlQuery::batchExec()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::BatchOperations))
        QSKIP("Database can't do BatchOperations", SkipSingle);

    QSqlQuery q(db);

    q.exec("drop table " + qTableName("qtest_batch"));

    QVERIFY2(q.exec("create table " + qTableName("qtest_batch") +
                " (id int, name varchar(20), dt date, num numeric(8, 4))"), q.lastError().text().toLocal8Bit());
    QVERIFY2(q.prepare("insert into " + qTableName("qtest_batch") + " (id, name, dt, num) "
                "values (?, ?, ?, ?)"), q.lastError().text().toLocal8Bit());

    QVariantList intCol;
    intCol << 1 << 2 << QVariant(QVariant::Int);

    QVariantList charCol;
    charCol << QLatin1String("harald") << QLatin1String("boris") << QVariant(QVariant::String);

    QVariantList dateCol;
    QDateTime dt = QDateTime(QDate::currentDate(), QTime(1, 2, 3));
    dateCol << dt << dt.addDays(-1) << QVariant(QVariant::DateTime);

    QVariantList numCol;
    numCol << 2.3 << 3.4 << QVariant(QVariant::Double);

    q.addBindValue(intCol);
    q.addBindValue(charCol);
    q.addBindValue(dateCol);
    q.addBindValue(numCol);

    ResultHelper *helper = static_cast<ResultHelper *>(const_cast<QSqlResult *>(q.result()));
    QVERIFY2(helper->execBatch(), q.lastError().text().toLocal8Bit());

    QVERIFY2(q.exec("select id, name, dt, num from " + qTableName("qtest_batch") + " order by id"), q.lastError().text().toLocal8Bit());
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toString(), QString("harald"));
    QCOMPARE(q.value(2).toDateTime(), dt);
    QCOMPARE(q.value(3).toDouble(), 2.3);

    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 2);
    QCOMPARE(q.value(1).toString(), QString("boris"));
    QCOMPARE(q.value(2).toDateTime(), dt.addDays(-1));
    QCOMPARE(q.value(3).toDouble(), 3.4);

    QVERIFY(q.next());
    QVERIFY(q.value(0).isNull());
    QVERIFY(q.value(1).isNull());
    QVERIFY(q.value(2).isNull());
    QVERIFY(q.value(3).isNull());
}

void tst_QSqlQuery::oraArrayBind()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::BatchOperations))
        QSKIP("Database can't do BatchOperations", SkipSingle);

    QSqlQuery q(db);

    QVERIFY2(q.exec("CREATE OR REPLACE PACKAGE ora_array_test "
                      "IS "
                        "TYPE names_type IS TABLE OF VARCHAR(64) NOT NULL INDEX BY BINARY_INTEGER; "
                        "names_tab names_type; "
                        "PROCEDURE set_name(name_in IN VARCHAR2, row_in in INTEGER); "
                        "PROCEDURE get_name(row_in IN INTEGER, str_out OUT VARCHAR2); "
                        "PROCEDURE get_table(tbl OUT names_type); "
                        "PROCEDURE set_table(tbl IN names_type); "
                      "END ora_array_test; "), q.lastError().text().toLocal8Bit());

    QVERIFY2(q.exec("CREATE OR REPLACE PACKAGE BODY ora_array_test "
                     "IS "
                         "PROCEDURE set_name(name_in IN VARCHAR2, row_in in INTEGER) "
                         "IS "
                         "BEGIN "
                             "names_tab(row_in) := name_in; "
                         "END set_name; "

                         "PROCEDURE get_name(row_in IN INTEGER, str_out OUT VARCHAR2) "
                         "IS "
                         "BEGIN "
                            "str_out := names_tab(row_in); "
                         "END get_name; "

                         "PROCEDURE get_table(tbl OUT names_type) "
                         "IS "
                         "BEGIN "
	                     "tbl:=names_tab; "
                         "END get_table; "

                         "PROCEDURE set_table(tbl IN names_type) "
                         "IS "
                         "BEGIN "
	                     "names_tab := tbl; "
                         "END set_table; "
                     "END ora_array_test; "), q.lastError().text().toLocal8Bit());

    QVariantList list;
    list << QString("boris") << QString("and") << QString("harald") << QString("both") << QString("hate") << QString("oracle");

    ResultHelper *r = static_cast<ResultHelper*>(const_cast<QSqlResult*>(q.result()));
    QVERIFY2(q.prepare("BEGIN "
                           "ora_array_test.set_table(?); "
                        "END;"), q.lastError().text().toLocal8Bit());

    q.bindValue(0, list, QSql::In);
    QVERIFY2(r->execBatch(true), q.lastError().text().toLocal8Bit());

    QVERIFY2(q.prepare("BEGIN "
                           "ora_array_test.get_table(?); "
                       "END;"), q.lastError().text().toLocal8Bit());

    list.clear();
    list << QString(64,' ') << QString(64,' ') << QString(64,' ') << QString(64,' ') << QString(64,' ') << QString(64,' ');
    q.bindValue(0, list, QSql::Out);

    QVERIFY2(r->execBatch(true), q.lastError().text().toLocal8Bit());
    QVariantList out_list = q.boundValue(0).toList();

    QCOMPARE(out_list.at(0).toString(), QString("boris"));
    QCOMPARE(out_list.at(1).toString(), QString("and"));
    QCOMPARE(out_list.at(2).toString(), QString("harald"));
    QCOMPARE(out_list.at(3).toString(), QString("both"));
    QCOMPARE(out_list.at(4).toString(), QString("hate"));
    QCOMPARE(out_list.at(5).toString(), QString("oracle"));

    QVERIFY2(q.exec("DROP PACKAGE ora_array_test"), q.lastError().text().toLocal8Bit());
}

/*
    Tests that QSqlDatabase::record and QSqlQuery::record returns the same thing
    otherwise our models get confused.
 */
void tst_QSqlQuery::record_sqlite()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (db.driverName() != "QSQLITE")
        QSKIP("This test requires sqlite support", SkipAll);

    QSqlQuery q(db);

    QVERIFY2(q.exec("create table record_sqlite(id integer primary key, name varchar, title int)"),
             qPrintable(q.lastError().text()));

    QSqlRecord rec = db.record("record_sqlite");

    QCOMPARE(rec.count(), 3);
    QCOMPARE(rec.field(0).type(), QVariant::Int);
    QCOMPARE(rec.field(1).type(), QVariant::String);
    QCOMPARE(rec.field(2).type(), QVariant::Int);

    /* important - select from an empty table */
    QVERIFY2(q.exec("select id, name, title from record_sqlite"), qPrintable(q.lastError().text()));

    rec = q.record();
    QCOMPARE(rec.count(), 3);
    QCOMPARE(rec.field(0).type(), QVariant::Int);
    QCOMPARE(rec.field(1).type(), QVariant::String);
    QCOMPARE(rec.field(2).type(), QVariant::Int);
}

void tst_QSqlQuery::oraLong()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QOCI"))
        QSKIP("This test requires oracle support", SkipAll);

    QSqlQuery q(db);

    QString aLotOfText(127000, QLatin1Char('H'));

    QVERIFY2(q.exec("create table " + qTableName("qtest_longstr") +
                    " (id int primary key, astr long)"), qPrintable(q.lastError().text()));
    QVERIFY2(q.prepare("insert into " + qTableName("qtest_longstr") + " (id, astr) values (?, ?)"),
             qPrintable(q.lastError().text()));
    q.addBindValue(1);
    q.addBindValue(aLotOfText);
    QVERIFY2(q.exec(), qPrintable(q.lastError().text()));

    QVERIFY2(q.exec("select id,astr from " + qTableName("qtest_longstr")),
             qPrintable(q.lastError().text()));

    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toString(), aLotOfText);
}

void tst_QSqlQuery::execErrorRecovery()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(db);

    tst_Databases::safeDropTable(db, qTableName("qtest_exerr"));
    QVERIFY_SQL(q, q.exec("create table " + qTableName("qtest_exerr") + " (id int primary key)"));

    QVERIFY_SQL(q, q.prepare("insert into " + qTableName("qtest_exerr") + " values (?)"));

    q.addBindValue(1);
    QVERIFY_SQL(q, q.exec());

    q.addBindValue(1); // binding the same pkey - should fail
    QVERIFY(!q.exec());

    q.addBindValue(2); // this should work again
    QVERIFY_SQL(q, q.exec());
}

void tst_QSqlQuery::lastInsertId()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driver()->hasFeature(QSqlDriver::LastInsertId))
        QSKIP("Database doesn't support lastInsertId", SkipSingle);

    QSqlQuery q(db);

    QVERIFY2(q.exec("insert into " + qTableName("qtest") + " values (41, 'VarChar41', 'Char41')"),
	    tst_Databases::printError(q.lastError()));

    QVariant v = q.lastInsertId();
    QVERIFY(v.isValid());
}

void tst_QSqlQuery::lastQuery()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlQuery q(db);
    QString sql = "select * from " + qTableName("qtest");
    QVERIFY2(q.exec(sql), tst_Databases::printError(q.lastError()));
    QCOMPARE(q.lastQuery(), sql);
    QCOMPARE(q.executedQuery(), sql);
}

void tst_QSqlQuery::bindWithDoubleColonCastOperator()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    // Only PostgreSQL support the double-colon cast operator
    if (!db.driverName().startsWith("QPSQL")) {
        QSKIP("Test requires PostgreSQL", SkipSingle);
        return;
    }

    QString tablename = qTableName("bindtest");
    QSqlQuery q(db);

    tst_Databases::safeDropTable(db, tablename);
    QVERIFY_SQL(q, q.exec("create table " + tablename + " (id1 int, id2 int, id3 int, fld1 int, fld2 int)"));
    QVERIFY_SQL(q, q.exec("insert into " + tablename + " values (1, 2, 3, 10, 5)"));

    QVERIFY2(q.prepare("select sum((fld1 - fld2)::int) from " + tablename + " where id1 = :myid1 and id2 =:myid2 and id3=:myid3"), qPrintable(q.lastError().text()));
    q.bindValue(":myid1", 1);
    q.bindValue(":myid2", 2);
    q.bindValue(":myid3", 3);
    
    QVERIFY2(q.exec(), qPrintable(q.lastError().text()));
    QVERIFY2(q.next(), qPrintable(q.lastError().text()));
    QCOMPARE(q.executedQuery(), QString("select sum((fld1 - fld2)::int) from " + tablename + " where id1 = 1 and id2 =2 and id3=3"));

    tst_Databases::safeDropTable(db, tablename);
}

/* For task 157397: Using QSqlQuery with an invalid QSqlDatabase
   does not set the last error of the query.
   This test function will output some warnings, that's ok.
*/
void tst_QSqlQuery::queryOnInvalidDatabase()
{
    {
        QTest::ignoreMessage(QtWarningMsg, "QSqlDatabase: INVALID driver not loaded");
        QSqlDatabase db = QSqlDatabase::addDatabase("INVALID", "invalidConnection");
        QVERIFY2(db.lastError().isValid(),
            qPrintable(QString("db.lastError().isValid() should be true!")));

        QTest::ignoreMessage(QtWarningMsg, "QSqlQuery::exec: database not open");
        QSqlQuery query("SELECT 1 AS ID", db);
        QVERIFY2(query.lastError().isValid(),
            qPrintable(QString("query.lastError().isValid() should be true!")));
    }
    QSqlDatabase::removeDatabase("invalidConnection");
    
    {
    QSqlDatabase db = QSqlDatabase::database ("this connection does not exist");
    QTest::ignoreMessage(QtWarningMsg, "QSqlQuery::exec: database not open");
    QSqlQuery query ("SELECT 1 AS ID", db);
    QVERIFY2(query.lastError().isValid(),
        qPrintable(QString("query.lastError().isValid() should be true!")));
    }
}

/* For task 159138: Error on instantiating a sql-query before explicitly
   opening the database. This is something we don't support, so this isn't
   really a bug. However some of the drivers are nice enough to support it.
*/
void tst_QSqlQuery::createQueryOnClosedDatabase()
{
    QFETCH(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    // Only supported by these drivers
    if (!db.driverName().startsWith("QPSQL") 
        && !db.driverName().startsWith("QOCI")
            && !db.driverName().startsWith("QMYSQL") 
            && !db.driverName().startsWith("QDB2")) {
        QSKIP("Test is specific for PostgreSQL, Oracle, MySql and DB2", SkipSingle);
        return;
    }

    db.close();
    QSqlQuery query(db);
    db.open();
    QVERIFY2(query.exec(QString("select * from %1 where id = 0").arg(qTableName("qtest"))),
        qPrintable(query.lastError().text()));

    QVERIFY2(query.next(), qPrintable(query.lastError().text()));
    QCOMPARE(query.value(0).toInt(), 0);
    QCOMPARE(query.value(1).toString().trimmed(), QLatin1String("VarChar0"));
    QCOMPARE(query.value(2).toString().trimmed(), QLatin1String("Char0"));

    db.close();
    QVERIFY2(!query.exec(QString("select * from %1 where id = 0").arg(qTableName("qtest"))),
        qPrintable(QString("This can't happen! The query should not have been executed!")));
}

QTEST_MAIN(tst_QSqlQuery)
#include "tst_qsqlquery.moc"
