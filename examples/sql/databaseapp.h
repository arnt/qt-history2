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

class DatabaseFrontEnd : public QFrame
{
    Q_OBJECT
public:
    DatabaseFrontEnd( QWidget * parent, const char * name = 0 );

public slots:
    void insertCustomer();
    void updateCustomer();
    void deleteCustomer();
    void updateCustomerInfo( const QSqlRecord * );

    void insertInvoice();
    void updateInvoice();
    void deleteInvoice();
    void insertingInvoice( QSqlRecord* buf );

protected:
    void init();
    void resizeEvent( QResizeEvent * );

private:
    QSqlTable * invoiceTable, * customerTable;
    QLabel    * customerInfo;
    QSplitter * vSplitter, * hSplitter;

    CustomerCursor customerCr;
    InvoiceCursor invoiceCr;
};

class DatabaseApp : public QMainWindow
{
    Q_OBJECT
public:
    DatabaseApp( QWidget * parent = 0, const char * name = 0 );

protected:
    void init();

protected slots:
    void createDatabase();
    void dropDatabase();
    void customerReport();
};

#endif // DATABASEAPP_H

