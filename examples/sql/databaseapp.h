#ifndef DATABASEAPP
#define DATABASEAPP

#include <qmainwindow.h>
#include <qsqlcursor.h>
#include <qdialog.h>
#include <qsqltable.h>
#include <qframe.h>
#include <qmap.h>

class QLabel;
class QFrame;
class QSplitter;
class QSqlForm;


class CustomerCursor : public QSqlCursor
{
public:
    CustomerCursor();
protected:
    void primeInsert( QSqlRecord* buf );    
};

class ProductCursor : public QSqlCursor
{
public:
    ProductCursor();
protected:
    void primeInsert( QSqlRecord* buf );    
};

class InvoiceCursor : public QSqlCursor
{
public:
    InvoiceCursor();
protected:
    void primeInsert( QSqlRecord* buf );    
};

class InvoiceItemCursor : public QSqlCursor
{
public:
    InvoiceItemCursor();
protected:    
    void primeInsert( QSqlRecord* buf );
    QVariant  calculateField( uint fieldNumber );
private:
    ProductCursor pr;    
};


class DatabaseDlg : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    DatabaseDlg( QSqlCursor * view, Mode mode, QWidget * parent = 0,
		 const char * name = 0 );
public slots:
    void close();
    void execute();

private:
    Mode mMode;
    QSqlForm * mForm;
};

class InvoiceDlg : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    InvoiceDlg( QSqlCursor * view, Mode mode, QWidget * parent = 0,
		const char * name = 0 );
public slots:
    void updateInvoiceItem();
    void insertInvoiceItem();
    void deleteInvoiceItem();

    void close();
    void execute();
    void updateProductTable( const QSqlRecord * r );

private:
    QVariant invoiceId;
    Mode mMode;
    QSqlForm   * invoiceForm;
    QSqlTable  * invoiceItems;
    QSqlCursor * productCr;
    InvoiceItemCursor itemCursor;
};

class DatabaseWgt : public QFrame
{
    Q_OBJECT
public:
    DatabaseWgt( QWidget * parent, const char * name = 0 );

    QFrame * customerBtnFrm, * invoiceBtnFrm;
    QSqlTable * invoices;
    QSqlTable * customers;

    QLabel * customer, * customer_inf_lbl, * customer_lbl, * invoice_lbl;
    QSplitter * v_splitter, * h_splitter;

public slots:
    void insertCustomer();
    void updateCustomer();
    void deleteCustomer();

    void insertInvoice();
    void updateInvoice();
    void deleteInvoice();


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

#endif // DATABASEAPP

