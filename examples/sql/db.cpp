#include "db.h"
#include "databaseapp.h"
#include <qdatetime.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qpixmap.h>

void drop_db();

void create_db()
{
    drop_db();

    /* get the default app database */
    QSqlDatabase* db = QSqlDatabase::database();

    QSqlRecord* buf = 0;    
    /* create sample database with some sample data */
    db->exec("create table CUSTOMER "
	     "(ID numeric(10) primary key,"
	     "NAME char(30),"
	     "ADD1 char(50),"
	     "ADD2 char(50),"
	     "CITY char(50),"
	     "POSTALCODE char(20),"
	     "COUNTRY char(2),"
	     "BILLTOADD1 char(50),"
	     "BILLTOADD2 char(50),"
	     "BILLTOCITY char(50),"
	     "BILLTOPOSTALCODE char(20),"
	     "BILLTOCOUNTRY char(2),"
	     "SHIPTOADD1 char(50),"
	     "SHIPTOADD2 char(50),"
	     "SHIPTOCITY char(50),"
	     "SHIPTOPOSTALCODE char(20),"
	     "SHIPTOCOUNTRY char(2));");
    CustomerCursor customer;
    buf = customer.insertBuffer();
    buf->setValue( "NAME",  "Trolltech" );
    buf->setValue( "ADD1",  "Waldemar Thranesgt. 98b" );
    buf->setValue( "ADD2",  "" );
    buf->setValue( "CITY",  "Oslo" );
    buf->setValue( "POSTALCODE",  "N-0175" );
    buf->setValue( "COUNTRY",  "NO" );
    customer.insert();

    db->exec("create table PRODUCT "
	     "(ID numeric(10) primary key,"
	     "NAME char(30),"
	     "PIC char(255));");
    ProductCursor product;
    buf = product.insertBuffer();
    buf->setValue( "NAME",  "Widget" );
    buf->setValue( "PIC",  "" );
    product.insert();
    buf = product.insertBuffer();
    buf->setValue( "NAME",  "Gadget" );
    buf->setValue( "PIC",  "" );
    product.insert();

    db->exec("create table INVOICE "
	     "(ID numeric(10) primary key,"
	     "CUSTOMERID numeric(10) references customer,"
	     "NUM numeric(10),"
	     "PAID numeric(1),"
	     "CREATEDATE date,"
	     "TAX numeric(10,5),"
	     "SHIPPING numeric(15,2),"
	     "TOTAL numeric(15,2));");
    InvoiceCursor invoice;
    buf = invoice.insertBuffer();
    buf->setValue( "CUSTOMERID",  customer.value("id") );
    buf->setValue( "NUM",  1 );
    buf->setValue( "PAID",  TRUE );
    buf->setValue( "CREATEDATE",  QDate(2000,1,1) );
    buf->setValue( "TAX",  0 );
    buf->setValue( "SHIPPING",  0 );
    buf->setValue( "TOTAL",  100.00 );
    invoice.insert();

    db->exec("create table INVOICEITEM "
	     "(ID numeric(10) primary key,"
	     "INVOICEID numeric(10) references invoice,"
	     "PRODUCTID numeric(10) references product,"
	     "QUANTITY numeric(10),"
	     "TOTAL numeric(15,2));");
    InvoiceItemCursor invoiceitem;
    buf = invoiceitem.insertBuffer();
    buf->setValue( "INVOICEID",  invoice.value("id") );
    buf->setValue( "PRODUCTID",  product.value("id") );
    buf->setValue( "QUANTITY",  10 );
    buf->setValue( "TOTAL",  10*100.0 );
    invoiceitem.insert();
}

void drop_db()
{
    QSqlDatabase* db = QSqlDatabase::database();
    db->exec("drop table INVOICEITEM;");
    db->exec("drop table INVOICE;");
    db->exec("drop table PRODUCT;");
    db->exec("drop table CUSTOMER;");
}
