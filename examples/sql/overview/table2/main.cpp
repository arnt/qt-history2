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
#include <qsqltable.h>

bool create_connections();

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( create_connections() ) {
	QSqlCursor staffCursor( "staff" );

	staffCursor.setDisplayLabel( "forename", "Forename" );
	staffCursor.setDisplayLabel( "surname", "Surname" );
	staffCursor.setDisplayLabel( "salary", "Annual Salary" );
	staffCursor.setAlignment( "salary", Qt::AlignRight );

	QSqlTable *staffTable = new QSqlTable( &staffCursor );

	app.setMainWidget( staffTable );

	staffTable->addColumn( staffCursor.field( "forename" ) );
	staffTable->addColumn( staffCursor.field( "surname" ) );
	staffTable->addColumn( staffCursor.field( "salary" ) );

	QStringList order = QStringList() << "surname" << "forename";
	staffTable->setSort( order );

	staffTable->refresh();
	staffTable->show();

	return app.exec();
    }

    return 1;
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

    QSqlDatabase *oracle = QSqlDatabase::addDatabase( "QPSQL6", "oracle" );
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
