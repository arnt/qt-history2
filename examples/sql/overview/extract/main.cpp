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
	return FALSE;
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
	return FALSE;
    }

    return TRUE;
}
