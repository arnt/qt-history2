#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

/*
    This file defines a helper function to open a connection to an
    in-memory SQLITE database and to create a test table.

    If you want to use another database, simply modify the code
    below. All the examples in this directory use this function to
    connect to a database.
*/
static bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        qSystemWarning("Cannot open database: %s",
                       db.lastError().text().ascii());
        return false;
    }

    QSqlQuery query;
    query.exec("create table person (id int primary key, "
               "firstname varchar(20), lastname varchar(20))");
    query.exec("insert into person values(1, 'Danny', 'Young')");
    query.exec("insert into person values(2, 'Peter', 'Holand')");
    query.exec("insert into person values(3, 'Chris', 'Dangerous')");
    return true;
}

#endif
