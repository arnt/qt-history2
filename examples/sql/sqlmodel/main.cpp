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

/* This examples demonstrates how to use QSqlModel to display
   a database table. The QSQLITE driver is required for this example.
 */

#include <qapplication.h>
#include <qgenerictableview.h>
#include <qsqldatabase.h>
#include <qsqlmodel.h>
#include <qsqlquery.h>

static void createConnection()
{
    // create the database connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        qWarning("This example requires the QSQLITE driver");
        return;
    }

    // create and populate a test table
    QSqlQuery q;
    q.exec("create table persons (id int primary key, firstname varchar(20), "
           "lastname varchar(20)");
    q.exec("insert into persons values(1, 'Arthur', 'Tulip')");
    q.exec("insert into persons values(2, 'Scribe', 'Dent')");
    q.exec("insert into persons values(3, 'Peter', 'Arthurson')");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();

    QSqlModel *model = new QSqlModel(&app);
    model->setQuery("select * from persons");

    QGenericTableView view(model, 0);
    app.setMainWidget(&view);
    view.show();

    return app.exec();
}

