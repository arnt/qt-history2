#ifndef CURSORS_H
#define CURSORS_H

#include <qsqlcursor.h>
#include <qmap.h>

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


#endif // CURSORS_H

