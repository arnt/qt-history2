#include "qsqlconnectiondialog.h"
#include "ui_qsqlconnectiondialog.h"

#include <qsqldatabase.h>

QSqlConnectionDialog::QSqlConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QSqlConnectionDialogUi;
    ui->setupUi(this);

    ui->comboDriver->insertStringList(QSqlDatabase::drivers());
}

QString QSqlConnectionDialog::driverName() const
{
    return ui->comboDriver->currentText();
}

QString QSqlConnectionDialog::databaseName() const
{
    return ui->editDatabase->text();
}

QString QSqlConnectionDialog::userName() const
{
    return ui->editUsername->text();
}

QString QSqlConnectionDialog::password() const
{
    return ui->editPassword->text();
}

QString QSqlConnectionDialog::hostName() const
{
    return ui->editHostname->text();
}

int QSqlConnectionDialog::port() const
{
    return ui->portSpinBox->value();
}

void QSqlConnectionDialog::on_okButton_clicked()
{
    if (ui->comboDriver->currentText().isEmpty()) {
        QMessageBox::information(this, tr("No database driver selected"),
                                 tr("Please select a database driver"));
        ui->comboDriver->setFocus();
    } else {
        accept();
    }
}

