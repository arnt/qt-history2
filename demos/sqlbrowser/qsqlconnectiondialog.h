#ifndef QSQLCONNECTIONDIALOG_H
#define QSQLCONNECTIONDIALOG_H

#include <qdialog.h>
#include <qmessagebox.h>

#include "ui_qsqlconnectiondialog.h"

class QSqlConnectionDialog: public QDialog
{
    Q_OBJECT
public:
    QSqlConnectionDialog(QWidget *parent = 0);
    ~QSqlConnectionDialog();

    QString driverName() const;
    QString databaseName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    int port() const;

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked() { reject(); }

private:
    Ui::QSqlConnectionDialogUi ui;
};

#endif

