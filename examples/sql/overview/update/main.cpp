/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>

bool create_connections();

int main( int argc, char *argv[] )
{
    if ( create_connections() ) {
    	QSqlCursor cur( "prices" ); 
	cur.select( "id=202" );
	if ( cur.next() ) {
	    QSqlRecord *buffer = cur.primeUpdate();
	    double price = buffer->value( "price" ).toDouble();
	    double newprice = price * 1.05;
	    buffer->setValue( "price", newprice );
	    cur.update();
	}
    }

    return 0;
}


bool create_connections()
{

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



