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

bool create_connections();


int main( int argc, char *argv[] )
{
    int rows = 0;

    if ( create_connections() ) {
	QSqlQuery q( "INSERT INTO staff ( id, forename, surname, salary ) "
		     "VALUES ( 1155, 'Ginger', 'Davis', 50000 );" );
	if ( q.isActive() ) rows += q.numRowsAffected() ;

	q.exec( "UPDATE staff SET salary=60000 WHERE id=1155;" );
	if ( q.isActive() ) rows += q.numRowsAffected() ;

	q.exec( "DELETE FROM staff WHERE id=1155;" ); 
	if ( q.isActive() ) rows += q.numRowsAffected() ;
    }

    return ( rows == 3 ) ? 0 : 1; 
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



