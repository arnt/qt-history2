#include "db.h"
#include <qdatetime.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qpixmap.h>
// #include "/home/trond/pics/radio_t.xpm"
// #include "/home/trond/pics/radio_f.xpm"
// #include "/home/trond/pics/slider_v.xpm"
// #include "/home/trond/pics/slider_h.xpm"

void drop_db();

void create_db()
{
    drop_db();

    QSqlDatabase* db = QSqlDatabase::database();

    /* create sample database with some sample data */

    db->exec("create table customer "
	     "(id numeric(10) primary key,"
	     "name char(30),"
	     "add1 char(50),"
	     "add2 char(50),"
	     "city char(50),"
	     "postalcode char(20),"
	     "country char(2),"
	     "billtoadd1 char(50),"
	     "billtoadd2 char(50),"
	     "billtocity char(50),"
	     "billtopostalcode char(20),"
	     "billtocountry char(2),"
	     "shiptoadd1 char(50),"
	     "shiptoadd2 char(50),"
	     "shiptocity char(50),"
	     "shiptopostalcode char(20),"
	     "shiptocountry char(2));");
    QSqlCursor customerView( "customer" );
    customerView.setMode( QSqlCursor::Writable );
    customerView["id"] = 1;
    customerView["name"] = "Trolltech";
    customerView["add1"] = "Waldemar Thranesgt. 98b";
    customerView["add2"] = "";
    customerView["city"] = "Oslo";
    customerView["postalcode"] = "N-0175";
    customerView["country"] = "NO";
    customerView["billtoadd1"] = "";
    customerView["billtoadd2"] =  "";
    customerView["billtocity"] = "";
    customerView["billtopostalcode"] =  "";
    customerView["billtocountry"] =  "";
    customerView["shiptoadd1"] =  "";
    customerView["shiptoadd2"] =  "";
    customerView["shiptocity"] =  "";
    customerView["shiptopostalcode"] =  "";
    customerView["shiptocountry"] =  "";
    customerView.insert();

    db->exec("create table product "
	     "(id numeric(10) primary key,"
	     "name char(30),"
	     "pic oid);");
    QSqlCursor productView( "product" );
    //productView.field("pic")->setType( QVariant::Pixmap );
  //  QPixmap px( (const char **) radio_t );
//    productView["pic"] = px;
    productView.setMode( QSqlCursor::Writable );
    productView["id"] = 1;
    productView["name"] = "Widget";
    productView["pic"] = 0;
    productView.insert();
    productView["id"] = 2;
    productView["name"] = "Test";
    productView["pic"] = 0;
    productView.insert();
    productView["id"] = 3;
    productView["name"] = "Thingy";
    productView["pic"] = 0;
    productView.insert();
    productView["id"] = 4;
    productView["name"] = "Button";
    productView["pic"] = 0;
    productView.insert();
    db->exec("update product set pic = lo_import('/tmp/radio_t.xpm') where id = '1';");
    db->exec("update product set pic = lo_import('/tmp/radio_f.xpm') where id = '2';");
    db->exec("update product set pic = lo_import('/tmp/slider_v.xpm') where id = '3';");
    db->exec("update product set pic = lo_import('/tmp/slider_h.xpm') where id = '4';");
    
    db->exec("create table invoice "
	     "(id numeric(10) primary key,"
	     "customerid numeric(10) references customer,"
	     "number numeric(10),"
	     "paid numeric(1),"
	     "createdate date,"
	     "tax numeric(10,5),"
	     "shipping numeric(15,2),"
	     "total numeric(15,2));");
    QSqlCursor invoiceView( "invoice" );
    invoiceView.setMode( QSqlCursor::Writable );
    invoiceView["id"] = 1;
    invoiceView["customerid"] = 1;
    invoiceView["number"] = 1;
    invoiceView["paid"] = TRUE;
    invoiceView["createdate"] = QDate(2000,1,1);
    invoiceView["tax"] = 0;
    invoiceView["shipping"] = 0;
    invoiceView["total"] = 100.00;
    invoiceView.insert();

    db->exec("create table invoiceitem "
	     "(id numeric(10) primary key,"
	     "invoiceid numeric(10) references invoice,"
	     "productid numeric(10) references product,"
	     "quantity numeric(10),"
	     "total numeric(15,2));");
    QSqlCursor invoiceitemView( "invoiceitem" );
    invoiceitemView.setMode( QSqlCursor::Writable );
    invoiceitemView["id"] = 1;
    invoiceitemView["invoiceid"] = 1;
    invoiceitemView["productid"] = 1;
    invoiceitemView["quantity"] = 10;
    invoiceitemView["total"] = 10*100.0;
    invoiceitemView.insert();

}

void drop_db()
{
    QSqlDatabase* db = QSqlDatabase::database();
    db->exec("drop table invoiceitem;");
    db->exec("drop table invoice;");
    db->exec("drop table product;");
    db->exec("drop table customer;");
}
