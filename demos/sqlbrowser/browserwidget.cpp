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

#include <qtableview.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlmodel.h>
#include <qsqlquery.h>
#include <qsplitter.h>
#include <qtextedit.h>
#include <qvbox.h>

#include "connectionwidget.h"
#include "qsqlconnectiondialog.h"

BrowserWidget::BrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBox *box = new QHBox(this);
    box->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QSplitter *splitter = new QSplitter(box);
    dbc = new ConnectionWidget(splitter);

    QVBox *vbox = new QVBox(splitter);
    QSplitter *splitter2 = new QSplitter(Qt::Vertical, vbox);
    view = new QTableView(splitter2);
    edit = new QTextEdit(splitter2);

    QHBox *hbox = new QHBox(this);
    hbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    submitButton = new QPushButton(tr("&Submit"), hbox);
    connect(submitButton, SIGNAL(clicked()), SLOT(exec()));

    layout->addWidget(box);
    layout->addWidget(hbox);

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
    QSqlModel *model = new QSqlModel(view);
    model->setQuery(QSqlQuery(edit->plainText(), dbc->currentDatabase()));
    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    view->setModel(model);
}

void BrowserWidget::addConnection()
{
/*
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();
    QSqlQuery q("create table foo(int id)");
*/
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

