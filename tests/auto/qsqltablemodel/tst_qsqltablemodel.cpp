/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include "../qsqldatabase/tst_databases.h"

#include <QtSql>

//TESTED_CLASS=
//TESTED_FILES=sql/gui/qsqltablemodel.h sql/gui/qsqltablemodel.cpp

Q_DECLARE_METATYPE(QModelIndex)

class tst_QSqlTableModel : public QObject
{
    Q_OBJECT

public:
    tst_QSqlTableModel();
    virtual ~tst_QSqlTableModel();


    void dropTestTables();
    void createTestTables();
    void recreateTestTables();
    void repopulateTestTables();

    tst_Databases dbs;

public slots:
    void initTestCase_data();
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:

    void select();
    void submitAll();
    void setRecord();
    void insertRow();
    void insertRecord();
    void insertMultiRecords();
    void removeRow();
    void removeRows();
    void removeInsertedRow();
    void setFilter();
    void setInvalidFilter();

    void emptyTable();
    void tablesAndSchemas();
    void whitespaceInIdentifiers();
    void primaryKeyOrder();

    void sqlite_bigTable();
};

tst_QSqlTableModel::tst_QSqlTableModel()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    dbs.open();
}

tst_QSqlTableModel::~tst_QSqlTableModel()
{
}

static bool testWhiteSpaceNames(const QString &name)
{
    return name.startsWith("QPSQL");
}

void tst_QSqlTableModel::dropTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        tst_Databases::safeDropTable(db, qTableName("test"));
        tst_Databases::safeDropTable(db, qTableName("test2"));
        tst_Databases::safeDropTable(db, qTableName("emptytable"));

        if (db.driverName().startsWith("QPSQL")) {
            q.exec("DROP SCHEMA " + qTableName("testschema") + " CASCADE");
        }
        if (testWhiteSpaceNames(db.driverName()))
            q.exec("DROP TABLE \"" + qTableName("qtestw") + " hitespace\"");
    }
}

void tst_QSqlTableModel::createTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        QVERIFY2(q.exec("create table " + qTableName("test")
                       + "(id int primary key, name varchar(20), title int)"),
                q.lastError().text().toLatin1());

        QVERIFY2(q.exec("create table " + qTableName("test2")
                       + "(id int primary key, title varchar(20))"),
                q.lastError().text().toLatin1());

        QVERIFY2(q.exec("create table " + qTableName("emptytable")
                       + "(id int primary key)"),
                q.lastError().text().toLatin1());

        if (testWhiteSpaceNames(db.driverName())) {
            QString qry = "create table \"" + qTableName("qtestw") + " hitespace\" (\"a field\" int)";
            QVERIFY2(q.exec(qry), q.lastError().text().toLatin1());
        }
    }
}

void tst_QSqlTableModel::repopulateTestTables()
{
    for (int i = 0; i < dbs.dbNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(dbs.dbNames.at(i));
        QSqlQuery q(db);

        QVERIFY2(q.exec("delete from " + qTableName("test")), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test")
                       + " values(1, 'harry', 1)"), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test")
                       + " values(2, 'trond', 2)"), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test")
                       + " values(3, 'vohi', 3)"), q.lastError().text().toLatin1());

        QVERIFY2(q.exec("delete from " + qTableName("test2")), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test2")
                       + " values(1, 'herr')"), q.lastError().text().toLatin1());
        QVERIFY2(q.exec("insert into " + qTableName("test2")
                       + " values(2, 'mister')"), q.lastError().text().toLatin1());
    }
}

void tst_QSqlTableModel::recreateTestTables()
{
    dropTestTables();
    createTestTables();
    repopulateTestTables();
}

void tst_QSqlTableModel::initTestCase_data()
{
    if (dbs.fillTestTable() == 0) {
        qWarning("NO DATABASES");
        QSKIP("No database drivers are available in this Qt configuration", SkipAll);
    }
}

void tst_QSqlTableModel::initTestCase()
{
    recreateTestTables();
}

void tst_QSqlTableModel::cleanupTestCase()
{
    dropTestTables();
    dbs.close();
}

void tst_QSqlTableModel::init()
{
}

void tst_QSqlTableModel::cleanup()
{
    repopulateTestTables();
}

void tst_QSqlTableModel::select()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.columnCount(), 3);

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(0, 2)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 3)), QVariant());

    QCOMPARE(model.data(model.index(1, 0)).toInt(), 2);
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));
    QCOMPARE(model.data(model.index(1, 2)).toInt(), 2);
    QCOMPARE(model.data(model.index(1, 3)), QVariant());

    QCOMPARE(model.data(model.index(2, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(2, 2)).toInt(), 3);
    QCOMPARE(model.data(model.index(2, 3)), QVariant());

    QCOMPARE(model.data(model.index(3, 0)), QVariant());
    QCOMPARE(model.data(model.index(3, 1)), QVariant());
    QCOMPARE(model.data(model.index(3, 2)), QVariant());
    QCOMPARE(model.data(model.index(3, 3)), QVariant());
}

void tst_QSqlTableModel::setRecord()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    for (int i = 0; i < model.rowCount(); ++i) {
        QSignalSpy spy(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

        QSqlRecord rec = model.record(i);
        rec.setValue(1, rec.value(1).toString() + "X");
        QVERIFY(model.setRecord(i, rec));

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).count(), 2);
        QCOMPARE(qvariant_cast<QModelIndex>(spy.at(0).at(0)), model.index(i, 1));
        QCOMPARE(qvariant_cast<QModelIndex>(spy.at(0).at(1)), model.index(i, 1));
    }

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harryX"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trondX"));
    QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohiX"));
}

void tst_QSqlTableModel::insertRow()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setEditStrategy(QSqlTableModel::OnRowChange);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QVERIFY(model.insertRow(2));
    QSqlRecord rec = model.record(1);
    rec.setValue(0, 42);
    rec.setValue(1, QString("vohi"));
    QVERIFY(model.setRecord(2, rec));

    QCOMPARE(model.data(model.index(2, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(2, 2)).toInt(), 2);

    QVERIFY(model.submitAll());
}

void tst_QSqlTableModel::insertRecord()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QSqlRecord rec = model.record();
    rec.setValue(0, 42);
    rec.setValue(1, QString("vohi"));
    rec.setValue(2, 1);
    QVERIFY(model.insertRecord(1, rec));
    QCOMPARE(model.rowCount(), 4);

    QCOMPARE(model.data(model.index(1, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(1, 2)).toInt(), 1);

    model.revertAll();
    model.setEditStrategy(QSqlTableModel::OnRowChange);

    QVERIFY(model.insertRecord(-1, rec));

    QCOMPARE(model.data(model.index(3, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(3, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(3, 2)).toInt(), 1);
}

void tst_QSqlTableModel::insertMultiRecords()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.insertRow(2));

    QCOMPARE(model.data(model.index(2, 0)), QVariant());
    QCOMPARE(model.data(model.index(2, 1)), QVariant());
    QCOMPARE(model.data(model.index(2, 2)), QVariant());

    QVERIFY(model.insertRow(3));
    QVERIFY(model.insertRow(0));

    QCOMPARE(model.data(model.index(5, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(5, 1)).toString(), QString("vohi"));
    QCOMPARE(model.data(model.index(5, 2)).toInt(), 3);

    QVERIFY(model.setData(model.index(0, 0), QVariant(42)));
    QVERIFY(model.setData(model.index(3, 0), QVariant(43)));
    QVERIFY(model.setData(model.index(4, 0), QVariant(44)));
    QVERIFY(model.setData(model.index(4, 1), QVariant(QLatin1String("gunnar"))));
    QVERIFY(model.setData(model.index(4, 2), QVariant(1)));

    QVERIFY(model.submitAll());
    model.clear();
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0)).toInt(), 2);
    QCOMPARE(model.data(model.index(2, 0)).toInt(), 3);
    QCOMPARE(model.data(model.index(3, 0)).toInt(), 42);
    QCOMPARE(model.data(model.index(4, 0)).toInt(), 43);
    QCOMPARE(model.data(model.index(5, 0)).toInt(), 44);
    QCOMPARE(model.data(model.index(5, 1)).toString(), QString("gunnar"));
    QCOMPARE(model.data(model.index(5, 2)).toInt(), 1);
}

void tst_QSqlTableModel::submitAll()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QVERIFY(model.setData(model.index(0, 1), "harry2", Qt::EditRole));
    QVERIFY(model.setData(model.index(1, 1), "trond2", Qt::EditRole));

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry2"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond2"));

    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry2"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond2"));

    QVERIFY(model.setData(model.index(0, 1), "harry", Qt::EditRole));
    QVERIFY(model.setData(model.index(1, 1), "trond", Qt::EditRole));

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));

    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));
}

void tst_QSqlTableModel::removeRow()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.removeRow(1));
    QVERIFY(model.submitAll());
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(1, 0)).toInt(), 3);
    model.clear();

    recreateTestTables();

    model.setTable(qTableName("test"));
    model.setEditStrategy(QSqlTableModel::OnRowChange);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.removeRow(1));
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QString("vohi"));
}

void tst_QSqlTableModel::removeRows()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setSort(0, Qt::AscendingOrder);
    model.setEditStrategy(QSqlTableModel::OnFieldChange);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);

    QVERIFY2(model.removeRows(0, 2), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("vohi"));
    model.clear();

    recreateTestTables();

    model.setTable(qTableName("test"));
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.removeRows(0, 2,QModelIndex()));
    QCOMPARE(model.rowCount(), 3);

    QVERIFY(model.submitAll());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QString("vohi"));
}

void tst_QSqlTableModel::removeInsertedRow()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    for (int i = 0; i <= 1; ++i) {

        QSqlTableModel model(0, db);
        model.setTable(qTableName("test"));

        model.setEditStrategy(i == 0
                ? QSqlTableModel::OnRowChange : QSqlTableModel::OnManualSubmit);
        QVERIFY2(model.select(), model.lastError().text().toLatin1());
        QCOMPARE(model.rowCount(), 3);

        QVERIFY(model.insertRow(1));
        QCOMPARE(model.rowCount(), 4);

        QVERIFY(model.removeRow(1));
        QCOMPARE(model.rowCount(), 3);

        QCOMPARE(model.data(model.index(0, 1)).toString(), QString("harry"));
        QCOMPARE(model.data(model.index(1, 1)).toString(), QString("trond"));
        QCOMPARE(model.data(model.index(2, 1)).toString(), QString("vohi"));
        model.clear();

        recreateTestTables();
    }
}

void tst_QSqlTableModel::emptyTable()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);

    model.setTable(qTableName("emptytable"));
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 1);

    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 1);
}

void tst_QSqlTableModel::tablesAndSchemas()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("PQSQL"))
        QSKIP("Postgres specific test", SkipSingle);

    QSqlQuery q(db);
    q.exec("DROP SCHEMA " + qTableName("testschema") + " CASCADE");
    QVERIFY2(q.exec("create schema " + qTableName("testschema")), q.lastError().text().toLatin1());
    QString tableName = qTableName("testschema") + "." + qTableName("testtable");
    QVERIFY2(q.exec("create table " + tableName + "(id int)"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + tableName + " values(1)"), q.lastError().text().toLatin1());
    QVERIFY2(q.exec("insert into " + tableName + " values(2)"), q.lastError().text().toLatin1());

    QSqlTableModel model(0, db);
    model.setTable(tableName);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.columnCount(), 1);
}

void tst_QSqlTableModel::whitespaceInIdentifiers()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!testWhiteSpaceNames(db.driverName()))
        QSKIP("DBMS doesn't support whitespaces in identifiers", SkipSingle);

    QString tableName = qTableName("qtestw") + " hitespace";

    QSqlTableModel model(0, db);
    model.setTable(tableName);
    QVERIFY2(model.select(), model.lastError().text().toLatin1());
}

void tst_QSqlTableModel::primaryKeyOrder()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "primaryKeyOrderTest");
    db.setDatabaseName(":memory:");
    QVERIFY2(db.open(), db.lastError().text().toLatin1());

    QSqlQuery q(db);

    QVERIFY2(q.exec("create table foo(a varchar(20), id int primary key, b varchar(20))"),
            q.lastError().text().toLatin1());

    QSqlTableModel model(0, db);
    model.setTable("foo");

    QSqlIndex pk = model.primaryKey();
    QCOMPARE(pk.count(), 1);
    QCOMPARE(pk.fieldName(0), QString::fromLatin1("id"));

    QVERIFY(model.insertRow(0));
    QVERIFY(model.setData(model.index(0, 0), "hello"));
    QVERIFY(model.setData(model.index(0, 1), 42));
    QVERIFY(model.setData(model.index(0, 2), "blah"));
    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QVERIFY(model.setData(model.index(0, 1), 43));
    QVERIFY2(model.submitAll(), model.lastError().text().toLatin1());

    QCOMPARE(model.data(model.index(0, 1)).toInt(), 43);
}

void tst_QSqlTableModel::setInvalidFilter()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    // set an invalid filter, make sure it fails
    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setFilter("blahfahsel");

    QCOMPARE(model.filter(), QString("blahfahsel"));
    QVERIFY(!model.select());

    // set a valid filter later, make sure if passes
    model.setFilter("id = 1");
    QVERIFY2(model.select(), qPrintable(model.lastError().text()));
}

void tst_QSqlTableModel::setFilter()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    QSqlTableModel model(0, db);
    model.setTable(qTableName("test"));
    model.setFilter("id = 1");
    QCOMPARE(model.filter(), QString("id = 1"));
    QVERIFY2(model.select(), model.lastError().text().toLatin1());

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);

    QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QSignalSpy rowsAboutToBeRemovedSpy(&model,
            SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy rowsAboutToBeInsertedSpy(&model,
            SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    model.setFilter("id = 2");

    // check the signals
    QCOMPARE(rowsAboutToBeRemovedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsAboutToBeInsertedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QList<QVariant> args = rowsAboutToBeRemovedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);
    args = rowsRemovedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);
    args = rowsInsertedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);
    args = rowsAboutToBeInsertedSpy.takeFirst();
    QCOMPARE(args.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), QModelIndex());
    QCOMPARE(args.at(1).toInt(), 0);
    QCOMPARE(args.at(2).toInt(), 0);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 2);
}

void tst_QSqlTableModel::sqlite_bigTable()
{
    QFETCH_GLOBAL(QString, dbName);
    QSqlDatabase db = QSqlDatabase::database(dbName);
    CHECK_DATABASE(db);

    if (!db.driverName().startsWith("QSQLITE"))
        QSKIP("SQLite specific test", SkipSingle);

    QSqlQuery q(db);
    QVERIFY_SQL(q, q.exec("create table foo(id int primary key, name varchar)"));
    QVERIFY_SQL(q, q.prepare("insert into foo(id, name) values (?, ?)"));
    for (int i = 0; i < 10000; ++i) {
        q.addBindValue(i);
        q.addBindValue(QString::number(i));
        QVERIFY_SQL(q, q.exec());
    }
    q.clear();

    QSqlTableModel model(0, db);
    model.setTable("foo");
    QVERIFY_SQL(model, model.select());

    QSqlRecord rec = model.record();
    rec.setValue("id", 424242);
    rec.setValue("name", "Guillaume");
    QVERIFY_SQL(model, model.insertRecord(-1, rec));

    model.clear();

    QVERIFY_SQL(q, q.exec("drop table foo"));
}

QTEST_MAIN(tst_QSqlTableModel)
#include "tst_qsqltablemodel.moc"
