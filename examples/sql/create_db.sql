create table CUSTOMER
(ID numeric(10) primary key,
NAME char(30),
ADD1 char(50),
ADD2 char(50),
CITY char(50),
POSTALCODE char(20),
COUNTRY char(2),
BILLTOADD1 char(50),
BILLTOADD2 char(50),
BILLTOCITY char(50),
BILLTOPOSTALCODE char(20),
BILLTOCOUNTRY char(2),
SHIPTOADD1 char(50),
SHIPTOADD2 char(50),
SHIPTOCITY char(50),
SHIPTOPOSTALCODE char(20),
SHIPTOCOUNTRY char(2));

insert into CUSTOMER 
(ID,NAME, ADD1, CITY, POSTALCODE, COUNTRY) values
(1,'Trolltech', 'Waldemar Thranesgt. 98b','Oslo', 'N-0175', 'NO');

create table PRODUCT
(ID numeric(10) primary key,
NAME char(30),
PIC char(255));

insert into PRODUCT
(ID, NAME) values
(1, 'Widget');

insert into PRODUCT
(ID, NAME) values
(2, 'Gadget');

create table INVOICE 
(ID numeric(10) primary key,
CUSTOMERID numeric(10) references customer,
PRODUCTID numeric(10) references product,
NUM numeric(10),
PAID numeric(1),
CREATEDATE date,
TAX numeric(10,5),
SHIPPING numeric(15,2),
TOTAL numeric(15,2));

insert into INVOICE
(ID,CUSTOMERID,NUM,PAID,CREATEDATE,TAX,SHIPPING,TOTAL) values
(1,1,1,,1,'1999-9-9',0,0,100.00);
