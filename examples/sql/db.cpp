#include "db.h"
#include <qdatetime.h>
#include <qsqldatabase.h>
#include <qsqlview.h>

void drop_db();

void create_db()
{
    drop_db();

    QSqlDatabase* db = QSqlConnection::database();

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
    QSqlView customerView( "customer" );
    customerView.setMode( SQL_Writable );
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
	     "(id number primary key,"
	     "name char(30),"
	     "pic char(255));");
    QSqlView productView( "product" );
    productView["id"] = 1;
    productView["name"] = "Widget";
    productView["pic"] = "";
    productView.insert();

    db->exec("create table invoice "
	     "(id numeric(10) primary key,"
	     "customerid numeric(10) references customer,"
	     "number numeric(10),"
	     "paid numeric(1),"
	     "createdate date,"
	     "tax numeric(10,5),"
	     "shipping numeric(15,2),"
	     "total numeric(15,2));");
    QSqlView invoiceView( "invoice" );
    invoiceView.setMode( SQL_Writable );
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
    QSqlView invoiceitemView( "invoiceitem" );
    invoiceitemView["id"] = 1;
    invoiceitemView["invoiceid"] = 1;
    invoiceitemView["productid"] = 1;
    invoiceitemView["quantity"] = 10;
    invoiceitemView["total"] = 10*100.0;
    invoiceitemView.insert();

}

void drop_db()
{
    QSqlDatabase* db = QSqlConnection::database();
    db->exec("drop table invoiceitem;");
    db->exec("drop table invoice;");
    db->exec("drop table product;");
    db->exec("drop table customer;");
}
