/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#ifndef CONNECTION_H
#define CONNECTION_H

#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlquery.h>

/* This file defines a helper function to open a connection to
   an in-memory SQLITE database and to create some test tables.

   If you want to use another database, adjust the values below.
 */

static void createConnection()
{
    // create the database connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        qWarning("Error opening the SQLITE Qt driver: %s", db.lastError().text().ascii());
        return;
    }

    // create and populate a test table
    QSqlQuery q;
    q.exec("create table persons (id int primary key, firstname varchar(20), "
            "lastname varchar(20))");
    q.exec("insert into persons values(1, 'Arthur', 'Tulip')");
    q.exec("insert into persons values(2, 'Scribe', 'Dent')");
    q.exec("insert into persons values(3, 'Peter', 'Arthurson')");
}

#endif

