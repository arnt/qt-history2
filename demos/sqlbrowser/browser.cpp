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
    else
        QMetaObject::invokeMember(this, "addConnection", Qt::QueuedConnection);

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
    QSqlConnectionDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
        return;

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

