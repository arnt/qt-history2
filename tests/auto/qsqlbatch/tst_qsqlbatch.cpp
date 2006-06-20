/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if QT_VERSION < 0x040200
QTEST_NOOP_MAIN
#else

#include <QtSql/QtSql>

//TESTED_FILES=

class tst_QSqlBatch: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();

    void batchExec();
    void oraArrayBind();

    void cleanupTestCase();
};

void tst_QSqlBatch::initTestCase()
{
    if (!QSqlDatabase::drivers().contains("QOCI"))
        QSKIP("This test requires the QOCI sql driver", SkipAll);

    QSqlDatabase db = QSqlDatabase::addDatabase("QOCI");
    db.setUserName("scott");
    db.setPassword("tiger");
    db.setDatabaseName("pony");

    QVERIFY2(db.open(), qPrintable(db.lastError().text()));
}

void tst_QSqlBatch::cleanupTestCase()
{
    QSqlDatabase::database().close();
}

void tst_QSqlBatch::batchExec()
{
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isValid() || !db.driver()->hasFeature(QSqlDriver::BatchOperations))
        QSKIP("Database can't do BatchOperations", SkipAll);

    QSqlQuery q(db);

    q.exec("drop table qtest_batch");

    QVERIFY2(q.exec("create table qtest_batch (id int, name varchar(20), dt date, "
                "num numeric(8, 4))"), qPrintable(q.lastError().text()));
    QVERIFY2(q.prepare("insert into qtest_batch (id, name, dt, num) "
                "values (?, ?, ?, ?)"), qPrintable(q.lastError().text()));

    QDateTime dt = QDateTime(QDate::currentDate(), QTime(1, 2, 3));
    int i;
    for (i = 0; i < 10; ++i) {

        QVariantList intCol;
        intCol << 1 << 2 << QVariant(QVariant::Int);

        QVariantList charCol;
        charCol << QLatin1String("harald") << QLatin1String("boris") << QVariant(QVariant::String);

        QVariantList dateCol;
        dateCol << dt << dt.addDays(-1) << QVariant(QVariant::DateTime);

        QVariantList numCol;
        numCol << 2.3 << 3.4 << QVariant(QVariant::Double);

        q.addBindValue(intCol);
        q.addBindValue(charCol);
        q.addBindValue(dateCol);
        q.addBindValue(numCol);

        QVERIFY2(q.execBatch(), qPrintable(q.lastError().text()));
    }

    QVERIFY2(q.exec("select id, name, dt, num from qtest_batch order by id"),
             qPrintable(q.lastError().text()));

    for (i = 0; i < 10; ++i) {
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 1);
        QCOMPARE(q.value(1).toString(), QString("harald"));
        QCOMPARE(q.value(2).toDateTime(), dt);
        QCOMPARE(q.value(3).toDouble(), 2.3);
    }

    for (i = 0; i < 10; ++i) {
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 2);
        QCOMPARE(q.value(1).toString(), QString("boris"));
        QCOMPARE(q.value(2).toDateTime(), dt.addDays(-1));
        QCOMPARE(q.value(3).toDouble(), 3.4);
    }

    for (i = 0; i < 10; ++i) {
        QVERIFY(q.next());
        QVERIFY(q.value(0).isNull());
        QVERIFY(q.value(1).isNull());
        QVERIFY(q.value(2).isNull());
        QVERIFY(q.value(3).isNull());
    }
    QVERIFY(!q.next());
}

void tst_QSqlBatch::oraArrayBind()
{
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isValid() || !db.driver()->hasFeature(QSqlDriver::BatchOperations))
        QSKIP("Database can't do BatchOperations", SkipSingle);

    QSqlQuery q(db);

    QVERIFY2(q.exec("CREATE OR REPLACE PACKAGE ora_array_test "
                      "IS "
                        "TYPE names_type IS TABLE OF VARCHAR(64) NOT NULL INDEX BY BINARY_INTEGER; "
                        "names_tab names_type; "
                        "PROCEDURE set_name(name_in IN VARCHAR2, row_in in INTEGER); "
                        "PROCEDURE get_name(row_in IN INTEGER, str_out OUT VARCHAR2); "
                        "PROCEDURE get_table(tbl OUT names_type, dummy OUT INTEGER); "
                        "PROCEDURE set_table(tbl IN names_type); "
                      "END ora_array_test; "), qPrintable(q.lastError().text()));

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

                         "PROCEDURE get_table(tbl OUT names_type, dummy OUT INTEGER) "
                         "IS "
                         "BEGIN "
	                     "tbl:=names_tab; "
                             "dummy:=43; "
                         "END get_table; "

                         "PROCEDURE set_table(tbl IN names_type) "
                         "IS "
                         "BEGIN "
	                     "names_tab := tbl; "
                         "END set_table; "
                     "END ora_array_test; "), qPrintable(q.lastError().text()));

    QVariantList list;
    list << QString("boris") << QString("and") << QString("harald") << QString("both") << QString("like") << QString("oracle");

    QVERIFY2(q.prepare("BEGIN "
                           "ora_array_test.set_table(?); "
                        "END;"), qPrintable(q.lastError().text()));

    q.bindValue(0, list, QSql::In);
    QVERIFY2(q.execBatch(QSqlQuery::ValuesAsColumns), qPrintable(q.lastError().text()));

    QVERIFY2(q.prepare("BEGIN "
                           "ora_array_test.get_table(?, ?); "
                       "END;"), qPrintable(q.lastError().text()));

    // execute ten times
    for (int i = 0; i < 10; ++i) {

        list.clear();
        list << QString(64,' ') << QString(64,' ') << QString(64,' ') << QString(64,' ') << QString(64,' ') << QString(64,' ');
        q.bindValue(0, list, QSql::Out);
        q.bindValue(1, 42, QSql::Out);

        QVERIFY2(q.execBatch(QSqlQuery::ValuesAsColumns), qPrintable(q.lastError().text()));
        QVariantList out_list = q.boundValue(0).toList();

        QCOMPARE(out_list.size(), 6);
        QCOMPARE(out_list.at(0).toString(), QString("boris"));
        QCOMPARE(out_list.at(1).toString(), QString("and"));
        QCOMPARE(out_list.at(2).toString(), QString("harald"));
        QCOMPARE(out_list.at(3).toString(), QString("both"));
        QCOMPARE(out_list.at(4).toString(), QString("like"));
        QCOMPARE(out_list.at(5).toString(), QString("oracle"));

        QCOMPARE(q.boundValue(1).toInt(), 43);
    }

    QVERIFY2(q.exec("DROP PACKAGE ora_array_test"), qPrintable(q.lastError().text()));
}

QTEST_MAIN(tst_QSqlBatch)
#include "tst_qsqlbatch.moc"

#endif // QT 4.2.0
