#ifndef DIALOGS_H
#define DIALOGS_H

#include "cursors.h"

#include <qdialog.h>
#include <qsqltable.h>
#include <qframe.h>

class QSqlForm;

class DatabaseDlg : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    DatabaseDlg( QSqlCursor * cursor, QSqlRecord* buf, Mode mode, QWidget * parent = 0,
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

    InvoiceDlg( QSqlCursor * cursor, QSqlRecord* buf, Mode mode, QWidget * parent = 0,
		const char * name = 0 );
public slots:
    void updateInvoiceItem();
    void insertInvoiceItem();
    void deleteInvoiceItem();
    void insertingInvoiceItem( QSqlRecord* buf );

    void close();
    void execute();
    void updateProductTable( const QSqlRecord * r );

private:
    QVariant invoiceId;
    Mode mMode;
    QSqlForm   * invoiceForm;
    QSqlTable  * invoiceItems;
    ProductCursor * productCr;
    InvoiceItemCursor itemCursor;
};

#endif // DIALOGS_H

