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
#include "../login.h"

bool createConnections();

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( createConnections() ) {
	QSqlCursor cur( "creditors" );

	QStringList orderFields = QStringList() << "surname" << "forename";
	QSqlIndex order = cur.index( orderFields );

	QStringList filterFields = QStringList() << "surname" << "city";
	QSqlIndex filter = cur.index( filterFields );
	cur.setValue( "surname", "Chirac" );
	cur.setValue( "city", "Paris" );

	cur.select( filter, order );

	while ( cur.next() ) {
	    int id = cur.value( "id" ).toInt();
	    QString name = cur.value( "forename" ).toString() + " " +
			   cur.value( "surname" ).toString();
	    qDebug( QString::number( id ) + ": " + name );
	}
    }

    return 0;
}


bool createConnections()
{

    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_SALES_DRIVER );
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
