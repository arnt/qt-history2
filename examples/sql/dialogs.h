#ifndef DIALOGS_H
#define DIALOGS_H

#include "cursors.h"

#include <qdialog.h>
#include <qsqltable.h>
#include <qframe.h>

class QSqlForm;

class GenericDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    GenericDialog( QSqlRecord* buf, Mode mode, QWidget * parent = 0,
		   const char * name = 0 );
public slots:
    void close();
    void execute();

private:
    Mode mMode;
    QSqlForm * form;
};

class InvoiceDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };

    InvoiceDialog( QSqlRecord* buf, Mode mode,
		   QWidget * parent = 0, const char * name = 0 );
public slots:
    void updateInvoiceItem();
    void insertInvoiceItem();
    void deleteInvoiceItem();
    void insertingInvoiceItem( QSqlRecord* buf );

    void close();
    void execute();
    void updateProductTable( const QSqlRecord * r );

private:
    void checkInsertInvoice();
    Mode mMode;
    bool cleanupOrphans;
    QVariant invoiceId;
    QSqlForm   * invoiceForm;
    QSqlTable  * invoiceItems;
    InvoiceItemCursor itemCursor;
};

#endif // DIALOGS_H

