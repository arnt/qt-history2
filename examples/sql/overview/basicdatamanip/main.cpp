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
#include <qsqlquery.h>
#include "../login.h"

bool createConnections();


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    int rows = 0;

    if ( createConnections() ) {
	QSqlQuery query( "INSERT INTO staff ( id, forename, surname, salary ) "
		     "VALUES ( 1155, 'Ginger', 'Davis', 50000 );" );
	if ( query.isActive() ) rows += query.numRowsAffected() ;

	query.exec( "UPDATE staff SET salary=60000 WHERE id=1155;" );
	if ( query.isActive() ) rows += query.numRowsAffected() ;

	query.exec( "DELETE FROM staff WHERE id=1155;" ); 
	if ( query.isActive() ) rows += query.numRowsAffected() ;
    }

    return ( rows == 3 ) ? 0 : 1; 
}


bool createConnections()
{

    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_SALES_DRIVER );
    if ( ! defaultDB ) {
	qWarning( "Failed to connect to driver" );
	return FALSE;
    }
    defaultDB->setDatabaseName( DB_SALES_DBNAME );
    defaultDB->setUserName( DB_SALES_USER );
    defaultDB->setPassword( DB_SALES_PASSWD );
    defaultDB->setHostName( DB_SALES_HOST );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open sales database: " + 
		  defaultDB->lastError().driverText() );
	qWarning( defaultDB->lastError().databaseText() );
	return FALSE;
    }

    QSqlDatabase *oracle = QSqlDatabase::addDatabase( DB_ORDERS_DRIVER, "ORACLE" );
    if ( ! oracle ) {
	qWarning( "Failed to connect to oracle driver" );
	return FALSE;
    }
    oracle->setDatabaseName( DB_ORDERS_DBNAME );
    oracle->setUserName( DB_ORDERS_USER );
    oracle->setPassword( DB_ORDERS_PASSWD );
    oracle->setHostName( DB_ORDERS_HOST );
    if ( ! oracle->open() ) {
	qWarning( "Failed to open orders database: " + 
		  oracle->lastError().driverText() );
	qWarning( oracle->lastError().databaseText() );
	return FALSE;
    }

    return TRUE;
}



