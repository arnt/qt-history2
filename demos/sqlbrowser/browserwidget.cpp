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

#include "browserwidget.h"
#include "connectionwidget.h"
#include "qsqlconnectiondialog.h"

#include <QtGui>
#include <QtSql>

BrowserWidget::BrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    QHBoxWidget *box = new QHBoxWidget(splitter);
    QSplitter *splitter2 = new QSplitter(Qt::Horizontal, box);

    dbc = new ConnectionWidget(splitter2);
    connect(dbc, SIGNAL(tableActivated(QString)), SLOT(showTable(QString)));
    view = new QTableView(splitter2);

    QGroupBox *gbox = new QGroupBox(tr("SQL Query"), splitter);
    QVBoxLayout *boxLayout = new QVBoxLayout(gbox);

    edit = new QTextEdit(gbox);

    QHBoxWidget *hbox = new QHBoxWidget(gbox);
    hbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    submitButton = new QPushButton(tr("&Submit"), hbox);
    connect(submitButton, SIGNAL(clicked()), SLOT(exec()));

    boxLayout->addWidget(edit);
    boxLayout->addWidget(hbox);

    layout->addWidget(splitter);
//    layout->addWidget(hbox);

    if (QSqlDatabase::drivers().isEmpty())
        QMessageBox::information(this, tr("No database drivers found"),
                                 tr("This demo requires at least one Qt database driver. "
                                    "Please check the documentation how to build the "
                                    "Qt SQL plugins."));
    else
        addConnection();

    emit statusMessage(tr("Ready."));
}

BrowserWidget::~BrowserWidget()
{
}

void BrowserWidget::exec()
{
    QSqlQueryModel *model = new QSqlQueryModel(view);
    model->setQuery(QSqlQuery(edit->toPlainText(), dbc->currentDatabase()));
    view->setModel(model);

    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    else if (model->query().isSelect())
        emit statusMessage(tr("Query OK."));
    else
        emit statusMessage(tr("Query OK, number of affected rows: %1").arg(
                    model->query().numRowsAffected()));
}

void BrowserWidget::addConnection()
{
    QSqlConnectionDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
        return;

    static int cCount = 0;
    QSqlDatabase db = QSqlDatabase::addDatabase(dialog.driverName(),
                                                QString("BrowserWidget%1").arg(++cCount));
    db.setDatabaseName(dialog.databaseName());
    db.setHostName(dialog.hostName());
    db.setUserName(dialog.userName());
    db.setPassword(dialog.password());
    db.setPort(dialog.port());
    if (!db.open())
        QMessageBox::warning(this, tr("Unable to open database"), tr("An error occured while "
                             "opening the connection: ") + db.lastError().text());

    dbc->refresh();
}

void BrowserWidget::showTable(const QString &table)
{
    QSqlTableModel *model = new QSqlTableModel(view, dbc->currentDatabase());
    model->setTable(table);
    model->select();
    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    view->setModel(model);
}

