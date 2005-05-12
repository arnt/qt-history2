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

#include "qsqlconnectiondialog.h"
#include "ui_qsqlconnectiondialog.h"

#include <qsqldatabase.h>

QSqlConnectionDialog::QSqlConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    QStringList drivers(QSqlDatabase::drivers());
    if (!drivers.contains("QSQLITE"))
        ui.dbCheckBox->setEnabled(false);

    ui.comboDriver->addItems(drivers);
}

QSqlConnectionDialog::~QSqlConnectionDialog()
{
}

QString QSqlConnectionDialog::driverName() const
{
    return ui.comboDriver->currentText();
}

QString QSqlConnectionDialog::databaseName() const
{
    return ui.editDatabase->text();
}

QString QSqlConnectionDialog::userName() const
{
    return ui.editUsername->text();
}

QString QSqlConnectionDialog::password() const
{
    return ui.editPassword->text();
}

QString QSqlConnectionDialog::hostName() const
{
    return ui.editHostname->text();
}

int QSqlConnectionDialog::port() const
{
    return ui.portSpinBox->value();
}

bool QSqlConnectionDialog::useInMemoryDatabase() const
{
    return ui.dbCheckBox->isChecked();
}

void QSqlConnectionDialog::on_okButton_clicked()
{
    if (ui.comboDriver->currentText().isEmpty()) {
        QMessageBox::information(this, tr("No database driver selected"),
                                 tr("Please select a database driver"));
        ui.comboDriver->setFocus();
    } else {
        accept();
    }
}

