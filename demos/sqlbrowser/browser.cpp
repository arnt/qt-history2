/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "browser.h"
#include "qsqlconnectiondialog.h"

#include <QtGui>
#include <QtSql>

Browser::Browser(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    if (QSqlDatabase::drivers().isEmpty())
        QMessageBox::information(this, tr("No database drivers found"),
                                 tr("This demo requires at least one Qt database driver. "
                                    "Please check the documentation how to build the "
                                    "Qt SQL plugins."));
    else {
        QMetaObject::invokeMethod(this, "addConnection", Qt::QueuedConnection);
    }

    emit statusMessage(tr("Ready."));
}

Browser::~Browser()
{
}

void Browser::exec()
{
    QSqlQueryModel *model = new QSqlQueryModel(table);
    model->setQuery(QSqlQuery(sqlEdit->toPlainText(), connectionWidget->currentDatabase()));
    table->setModel(model);

    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    else if (model->query().isSelect())
        emit statusMessage(tr("Query OK."));
    else
        emit statusMessage(tr("Query OK, number of affected rows: %1").arg(
                    model->query().numRowsAffected()));
}

void Browser::addConnection()
{
    QSqlConnectionDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (dialog.useInMemoryDatabase()) {
        QSqlDatabase::database("in_mem_db", false).close();
        QSqlDatabase::removeDatabase("in_mem_db");
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "in_mem_db");
        db.setDatabaseName(":memory:");
        if (!db.open())
            QMessageBox::warning(this, tr("Unable to open database"), tr("An error occured while "
                                                                         "opening the connection: ") + db.lastError().text());
        QSqlQuery q("", db);
        q.exec("drop table Movies");
        q.exec("drop table Names");
        q.exec("create table Movies (id int primary key, Title varchar(50), Director varchar(20), Rating number(2,2))");
        q.exec("insert into Movies values (0, 'Metropolis', 'Fritz Lang', '8.4')");
        q.exec("insert into Movies values (1, 'Nosferatu, eine Symphonie des Grauens', 'F.W. Murnau', '8.1')");
        q.exec("insert into Movies values (2, 'Bis ans Ende der Welt', 'Wim Wenders', '6.5')");
        q.exec("insert into Movies values (3, 'Hardware', 'Richard Stanley', '5.2')");
        q.exec("insert into Movies values (4, 'Mitchell', 'Andrew V. McLaglen', '2.1')");
        q.exec("create table Names (id int primary key, Firstname varchar(20), Lastname varchar(20), City varchar(20))");
        q.exec("insert into Names values (0, 'Sala', 'Palmer', 'Morristown')");
        q.exec("insert into Names values (1, 'Christopher', 'Walker', 'Morristown')");
        q.exec("insert into Names values (2, 'Donald', 'Duck', 'Andeby')");
        q.exec("insert into Names values (3, 'Buck', 'Rogers', 'Paris')");
        q.exec("insert into Names values (4, 'Sherlock', 'Holmes', 'London')");
    } else {
        static int cCount = 0;
        QSqlDatabase db = QSqlDatabase::addDatabase(dialog.driverName(),
                                                    QString("Browser%1").arg(++cCount));
        db.setDatabaseName(dialog.databaseName());
        db.setHostName(dialog.hostName());
        db.setUserName(dialog.userName());
        db.setPassword(dialog.password());
        db.setPort(dialog.port());
        if (!db.open())
            QMessageBox::warning(this, tr("Unable to open database"), tr("An error occured while "
                                                                         "opening the connection: ") + db.lastError().text());
    }
    connectionWidget->refresh();
}

void Browser::showTable(const QString &t)
{
    QSqlTableModel *model = new QSqlTableModel(table, connectionWidget->currentDatabase());
    model->setTable(t);
    model->select();
    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    table->setModel(model);
}

