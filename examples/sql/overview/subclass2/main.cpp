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
    setDisplayLabel( "pricesid", "Product" );
    setDisplayLabel( "quantity", "Quantity" );
    setDisplayLabel( "paiddate", "Date" );
    setAlignment( "quantity", Qt::AlignRight );
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( create_connections() ) {
	InvoiceItemCursor invoiceItemCursor;

	QSqlTable *invoiceItemTable = new QSqlTable( &invoiceItemCursor );

	app.setMainWidget( invoiceItemTable );

	invoiceItemTable->addColumn( invoiceItemCursor.field( "pricesid" ) );
	invoiceItemTable->addColumn( invoiceItemCursor.field( "quantity" ) );
	invoiceItemTable->addColumn( invoiceItemCursor.field( "paiddate" ) );

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


