/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "main.h"


InvoiceItemCursor::InvoiceItemCursor() : 
    QSqlCursor( "invoiceitem" )
{
    setDisplayLabel( "quantity", "Quantity" );
    setDisplayLabel( "paiddate", "Date" );
    setAlignment( "quantity", Qt::AlignRight );

    QSqlField productName( "productname", QVariant::String );
    append( productName );
    setDisplayLabel( "productname", "Product" );
    setCalculated( productName.name(), TRUE );

    QSqlField productPrice( "price", QVariant::Double );
    append( productPrice );
    setDisplayLabel( "price", "Price" );
    setAlignment( "price", Qt::AlignRight );
    setCalculated( productPrice.name(), TRUE );

    QSqlField productCost( "cost", QVariant::Double );
    append( productCost );
    setDisplayLabel( "cost", "Cost" );
    setAlignment( "cost", Qt::AlignRight );
    setCalculated( productCost.name(), TRUE );
}


QVariant InvoiceItemCursor::calculateField( const QString & name )
{

    if ( name == "productname" ) {
	QSqlQuery q( "SELECT name FROM prices WHERE id=" +
		     field( "pricesid" )->value().toString() + ";" );
	if ( q.next() ) 
	    return q.value( 0 );
    }
    else if ( name == "price" ) {
	QSqlQuery q( "SELECT price FROM prices WHERE id=" +
		     field( "pricesid" )->value().toString() + ";" );
	if ( q.next() ) 
	    return q.value( 0 );
    }
    else if ( name == "cost" ) {
	QSqlQuery q( "SELECT price FROM prices WHERE id=" +
		     field( "pricesid" )->value().toString() + ";" );
	if ( q.next() ) 
	    return QVariant( q.value( 0 ).toDouble() * 
			     value( "quantity").toDouble() );
    }

    return QVariant( QString::null );
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( create_connections() ) {
	InvoiceItemCursor invoiceItemCursor;

	QSqlTable *invoiceItemTable = new QSqlTable( &invoiceItemCursor );

	app.setMainWidget( invoiceItemTable );

	invoiceItemTable->addColumn( "productname" );
	invoiceItemTable->addColumn( "price" );
	invoiceItemTable->addColumn( "quantity" );
	invoiceItemTable->addColumn( "cost" );
	invoiceItemTable->addColumn( "paiddate" );

	invoiceItemTable->refresh();
	invoiceItemTable->show();

	return app.exec();
    }

    return 1;
}


bool create_connections()
{
    // create the default database connection
    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( "QODBC" );
    defaultDB->setDatabaseName( "sales" );
    defaultDB->setUserName( "salesuser" );
    defaultDB->setPassword( "salespw" );
    defaultDB->setHostName( "saleshost" );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open sales database: " + 
		  defaultDB->lastError().driverText() );
	qWarning( defaultDB->lastError().databaseText() );
	return false;
    }

    // create a named connection to oracle
    QSqlDatabase *oracle = QSqlDatabase::addDatabase( "QOCI", "ORACLE" );
    oracle->setDatabaseName( "orders" );
    oracle->setUserName( "ordersuser" );
    oracle->setPassword( "orderspw" );
    oracle->setHostName( "ordershost" );
    if ( ! oracle->open() ) {
	qWarning( "Failed to open orders database: " + 
		  oracle->lastError().driverText() );
	qWarning( oracle->lastError().databaseText() );
	return false;
    }

    return true;
}


