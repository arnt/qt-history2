#ifndef DATABASEAPP_H
#define DATABASEAPP_H

#include "cursors.h"

#include <qmainwindow.h>
#include <qsqlcursor.h>
#include <qsqltable.h>
#include <qframe.h>

class QLabel;
class QSplitter;
class QSqlForm;

class DatabaseWgt : public QFrame
{
    Q_OBJECT
public:
    DatabaseWgt( QWidget * parent, const char * name = 0 );

    QSqlTable * invoiceTable, * customerTable;
    QLabel    * customerInfo;
    QSplitter * vSplitter, * hSplitter;

public slots:
    void insertCustomer();
    void updateCustomer();
    void deleteCustomer();

    void insertInvoice();
    void updateInvoice();
    void deleteInvoice();
    void insertingInvoice( QSqlRecord* buf );

protected:
    void init();
    void resizeEvent( QResizeEvent * );
};

class DatabaseApp : public QMainWindow
{
    Q_OBJECT
public:
    DatabaseApp( QWidget * parent = 0, const char * name = 0 );

protected:
    void init();

protected slots:
    void updateCustomerInfo( const QSqlRecord * );
    void createDB();
    void dropDB();

private:
    DatabaseWgt * d;

    CustomerCursor customerCr;
    InvoiceCursor invoiceCr;
};

#endif // DATABASEAPP_H

